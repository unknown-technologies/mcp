#include <emulight/ispf.h>

#include "settings.h"

SPFPanelDefinition start = {
	.init = CFGInit,
	.draw = NULL,
	.update = NULL,
	.process = NULL,
	.destroy = NULL
};
