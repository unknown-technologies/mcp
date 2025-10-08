#include <emulight/ispf.h>
#include <emulight/string.h>

#include "settings.h"

static SPFLabel label_title;
static SPFLabel label_options;
static SPFLabel label_info;
static SPFLabel label_reverse_video;
static SPFCheckbox reverse_video;

static SPFWidget* widgets[] = {
	(SPFWidget*) &label_title,
	(SPFWidget*) &label_options,
	(SPFWidget*) &label_info,
	(SPFWidget*) &label_reverse_video,
	(SPFWidget*) &reverse_video,
	NULL
};

static void CFGSetReverseVideo(SPFCheckbox* self)
{
	BOOL state = SPFGetCheckboxState(self);
	SPFPanel* panel = SPFGetPanel((SPFWidget*) self);
	SPF* spf = panel->spf;
	if(state) {
		VTDecEn(DECSCNM);
		SPFSetOption(spf, SPF_OPTION_REVERSE);
	} else {
		VTDecDis(DECSCNM);
		SPFClearOption(spf, SPF_OPTION_REVERSE);
	}
}

void CFGInit(SPFPanel* self)
{
	SPFShowCursor(self);
	SPFSetPanelTitle(self, "Settings");

	u16 width = VTGetWidth(&self->spf->vt);

	SPFCreateCheckbox(&reverse_video, 0, 8, 3, CFGSetReverseVideo);
	SPFSetCheckboxState(&reverse_video,
			SPFGetOption(self->spf, SPF_OPTION_REVERSE));

	const char* title = "MCP Settings";
	u16 center = (width - strlen(title)) / 2;

	SPFCreateLabel(&label_title, 3, center + 1, title, SPF_LABEL_NORMAL);
	SPFCreateLabel(&label_options, 6, 2, "Options", SPF_LABEL_NORMAL);
	SPFCreateLabel(&label_info, 7, 2, "Enter \"/\" to select option",
			SPF_LABEL_NORMAL);
	SPFCreateLabel(&label_reverse_video, 8, 6,
			"Enable reverse video mode", SPF_LABEL_NORMAL);
	SPFSetWidgets(self, widgets);
}
