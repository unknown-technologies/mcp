#include <emulight/types.h>
#include <emulight/string.h>
#include <emulight/vt.h>
#include <emulight/ispf.h>

void SPFCreateCheckbox(SPFCheckbox* checkbox, unsigned int id,
		unsigned int line, unsigned int col,
		void (*callback)(SPFCheckbox*))
{
	SPFCreateWidget(&checkbox->widget, SPF_WIDGET_CHECKBOX, id);
	checkbox->widget.pos_x = col;
	checkbox->widget.pos_y = line;
	checkbox->callback = callback;
	checkbox->state = FALSE;
}

void SPFDrawCheckbox(SPFCheckbox* checkbox)
{
	VTGoto(checkbox->widget.pos_y, checkbox->widget.pos_x);
	VTWrite(CSI "0;1;4m", CSILEN + 6);
	VTPutc(checkbox->state ? '/' : ' ');
	VTSGR(0);
	VTCUB(1);
}

void SPFFocusCheckbox(SPFCheckbox* checkbox)
{
	VTGoto(checkbox->widget.pos_y, checkbox->widget.pos_x);
}

static void SPFCheckboxStateChanged(SPFCheckbox* checkbox)
{
	if(checkbox->callback) {
		checkbox->callback(checkbox);
	}
}

BOOL SPFProcessCheckbox(SPFCheckbox* checkbox, u16 key)
{
	switch(key) {
		case '/':
			checkbox->state = TRUE;
			SPFCheckboxStateChanged(checkbox);
			SPFDrawCheckbox(checkbox);
			return TRUE;
		case VT_KEY_BS:
		case VT_KEY_DEL:
			checkbox->state = FALSE;
			SPFCheckboxStateChanged(checkbox);
			SPFDrawCheckbox(checkbox);
			return TRUE;
		case VT_KEY_SPACE:
			checkbox->state = !checkbox->state;
			SPFCheckboxStateChanged(checkbox);
			SPFDrawCheckbox(checkbox);
			return TRUE;
		default:
			return FALSE;
	}
}
