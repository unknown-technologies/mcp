#include <emulight/ispf.h>

#include "calendar.h"

SPFPanelDefinition start = {
	.init = CALInit,
	.draw = CALDraw,
	.update = NULL,
	.process = CALProcess,
	.destroy = NULL
};
