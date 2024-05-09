#include <emulight/types.h>
#include <emulight/ispf.h>

#include "ispf.h"

void ISPFRootInit(SPFPanel* self)
{
	SPFSetPanelTitle(self, "Desktop");
	SPFHideCursor(self);
}

static SPFPanelDefinition root_panel = {
	.init = ISPFRootInit,
	.draw = NULL,
	.update = NULL,
	.process = NULL,
	.destroy = NULL
};

void ISPFInitRoot(SPF* spf)
{
	SPFCreatePanel(spf, &root_panel);
}
