#ifndef __DS_H__
#define __DS_H__

#include <emulight/types.h>
#include <emulight/mcp/fs.h>

#define	MAX_DATASETS	2

int	DSOpen(const char* dsn);
void	DSGetInfo(int ds, DSInfo* info);
size_t	DSRead(int ds, void* dst, size_t size);
size_t	DSWrite(int ds, void* src, size_t size);
void	DSSeek(int ds, size_t pos);
void	DSClose(int ds);

#endif
