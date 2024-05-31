#include <emulight/types.h>
#include <emulight/string.h>
#include <emulight/r4.h>
#include <emulight/mcp/mon.h>
#include <emulight/mcp/syslog.h>
#include <emulight/mcp/fs.h>
#include <emulight/mcp/vol.h>

static unsigned int lock = 0;

static char dsnbuf[31];

static BOOL startswith(const char* a, const char* b)
{
	while(*a == *b) {
		if(!*a) {
			return TRUE;
		}
		a++;
		b++;
	}

	return !*b;
}

void FSInit(unsigned int id, VOLUME* vol)
{
	/* initialize ROM volume */
	DSCB_VTOC* vtoc = (DSCB_VTOC*) __sector_buffer;
	VOLReadBlock(id, __sector_buffer, 1);

	if(vtoc->type != EFS_DSCB_VTOC || memcmp(vtoc->magic, EFS_MAGIC, 8)) {
		OSLog(NUC_FS_CORRUPT, "VOLUME %u IS CORRUPT", id);
		return;
	}

	char* buf = (char*) &vtoc[1];
	R4DecodeString(buf, vtoc->label, 3);
	OSLog(NUC_FS_INFO, "VOLUME %u LABEL: %s", id, buf);

	vol->vtoc_size = vtoc->vtoc_size;
	vol->vtoc_block = vtoc->vtoc_block;

	memcpy(vol->label, vtoc->label, 6);
}

void FSList(const char* prefix,
		void (*callback)(const char* dsn, void* userdata),
		void* userdata)
{
	if(!callback) {
		return;
	}

	DSCB_DS* vtoc = (DSCB_DS*) __sector_buffer;
	char* name = dsnbuf;

	MONLock(&lock);
	for(unsigned int volid = 0; volid < MAX_VOLUMES; volid++) {
		if(!VOLIsActive(volid)) {
			continue;
		}

		VOLUME* vol = VOLGetVolume(volid);

		for(unsigned int i = 0; i < vol->vtoc_size; i++) {
			VOLReadBlock(volid, __sector_buffer, vol->vtoc_block + i);
			for(unsigned int j = 0; j < 512 / sizeof(DSCB_DS); j++) {
				DSCB_DS* ds = &vtoc[j];
				if(ds->type == EFS_DSCB_UNUSED
						|| ds->type == EFS_DSCB_FREE) {
					continue;
				}
				R4DecodeString(name, ds->name, 10);
				if(prefix && !startswith(name, prefix)) {
					continue;
				}
				callback(name, userdata);
			}
		}
	}
	MONUnlock(&lock);
}

BOOL FSLocate(const char* dsn, DSCB_DS* ds, unsigned int* vtocix,
		unsigned int* volume)
{
	if(!dsn || !ds) {
		return FALSE;
	}

	DSCB_DS* vtoc = (DSCB_DS*) __sector_buffer;
	u16* name = (u16*) dsnbuf;
	R4EncodeString(name, dsn, 10);

	BOOL found = FALSE;

	MONLock(&lock);
	for(unsigned int volid = 0; volid < MAX_VOLUMES; volid++) {
		if(!VOLIsActive(volid)) {
			continue;
		}

		VOLUME* vol = VOLGetVolume(volid);

		for(unsigned int i = 0; i < vol->vtoc_size; i++) {
			VOLReadBlock(volid, __sector_buffer, vol->vtoc_block + i);
			for(unsigned int j = 0; j < 512 / sizeof(DSCB_DS); j++) {
				DSCB_DS* dscb = &vtoc[j];
				if(dscb->type == EFS_DSCB_UNUSED
						|| dscb->type == EFS_DSCB_FREE) {
					continue;
				}

				if(memcmp(name, dscb->name, 20)) {
					continue;
				}

				found = TRUE;
				if(vtocix) {
					*vtocix = i * (512 / sizeof(DSCB_DS))
						+ j;
				}
				if(volume) {
					*volume = volid;
				}
				memcpy(ds, dscb, sizeof(DSCB_DS));

				goto end;
			}
		}
	}
end:
	MONUnlock(&lock);

	return found;
}

BOOL DSIsFB(DS* ds)
{
	return DSGetRecLen(ds) > 0;
}

size_t DSGetRecLen(DS* ds)
{
	unsigned int lrecl = ds->info.flags & 0x0FFF;
	if(lrecl) {
		return lrecl + 1;
	} else {
		return 0;
	}
}

BOOL DSOpen(DS* ds, const char* dsn)
{
#ifdef DEBUG
	OSLog(NUC_FILE_OPEN, "OPENING DATASET WITH DSN %s", dsn);
#endif

	DSCB_DS dscb;
	BOOL result = FSLocate(dsn, &dscb, &ds->info.vtocix, &ds->info.volume);
	if(result) {
		ds->info.block = dscb.start_block;
		ds->info.size = dscb.size;
		ds->current_block = ds->info.block;
		ds->current_size = ds->info.size;
		ds->pos = 0;
		ds->buf_block = 0;
		ds->info.flags = dscb.flags;
	}
	return result;
}

void DSGetInfo(DS* ds, DSInfo* info)
{
	*info = ds->info;
}

size_t DSRead(DS* ds, void* dst, size_t size)
{
	MONLock(&lock);
	size_t read = 0;
	while(size > 0) {
		/* read new block? */
		if(ds->pos == 0) {
			/* read complete block */
			if(size >= 512 && ds->current_size > 0) {
				VOLReadBlock(ds->info.volume, dst,
						ds->current_block);
				ds->buf_block = ds->current_block;
				dst += 512;
				size -= 512;
				read += 512;
				ds->current_block++;
				ds->current_size--;
				continue;
			} else if(ds->current_size) {
				VOLReadBlock(ds->info.volume, __sector_buffer,
						ds->current_block);
				ds->buf_block = ds->current_block;
				ds->current_block++;
				ds->current_size--;
			} else {
				MONUnlock(&lock);
				return read;
			}
		} else if(ds->buf_block != current_block) {
			VOLReadBlock(ds->info.volume, __sector_buffer,
					ds->buf_block);
		}

		size_t remaining = 512 - ds->pos;
		if(remaining >= size) {
			/* last block of the read operation */
			memcpy(dst, __sector_buffer + ds->pos, size);
			ds->pos += size;
			read += size;
			MONUnlock(&lock);
			if(ds->pos == 512) {
				ds->pos = 0;
			}
			return read;
		} else {
			/* align to block size */
			memcpy(dst, __sector_buffer + ds->pos, remaining);
			ds->pos = 0;
			read += remaining;
			dst += remaining;
			size -= remaining;
		}
	}
	MONUnlock(&lock);

	return read;
}

void DSSeek(DS* ds, size_t pos)
{
	MONLock(&lock);
	size_t blocks = pos / 512;
	if(blocks < ds->info.size) {
		ds->current_block = ds->info.block + blocks;
		ds->current_size = ds->info.size - blocks;
		ds->pos = pos % 512;

		if(ds->pos) {
			VOLReadBlock(ds->info.volume, __sector_buffer,
					ds->current_block);
			ds->buf_block = ds->current_block;
			ds->current_block++;
			ds->current_size--;
		}
	}
	MONUnlock(&lock);
}
