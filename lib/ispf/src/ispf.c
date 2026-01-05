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

	SPFPanel* current = SPFCurrentPanel(spf);

	BOOL is_fullscreen = current &&
			(current->flags & SPF_FLAG_FULLSCREEN);

	/* When not in fullscreen mode, we have to draw this part first,
	 * otherwise the cursor positioning would be wrong. */
	if(!is_fullscreen) {
		SPFDrawMenuBar(spf);
		SPFDrawStatusBar(spf);
	}

	VTHome();
	if(is_fullscreen) {
		VTSTBM(1, 24);
	} else {
		VTSTBM(2, 23);
	}
	VTSGR(0);

	if(spf->current_panel != -1) {
		SPFDrawPanel(current);
	}

	/* If we are in fullscreen mode and the menu is active, draw it
	 * at the end. Otherwise the first/last line of the panel would
	 * be drawn over the menu or status bar. */
	if(is_fullscreen && spf->menu_active) {
		SPFDrawMenuBar(spf);
		SPFDrawStatusBar(spf);
	}

	if(spf->menu_active) {
		VTWrite(CSI "?25l", CSILEN + 4);
		SPFDrawMenu(spf);
	} else if(spf->current_panel != -1) {
		if(SPFIsCursorVisible(current)) {
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

BOOL SPFProcess(SPF* spf)
{
	SPFCleanupPanels(spf);
	SPFPanel* panel = SPFCurrentPanel(spf);

	u16 key = VTReceive(&spf->vt, NULL, NULL);

	if(spf->error_active) {
		if(key != VT_KEY_NONE) {
			spf->error_active = FALSE;
			SPFRepaint(spf);
			return TRUE;
		}
		return FALSE;
	}

	BOOL handled = FALSE;
	if(key != VT_KEY_NONE && spf->default_actions) {
		for(const SPFAction* action = spf->default_actions;
				action->name; action++) {
			if(action->key == key && action->action) {
				action->action(spf, action);
				handled = TRUE;
				break;
			}
		}
	}

	if(!handled) {
		/* any other key is forwarded to the current panel */
		if(spf->menu_active) {
			SPFProcessMenu(spf, key);
		} else if(key != VT_KEY_NONE) {
			if(!SPFProcessMenuHotkeys(spf, key)) {
				SPFProcessPanel(panel, key);
			}
		} else {
			SPFProcessPanel(panel, key);
		}
	}

	if(spf->dirty) {
		SPFDraw(spf);
	} else if(!spf->menu_active) {
		SPFUpdate(spf);
		if(!panel || !(panel->flags & SPF_FLAG_FULLSCREEN)) {
			SPFUpdateStatusBar(spf);
		}
	} else {
		SPFUpdateStatusBar(spf);
	}

	return key != VT_KEY_NONE;
}

BOOL SPFLoadProgram(SPF* spf, const char* dsn)
{
	void* program = OSLoadProgram(dsn);
	if(!program) {
		SPFShowError(spf, "Failed to load program");
		return FALSE;
	}

	SPFPanelDefinition* def = (SPFPanelDefinition*) program;
	SPFPanel* panel = SPFCreatePanel(spf, def);
	if(panel) {
		SPFSetPanelProgram(panel, program);
		return TRUE;
	} else {
		OSLog(ISPF_FACILITY, ISPF_MSG_LOAD, LOG_WARN,
				"PROGRAM FAILED TO CREATE PANEL");
		OSUnloadProgram(program);
		return FALSE;
	}
}
