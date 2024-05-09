#ifndef __SI_H__
#define __SI_H__

#include <emulight/types.h>

static inline void SITransmit(const char* buf, unsigned int size)
{
	register const char* r0 asm("r0") = buf;
	register unsigned int r1 asm("r1") = size;
	asm volatile ("emt 1" : "=r"(r0) : "r"(r0), "r"(r1) : "memory");
}

static inline size_t SIReceive(char* buf, unsigned int size)
{
	register char* r0 asm("r0") = buf;
	register unsigned int r1 asm("r1") = size;
	asm volatile ("emt 2" : "=r"(r0) : "r"(r0), "r"(r1) : "memory");
	return (size_t) r0;
}

static inline BOOL SIReceiveByte(u8* byte)
{
	return SIReceive((char*) byte, 1) == 1;
}

#endif
