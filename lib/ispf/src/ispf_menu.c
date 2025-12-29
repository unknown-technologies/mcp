#include <emulight/types.h>
#include <emulight/string.h>
#include <emulight/vt.h>
#include <emulight/ispf.h>

/******************************************************************************/
/* Helper routines                                                            */
/******************************************************************************/
void SPFSetMenu(SPF* spf, unsigned int menu);
void SPFDrawMenuItem(SPF* spf, int index, const SPFMenuItem* item);

void SPFSetMenuBar(SPF* spf, const SPFMenuData* data, SPFMenuHandler handler)
{
	spf->menu.data = data;
	spf->menu.handler = handler;
	SPFRepaint(spf);
}

void SPFShowMenu(SPF* spf)
{
	spf->menu_active = TRUE;

	SPFPanel* current = SPFCurrentPanel(spf);
	BOOL is_fullscreen = current &&
			(current->flags & SPF_FLAG_FULLSCREEN);

	if(is_fullscreen) {
		SPFRepaint(spf);
	} else {
		VTWrite(CSI "?25l", CSILEN + 4);
		SPFSetMenu(spf, 0);
		SPFDrawMenu(spf);
	}
}

void SPFHideMenu(SPF* spf)
{
	spf->menu_active = FALSE;
	SPFRepaint(spf);
}

void SPFSetMenu(SPF* spf, unsigned int menu)
{
	SPFMenu* dlg = &spf->menu;
	const SPFMenuData* menus = dlg->data;

	if(!menus) {
		return;
	}

	unsigned int offset = 0;
	for(unsigned int i = 0; i < menu; i++) {
		offset += strlen(menus[i].name) + 2;
	}

	dlg->offset = offset;

	dlg->selected = 0;
	dlg->menu = menu;

	unsigned int len;
	unsigned int maxwidth = 0;

	for(len = 0; menus[menu].items[len].name; len++) {
		unsigned int l = strlen(menus[menu].items[len].name);
		if(l > maxwidth) {
			maxwidth = l;
		}
	}

	dlg->width = maxwidth;
	dlg->length = len;

	/* highlight current menu title */
	VTGoto(1, offset);
	VTWrite(CSI "0;1m", CSILEN + 4);
	VTPutc(' ');
	VTPrint(menus[menu].name);
	VTWrite(" " CSI "0m", CSILEN + 3);
}

void SPFProcessMenu(SPF* spf, u16 key)
{
	SPFMenu* dlg = &spf->menu;
	if(!dlg->data) {
		return;
	}

	const SPFMenuData* menu = &dlg->data[dlg->menu];

	switch(key) {
		case VT_KEY_UP:
			if(dlg->selected > 0) {
				int old = dlg->selected;
				dlg->selected--;
				if(menu->items[dlg->selected].name[0] == '-') {
					dlg->selected--;
				}
				SPFDrawMenuItem(spf, old, &menu->items[old]);
				SPFDrawMenuItem(spf, dlg->selected,
						&menu->items[dlg->selected]);
			}
			break;
		case VT_KEY_DOWN:
			if(dlg->selected + 1 < dlg->length) {
				int old = dlg->selected;
				dlg->selected++;
				if(menu->items[dlg->selected].name[0] == '-') {
					dlg->selected++;
				}
				SPFDrawMenuItem(spf, old, &menu->items[old]);
				SPFDrawMenuItem(spf, dlg->selected,
						&menu->items[dlg->selected]);
			}
			break;
		case VT_KEY_LEFT:
			if(dlg->menu > 0) {
				dlg->menu--;
				SPFSetMenu(spf, dlg->menu);
				SPFRepaint(spf);
			}
			break;
		case VT_KEY_RIGHT:
			if(dlg->data[dlg->menu + 1].name) {
				dlg->menu++;
				SPFSetMenu(spf, dlg->menu);
				SPFRepaint(spf);
			}
			break;
		case VT_KEY_ENTER:
			SPFHideMenu(spf);
			if(dlg->handler) {
				int action = menu->items[dlg->selected].action;
				dlg->handler(spf, action);
			}
			break;
	}
}

BOOL SPFProcessMenuHotkeys(SPF* spf, u16 key)
{
	SPFMenu* dlg = &spf->menu;

	if(!dlg->data) {
		return FALSE;
	}

	if(!dlg->handler) {
		return FALSE;
	}

	/* go through all menus to check hotkeys */
	for(const SPFMenuData* menu = spf->menu.data; menu->name; menu++) {
		for(const SPFMenuItem* item = menu->items; item && item->name;
				item++) {
			if(item->key == key) {
				dlg->handler(spf, item->action);
				return TRUE;
			}
		}
	}

	return FALSE;
}

void SPFDrawMenuItem(SPF* spf, int index, const SPFMenuItem* item)
{
	SPFMenu* dlg = &spf->menu;

	VTGoto(2 + index, 1 + dlg->offset);

	VTWrite(CSI "0m", CSILEN + 2);

	if(item->name == 0) {
		/* end of menu */
		VTWrite(ESC "(0m", 4);

		for(int i = 0; i < dlg->width; i++) {
			VTPutc('q');
		}

		VTWrite("j" ESC "(B", 4);
	} else if(item->name[0] == '-') {
		/* separator */
		VTWrite(ESC "(0t", 4);

		for(int i = 0; i < dlg->width; i++) {
			VTPutc('q');
		}

		VTWrite("u" ESC "(B", 4);
	} else {
		/* normal menu item */
		VTWrite(ESC "(0x" ESC "(B", 7);

		if(dlg->selected == index) {
			VTWrite(CSI "7m", CSILEN + 2);
		} else {
			VTWrite(CSI "0m", CSILEN + 2);
		}

		VTPrint(item->name);

		int len = strlen(item->name);
		for(; len < dlg->width; len++) {
			VTPutc(' ');
		}

		VTWrite(CSI "0m" ESC "(0x" ESC "(B", CSILEN + 9);
	}
}

void SPFDrawMenuData(SPF* spf)
{
	SPFMenu* dlg = &spf->menu;
	const SPFMenuData* menus = dlg->data;

	if(!menus) {
		return;
	}

	const SPFMenuData* menu = &menus[dlg->menu];

	const SPFMenuItem* item = menu->items;

	for(int i = 0; item; i++, item++) {
		SPFDrawMenuItem(spf, i, item);
		if(!item->name) {
			break;
		}
	}
}

void SPFDrawMenu(SPF* spf)
{
	SPFDrawMenuData(spf);
}

void SPFDrawMenuBar(SPF* spf)
{
	int pos = 1;

	VTGoto(1, 1);

	VTWrite(CSI "0;7m", CSILEN + 4);

	if(spf->menu.data) {
		const SPFMenuData* menu = spf->menu.data;
		while(menu->name) {
			BOOL highlight = spf->menu_active
				&& menu == &spf->menu.data[spf->menu.menu];
			if(highlight) {
				VTWrite(CSI "0;1m ", CSILEN + 5);
			} else {
				VTPutc(' ');
			}

			pos += VTPrint(menu->name);

			if(highlight) {
				VTWrite(" " CSI "0;7m", CSILEN + 5);
			} else {
				VTPutc(' ');
			}

			menu++;
			pos += 2;
		}
	}

	const char* panel_title = SPFGetCurrentPanelTitle(spf);
	if(panel_title) {
		unsigned int len = strlen(panel_title);

		int start = VTGetWidth(&spf->vt) - len - 2;
		if(start < pos) {
			VTWrite("  [", 3);
			pos += 3;
			while(pos < VTGetWidth(&spf->vt) - 4) {
				VTPutc(*(panel_title++));
				pos++;
			}
			VTWrite("...] ", 5);
		} else {
			while(pos < start) {
				VTPutc(' ');
				pos++;
			}

			VTPutc('[');
			VTWrite(panel_title, len);
			VTWrite("] ", 2);
		}
	} else {
		/* fill rest of menu bar with reverse color attribute */
		while(pos <= VTGetWidth(&spf->vt)) {
			VTPutc(' ');
			pos++;
		}
	}

	VTWrite(CSI "0m", CSILEN + 2);
}
