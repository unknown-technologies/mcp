#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "types.h"
#include "efs.h"
#include "r4.h"

#define	EFS_MAGIC	"EFFS-R01"

void EFSSetCurrentDate(u16* efs_year, u8* efs_date, u16* efs_time)
{
	time_t t;
	time(&t);
	struct tm now = *localtime(&t);

	u8 current_day = now.tm_mday;
	u8 current_month = now.tm_mon + 1;
	u16 current_year = now.tm_year + 1900;

	u8 current_hour = now.tm_hour;
	u8 current_minute = now.tm_min;
	u8 current_second = now.tm_sec;

	u16 create_date = current_month * 32 + current_day;
	u16 create_year = current_year;
	if(create_date > 256) {
		create_year |= 0x8000;
		create_date &= 0xFF;
	}

	u32 create_time = (current_hour * 60 + current_minute) * 60
			+ current_second;
	if(create_time > 0xFFFF) {
	create_year |= 0x4000;
	create_time &= 0xFFFF;
	}

	*efs_year = create_year;
	*efs_date = create_date;
	*efs_time = create_time;
}

void EFSInit(EFS* efs, void* userdata, void (*read)(void*, void*, unsigned int), void (*write)(void*, void*, unsigned int))
{
	memset(efs, 0, sizeof(EFS));
	efs->userdata = userdata;
	efs->read = read;
	efs->write = write;
}

void EFSInitDisk(EFS* efs, const char* label, size_t size, unsigned int vtoc_size, unsigned int start_block)
{
	u32 sector[512 / sizeof(u32)];
	memset(sector, 0, sizeof(sector));

	/* create VTOC */
	DSCB_VTOC vtoc;
	memset(&vtoc, 0, sizeof(vtoc));

	unsigned int bitmap_size = (size + (512 * 8 - 1)) / (512 * 8);

	memcpy(vtoc.magic, EFS_MAGIC, 8);
	R4EncodeString(vtoc.label, label, 3);
	vtoc.size = size;
	vtoc.vtoc_size = vtoc_size;
	vtoc.vtoc_block = start_block;
	vtoc.bitmap = vtoc.vtoc_block + vtoc.vtoc_size;
	// vtoc.bitmap_backup;
	vtoc.flags = 0;
	EFSSetCurrentDate(&vtoc.create_year, &vtoc.create_date, &vtoc.create_time);
	vtoc.type = EFS_DSCB_VTOC;

	printf("VOL    blocks=%u\n", vtoc.size);
	printf("VTOC   block=%u size=%u\n", vtoc.vtoc_block, vtoc_size);
	printf("BITMAP block=%u size=%u\n", vtoc.bitmap, bitmap_size);

	memcpy(sector, &vtoc, sizeof(DSCB_VTOC));
	efs->write(efs->userdata, sector, 1);

	/* initialize bitmap */
	memset(sector, 0xFF, sizeof(sector));
	for(unsigned int block = vtoc.bitmap, i = 0; i < bitmap_size; block++, i++) {
		efs->write(efs->userdata, sector, block);
	}

	/* mark blocks of bootloader, VTOC and bitmap as allocated */
	printf("Marking bits 0 - %u\n", vtoc.bitmap + bitmap_size - 1);
	u8* bitmap = (u8*) sector;
	for(unsigned int i = 0; i < (vtoc.bitmap + bitmap_size); i++) {
		bitmap[i / 8] &= ~(1 << (i % 8));
	}
	efs->write(efs->userdata, sector, vtoc.bitmap);
}

u16 EFSGetBlock(EFS* efs, size_t size)
{
	u32 sector[512 / sizeof(u32)];

	/* read VTOC */
	efs->read(efs->userdata, sector, 1);

	DSCB_VTOC vtoc;
	memcpy(&vtoc, sector, sizeof(DSCB_VTOC));

	if(memcmp(vtoc.magic, EFS_MAGIC, 8)) {
		printf("Invalid EFS magic!\n");
		exit(1);
	}

	/* read bitmap */
	unsigned int lastbitblock = (unsigned int) -1;
	size_t count = size;
	for(unsigned int block = 0; block < vtoc.size; block++) {
		unsigned int bitblock = block / 8 / 512 + vtoc.bitmap;
		if(lastbitblock != bitblock) {
			efs->read(efs->userdata, sector, bitblock);
		}
		lastbitblock = bitblock;

		u8* bits = (u8*) sector;
		u8 bit = bits[block / 8] & (1 << (block % 8));
		if(!bit) {
			count = size;
			continue;
		} else if(count == 0) {
			return block - size;
		} else {
			count--;
		}
	}

	return 0;
}

void EFSSetBlocks(EFS* efs, u16 start, size_t count)
{
	u32 sector[512 / sizeof(u32)];

	/* read VTOC */
	efs->read(efs->userdata, sector, 1);

	DSCB_VTOC vtoc;
	memcpy(&vtoc, sector, sizeof(DSCB_VTOC));

	if(memcmp(vtoc.magic, EFS_MAGIC, 8)) {
		printf("Invalid EFS magic!\n");
		exit(1);
	}

	/* read bitmap */
	unsigned int lastbitblock = (unsigned int) -1;
	u8* bits = (u8*) sector;
	unsigned int blockid = 0;
	for(unsigned int block = start, i = 0; i < count; block++, i++) {
		unsigned int bitblock = block / 8 / 512 + vtoc.bitmap;
		blockid = bitblock;
		if(lastbitblock != bitblock) {
			efs->read(efs->userdata, sector, bitblock);
		}
		lastbitblock = bitblock;

		bits[block / 8] &= ~(1 << (block % 8));

		unsigned int nextbitblock = (block + 1) / 8 / 512 + vtoc.bitmap;
		if(nextbitblock != bitblock) {
			efs->write(efs->userdata, sector, bitblock);
		}
	}
	if(blockid) {
		efs->write(efs->userdata, sector, blockid);
	}
}

u16 EFSGetUsedBlocks(EFS* efs)
{
	u32 sector[512 / sizeof(u32)];

	/* read VTOC */
	efs->read(efs->userdata, sector, 1);

	DSCB_VTOC vtoc;
	memcpy(&vtoc, sector, sizeof(DSCB_VTOC));

	if(memcmp(vtoc.magic, EFS_MAGIC, 8)) {
		printf("Invalid EFS magic!\n");
		exit(1);
	}

	/* read bitmap */
	unsigned int lastbitblock = (unsigned int) -1;
	u16 size = 0;
	for(unsigned int block = 0; block < vtoc.size; block++) {
		unsigned int bitblock = block / 8 / 512 + vtoc.bitmap;
		if(lastbitblock != bitblock) {
			efs->read(efs->userdata, sector, bitblock);
		}
		lastbitblock = bitblock;

		u8* bits = (u8*) sector;
		u8 bit = bits[block / 8] & (1 << (block % 8));
		if(!bit) {
			size++;
		}
	}

	return size;
}

u16 EFSGetFreeBlocks(EFS* efs)
{
	u32 sector[512 / sizeof(u32)];

	/* read VTOC */
	efs->read(efs->userdata, sector, 1);

	DSCB_VTOC vtoc;
	memcpy(&vtoc, sector, sizeof(DSCB_VTOC));

	if(memcmp(vtoc.magic, EFS_MAGIC, 8)) {
		printf("Invalid EFS magic!\n");
		exit(1);
	}

	/* read bitmap */
	unsigned int lastbitblock = (unsigned int) -1;
	u16 size = 0;
	for(unsigned int block = 0; block < vtoc.size; block++) {
		unsigned int bitblock = block / 8 / 512 + vtoc.bitmap;
		if(lastbitblock != bitblock) {
			efs->read(efs->userdata, sector, bitblock);
		}
		lastbitblock = bitblock;

		u8* bits = (u8*) sector;
		u8 bit = bits[block / 8] & (1 << (block % 8));
		if(bit) {
			size++;
		}
	}

	return size;
}

void EFSStoreSequential(EFS* efs, const char* dsn, size_t size, void* data, unsigned int flags)
{
	u32 sector[512 / sizeof(u32)];

	/* read VTOC */
	efs->read(efs->userdata, sector, 1);

	DSCB_VTOC vtoc;
	memcpy(&vtoc, sector, sizeof(DSCB_VTOC));

	if(memcmp(vtoc.magic, EFS_MAGIC, 8)) {
		printf("Invalid EFS magic!\n");
		exit(1);
	}

	/* find free VTOCIX */
	unsigned int vtocix = (unsigned int) -1;
	DSCB_DS* dscb = (DSCB_DS*) sector;
	unsigned int dscbs_per_block = 512 / sizeof(DSCB_DS);

	for(unsigned int block = vtoc.vtoc_block, i = 0; i < vtoc.vtoc_size; block++, i++) {
		efs->read(efs->userdata, dscb, block);
		for(unsigned int j = 0; j < dscbs_per_block; j++) {
			if(dscb[j].type == EFS_DSCB_UNUSED || dscb[j].type == EFS_DSCB_FREE) {
				vtocix = i * dscbs_per_block + j;
				dscb = &dscb[j];
				goto vtocix_found;
			}
		}
	}
	printf("No free VTOCIX found!\n");
	exit(1);

vtocix_found:
	printf("Allocating DSN %s with VTOCIX %u\n", dsn, vtocix);

	/* find free space */
	u16 blocks = (size + 511) / 512;
	u16 block = EFSGetBlock(efs, blocks);

	if(!block) {
		printf("No free space found\n");
		exit(1);
	}

	printf("Allocating data set at block %u, %u blocks\n", block, blocks);
	EFSSetBlocks(efs, block, blocks);

	/* Create DSCB */
	efs->read(efs->userdata, sector, vtoc.vtoc_block + (vtocix / dscbs_per_block));

	R4EncodeString(dscb->name, dsn, sizeof(dscb->name) / sizeof(u16));
	dscb->start_block = block;
	dscb->size = blocks;
	dscb->flags = flags;
	EFSSetCurrentDate(&dscb->create_year, &dscb->create_date, &dscb->create_time);
	dscb->type = EFS_DSCB_DATASET;

	efs->write(efs->userdata, sector, vtoc.vtoc_block + (vtocix / dscbs_per_block));

	/* store data */
	size_t sz = size;
	for(u16 i = 0; i < blocks; i++, sz -= 512, data += 512) {
		if(sz < 512) {
			memset(sector, 0, 512);
			memcpy(sector, data, sz);
		} else {
			memcpy(sector, data, 512);
		}
		efs->write(efs->userdata, sector, i + block);
	}
}
