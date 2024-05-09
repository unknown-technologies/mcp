#include <emulight/ispf.h>

#include "clockcfg.h"

SPFPanelDefinition start = {
	.init = CFGInit,
	.draw = CFGDraw,
	.update = NULL,
	.process = CFGProcess,
	.destroy = NULL
};
