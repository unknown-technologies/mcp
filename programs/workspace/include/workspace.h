#ifndef __WORKSPACE_H__
#define __WORKSPACE_H__

#include <emulight/types.h>
#include <emulight/ispf.h>

typedef struct {
	const char*	name;
	const char*	description;
	const char*	program;
} MENU;

extern const MENU main_menu[];
extern int main_menu_max;

void	WSInit(SPFPanel* self);
void	WSDraw(SPFPanel* self);
void	WSProcess(SPFPanel* self, u16 key);
void	WSDestroy(SPFPanel* self);

void	WSSetCursor(SPFPanel* self);

BOOL	WSExecuteCommand(SPFPanel* self, const char* cmd);

#endif
