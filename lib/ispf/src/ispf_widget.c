#include <emulight/types.h>
#include <emulight/string.h>
#include <emulight/ispf.h>

void SPFCreateWidget(SPFWidget* widget, unsigned int type, unsigned int id)
{
	bzero(widget, sizeof(SPFWidget));
	widget->type = type;
	widget->id = id;
}

void SPFDrawWidget(SPFWidget* widget)
{
	switch(widget->type) {
		case SPF_WIDGET_BUTTON:
			break;
		case SPF_WIDGET_CHECKBOX:
			SPFDrawCheckbox((SPFCheckbox*) widget);
			break;
		case SPF_WIDGET_INPUT_STR:
		case SPF_WIDGET_INPUT_DEC:
		case SPF_WIDGET_INPUT_HEX:
		case SPF_WIDGET_INPUT_PAN:
			break;
		case SPF_WIDGET_LABEL:
			SPFDrawLabel((SPFLabel*) widget);
			break;
		case SPF_WIDGET_DYNAMIC_LBL:
			break;
	}
}

BOOL SPFProcessWidget(SPFWidget* widget, u16 key)
{
	switch(widget->type) {
		case SPF_WIDGET_BUTTON:
			return FALSE;
		case SPF_WIDGET_CHECKBOX:
			return SPFProcessCheckbox((SPFCheckbox*) widget, key);
		case SPF_WIDGET_INPUT_STR:
		case SPF_WIDGET_INPUT_DEC:
		case SPF_WIDGET_INPUT_HEX:
		case SPF_WIDGET_INPUT_PAN:
		case SPF_WIDGET_LABEL:
		case SPF_WIDGET_DYNAMIC_LBL:
			return FALSE;
		default:
			return FALSE;
	}
}

void SPFFocusWidget(SPFWidget* widget)
{
	switch(widget->type) {
		case SPF_WIDGET_BUTTON:
			break;
		case SPF_WIDGET_CHECKBOX:
			SPFFocusCheckbox((SPFCheckbox*) widget);
			break;
		case SPF_WIDGET_INPUT_STR:
		case SPF_WIDGET_INPUT_DEC:
		case SPF_WIDGET_INPUT_HEX:
		case SPF_WIDGET_INPUT_PAN:
		case SPF_WIDGET_LABEL:
		case SPF_WIDGET_DYNAMIC_LBL:
			break;
	}
}

BOOL SPFCanFocusWidget(SPFWidget* widget)
{
	switch(widget->type) {
		case SPF_WIDGET_BUTTON:
		case SPF_WIDGET_CHECKBOX:
		case SPF_WIDGET_INPUT_STR:
		case SPF_WIDGET_INPUT_DEC:
		case SPF_WIDGET_INPUT_HEX:
		case SPF_WIDGET_INPUT_PAN:
			return TRUE;
		case SPF_WIDGET_LABEL:
		case SPF_WIDGET_DYNAMIC_LBL:
		default:
			return FALSE;
	}
}
