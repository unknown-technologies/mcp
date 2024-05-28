#include <emulight/types.h>
#include <emulight/string.h>
#include <emulight/r4.h>
#include <emulight/mcp/mon.h>
#include <emulight/mcp/syslog.h>
#include <emulight/mcp/fs.h>

static unsigned int lock = 0;
static unsigned int current_block = 0;

static unsigned int vtoc_size;
static unsigned int vtoc_block;

extern char __reltab[];

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

void FSInit(void)
{
	DSCB_VTOC* vtoc = (DSCB_VTOC*) __sector_buffer;
	MONReadBlock(__sector_buffer, 1);
	current_block = 1;

	if(vtoc->type != EFS_DSCB_VTOC || memcmp(vtoc->magic, EFS_MAGIC, 8)) {
		OSLog(NUC_FS_CORRUPT, "VOLUME IS CORRUPT");
		return;
	}

	char* buf = (char*) &vtoc[1];
	R4DecodeString(buf, vtoc->label, 3);
	OSLog(NUC_FS_INFO, "VOLUME ID: %s", buf);

	vtoc_size = vtoc->vtoc_size;
	vtoc_block = vtoc->vtoc_block;
}

void FSList(const char* prefix,
		void (*callback)(const char* dsn, void* userdata),
		void* userdata)
{
	if(!callback) {
		return;
	}

	DSCB_DS* vtoc = (DSCB_DS*) __sector_buffer;
	char* name = __reltab;

	MONLock(&lock);
	for(unsigned int i = 0; i < vtoc_size; i++) {
		current_block = vtoc_block + i;
		MONReadBlock(__sector_buffer, current_block);
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
	MONUnlock(&lock);
}

BOOL DSOpen(DS* ds, const char* dsn)
{
#ifdef DEBUG
	OSLog(NUC_FILE_OPEN, "OPENING DATASET WITH DSN %s", dsn);
#endif

	MONLock(&lock);
	BOOL result = MONLocate(dsn, &ds->vtocix, &ds->block, &ds->size);
	ds->current_block = ds->block;
	ds->current_size = ds->size;
	ds->pos = 0;
	ds->buf_block = 0;
	current_block = 0;
	MONUnlock(&lock);
	return result;
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
				MONReadBlock(dst, ds->current_block);
				ds->buf_block = ds->current_block;
				current_block = ds->current_block;
				dst += 512;
				size -= 512;
				read += 512;
				ds->current_block++;
				ds->current_size--;
				continue;
			} else if(ds->current_size) {
				MONReadBlock(__sector_buffer, ds->current_block);
				ds->buf_block = ds->current_block;
				current_block = ds->current_block;
				ds->current_block++;
				ds->current_size--;
			} else {
				MONUnlock(&lock);
				return read;
			}
		} else if(ds->buf_block != current_block) {
			MONReadBlock(__sector_buffer, ds->buf_block);
			current_block = ds->buf_block;
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
	if(blocks < ds->size) {
		ds->current_block = ds->block + blocks;
		ds->current_size = ds->size - blocks;
		ds->pos = pos % 512;

		if(ds->pos) {
			MONReadBlock(__sector_buffer, ds->current_block);
			ds->buf_block = ds->current_block;
			current_block = ds->current_block;
			ds->current_block++;
			ds->current_size--;
		}
	}
	MONUnlock(&lock);
}
