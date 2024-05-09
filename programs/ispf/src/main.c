#include <emulight/types.h>
#include <emulight/string.h>
#include <emulight/vt.h>
#include <emulight/os.h>
#include <emulight/ds.h>
#include <emulight/ispf.h>

void ISPFInit(void);
void ISPFProcess(void);

void start(void)
{
	ISPFInit();
	VTGoto(1, 1);
	VTShowCursor();

	VTClearScreen();
	VTHome();

	OSLog(ISPF_FACILITY, 100, LOG_INFO, "ISPF STARTED");

	while(1) {
		ISPFProcess();
		OSYield();
	}
}
