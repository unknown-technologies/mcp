#include <emulight/types.h>
#include <emulight/string.h>
#include <emulight/vt.h>
#include <emulight/os.h>
#include <emulight/ds.h>
#include <emulight/ispf.h>

#include "ispf.h"

volatile BOOL exit_ispf = FALSE;

void start(void)
{
	ISPFInit();

	OSLog(ISPF_FACILITY, 100, LOG_INFO, "ISPF STARTED");

	while(!exit_ispf) {
		ISPFProcess();
		OSYield();
	}

	ISPFDestroy();

	OSLog(ISPF_FACILITY, 101, LOG_INFO, "ISPF EXITED");
}
