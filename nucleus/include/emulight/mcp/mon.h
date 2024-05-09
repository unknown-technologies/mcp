#ifndef __MON_H__
#define __MON_H__

#include <emulight/types.h>

extern u8*	__sector_buffer;
extern void*	__relbase;
extern void**	__emttbl;

static inline void MONHalt(void)
{
	asm volatile ("emt 0");
}

static inline void MONTTO(const char* buf, unsigned int size)
{
	register const char* r0 asm("r0") = buf;
	register unsigned int r1 asm("r1") = size;
	asm volatile ("emt 1" : "=r"(r0) : "r"(r0), "r"(r1) : "memory");
}

static inline size_t MONTTI(const char* buf, unsigned int size)
{
	register unsigned int r0 asm("r0") = (unsigned int) buf;
	register unsigned int r1 asm("r1") = size;
	asm volatile ("emt 2" : "=r"(r0) : "r"(r0), "r"(r1) : "memory");

	return (size_t) r0;
}

static inline void MONReadBlock(void* buf, unsigned int block)
{
	register unsigned int r0 asm("r0") = block;
	register void* r1 asm("r1") = buf;
	asm volatile ("emt 3" : "=r"(r0) : "r"(r0), "r"(r1) : "memory");
}

static inline BOOL MONLocate(const char* name, unsigned int* vtocix,
		unsigned int* block, unsigned int* size)
{
	register unsigned int r0 asm("r0") = (unsigned int) name;
	register unsigned int r1 asm("r1");
	register unsigned int r2 asm("r2");

	asm volatile ("emt 4" : "=r"(r0), "=r"(r1), "=r"(r2) : "r"(r0) : "memory");

	if(r0 == (unsigned int) -1) {
		return FALSE;
	} else {
		if(vtocix) {
			*vtocix = r0;
		}
		if(block) {
			*block = r1;
		}
		if(size) {
			*size = r2;
		}
		return TRUE;
	}
}

static inline void* MONSpawn(void* sp, void* pc)
{
	register void* r0 asm("r0") = sp;
	register void* r1 asm("r1") = pc;
	asm volatile ("emt 5" : "=r"(r0) : "r"(r0), "r"(r1) : "memory");

	return r0;
}

static inline void MONYield(void)
{
	asm volatile ("emt 6");
}

static inline void MONLock(void* word)
{
	register void* r0 asm("r0") = word;
	asm volatile ("emt 7" : "=r"(r0) : "r"(r0));
}

static inline void MONUnlock(void* word)
{
	register void* r0 asm("r0") = word;
	asm volatile ("emt 8" : "=r"(r0) : "r"(r0));
}

#endif
