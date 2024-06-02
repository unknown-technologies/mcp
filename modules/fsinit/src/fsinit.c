#include <emulight/types.h>
#include <emulight/string.h>
#include <emulight/r4.h>
#include <emulight/ds.h>
#include <emulight/mcp/fs.h>
#include <emulight/mcp/syslog.h>

void FSInit(unsigned int id, VOLUME* vol, void* __sector_buffer)
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
