#ifndef __CLOCK_H__
#define __CLOCK_H__

#include <emulight/types.h>
#include <emulight/os.h>

typedef struct {
	OSTime	time;
	char	timebuf[12];
} Clock;

void CLKInit(SPFPanel* self);
BOOL CLKUpdate(SPFPanel* self);
void CLKDraw(SPFPanel* self);
void CLKProcess(SPFPanel* self, u16 key);

#endif
