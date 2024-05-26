#ifndef __ISPF__
#define __ISPF__

#include <emulight/types.h>
#include <emulight/ispf.h>

extern volatile BOOL exit_ispf;

void ISPFInit(void);
void ISPFProcess(void);
void ISPFInitRoot(SPF* spf);

#endif
