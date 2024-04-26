#ifndef __FS_H__
#define __FS_H__

#include <emulight/types.h>

typedef struct {
	unsigned int	pos;
	unsigned int	current_block;
	unsigned int	current_size;
	unsigned int	block;
	unsigned int	size;
	unsigned int	vtocix;
} DS;

BOOL	DSOpen(DS* ds, const char* dsn);
size_t	DSRead(DS* ds, void* dst, size_t size);
void	DSSeek(DS* ds, size_t pos);

#endif
