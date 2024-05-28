#ifndef __OS_H__
#define __OS_H__

#include <emulight/types.h>
#include <emulight/syscall.h>

#define LOG_DEBUG	0
#define LOG_INFO	1
#define LOG_AUDIT	2
#define LOG_WARN	3
#define LOG_ERR		4
#define LOG_CRIT	5

typedef struct {
	u8		day;
	u8		month;
	u16		year;
} OSDate;

typedef struct {
	u8		hour;
	u8		minute;
	u8		second;
	u8		tick;
} OSTime;

typedef struct {
	OSDate		date;
	OSTime		time;
} OSDateTime;

static inline void OSYield(void)
{
	asm volatile ("emt " __SC(SYS_YIELD));
}

static inline void OSLock(void* word)
{
	register void* r0 asm("r0") = word;
	asm volatile ("emt " __SC(SYS_MTXLK) : "=r"(r0) : "r"(r0) : "memory");
}

static inline void OSUnlock(void* word)
{
	register void* r0 asm("r0") = word;
	asm volatile ("emt " __SC(SYS_MTXUL) : "=r"(r0) : "r"(r0) : "memory");
}

static inline void OSLog(unsigned int sys, unsigned int code,
		unsigned int severity, const char* msg)
{
	register unsigned int r0 asm("r0") = sys;
	register unsigned int r1 asm("r1") = code;
	register unsigned int r2 asm("r2") = severity;
	register const char* r3 asm("r3") = msg;
	asm volatile ("emt " __SC(SYS_OSLOG) : "=r"(r0)
			: "r"(r0), "r"(r1), "r"(r2), "r"(r3)
			: "memory");
}

static inline void* OSLoadProgram(const char* dsn)
{
	register void* r0 asm("r0") = (void*) dsn;
	asm volatile ("emt " __SC(SYS_OSLOADPROG) : "=r"(r0)
			: "r"(r0)
			: "memory");
	return r0;
}

static inline void OSUnloadProgram(void* program)
{
	register void* r0 asm("r0") = program;
	asm volatile ("emt " __SC(SYS_OSULPROG) : "=r"(r0) : "r"(r0));
}

static inline int OSSpawn(const char* name, void (*pc)(void), void* sp,
		unsigned int flags)
{
	register unsigned int r0 asm("r0") = (unsigned int) name;
	register unsigned int r1 asm("r1") = (unsigned int) pc;
	register unsigned int r2 asm("r2") = (unsigned int) sp;
	register unsigned int r3 asm("r3") = (unsigned int) flags;
	asm volatile ("emt " __SC(SYS_OSSPAWN) : "=r"(r0)
			: "r"(r0), "r"(r1), "r"(r2), "r"(r3)
			: "memory");
	return (int) r0;
}

static inline int OSJoin(int pid)
{
	register int r0 asm("r0") = pid;
	asm volatile ("emt " __SC(SYS_OSJOIN) : "=r"(r0) : "r"(r0));
	return r0;
}

static inline void OSExit(int status)
{
	register int r0 asm("r0") = status;
	asm volatile ("emt " __SC(SYS_OSEXIT) : "=r"(r0) : "r"(r0));
}

static inline void OSGetTime(OSTime* time)
{
	register int r0 asm("r0");
	register int r1 asm("r1");
	register int r2 asm("r2");
	asm volatile ("emt " __SC(SYS_GETTIME) : "=r"(r0), "=r"(r1), "=r"(r2));
	time->hour = r0;
	time->minute = r1;
	time->second = r2;
}

static inline void OSSetTime(OSTime* time)
{
	register int r0 asm("r0") = time->hour;
	register int r1 asm("r1") = time->minute;
	register int r2 asm("r2") = time->second;
	asm volatile ("emt " __SC(SYS_SETTIME) : : "r"(r0), "r"(r1), "r"(r2));
}

static inline void OSGetDate(OSDate* date)
{
	register void* r0 asm("r0") = date;
	asm volatile ("emt " __SC(SYS_GETDATE) : "=r"(r0) : "r"(r0) : "memory");
}

static inline void OSSetDate(OSDate* date)
{
	register void* r0 asm("r0") = date;
	asm volatile ("emt " __SC(SYS_SETDATE) : "=r"(r0) : "r"(r0) : "memory");
}

static inline void OSGetDateTime(OSDateTime* datetime)
{
	OSGetDate(&datetime->date);
	OSGetTime(&datetime->time);
}

static inline void OSSetDateTime(OSDateTime* datetime)
{
	OSSetDate(&datetime->date);
	OSSetTime(&datetime->time);
}

static inline void OSGetLog(void (*callback)(const char*))
{
	register void* r0 asm("r0") = callback;
	asm volatile ("emt " __SC(SYS_OSGETLOG) : "=r"(r0) : "r"(r0) : "memory");
}

size_t MEMAvailable(void);

#endif
