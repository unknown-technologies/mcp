#include <emulight/types.h>
#include <emulight/string.h>
#include <emulight/r4.h>
#include <emulight/mcp/mon.h>
#include <emulight/mcp/syslog.h>
#include <emulight/mcp/fs.h>
#include <emulight/mcp/vol.h>
#include <emulight/mcp/program.h>

static unsigned int lock = 0;

void FSInit(unsigned int id, VOLUME* vol)
{
	void* fsinit = OSLoadModule("FSINIT");
	if(fsinit) {
		void (*FSiInit)(unsigned int id, VOLUME* vol,
				void* __sector_buffer)
			= (void (*)(unsigned int, VOLUME*, void*)) fsinit;
		FSiInit(id, vol, __sector_buffer);
		OSUnloadModule(fsinit);
	}
}

void FSList(const char* prefix,
		BOOL (*callback)(const char* dsn, void* userdata),
		void* userdata)
{
	if(!callback) {
		return;
	}

	void* fslist = OSLoadModule("FSLIST");
	if(fslist) {
		void (*FSiList)(const char* prefix,
				BOOL (*callback)(const char* dsn,
					void* userdata), void* userdata,
				void* __sector_buffer, void* lock)
			= (void (*)(const char*, BOOL (*)(const char*, void*),
						void*, void*, void*)) fslist;
		FSiList(prefix, callback, userdata, __sector_buffer, &lock);
		OSUnloadModule(fslist);
	}
}

BOOL FSLocate(const char* dsn, DSCB_DS* ds, unsigned int* vtocix,
		unsigned int* volume)
{
	if(!dsn || !ds) {
		return FALSE;
	}

	if(!memcmp(dsn, "SYS1.MOD.", 9)) {
		if(volume) {
			*volume = ROMVOL_ID;
		}
		bzero(ds, sizeof(DSCB_DS));
		unsigned int block = 0;
		unsigned int size = 0;
		BOOL result = MONLocate(dsn, vtocix, &block, &size);
		ds->start_block = block;
		ds->size = size;
		return result;
	}

	void* fslocate = OSLoadModule("FSLOCATE");
	if(fslocate) {
		BOOL (*FSiLocate)(const char* dsn, DSCB_DS* ds,
				unsigned int* vtocix, unsigned int* volume,
				void* __sector_buffer, void* lock)
			= (BOOL (*)(const char*, DSCB_DS*, unsigned int*,
						unsigned int*, void*, void*))
				fslocate;
		BOOL result = FSiLocate(dsn, ds, vtocix, volume,
				__sector_buffer, &lock);
		OSUnloadModule(fslocate);
		return result;
	} else {
		return FALSE;
	}
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

size_t DSGetPhysicalRecLen(DS* ds)
{
	size_t lrecl = DSGetRecLen(ds);
	u16 type = ds->info.flags & 0xC000;
	if(type == EFS_DSCB_DS_TYPE_TXT6B) {
		/* 6bit text encoding */
		return (lrecl + 3) / 4 * 3;
	} else if(type == EFS_DSCB_DS_TYPE_TXT7B) {
		/* 7bit text encoding */
		return (lrecl + 7) / 8 * 7;
	} else {
		return lrecl;
	}
}

BOOL DSiOpen(DS* ds, const char* dsn)
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

void DSiGetInfo(DS* ds, DSInfo* info)
{
	*info = ds->info;
}

size_t DSiRead(DS* ds, void* dst, size_t size)
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

void DSiSeek(DS* ds, size_t pos)
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
