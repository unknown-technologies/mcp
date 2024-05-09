#ifndef __CLOCKCFG_H__
#define __CLOCKCFG_H__

#include <emulight/types.h>
#include <emulight/os.h>

typedef struct {
	OSDateTime	clock;
	u8		position;
	u8		field;
	u8		action;
	char		datebuf[16];
	char		timebuf[16];
} Config;

void CFGInit(SPFPanel* panel);
void CFGDraw(SPFPanel* panel);
void CFGProcess(SPFPanel* panel, u16);

#endif
