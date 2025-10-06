#include <emulight/ispf.h>

#include "workspace.h"

SPFPanelDefinition start = {
	.init = WSInit,
	.draw = WSDraw,
	.update = NULL,
	.process = WSProcess,
	.destroy = WSDestroy
};
