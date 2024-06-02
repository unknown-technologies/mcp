#include <emulight/types.h>
#include <emulight/string.h>
#include <emulight/vt.h>
#include <emulight/ispf.h>

void SPFInit(SPF* spf)
{
	/* clear entire data structure */
	bzero(spf, sizeof(SPF));

	/* initialize basic VT support library */
	VTInit(&spf->vt);

	spf->current_panel = -1;

	SPFInitStatusBar(spf);
	SPFRepaint(spf);
}

void SPFDraw(SPF* spf)
{
	spf->dirty = FALSE;

	VTWrite(CSI "?25l", CSILEN + 4);
	VTClearScreen();

	SPFDrawMenuBar(spf);
	SPFDrawStatusBar(spf);

	VTHome();
	VTSTBM(2, 23);
	VTSGR(0);

	if(spf->current_panel != -1) {
		SPFDrawPanel(SPFCurrentPanel(spf));
	}

	if(spf->menu_active) {
		VTWrite(CSI "?25l", CSILEN + 4);
		SPFDrawMenu(spf);
	} else if(spf->current_panel != -1) {
		if(SPFIsCursorVisible(SPFCurrentPanel(spf))) {
			VTWrite(CSI "?25h", CSILEN + 4);
		} else {
			VTWrite(CSI "?25l", CSILEN + 4);
		}
	}
}

void SPFUpdate(SPF* spf)
{
	if(spf->menu_active || spf->error_active) {
		return;
	}

	if(spf->current_panel != -1) {
		SPFUpdatePanel(SPFCurrentPanel(spf));
	}
}

void SPFRepaint(SPF* spf)
{
	spf->dirty = TRUE;
}

void SPFProcess(SPF* spf)
{
	SPFCleanupPanels(spf);

	u16 key = VTReceive(&spf->vt, NULL, NULL);

	if(spf->error_active) {
		if(key != VT_KEY_NONE) {
			spf->error_active = FALSE;
			SPFRepaint(spf);
		}
		return;
	}

	switch(key) {
		/* menu key */
		case VT_KEY_F10:
			if(spf->menu_active) {
				SPFHideMenu(spf);
			} else {
				SPFShowMenu(spf);
			}
			break;
		/* screen refresh key */
		case VTGetCTRL('L'):
			SPFRepaint(spf);
			break;
		/* any other key is forwarded to the current panel */
		default:
			if(spf->menu_active) {
				SPFProcessMenu(spf, key);
			} else if(key != VT_KEY_NONE) {
				if(!SPFProcessMenuHotkeys(spf, key)) {
					SPFPanel* panel = SPFCurrentPanel(spf);
					SPFProcessPanel(panel, key);
				}
			} else {
				SPFProcessPanel(SPFCurrentPanel(spf), key);
			}
	}

	if(spf->dirty) {
		SPFDraw(spf);
	} else if(!spf->menu_active) {
		SPFUpdate(spf);
		SPFUpdateStatusBar(spf);
	} else {
		SPFUpdateStatusBar(spf);
	}
}
