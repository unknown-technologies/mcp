#include <emulight/types.h>
#include <emulight/string.h>
#include <emulight/vt.h>
#include <emulight/ispf.h>

void SPFCreatePanel(SPF* spf, const SPFPanelDefinition* dlg)
{
	if(spf->current_panel + 1 >= SPF_MAX_PANELS) {
		SPFShowError(spf, "Cannot open new panel: out of resources");
		return;
	}

	SPFPanel* current = SPFCurrentPanel(spf);
	void* program = current ? SPFGetPanelProgram(current) : NULL;

	SPFPanel* panel = &spf->panels[++spf->current_panel];
	bzero(panel, sizeof(SPFPanel));

	panel->proto = dlg;
	panel->spf = spf;
	panel->program = program;

	if(panel->proto->init) {
		panel->proto->init(panel);
	}

	/* draw panel for the first time */
	SPFRepaint(spf);
}

void SPFDrawPanel(SPFPanel* panel)
{
	/* NULL is a valid argument, ignore it */
	if(!panel) {
		return;
	}

	if(panel->proto->draw) {
		panel->proto->draw(panel);
	}
}

void SPFUpdatePanel(SPFPanel* panel)
{
	/* NULL is a valid argument, ignore it */
	if(!panel) {
		return;
	}

	if(panel->proto->update) {
		if(panel->proto->update(panel)) {
			if(panel->show_cursor) {
				VTWrite(CSI "?25h", CSILEN + 4);
			} else {
				VTWrite(CSI "?25l", CSILEN + 4);
			}
		}
	}
}

void SPFProcessPanel(SPFPanel* panel, u16 key)
{
	/* NULL is a valid argument, ignore it */
	if(!panel) {
		return;
	}

	if(panel->proto->process) {
		panel->proto->process(panel, key);
	}

	SPFUpdatePanel(panel);
}

void SPFDestroyPanel(SPFPanel* panel)
{
	/* NULL is a valid argument, ignore it */
	if(!panel) {
		return;
	}

	SPF* spf = panel->spf;
	void* program = SPFGetPanelProgram(panel);

	if(panel->proto->destroy) {
		panel->proto->destroy(panel);
	}

	/* TODO: check if this panel is really the topmost one */
	spf->current_panel--;

	if(spf->delete_handler) {
		BOOL delete = TRUE;
		for(int i = 0; i <= spf->current_panel; i++) {
			if(spf->panels[i].program == program) {
				delete = FALSE;
			}
		}

		if(delete) {
			spf->delete_handler(spf, program);
		}
	}

	SPFRepaint(spf);
}
