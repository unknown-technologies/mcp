#ifndef __DS_H__
#define __DS_H__

#include <emulight/types.h>
#include <emulight/syscall.h>

typedef struct {
	unsigned int	block;		/* start block */
	unsigned int	size;		/* size in blocks */
	unsigned int	vtocix;		/* VTOC index */
	unsigned int	flags;		/* DSCB flags */
	unsigned int	volume;		/* volume ID */
} DS;

#define	EFS_DSCB_DS_TYPE_EXEC	(0 << 14)
#define	EFS_DSCB_DS_TYPE_TXT6B	(1 << 14)
#define	EFS_DSCB_DS_TYPE_TXT7B	(2 << 14)
#define	EFS_DSCB_DS_TYPE_BIN	(3 << 14)

static inline unsigned int DSGetType(DS* ds)
{
	return (ds->flags >> 14) & 0x03;
}

static inline unsigned int DSGetRecLen(DS* ds)
{
	unsigned int lrecl = ds->flags & 0x0FFF;
	if(lrecl > 0) {
		return lrecl + 1;
	} else {
		return 0;
	}
}

static inline BOOL DSIsFB(DS* ds)
{
	return DSGetRecLen(ds) > 0;
}

static inline int DSOpen(const char* dsn)
{
	register unsigned int r0 asm("r0") = (unsigned int) dsn;
	asm volatile ("emt " __SC(SYS_DSOPEN) : "=r"(r0) : "r"(r0) : "memory");
	return (int) r0;
}

static inline void DSGetInfo(int ds, DS* info)
{
	register int r0 asm("r0") = ds;
	register DS* r1 asm("r1") = info;
	asm volatile ("emt " __SC(SYS_DSGETINFO) : "=r"(r0) : "r"(r0), "r"(r1)
			: "memory");
}

static inline size_t DSRead(int ds, void* dst, size_t size)
{
	register unsigned int r0 asm("r0") = (unsigned int) ds;
	register void* r1 asm("r1") = dst;
	register size_t r2 asm("r2") = size;
	asm volatile ("emt " __SC(SYS_DSREAD) : "=r"(r0)
			: "r"(r0), "r"(r1), "r"(r2)
			: "memory");
	return (size_t) r0;
}

static inline size_t DSWrite(int ds, void* src, size_t size)
{
	register unsigned int r0 asm("r0") = (unsigned int) ds;
	register void* r1 asm("r1") = src;
	register size_t r2 asm("r2") = size;
	asm volatile ("emt " __SC(SYS_DSWRITE) : "=r"(r0)
			: "r"(r0), "r"(r1), "r"(r2)
			: "memory");
	return (size_t) r0;
}

static inline void DSSeek(int ds, size_t pos)
{
	register int r0 asm("r0") = ds;
	register size_t r1 asm("r1") = pos;
	asm volatile ("emt " __SC(SYS_DSSEEK) : "=r"(r0) : "r"(r0), "r"(r1));
}

static inline void DSClose(int ds)
{
	register int r0 asm("r0") = ds;
	asm volatile ("emt " __SC(SYS_DSCLOSE) : "=r"(r0) : "r"(r0));
}

static inline int FSList(const char* prefix,
		void (*callback)(const char* dsn, void* user), void* user)
{
	register unsigned int r0 asm("r0") = (unsigned int) prefix;
	register unsigned int r1 asm("r1") = (unsigned int) callback;
	register unsigned int r2 asm("r2") = (unsigned int) user;
	asm volatile ("emt " __SC(SYS_FSLIST) : "=r"(r0)
			: "r"(r0), "r"(r1), "r"(r2) : "memory");
	return (int) r0;
}

static inline void VOLGetLabel(unsigned int vol, char* label)
{
	register unsigned int r0 asm("r0") = vol;
	register char* r1 asm("r1") = label;
	asm volatile ("emt " __SC(SYS_VOLGETLABEL) : "=r"(r0)
			: "r"(r0), "r"(r1) : "memory");
}

#endif
