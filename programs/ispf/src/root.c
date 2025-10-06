#include <emulight/types.h>
#include <emulight/ispf.h>

#include "ispf.h"

SPFPanelDefinition root_panel = {
	.init = SPFHideCursor,
	.draw = NULL,
	.update = NULL,
	.process = NULL,
	.destroy = NULL
};
