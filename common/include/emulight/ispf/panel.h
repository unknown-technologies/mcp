#ifndef __ISPF_PANEL_H__
#define __ISPF_PANEL_H__

#include <emulight/types.h>
#include <emulight/vt.h>
#include <emulight/os.h>

#define	SPF_MAX_PANELS		8

#define	SPF_FLAG_ACTIVE		_BV(0)
#define	SPF_FLAG_CURSOR		_BV(1)
#define	SPF_FLAG_ACTIONS	_BV(2)
#define	SPF_FLAG_DESTROY	_BV(4)
#define	SPF_FLAG_EXEC		_BV(8)

#define	SPF_OPTION_REVERSE	_BV(0)
#define	SPF_OPTION_NOACTIONS	_BV(1)

struct SPF;
struct SPFPanel;

typedef struct SPF SPF;
typedef struct SPFPanel SPFPanel;

typedef void			(*SPFMenuHandler)(SPF* spf, int action);
typedef const char*		(*SPFStatusBarHandler)(SPF* spef);
typedef void			(*SPFDeleteHandler)(SPF* spef, void* program);

typedef struct {
	const char*		name;
	const int		action;
	const u16		key;
} SPFMenuItem;

typedef struct {
	const char*		name;
	const SPFMenuItem*	items;
} SPFMenuData;

typedef struct {
	const SPFMenuData*	data;
	void			(*handler)(SPF* spf, int action);
	int			menu;
	int			selected;
	int			offset;
	int			length;
	int			width;
} SPFMenu;

typedef struct {
	void			(*init)(SPFPanel* self);
	void			(*draw)(SPFPanel* self);
	BOOL			(*update)(SPFPanel* self);
	void			(*process)(SPFPanel* self, u16 key);
	void			(*destroy)(SPFPanel* self);
} SPFPanelDefinition;

typedef struct {
	const char*		(*draw)(SPF* self);
	const char*		(*update)(SPF* spf);
} SPFStatusBar;

struct SPFAction;
typedef struct SPFAction SPFAction;
struct SPFAction {
	const char*		name;
	u16			key;
	void			(*action)(SPF* spf, const SPFAction* self);
};

#include <emulight/ispf/widget.h>

struct SPFPanel {
	const SPFPanelDefinition* proto;
	const char*		title;
	SPF*			spf;
	void*			userdata;
	void*			program;
	SPFWidget**		widgets;
	SPFWidget*		focus;
	const SPFAction*	actions;
	u16			flags;
};

struct SPF {
	void*			userdata;
	void			(*delete_handler)(SPF* spf, void* program);
	SPFPanel		panels[SPF_MAX_PANELS];
	int			current_panel;
	VT			vt;
	SPFMenu			menu;
	BOOL			menu_active;
	SPFStatusBar		status;
	BOOL			error_active;
	BOOL			dirty;
	BOOL			delete_pending;
	volatile BOOL		exit_request;
	const SPFAction*	default_actions;
	u16			options;
	OSDateTime		clock;
};

/******************************************************************************/
/* Helper routines                                                            */
/******************************************************************************/
static inline SPFPanel* SPFCurrentPanel(SPF* spf)
{
	if(spf->current_panel == -1) {
		return NULL;
	} else {
		return &spf->panels[spf->current_panel];
	}
}

static inline const char* SPFGetCurrentPanelTitle(SPF* spf)
{
	SPFPanel* panel = SPFCurrentPanel(spf);
	if(panel) {
		return panel->title;
	} else {
		return NULL;
	}
}

static inline void SPFSetPanelTitle(SPFPanel* panel, const char* title)
{
	panel->title = title;
}

static inline void SPFSetPanelProgram(SPFPanel* panel, void* program)
{
	panel->program = program;
}

static inline void* SPFGetPanelProgram(SPFPanel* panel)
{
	return panel->program;
}

static inline void* SPFGetPanelUserData(SPFPanel* panel)
{
	return panel->userdata;
}

static inline void SPFSetPanelUserData(SPFPanel* panel, void* data)
{
	panel->userdata = data;
}

static inline void SPFShowCursor(SPFPanel* panel)
{
	panel->flags |= SPF_FLAG_CURSOR;
}

static inline void SPFHideCursor(SPFPanel* panel)
{
	panel->flags &= ~SPF_FLAG_CURSOR;
}

static inline BOOL SPFIsCursorVisible(SPFPanel* panel)
{
	return panel->flags & SPF_FLAG_CURSOR;
}

static inline void SPFShowActions(SPFPanel* panel)
{
	panel->flags |= SPF_FLAG_ACTIONS;
}

static inline void SPFHideActions(SPFPanel* panel)
{
	panel->flags &= ~SPF_FLAG_ACTIONS;
}

static inline void SPFSetDefaultActions(SPF* spf, const SPFAction* actions)
{
	spf->default_actions = actions;
}

static inline void SPFSetActions(SPFPanel* panel, const SPFAction* actions)
{
	panel->actions = actions;
}

static inline void SPFSetDeleteHandler(SPF* spf, SPFDeleteHandler handler)
{
	spf->delete_handler = handler;
}

static inline void SPFSetOptions(SPF* spf, u16 options)
{
	spf->options |= options;
}

static inline void SPFClearOptions(SPF* spf, u16 options)
{
	spf->options &= ~options;
}

static inline BOOL SPFGetOptions(SPF* spf, u16 options)
{
	return (spf->options &= options) == options;
}

#define	SPFSetOption(spf, flag)		SPFSetOptions(spf, flag)
#define	SPFClearOption(spf, flag)	SPFClearOptions(spf, flag)
#define	SPFGetOption(spf, flag)		SPFGetOptions(spf, flag)

#endif
