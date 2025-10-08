#include <emulight/types.h>
#include <emulight/ispf.h>

#include "system.h"

BOOL SYSExecuteCommand(SPFPanel* self, const char* cmd)
{
	SPF* spf = self->spf;

	const char* c = cmd;
	while(*c == ' ') {
		c++;
	}

	if((*c == 'x' || *c == 'X') && c[1] == 0) {
		/* terminate */
		SPFDestroyPanel(self);
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
		if(num > main_menu_max) {
			SPFShowError(spf, "Invalid option");
			return FALSE;
		}

		const MENU* menu = &main_menu[num];
		if(menu->panel) {
			SPFCreatePanel(self->spf, menu->panel);
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

