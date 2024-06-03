#include <emulight/string.h>
#include <emulight/os.h>
#include <emulight/vt.h>

#include "logon.h"

static char dsn[32];

BOOL run_logon(void)
{
	void* program = OSLoadProgram("SYS1.LOGON");
	if(!program) {
		OSLog(MSG_NOT_FOUND, "FAILED TO LOAD LOGON PROGRAM");
		return FALSE;
	}

	const char* (*main)(void) = (const char* (*)(void)) program;
	const char* result = main();
	if(result) {
		strcpy(dsn, result);
	}

	OSUnloadProgram(program);

	VTClearScreen();
	VTHome();
	VTShowCursor();

	return result != NULL;
}

BOOL logon(void)
{
	if(!run_logon()) {
		return FALSE;
	} else {
		void* program = OSLoadProgram(dsn);
		if(program) {
			void (*main)(void) = (void (*)(void)) program;
			main();
			OSUnloadProgram(program);
		}
		return TRUE;
	}
}
