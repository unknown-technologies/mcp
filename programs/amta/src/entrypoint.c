#include <emulight/ispf.h>

#include "game.h"

SPFPanelDefinition start = {
	.init = UIInit,
	.draw = UIDraw,
	.update = NULL,
	.process = UIProcess,
	.destroy = UIDestroy
};
