#ifndef __MEM_H__
#define __MEM_H__

#include <emulight/types.h>

void	MEMInit(void* start, void* end);
void*	MEMAlloc(size_t size);
void	MEMFree(void* ptr);
void*	MEMAllocBottom(size_t size);
void	MEMFreeBottom(void* ptr);
size_t	MEMAvailable(void);

#endif
