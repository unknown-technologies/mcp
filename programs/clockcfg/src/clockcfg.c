#include <emulight/string.h>
#include <emulight/os.h>
#include <emulight/ispf.h>

#include "clockcfg.h"

/******************************************************************************/
/* Date / Time configuration                                                  */
/******************************************************************************/

#define	UI_CLOCK_OK		0
#define	UI_CLOCK_CANCEL		1

static Config cfg;

void CFGInit(SPFPanel* self)
{
	SPFSetPanelTitle(self, "Clock Configuration");

	/* prepare data */
	OSGetDateTime(&cfg.clock);

	cfg.position = 0;
	cfg.field = 0;
	cfg.action = UI_CLOCK_OK;

	sprintf(cfg.datebuf, "%02d.%02d.%04d", cfg.clock.date.day,
			cfg.clock.date.month, cfg.clock.date.year);
	sprintf(cfg.timebuf, "%02d:%02d:%02d", cfg.clock.time.hour,
			cfg.clock.time.minute, cfg.clock.time.second);

	SPFShowCursor(self);
}

void CFGDraw(SPFPanel* self)
{
	u16 width = VTGetWidth(&self->spf->vt);
	u16 height = VTGetHeight(&self->spf->vt);

	u16 startX = (width - 23) / 2 + 1;
	u16 startY = (height - 7) / 2 + 1;

	VTGoto(startY, startX);
	VTPrint(ESC "(0l " ESC "(BClock Configuration " ESC "(0k");

	VTGoto(startY + 1, startX);
	VTPrint("x" CSI "21Cx");

	VTGoto(startY + 2, startX);
	VTPrintf(ESC "(0x  " ESC "(BDate:  " CSI "%sm%s"
			CSI "0m  " ESC "(0x",
			cfg.field == 0 ? "4;7" : "4", cfg.datebuf);

	VTGoto(startY + 3, startX);
	VTPrintf("x  " ESC "(BTime:  " CSI "%sm%s  "
			CSI "0m  " ESC "(0x",
			cfg.field == 1 ? "4;7" : "4", cfg.timebuf);

	VTGoto(startY + 4, startX);
	VTPrint("x" CSI "21Cx");

	VTGoto(startY + 5, startX);
	VTPrint("x    " ESC "(B");

	if(cfg.action == UI_CLOCK_OK) {
		VTPrint(CSI "7m[OK]" CSI "0m");
	} else {
		VTPrint("[OK]");
	}

	VTPrint(" ");

	if(cfg.action == UI_CLOCK_CANCEL) {
		VTPrint(CSI "7m[CANCEL]" CSI "0m");
	} else {
		VTPrint("[CANCEL]");
	}

	VTPrint("    " ESC "(0x");

	VTGoto(startY + 6, startX);
	VTPrint(ESC "(0mqqqqqqqqqqqqqqqqqqqqqj" ESC "(B");

	if(cfg.field == 0 || cfg.field == 1) {
		VTGoto(startY + 2 + cfg.field, startX + 10 + cfg.position);
		VTPrint(CSI "0;4;7m");
	} else {
		if(cfg.action == UI_CLOCK_OK) {
			VTGoto(startY + 5, startX + 6);
		} else {
			VTGoto(startY + 5, startX + 11);
		}
	}
}

void CFGProcess(SPFPanel* self, u16 key)
{
}
