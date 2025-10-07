#include <emulight/types.h>
#include <emulight/ispf.h>

#include "workspace.h"

BOOL WSExecuteCommand(SPFPanel* self, const char* cmd)
{
	SPF* spf = self->spf;

	const char* c = cmd;
	while(*c == ' ') {
		c++;
	}

	if((*c == 'x' || *c == 'X') && c[1] == 0) {
		/* terminate */
		SPFDestroyPanel(self);
		SPFTerminate(self->spf);
		return TRUE;
	}

	/* parse it as menu item */
	if(*c >= '0' && *c <= '9') {
		int num = *c - '0';
		c++;
		while(*c >= '0' && *c <= '9') {
			num *= 10;
			num += *(c++) - '0';
		}

		if(*c != 0 || num < 0) {
			SPFShowError(spf, "Invalid command");
			return FALSE;
		}

		/* now search the menu item */
		if(num >= main_menu_cnt) {
			SPFShowError(spf, "Invalid option");
			return FALSE;
		}

		const MENU* menu = &main_menu[num];
		if(menu->program) {
			SPFExecProgram(self, menu->program);
			return TRUE;
		} else {
			SPFShowError(spf, "No program associated with "
					"option");
			return FALSE;
		}
	}

	SPFShowError(spf, "Invalid command");
	return FALSE;
}
