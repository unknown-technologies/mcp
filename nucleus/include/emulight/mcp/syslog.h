#ifndef __SYSLOG_H__
#define __SYSLOG_H__

#include <emulight/types.h>
#include <emulight/mcp/msgid.h>

#define	LOG_DEBUG	0
#define	LOG_INFO	1
#define	LOG_AUDIT	2
#define	LOG_WARN	3
#define	LOG_ERR		4
#define	LOG_CRIT	5

void OSLogInit(void);
void OSSetLogLevel(int severity);
void OSLog(u16 sys, unsigned int code, int severity, const char* msg, ...);

#endif
