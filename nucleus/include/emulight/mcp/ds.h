#ifndef __DS_H__
#define __DS_H__

#include <emulight/types.h>

#define	MAX_DATASETS	2

int	__DSOpen(const char* dsn);
size_t	__DSRead(int ds, void* dst, size_t size);
void	__DSSeek(int ds, size_t pos);
void	__DSClose(int ds);

#endif
