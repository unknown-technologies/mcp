#include <emulight/string.h>
#include <emulight/mcp/mon.h>
#include <emulight/mcp/si.h>
#include <emulight/mcp/syslog.h>
#include <emulight/mcp/rl02.h>
#include <emulight/mcp/sched.h>
#include <emulight/mcp/mem.h>
#include <emulight/mcp/program.h>
#include <emulight/mcp/syscall.h>

extern void* __relbase;

void nucleus(void)
{
	OSSyscallInit();
	OSLogInit();
	OSSetLogLevel(LOG_DEBUG);
	OSLog(NUC_START, "THIS IS NUCLEUS R" REVISION " (COMMIT " COMMIT
			") BUILT ON " BUILDDATE " " BUILDTIME);

	/* memory is from 01000 (initial SP) to __relbase */
	MEMInit((void*) 01000, __relbase);
	OSLogToMemory(FALSE);

	OSSchedInit();
	RLInit();

	OSInitLibrary();
	OSLoadLibrary("SYSLIB", "SYS1.SYSLIB");

	OSLog(NUC_MEM, "%u BYTES AVAILABLE", MEMAvailable());

	void* ispf = OSLoadProgram("ISPF", "SYS1.ISPF");
	if(ispf) {
		void (*ispf_main)(void) = (void (*)(void)) ispf;
		ispf_main();
	}

	OSLog(MSGID_NUC, 500, LOG_INFO, "NUCLEUS EXITED");

	MONHalt();

	asm("HALT");
}
