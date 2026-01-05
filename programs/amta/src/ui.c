#include <emulight/string.h>
#include <emulight/ispf.h>

#include "game.h"

static GAME* game;

void UIInit(SPFPanel* self)
{
	SPFHideActions(self);
	SPFSetFullscreen(self);

	game = MEMAlloc(sizeof(GAME));
	if(!game) {
		SPFDestroyPanel(self);
		return;
	}

	game->name[0] = 0;

	if(!GAMEInit(game, "SYS1.ISPF.GAME.AMTA.STORY")) {
		SPFDestroyPanel(self);
		return;
	}

	if(!game->name[0]) {
		strcpy(game->name, "Untitled");
	}

	SPFSetPanelTitle(self, game->name);
}

void UIDraw(SPFPanel* self)
{
	unsigned int width = VTGetWidth(&self->spf->vt);
	unsigned int height = VTGetHeight(&self->spf->vt);

	if(game->title[0] != '-') {
		VTSGR(1);
		UIPrintCenter(self, 1, game->title);

		VTSGR(0);

		VTGoto(2, 1);
		VTGraphics();
		for(unsigned int i = 0; i < width; i++) {
			VTPutc('q');
		}
		VTAscii();
	}

	if(game->markup) {
		if(!UIMarkup(self, game->message)) {
			SPFDestroyPanel(self);
			return;
		}
	} else {
		VTGoto(4, 1);
		printindent(self, game->message, 2);
	}

	unsigned int line = height - game->btncnt;
	for(unsigned int i = 0; i < game->btncnt; i++) {
		/* buttons starting with '-' are invisible */
		if(game->buttons[i].name[0] == '-') {
			continue;
		}

		VTGoto(line + i, 1);
		if(i == game->selection) {
			VTSGR(7);
		}
		VTPrintf("%d: %s", i + 1, game->buttons[i].name);
		if(i == game->selection) {
			VTSGR(0);
		}
	}
}

void UIProcess(SPFPanel* self, u16 key)
{
	unsigned int height = VTGetHeight(&self->spf->vt);

	switch(key) {
		case VT_KEY_UP: {
			/* already at the first button */
			unsigned int sel = game->selection;
			if(sel == 0) {
				return;
			}

			unsigned int line = height - game->btncnt;
			VTGoto(line + sel, 1);
			VTPrintf("%d: %s", sel + 1,
					game->buttons[sel].name);

			sel--;

			VTGoto(line + sel, 1);
			VTSGR(7);
			VTPrintf("%d: %s", sel + 1,
					game->buttons[sel].name);
			VTSGR(0);

			game->selection = sel;
			break;
		}
		case VT_KEY_DOWN: {
			/* already at the last button */
			unsigned int sel = game->selection;
			if(sel + 1 >= game->btncnt) {
				return;
			}

			unsigned int line = height - game->btncnt;
			VTGoto(line + sel, 1);
			VTPrintf("%d: %s", sel + 1,
					game->buttons[sel].name);

			sel++;

			VTGoto(line + sel, 1);
			VTSGR(7);
			VTPrintf("%d: %s", sel + 1,
					game->buttons[sel].name);
			VTSGR(0);

			game->selection = sel;
			break;
		}
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9': {
			unsigned int id = key - '0';
			/* invalid button? */
			if(id < 1 || id > game->btncnt) {
				return;
			}

			id--;

			unsigned int sel = game->selection;

			/* no change? */
			if(sel == id) {
				return;
			}

			unsigned int line = height - game->btncnt;
			VTGoto(line + sel, 1);
			VTPrintf("%d: %s", sel + 1,
					game->buttons[sel].name);

			sel = id;

			VTGoto(line + sel, 1);
			VTSGR(7);
			VTPrintf("%d: %s", sel + 1,
					game->buttons[sel].name);
			VTSGR(0);

			game->selection = sel;
			break;
		}
		case VT_KEY_ENTER: {
			unsigned int sel = game->selection;
			unsigned int tgt = game->buttons[sel].target;
			if(!GAMEGotoState(game, tgt)) {
				SPFDestroyPanel(self);
			} else {
				SPFRepaint(self->spf);
			}
			break;
		}
	}
}

void UIDestroy(SPFPanel* self)
{
	if(game) {
		GAMEDestroy(game);
		MEMFree(game);
		game = NULL;
	}
}
