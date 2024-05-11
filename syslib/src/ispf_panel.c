#include <emulight/types.h>
#include <emulight/string.h>
#include <emulight/vt.h>
#include <emulight/ispf.h>

SPFPanel* SPFCreatePanel(SPF* spf, const SPFPanelDefinition* dlg)
{
	/* hack to avoid miscompilation on GCC 13 */
	void (*const init)(SPFPanel*) = dlg->init;

	if(spf->current_panel + 1 >= SPF_MAX_PANELS) {
		SPFShowError(spf, "Cannot open new panel: out of resources");
		return NULL;
	}

	SPFPanel* current = SPFCurrentPanel(spf);
	void* program = current ? SPFGetPanelProgram(current) : NULL;

	SPFPanel* panel = &spf->panels[++spf->current_panel];
	bzero(panel, sizeof(SPFPanel));

	panel->proto = dlg;
	panel->spf = spf;
	panel->program = program;
	panel->flags = SPF_FLAG_ACTIVE;

	if(init) {
		init(panel);
	}

	/* draw panel for the first time */
	SPFRepaint(spf);

	return panel;
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
			if(SPFIsCursorVisible(panel)) {
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

	if(panel->proto->destroy) {
		panel->proto->destroy(panel);
	}

	panel->flags |= SPF_FLAG_DESTROY;
	spf->delete_pending = TRUE;
}

#define	SPF_FLAGS_DESTROY	(SPF_FLAG_DESTROY | SPF_FLAG_ACTIVE)
static void SPFDeleteProgram(SPFPanel* panel)
{
	SPF* spf = panel->spf;

	/* is there even a deletion handler? */
	if(!spf->delete_handler) {
		return;
	}

	void* program = SPFGetPanelProgram(panel);

	/* is there a program attached? */
	if(!program) {
		return;
	}

	/* Check if this is the last active panel:
	 * If there is at least another active panel that is not marked for
	 * deletion, keep the program. If all other panels are also marked for
	 * deletion, delete the program now and remove it from the remaining
	 * panels. */

	BOOL delete = TRUE;

	/* Check if the program should be deleted */
	for(int i = 0; i <= spf->current_panel; i++) {
		SPFPanel* p = &spf->panels[i];
		BOOL active = p->flags & SPF_FLAG_ACTIVE;
		BOOL destroy = p->flags & SPF_FLAG_DESTROY;
		BOOL keep = active && !destroy;
		if(keep && (p->program == program)) {
			delete = FALSE;
			break;
		}
	}

	if(delete) {
		/* Detach the program from all other panels that are marked for
		 * deletion */
		for(int i = 0; i <= spf->current_panel; i++) {
			SPFPanel* p = &spf->panels[i];
			BOOL active = p->flags & SPF_FLAG_ACTIVE;
			BOOL destroy = p->flags & SPF_FLAG_DESTROY;
			if(active && destroy && (p->program == program)) {
				SPFSetPanelProgram(p, NULL);
			}
		}

		/* Finally delete the program */
		spf->delete_handler(spf, program);
	}
}

void SPFCleanupPanels(SPF* spf)
{
	if(!spf->delete_pending) {
		return;
	}

	/* delete programs if necessary */
	for(int i = 0; i <= spf->current_panel; i++) {
		SPFPanel* panel = &spf->panels[i];

		if((panel->flags & SPF_FLAGS_DESTROY) == SPF_FLAGS_DESTROY) {
			SPFDeleteProgram(panel);
			panel->flags = 0;
		}
	}

	/* compact panel storage and remove deleted panels */
	int top = -1;
	for(int i = 0, wr = 0; i <= spf->current_panel; i++) {
		if(spf->panels[i].flags & SPF_FLAG_ACTIVE) {
			if(i != wr) {
				spf->panels[wr] = spf->panels[i];
			}
			top = wr;
			wr++;
		}
	}

	spf->current_panel = top;
	spf->delete_pending = FALSE;

	SPFRepaint(spf);
}
