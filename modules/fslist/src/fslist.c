#include <emulight/types.h>
#include <emulight/string.h>
#include <emulight/r4.h>
#include <emulight/os.h>
#include <emulight/ds.h>
#include <emulight/mcp/fs.h>

static char dsnbuf[32];

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

void FSiList(const char* prefix,
		void (*callback)(const char* dsn, void* userdata),
		void* userdata, void* __sector_buffer, void* lock)
{
	if(!callback) {
		return;
	}

	DSCB_DS* vtoc = (DSCB_DS*) __sector_buffer;

	OSLock(lock);
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
				R4DecodeString(dsnbuf, ds->name, 10);
				if(prefix && !startswith(dsnbuf, prefix)) {
					continue;
				}
				callback(dsnbuf, userdata);
			}
		}
	}
	OSUnlock(lock);
}
