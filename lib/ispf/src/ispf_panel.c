#include <emulight/types.h>
#include <emulight/string.h>
#include <emulight/vt.h>
#include <emulight/ispf.h>

#define	DSN_MAXLEN	30

static char exec_buf[DSN_MAXLEN + 1];

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
	panel->flags = SPF_FLAG_ACTIVE | SPF_FLAG_ACTIONS;

	if(init) {
		init(panel);
	}

	/* draw panel for the first time */
	SPFRepaint(spf);

	return panel;
}

static void SPFDrawActions(const SPFAction* actions)
{
	for(const SPFAction* action = actions; action->name; action++) {
		if(action->key >= VT_KEY_F1 && action->key <= VT_KEY_F20) {
			VTPrintf("F%d", action->key - VT_KEY_F1 + 1);
		} else if(action->key >= VTGetCTRL('A')
				&& action->key <= VTGetCTRL('Z')) {
			VTPrintf("CTRL-%c", action->key - VTGetCTRL('A')
					+ 'A');
		}
		VTPutc('=');
		VTPrint(action->name);

		if(action[1].name) {
			VTPrint("  ");
		}
	}
}

void SPFDrawPanel(SPFPanel* panel)
{
	/* NULL is a valid argument, ignore it */
	if(!panel) {
		return;
	}

	if(panel->flags & SPF_FLAG_DESTROY) {
		return;
	}

	if((panel->actions || panel->spf->default_actions)
			&& (panel->flags & SPF_FLAG_ACTIONS)
			&& !SPFGetOption(panel->spf, SPF_OPTION_NOACTIONS)) {
		/* draw actions at the bottom of the screen */
		u16 height = VTGetHeight(&panel->spf->vt);
		VTGoto(height - 1, 2);
		BOOL space = FALSE;
		if(panel->spf->default_actions) {
			SPFDrawActions(panel->spf->default_actions);
			space = TRUE;
		}
		if(panel->actions) {
			if(space) {
				VTPrint("  ");
			}
			SPFDrawActions(panel->actions);
		}
	}

	if(panel->proto->draw) {
		panel->proto->draw(panel);
	}

	if(panel->widgets) {
		for(SPFWidget** widget = panel->widgets; *widget; widget++) {
			SPFDrawWidget(*widget);
		}

		if(panel->focus) {
			SPFFocusWidget(panel->focus);
		}
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

	BOOL done = FALSE;
	for(const SPFAction* action = panel->actions; action->name; action++) {
		if(action->key == key && action->action) {
			action->action(panel->spf, action);
			done = TRUE;
		}
	}

	if(!done && panel->widgets && panel->focus) {
		done = SPFProcessWidget(panel->focus, key);
	}

	if(!done && panel->proto->process) {
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

	/* ignore if already marked for deletion */
	if(panel->flags & SPF_FLAG_DESTROY) {
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

	BOOL exec = FALSE;

	/* delete programs if necessary */
	for(int i = 0; i <= spf->current_panel; i++) {
		SPFPanel* panel = &spf->panels[i];

		if((panel->flags & SPF_FLAGS_DESTROY) == SPF_FLAGS_DESTROY) {
			if(panel->flags & SPF_FLAG_EXEC) {
				exec = TRUE;
			}

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

	if(exec) {
		SPFLoadProgram(spf, exec_buf);
		bzero(exec_buf, sizeof(exec_buf));
	} else {
		SPFRepaint(spf);
	}
}

void SPFExecProgram(SPFPanel* panel, const char* dsn)
{
	SPFDestroyPanel(panel);
	panel->flags |= SPF_FLAG_EXEC;
	size_t len = strlen(dsn);
	if(len > DSN_MAXLEN) {
		len = DSN_MAXLEN;
	}
	memcpy(exec_buf, dsn, len);
	exec_buf[len] = 0;
}

void SPFSetWidgets(SPFPanel* panel, SPFWidget** widgets)
{
	panel->widgets = widgets;
	panel->focus = NULL;

	if(widgets) {
		for(SPFWidget** widget = widgets; *widget; widget++) {
			SPFSetWidgetPanel(*widget, panel);
			if(!panel->focus && SPFCanFocusWidget(*widget)) {
				panel->focus = *widget;
			}
		}
	}
}
