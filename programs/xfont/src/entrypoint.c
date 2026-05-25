#include <emulight/ispf.h>

#include "xfont.h"

SPFPanelDefinition start = {
	.init = UIInit,
	.draw = UIDraw,
	.update = NULL,
	.process = UIProcess,
	.destroy = UIDestroy
};
