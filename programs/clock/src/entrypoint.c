#include <emulight/ispf.h>

#include "clock.h"

SPFPanelDefinition start = {
	.init = CLKInit,
	.draw = CLKDraw,
	.update = CLKUpdate,
	.process = CLKProcess,
	.destroy = NULL
};
