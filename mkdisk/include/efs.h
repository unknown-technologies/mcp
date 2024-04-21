#ifndef __EFS_H__
#define __EFS_H__

#include "types.h"

/* This is the simplified EFS file system for devices with at most 32MiB size
 * when a 512 byte logical block size is used.
 *
 * Bitmap size:
 * - RX02: 1 block  [512512 / 512 / 8 / 512]
 * - RL02: 5 blocks [(2 * 40 * 512 * 256) / 512 / 8 / 512]
 *
 * Disk layout:
 * Block 0: boot loader
 * Block 1: VTOC
 * Block 2..5: monitor core image
 * Block 6..end: data
 */

#define	EFS_DSCB_UNUSED		0
#define	EFS_DSCB_VTOC		1
#define	EFS_DSCB_DATASET	2
#define	EFS_DSCB_FREE		0xFF

/* sizeof(DSCB_DS) == 32 */
typedef struct {
	u16	name[10];
	u16	start_block;
	u16	size;
	u16	flags;
	u16	create_year;
	u16	create_time;
	u8	create_date;
	u8	type;
} DSCB_DS;

/* sizeof(DSCB_VTOC) == 32 */
typedef struct {
	u8	magic[8];
	u16	label[3];
	u16	size;
	u16	vtoc_size;
	u16	vtoc_block;
	u16	bitmap;
	u16	bitmap_backup;
	u16	flags;
	u16	create_year;
	u16	create_time;
	u8	create_date;
	u8	type;
} DSCB_VTOC;

typedef struct {
	void	(*read)(void* userdata, void* data, unsigned int sector);
	void	(*write)(void* userdata, void* data, unsigned int sector);
	void*	userdata;
} EFS;

void	EFSInit(EFS* efs, void* userdata, void (*read)(void*, void*, unsigned int), void (*write)(void*, void*, unsigned int));
void	EFSInitDisk(EFS* efs, const char* label, size_t size, unsigned int vtoc_size, unsigned int start_block);
void	EFSStoreSequential(EFS* efs, const char* dsn, size_t size, void* data);

#endif
