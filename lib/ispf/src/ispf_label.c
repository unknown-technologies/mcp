#include <emulight/types.h>
#include <emulight/string.h>
#include <emulight/vt.h>
#include <emulight/ispf.h>

void SPFCreateLabel(SPFLabel* label, unsigned int line, unsigned int col,
		const char* text, unsigned int flags)
{
	SPFCreateWidget(&label->widget, SPF_WIDGET_LABEL, 0);
	label->widget.pos_x = col;
	label->widget.pos_y = line;
	label->text = text;
	label->flags = flags;
}

void SPFDrawLabel(SPFLabel* label)
{
	VTGoto(label->widget.pos_y, label->widget.pos_x);
	VTSGR(0);
	if(label->flags & SPF_LABEL_BOLD) {
		VTSGR(1);
	}
	if(label->flags & SPF_LABEL_UNDERLINE) {
		VTSGR(4);
	}
	if(label->flags & SPF_LABEL_REVERSE) {
		VTSGR(7);
	}
	if(label->flags & SPF_LABEL_BLINK) {
		VTSGR(5);
	}
	VTPrint(label->text);
	VTSGR(0);
}
