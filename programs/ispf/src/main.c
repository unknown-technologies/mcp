#include <emulight/types.h>
#include <emulight/string.h>
#include <emulight/vt.h>
#include <emulight/os.h>
#include <emulight/ds.h>
#include <emulight/ispf.h>

#include "ispf.h"

void start(void)
{
	ISPFInit();

	OSLog(ISPF_FACILITY, 100, LOG_INFO, "ISPF STARTED");

	while(1) {
		ISPFProcess();
		OSYield();
	}
}
