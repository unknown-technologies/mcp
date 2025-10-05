#include <emulight/string.h>
#include <emulight/mcp/mon.h>
#include <emulight/mcp/si.h>
#include <emulight/mcp/syslog.h>
#include <emulight/mcp/vol.h>
#include <emulight/mcp/fs.h>
#include <emulight/mcp/rl02.h>
#include <emulight/mcp/sched.h>
#include <emulight/mcp/mem.h>
#include <emulight/mcp/program.h>
#include <emulight/mcp/cal.h>
#include <emulight/mcp/syscall.h>

void init(void)
{
	OSSyscallInit();
	OSLogInit();
	OSSetLogLevel(LOG_DEBUG);
	OSLog(NUC_START, "THIS IS NUCLEUS R" REVISION " (COMMIT " COMMIT
			") BUILT ON " BUILDDATE " " BUILDTIME);

	/* memory is from 02000 (initial SP) to __relbase */
	MEMInit((void*) 02000, __relbase);
	OSLogToMemory(TRUE);

	OSSchedInit();
	VOLInit();
	RLInit();
	OSInitLibrary();
	OSInitCalendar();

	OSLog(NUC_MEM, "%u BYTES AVAILABLE", MEMAvailable());
}

void shutdown(void)
{
	OSLog(MSGID_NUC, 500, LOG_INFO, "NUCLEUS EXITED");

	MONHalt();
}

void nucleus(void)
{
	init();

	void* startup = OSLoadProgram("SYS1.STARTUP");
	if(startup) {
		void (*main)(void) = (void (*)(void)) startup;
		main();
	} else {
		OSLogToMemory(FALSE);
		OSLog(NUC_LOGON_FAIL, "FAILED TO LAUNCH STARTUP PROGRAM");
	}

	shutdown();

	asm("HALT");
}
