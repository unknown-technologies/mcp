#include <emulight/types.h>
#include <emulight/string.h>
#include <emulight/mcp/mon.h>
#include <emulight/mcp/syslog.h>
#include <emulight/mcp/fs.h>

BOOL DSOpen(DS* ds, const char* dsn)
{
#ifdef DEBUG
	OSLog(NUC_FILE_OPEN, "OPENING DATASET WITH DSN %s", dsn);
#endif
	BOOL result = MONLocate(dsn, &ds->vtocix, &ds->block, &ds->size);
	ds->current_block = ds->block;
	ds->current_size = ds->size;
	ds->pos = 0;
	return result;
}

size_t DSRead(DS* ds, void* dst, size_t size)
{
	size_t read = 0;
	while(size > 0) {
		/* read new block? */
		if(ds->pos == 0) {
			/* read complete block */
			if(size >= 512 && ds->current_size > 0) {
				MONReadBlock(dst, ds->current_block);
				dst += 512;
				size -= 512;
				read += 512;
				ds->current_block++;
				ds->current_size--;
				continue;
			} else if(ds->current_size) {
				MONReadBlock(__sector_buffer, ds->current_block);
				ds->current_block++;
				ds->current_size--;
			} else {
				return read;
			}
		}

		size_t remaining = 512 - ds->pos;
		if(remaining >= size) {
			/* last block of the read operation */
			memcpy(dst, __sector_buffer + ds->pos, size);
			ds->pos += size;
			read += size;
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

	return size;
}

void DSSeek(DS* ds, size_t pos)
{
	size_t blocks = pos / 512;
	if(blocks < ds->size) {
		ds->current_block = ds->block + blocks;
		ds->current_size = ds->size - blocks;
		ds->pos = pos % 512;

		MONReadBlock(__sector_buffer, ds->current_block);
	}
}
