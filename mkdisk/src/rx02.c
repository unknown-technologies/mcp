#include <string.h>

#include "rx02.h"

size_t get_offset(u16 track, u16 sector)
{
	return track * (26 * 256) + (sector - 1) * 256;
}

size_t get_sector(u32 lsn)
{
	u16 sector;
	u16 track = lsn / 26;
	u16 p2 = lsn % 26;
	p2 = (2 * p2 + (p2 >= 13 ? 1 : 0)) % 26;
	sector = 1 + (p2 + 6 * track) % 26;
	if(++track >= 77)
		track = 0;

	return get_offset(track, sector);
}

u8* get_block(u8* out, u8* floppy, u16 block)
{
	memcpy(out, floppy + get_sector(block * 2), 256);
	memcpy(out + 256, floppy + get_sector(block * 2 + 1), 256);
	return out;
}

void set_block(u8* floppy, u16 block, u8* data)
{
	memcpy(floppy + get_sector(block * 2), data, 256);
	memcpy(floppy + get_sector(block * 2 + 1), data + 256, 256);
}

static const char* r50chars = " ABCDEFGHIJKLMNOPQRSTUVWXYZ$.%0123456789";

void rad50dec(u8* out, u16 rad50)
{
	out[0] = r50chars[(rad50 / 1600) % 40];
	out[1] = r50chars[(rad50 / 40) % 40];
	out[2] = r50chars[rad50 % 40];
}

static inline u16 getidx(const char* s, int c)
{
	char* p = strchr(s, c);
	if(p == 0) {
		return 0xFFFF;
	} else {
		return (u16) ((uintptr_t)p - (uintptr_t)s);
	}
}

u16 rad50enc(const char* in)
{
	u16 c1 = getidx(r50chars, in[0]);
	u16 c2 = getidx(r50chars, in[1]);
	u16 c3 = getidx(r50chars, in[2]);
	if(c1 == 0xFFFF || c2 == 0xFFFF || c3 == 0xFFFF) {
		return 0;
	} else {
		return c1 * 1600 + c2 * 40 + c3;
	}
}
