#include <emulight/ispf.h>

#include "calc.h"

SPFPanelDefinition start = {
	.init = CALCInit,
	.draw = CALCDraw,
	.update = NULL,
	.process = CALCProcess,
	.destroy = NULL
};
