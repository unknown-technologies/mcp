#include <stdarg.h>
#include <emulight/types.h>
#include <emulight/string.h>
#include <emulight/rad50.h>
#include <emulight/mcp/si.h>
#include <emulight/mcp/syslog.h>

static int loglevel;
static char logbuf[96];

static const char severity_chars[] = { 'D', 'I', 'A', 'W', 'E', 'C' };

void OSLogInit(void)
{
	OSSetLogLevel(LOG_INFO);
	OSLog(NUC_SYSLOG_START, "THE SYSTEM LOG IS NOW ACTIVE");
}

void OSSetLogLevel(int severity)
{
	loglevel = severity;
}

void OSLog(u16 sys, unsigned int code, int severity, const char* msg, ...)
{
	char ch;
	char sysstr[4];

	if(severity < loglevel) {
		return;
	}

	RAD50Decode(sysstr, sys);
	sysstr[3] = 0;

	if(severity <= LOG_CRIT) {
		ch = severity_chars[severity];
	} else {
		ch = '?';
	}

	size_t len = sprintf(logbuf, "%s%03d%c ", sysstr, code, ch);

	SITransmit(logbuf, len);

	va_list args;
	va_start(args, msg);
	len = vsprintf(logbuf, msg, args);
	va_end(args);

	SITransmit(logbuf, len);
	SITransmit("\r\n", 2);
}
