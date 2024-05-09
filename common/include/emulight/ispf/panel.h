#ifndef __ISPF_PANEL_H__
#define __ISPF_PANEL_H__

#include <emulight/types.h>
#include <emulight/vt.h>
#include <emulight/os.h>

#define	SPF_MAX_PANELS		8

struct SPF;
struct SPFPanel;

typedef struct SPF SPF;
typedef struct SPFPanel SPFPanel;

typedef void		(*SPFMenuHandler)(SPF* spf, int action);
typedef const char*	(*SPFStatusBarHandler)(SPF* spef);

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

struct SPFPanel {
	const SPFPanelDefinition* proto;
	const char*		title;
	SPF*			spf;
	void*			userdata;
	BOOL			show_cursor;
};

struct SPF {
	void*			userdata;
	SPFPanel		panels[SPF_MAX_PANELS];
	int			current_panel;
	VT			vt;
	SPFMenu			menu;
	BOOL			menu_active;
	SPFStatusBar		status;
	BOOL			error_active;
	BOOL			dirty;
	OSDateTime		clock;
	char			buf[32];
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
	panel->show_cursor = TRUE;
}

static inline void SPFHideCursor(SPFPanel* panel)
{
	panel->show_cursor = FALSE;
}

#endif
