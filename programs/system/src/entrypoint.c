#include <emulight/ispf.h>

#include "system.h"

SPFPanelDefinition start = {
	.init = SYSInit,
	.draw = SYSDraw,
	.update = NULL,
	.process = SYSProcess,
	.destroy = SYSDestroy
};
