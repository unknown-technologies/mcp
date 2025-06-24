#include <emulight/types.h>
#include <emulight/ispf.h>

#include "ispf.h"

void ISPFRootInit(SPFPanel* self)
{
	SPFHideCursor(self);
}

void ISPFRootDraw(SPFPanel* self)
{
	u16 height = VTGetHeight(&self->spf->vt);

	/* status line */
	VTGoto(height - 1, 2);
	VTPrint("F10=Menu");
}

static SPFPanelDefinition root_panel = {
	.init = ISPFRootInit,
	.draw = ISPFRootDraw,
	.update = NULL,
	.process = NULL,
	.destroy = NULL
};

void ISPFInitRoot(SPF* spf)
{
	SPFCreatePanel(spf, &root_panel);
}
