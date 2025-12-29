#ifndef __CALENDAR_H__
#define __CALENDAR_H__

#include <emulight/types.h>
#include <emulight/ispf.h>

void CALInit(SPFPanel* self);
void CALDraw(SPFPanel* self);
void CALProcess(SPFPanel* self, u16 key);

#endif
