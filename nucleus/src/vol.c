#include <emulight/string.h>
#include <emulight/r4.h>
#include <emulight/mcp/mon.h>
#include <emulight/mcp/vol.h>
#include <emulight/mcp/fs.h>

static VOLUME volumes[MAX_VOLUMES];

unsigned int current_volume;
unsigned int current_block;

static void ROMReadBlock(void* buf, unsigned int block)
{
	MONReadBlock(buf, block);
}

void VOLInit(void)
{
	bzero(volumes, sizeof(volumes));

	VOLAddVolume(0, ROMReadBlock, NULL);
}

void VOLAddVolume(unsigned int id, void (*read)(void*, unsigned int),
		void (*write)(void*, unsigned int))
{
	if(id >= MAX_VOLUMES) {
		return;
	}

	volumes[id].read = read;
	volumes[id].write = write;

	FSInit(id, &volumes[id]);
}

void VOLGetLabel(unsigned int volume, char* label)
{
	if(volume >= MAX_VOLUMES) {
		return;
	}

	VOLUME* vol = VOLGetVolume(volume);
	R4DecodeString(label, vol->label, 3);
}

VOLUME* VOLGetVolume(unsigned int volume)
{
	if(volume >= MAX_VOLUMES) {
		return NULL;
	}

	return &volumes[volume];
}

BOOL VOLIsActive(unsigned int volume)
{
	if(volume >= MAX_VOLUMES) {
		return FALSE;
	}

	VOLUME* vol = VOLGetVolume(volume);
	return vol->read || vol->write;
}

void VOLReadBlock(unsigned int volume, void* buf, unsigned int block)
{
	if(volume >= MAX_VOLUMES) {
		return;
	}

	VOLUME* vol = &volumes[volume];
	if(vol->read) {
		if(current_block == block && volume == current_volume) {
			return;
		} else {
			current_block = block;
			current_volume = volume;
			vol->read(buf, block);
		}
	}
}

void VOLWriteBlock(unsigned int volume, void* buf, unsigned int block)
{
	if(volume >= MAX_VOLUMES) {
		return;
	}

	VOLUME* vol = &volumes[volume];
	if(vol->write) {
		current_block = block;
		current_volume = volume;
		vol->write(buf, block);
	}
}
