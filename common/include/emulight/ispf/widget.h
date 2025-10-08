#ifndef __ISPF_WIDGET_H__
#define __ISPF_WIDGET_H__

#define	SPF_WIDGET_BUTTON	0
#define	SPF_WIDGET_CHECKBOX	1
#define	SPF_WIDGET_INPUT_STR	2
#define	SPF_WIDGET_INPUT_DEC	3
#define	SPF_WIDGET_INPUT_HEX	4
#define	SPF_WIDGET_INPUT_PAN	5
#define	SPF_WIDGET_LABEL	6
#define	SPF_WIDGET_DYNAMIC_LBL	7

#define	SPF_LABEL_NORMAL	0
#define	SPF_LABEL_BOLD		_BV(0)
#define	SPF_LABEL_UNDERLINE	_BV(1)
#define	SPF_LABEL_REVERSE	_BV(2)
#define	SPF_LABEL_BLINK		_BV(3)

typedef struct {
	u8		type;
	u8		id;
	u8		pos_x;
	u8		pos_y;
	SPFPanel*	panel;
} SPFWidget;

typedef struct {
	SPFWidget	widget;
	const char*	text;
	unsigned int	flags;
} SPFLabel;

struct SPFCheckbox;
typedef struct SPFCheckbox SPFCheckbox;
struct SPFCheckbox {
	SPFWidget	widget;
	void		(*callback)(SPFCheckbox* self);
	BOOL		state;
};

/******************************************************************************/
/* Common routines                                                            */
/******************************************************************************/
void SPFCreateWidget(SPFWidget* widget, unsigned int type, unsigned int id);
void SPFDrawWidget(SPFWidget* widget);
BOOL SPFProcessWidget(SPFWidget* widget, u16 key);
void SPFFocusWidget(SPFWidget* widget);
BOOL SPFCanFocusWidget(SPFWidget* widget);

static inline unsigned int SPFGetWidgetID(SPFWidget* widget)
{
	return widget->id;
}

static inline void SPFSetWidgetPanel(SPFWidget* widget, SPFPanel* panel)
{
	widget->panel = panel;
}

static inline SPFPanel* SPFGetWidgetPanel(SPFWidget* widget)
{
	return widget->panel;
}

static inline SPFPanel* SPFGetPanel(SPFWidget* widget)
{
	return widget->panel;
}

/******************************************************************************/
/* Label routines                                                             */
/******************************************************************************/
void SPFCreateLabel(SPFLabel* label, unsigned int line, unsigned int col,
		const char* text, unsigned int flags);
void SPFDrawLabel(SPFLabel* label);

/******************************************************************************/
/* Checkbox routines                                                          */
/******************************************************************************/
void SPFCreateCheckbox(SPFCheckbox* checkbox, unsigned int id,
		unsigned int line, unsigned int col,
		void (*callback)(SPFCheckbox*));
void SPFDrawCheckbox(SPFCheckbox* checkbox);
void SPFFocusCheckbox(SPFCheckbox* checkbox);
BOOL SPFProcessCheckbox(SPFCheckbox* checkbox, u16 key);

static inline BOOL SPFGetCheckboxState(SPFCheckbox* checkbox)
{
	return checkbox->state;
}

static inline void SPFSetCheckboxState(SPFCheckbox* checkbox, BOOL state)
{
	checkbox->state = state;
}

#endif
