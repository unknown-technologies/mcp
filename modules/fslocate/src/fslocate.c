#include <emulight/types.h>
#include <emulight/string.h>
#include <emulight/r4.h>
#include <emulight/ds.h>
#include <emulight/os.h>
#include <emulight/mcp/fs.h>

static u16 dsnbuf[15];

BOOL FSLocate(const char* dsn, DSCB_DS* ds, unsigned int* vtocix,
		unsigned int* volume, void* __sector_buffer, void* lock)
{
	if(!dsn || !ds) {
		return FALSE;
	}

	DSCB_DS* vtoc = (DSCB_DS*) __sector_buffer;
	u16* name = (u16*) dsnbuf;
	R4EncodeString(name, dsn, 10);

	BOOL found = FALSE;

	OSLock(lock);
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
	OSUnlock(lock);

	return found;
}
