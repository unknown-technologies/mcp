#include <emulight/types.h>
#include <emulight/string.h>
#include <emulight/vt.h>
#include <emulight/os.h>
#include <emulight/ds.h>
#include <emulight/ispf.h>

#include "ispf.h"

#define	SOFTWARE_VERSION		"r" REVISION "~" COMMIT

#define	UI_ACTION_SYSINFO		0
#define	UI_ACTION_CLOCK			1
#define	UI_ACTION_CLOCKCFG		2
#define	UI_ACTION_PANEL_LIST		3
#define	UI_ACTION_CLOSE_PANEL		4
#define	UI_ACTION_CLOSE_ALL_PANELS	5
#define	UI_ACTION_ABOUT			6
#define	UI_ACTION_VERSION		7
#define	UI_ACTION_HELP			8

static const SPFMenuItem system_menu[] = {
	{ .name = "System Information",	.action = UI_ACTION_SYSINFO },
	{ .name = "Fullscreen Clock",	.action = UI_ACTION_CLOCK },
	{ .name = "-",			.action = 0 },
	{ .name = "Configure Clock",	.action = UI_ACTION_CLOCKCFG },
	{ .name = NULL,			.action = 0 }
};

static const SPFMenuItem panel_menu[] = {
	{ .name = "Panel List",		.action = UI_ACTION_PANEL_LIST },
	{ .name = "Close Panel",	.action = UI_ACTION_CLOSE_PANEL,
					.key = VTGetCTRL('W') },
	{ .name = "Close All Panels",	.action = UI_ACTION_CLOSE_ALL_PANELS },
	{ .name = NULL,			.action = 0 }
};

static const SPFMenuItem help_menu[] = {
	{ .name = "Online Help",	.action = UI_ACTION_HELP },
	{ .name = "-",			.action = 0 },
	{ .name = "Software Version",	.action = UI_ACTION_VERSION },
	{ .name = "About...",		.action = UI_ACTION_ABOUT },
	{ .name = NULL,			.action = 0 }
};

static const SPFMenuData menus[] = {
	{ .name = "SYSTEM",		.items = system_menu },
	{ .name = "PANEL",		.items = panel_menu },
	{ .name = "HELP",		.items = help_menu },
	{ .name = NULL,			.items = NULL }
};

static SPF ispf;

void ISPFLoadProgram(SPF* spf, const char* dsn)
{
	void* program = OSLoadProgram(dsn);
	if(!program) {
		SPFShowError(spf, "Failed to load program");
		return;
	}

	SPFPanelDefinition* def = (SPFPanelDefinition*) program;
	SPFPanel* panel = SPFCreatePanel(spf, def);
	if(panel) {
		SPFSetPanelProgram(panel, program);
	} else {
		OSUnloadProgram(program);
	}
}

/******************************************************************************/
void ISPFMenuHandler(SPF* spf, int action)
{
	switch(action) {
		case UI_ACTION_SYSINFO: {
			size_t mem = MEMAvailable();
			char buf[24];
			sprintf(buf, "Free: %u bytes", mem);
			SPFShowMessage(spf, "Memory", buf);
			break;
		}
		case UI_ACTION_CLOCK:
			ISPFLoadProgram(spf, "SYS1.PROG.CLOCK");
			break;
		case UI_ACTION_CLOCKCFG:
			ISPFLoadProgram(spf, "SYS1.PROG.CLOCKCFG");
			break;
		case UI_ACTION_CLOSE_PANEL:
			if(spf->current_panel > 0) {
				SPFDestroyPanel(SPFCurrentPanel(spf));
				SPFCleanupPanels(spf);
			}
			break;
		case UI_ACTION_CLOSE_ALL_PANELS:
			while(spf->current_panel > 0) {
				SPFDestroyPanel(SPFCurrentPanel(spf));
				SPFCleanupPanels(spf);
			}
			break;
		case UI_ACTION_VERSION:
			SPFShowMessage(spf, "Software Version",
					"This is software version "
					SOFTWARE_VERSION);
			break;
		default:
			SPFShowError(spf, "Function not implemented");
			break;
	}
}

void ISPFDeleteHandler(SPF* spf, void* program)
{
	if(program) {
		OSUnloadProgram(program);
	}
}

static char statusbar_buf[4];
static u8 statusbar_ram = 0;

const char* ISPFStatusDrawHandler(SPF* spf)
{
	(void) spf;
	size_t size = MEMAvailable();
	statusbar_ram = (size + 512) / 1024;
	statusbar_buf[0] = statusbar_ram / 10 + '0';
	statusbar_buf[1] = statusbar_ram % 10 + '0';
	statusbar_buf[2] = 'K';
	statusbar_buf[3] = 0;
	return statusbar_buf;
}

const char* ISPFStatusUpdateHandler(SPF* spf)
{
	(void) spf;
	size_t size = MEMAvailable();
	u8 ram = (size + 512) / 1024;
	if(statusbar_ram != ram) {
		return ISPFStatusDrawHandler(spf);
	} else {
		return NULL;
	}
}

/******************************************************************************/
void ISPFInit(void)
{
	SPFInit(&ispf);
	SPFSetMenuBar(&ispf, menus, ISPFMenuHandler);
	SPFSetDeleteHandler(&ispf, ISPFDeleteHandler);
	ISPFInitRoot(&ispf);
	SPFSetStatusBarDrawCallback(&ispf, ISPFStatusDrawHandler);
	SPFSetStatusBarUpdateCallback(&ispf, ISPFStatusUpdateHandler);
}

void ISPFProcess(void)
{
	SPFProcess(&ispf);
}
