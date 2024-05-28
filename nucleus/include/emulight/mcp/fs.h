#ifndef __FS_H__
#define __FS_H__

#include <emulight/types.h>

#define	EFS_MAGIC		"EFFS-R01"

#define	EFS_DSCB_UNUSED		0
#define	EFS_DSCB_VTOC		1
#define	EFS_DSCB_DATASET	2
#define	EFS_DSCB_FREE		0xFF

#define	EFS_DSCB_DS_TYPE_EXEC	(0 << 14)
#define	EFS_DSCB_DS_TYPE_TXT7B	(1 << 14)
#define	EFS_DSCB_DS_TYPE_TXT8B	(2 << 14)
#define	EFS_DSCB_DS_TYPE_BIN	(3 << 14)
#define	EFS_DSCB_DS_FLAG_REC_FB	_BV(13)

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
	unsigned int	pos;		/* position in block buffer */
	unsigned int	current_block;	/* current/next block */
	unsigned int	current_size;	/* remaining size */
	unsigned int	buf_block;	/* block in block buffer */
	unsigned int	block;		/* start block */
	unsigned int	size;		/* size in blocks */
	unsigned int	vtocix;		/* VTOC index */
	unsigned int	flags;		/* DSCB_DS flags */
} DS;

void	FSInit(void);
void	FSList(const char* prefix,
		void (*callback)(const char* dsn, void* userdata),
		void* userdata);

BOOL	DSOpen(DS* ds, const char* dsn);
size_t	DSRead(DS* ds, void* dst, size_t size);
void	DSSeek(DS* ds, size_t pos);

#endif
