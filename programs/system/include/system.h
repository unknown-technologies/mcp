#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include <emulight/types.h>
#include <emulight/ispf.h>

typedef struct {
	const char*	name;
	const char*	description;
	const SPFPanelDefinition* panel;
} MENU;

extern const SPFPanelDefinition syslog_panel;

extern const MENU main_menu[];
extern int main_menu_max;

void SYSInit(SPFPanel* self);
void SYSDraw(SPFPanel* self);
void SYSProcess(SPFPanel* self, u16 key);
void SYSDestroy(SPFPanel* self);

void SYSSetCursor(SPFPanel* self);
BOOL SYSExecuteCommand(SPFPanel* self, const char* cmd);

#endif
