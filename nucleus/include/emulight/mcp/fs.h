#ifndef __FS_H__
#define __FS_H__

#include <emulight/types.h>

typedef struct {
	unsigned int	pos;		/* position in block buffer */
	unsigned int	current_block;	/* current/next block */
	unsigned int	current_size;	/* remaining size */
	unsigned int	buf_block;	/* block in block buffer */
	unsigned int	block;		/* start block */
	unsigned int	size;		/* size in blocks */
	unsigned int	vtocix;		/* VTOC index */
} DS;

BOOL	DSOpen(DS* ds, const char* dsn);
size_t	DSRead(DS* ds, void* dst, size_t size);
void	DSSeek(DS* ds, size_t pos);

#endif
