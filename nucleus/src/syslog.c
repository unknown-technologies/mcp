#include <stdarg.h>
#include <emulight/types.h>
#include <emulight/string.h>
#include <emulight/rad50.h>
#include <emulight/r6.h>
#include <emulight/mcp/mem.h>
#include <emulight/mcp/si.h>
#include <emulight/mcp/syslog.h>

static unsigned int lock = 0;
static int loglevel;
static char logbuf[96];
static BOOL logmem = FALSE;
static OSLogRecord* log_ringbuf = NULL;
static unsigned int log_ringbuf_wr = 0;
static unsigned int log_ringbuf_sz = 0;

static const char severity_chars[] = { 'D', 'I', 'A', 'W', 'E', 'C' };

void OSLogInit(void)
{
	logmem = FALSE;
	OSSetLogLevel(LOG_INFO);
	OSLog(NUC_SYSLOG_START, "THE SYSTEM LOG IS NOW ACTIVE");
}

void OSLogToMemory(BOOL mem)
{
	logmem = mem;

	if(mem && !log_ringbuf) {
		log_ringbuf = MEMAlloc(LOG_BUF_SIZE * sizeof(OSLogRecord));
		if(!log_ringbuf) {
			logmem = FALSE;
		} else {
			bzero(log_ringbuf, LOG_BUF_SIZE * sizeof(OSLogRecord));
		}
	}
}

static void OSLogToBuffer(u16 sys, unsigned int code, int severity, const char* text, size_t len)
{
	if(!logmem) {
		return;
	}

	u16 c = (code & 0x3FF) | ((severity & 0x0F) << 12);

	OSLogRecord* log = &log_ringbuf[log_ringbuf_wr];

	bzero(log, sizeof(OSLogRecord));

	log->sys = sys;
	log->code = c;

	if(len > LOG_MAX_MSG * 4 / 3) {
		len = LOG_MAX_MSG * 4 / 3;
	}
	R6Encode(log->data, text, len);

	log_ringbuf_wr = (log_ringbuf_wr + 1) % LOG_BUF_SIZE;
	if(log_ringbuf_sz < LOG_BUF_SIZE) {
		log_ringbuf_sz++;
	}
}

void OSSetLogLevel(int severity)
{
	loglevel = severity;
}

void OSGetLog(void (*callback)(const char* msg, void* user), void* user)
{
	if(!callback) {
		return;
	}

	MONLock(&lock);

	unsigned int rd = log_ringbuf_wr - log_ringbuf_sz;
	if(log_ringbuf_sz > log_ringbuf_wr) {
		rd += LOG_BUF_SIZE;
	}

	for(unsigned int i = 0; i < log_ringbuf_sz; i++) {
		OSLogRecord* log = &log_ringbuf[rd];
		rd = (rd + 1) % LOG_BUF_SIZE;

		bzero(logbuf, sizeof(logbuf));
		sprintf(&logbuf[3], "%03d", log->code & 0x3FF);
		logbuf[6] = severity_chars[(log->code >> 12) & 0x0F];
		logbuf[7] = ' ';

		RAD50Decode(logbuf, log->sys);

		char* dec = &logbuf[8];
		R6Decode(dec, log->data, LOG_MAX_MSG);

		callback(logbuf, user);
	}

	MONUnlock(&lock);
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

	MONLock(&lock);
	size_t len = sprintf(logbuf, "%s%03d%c ", sysstr, code, ch);

	if(!logmem) {
		SITransmit(logbuf, len);
	}

	va_list args;
	va_start(args, msg);
	len = vsprintf(logbuf, msg, args);
	va_end(args);

	OSLogToBuffer(sys, code, severity, logbuf, len);

	if(!logmem) {
		SITransmit(logbuf, len);
		SITransmit("\r\n", 2);
	}
	MONUnlock(&lock);
}
