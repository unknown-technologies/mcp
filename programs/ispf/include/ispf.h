#ifndef __ISPF__
#define __ISPF__

#include <emulight/types.h>
#include <emulight/ispf.h>

extern SPFPanelDefinition root_panel;

void ISPFInit(void);
BOOL ISPFProcess(void);
void ISPFDestroy(void);

#endif
