#ifndef __DS_H__
#define __DS_H__

#include <emulight/types.h>
#include <emulight/mcp/fs.h>

#define	MAX_DATASETS	2

int	__DSOpen(const char* dsn);
void	__DSGetInfo(int ds, DSInfo* info);
size_t	__DSRead(int ds, void* dst, size_t size);
size_t	__DSWrite(int ds, void* src, size_t size);
void	__DSSeek(int ds, size_t pos);
void	__DSClose(int ds);

#endif
