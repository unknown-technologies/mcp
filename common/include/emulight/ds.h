#ifndef __DS_H__
#define __DS_H__

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

static inline int DSOpen(const char* dsn)
{
	register unsigned int r0 asm("r0") = (unsigned int) dsn;
	asm volatile ("emt 16" : "=r"(r0) : "r"(r0));
	return (int) r0;
}

static inline size_t DSRead(int ds, void* dst, size_t size)
{
	register unsigned int r0 asm("r0") = (unsigned int) ds;
	register void* r1 asm("r1") = dst;
	register size_t r2 asm("r2") = size;
	asm volatile ("emt 17" : "=r"(r0) : "r"(r0), "r"(r1), "r"(r2));
	return (size_t) r0;
}

static inline void DSSeek(int ds, size_t pos)
{
	register int r0 asm("r0") = ds;
	register size_t r1 asm("r1") = pos;
	asm volatile ("emt 18" : "=r"(r0) : "r"(r0), "r"(r1));
}

static inline void DSClose(int ds)
{
	register int r0 asm("r0") = ds;
	asm volatile ("emt 19" : "=r"(r0) : "r"(r0));
}

#endif
