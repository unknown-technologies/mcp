#include <emulight/types.h>
#include <emulight/ispf.h>
#include <emulight/os.h>
#include <emulight/string.h>

#include "system.h"
#include "clockcfg.h"

#define	INPUT_MAX	(80 - 14)

static const SPFPanelDefinition clockcfg_panel = {
	.init = CFGInit,
	.draw = CFGDraw,
	.update = NULL,
	.process = CFGProcess,
	.destroy = NULL
};

const MENU main_menu[] = {
	{
		.name = "System Log",
		.description = "View MCP system log",
		.panel = &syslog_panel
	}, {
		.name = "Date/Time",
		.description = "Configure date and time",
		.panel = &clockcfg_panel
	}, {
		.name = NULL,
		.description = NULL,
		.panel = NULL
	}
};

int main_menu_max;

static char input_buf[INPUT_MAX + 1];
static int input_pos;
static int input_len;

void SYSInit(SPFPanel* self)
{
	SPFShowCursor(self);
	SPFSetPanelTitle(self, "System");

	bzero(input_buf, sizeof(input_buf));
	input_pos = 0;
	input_len = 0;

	for(unsigned int i = 0; main_menu[i].name; i++) {
		main_menu_max = i;
	}
}

void SYSDraw(SPFPanel* self)
{
	u16 width = VTGetWidth(&self->spf->vt);
	u16 height = VTGetHeight(&self->spf->vt);

	/* title line */
	const char* title = "System Management";
	u16 center = (width - strlen(title)) / 2;
	VTGoto(3, center + 1);
	VTPrint(title);

	/* status line */
	VTGoto(height - 1, 2);
	VTPrint("F10=Menu   CTRL-L=Refresh   CTRL-W=Back");

	/* system info widget */
	size_t ram_free = MEMAvailable();
	size_t ram_self = 0;
	/* this is a hack with knowledge about how ISPF and the NUCLEUS work
	 * internally */
	void* program = self->program;
	if(program) {
		u16* ptr = (u16*) program;
		ram_self = ptr[-2];
	}
	size_t ram_ispf = ram_free + ram_self;
	u16 x = width - 24;
	VTGoto(6, x);
	VTPrintf("User RAM . : " CSI "1m%5u Bytes" CSI "0m",
			ram_ispf);
	VTGoto(7, x);
	VTPrintf("Avail RAM. : " CSI "1m%5u Bytes" CSI "0m",
			ram_free);
	VTGoto(8, x);
	VTPrint("Release. . : " CSI "1mr" REVISION CSI "0m");
	VTGoto(9, x);
	VTPrint("Commit . . : " CSI "1m" COMMIT CSI "0m");
	VTGoto(10, x);
	VTPrint("Build Date : " CSI "1m" BUILDDATE CSI "0m");
	VTGoto(11, x);
	VTPrint("Build Time : " CSI "1m" BUILDTIME CSI "0m");

	OSDateTime now;
	OSGetDateTime(&now);

	VTGoto(11, x);
	VTPrintf("Date . . . : " CSI "1m%02d.%02d.%04d" CSI "0m",
			now.date.day, now.date.month, now.date.year);
	VTGoto(12, x);
	VTPrintf("Time . . . : " CSI "1m%02d:%02d" CSI "0m",
			now.time.hour, now.time.minute);

	/* content */
	VTGoto(4, 2);
	VTPrint("Option ===> ");
	VTSGR(4);
	for(unsigned int i = 14; i < width; i++) {
		VTPutc(' ');
	}
	VTSGR(0);

	for(unsigned int i = 0; main_menu[i].name; i++) {
		const MENU* menu = &main_menu[i];
		VTGoto(6 + i, 2);
		VTSGR(1);
		VTPrintf("%2d %s", i, menu->name);
		VTSGR(0);
		VTGoto(6 + i, 20);
		VTPrint(menu->description);
	}

	/* output option input line */
	VTGoto(4, 14);
	VTSGR(4);
	VTWrite(input_buf, input_len);
	VTSGR(0);

	SYSSetCursor(self);
}

void SYSSetCursor(SPFPanel* self)
{
	VTGoto(4, 14 + input_pos);
}

void SYSProcess(SPFPanel* self, u16 key)
{
	if(key >= 0x20 && key <= 0x7E) {
		/* text input */
		if(input_pos < INPUT_MAX - 1) {
			for(int i = input_len; i > input_pos; i--) {
				input_buf[i] = input_buf[i - 1];
			}

			input_buf[input_pos++] = (char) key;
			input_len++;

			VTPrint(CSI "@"		/* ICH */
				CSI "4m");	/* underline */

			VTPutc((char) key);

			VTPrint(CSI "0m"	/* reset SGR */
				ESC "7"		/* store cursor */
				CSI "4;80H"	/* end of line */
				CSI "X"		/* clear cell */
				ESC "8");	/* restore cursor */
		}
	}

	switch(key) {
		case VT_KEY_ENTER:
			if(input_len > 0) {
				input_buf[input_len] = 0;

				if(SYSExecuteCommand(self, input_buf)) {
					bzero(input_buf, sizeof(input_buf));
					input_len = 0;
					input_pos = 0;
				}
			}
			break;
		case VT_KEY_LEFT:
			if(input_pos > 0) {
				input_pos--;
				VTCUB(1);
			}
			break;
		case VT_KEY_RIGHT:
			if(input_pos < input_len) {
				input_pos++;
				VTCUF(1);
			}
			break;
		case VT_KEY_BS:
		case VT_KEY_DEL:
			if(input_pos > 0) {
				input_pos--;
				input_len--;

				for(int i = input_pos; i < input_len; i++) {
					input_buf[i] = input_buf[i + 1];
				}

				VTPrint(CSI "D"		/* CUB */
					CSI "1P"	/* DCH */
					ESC "7"		/* store cursor */
					CSI "4;79H"	/* end of line */
					CSI "4m"	/* underline */
					" "		/* fill in SGR */
					ESC "8");	/* restore cursor */
			}
			break;
		case VTGetCTRL('A'):
			input_pos = 0;
			SYSSetCursor(self);
			break;
		case VTGetCTRL('E'):
			input_pos = input_len;
			SYSSetCursor(self);
			break;
		case VTGetCTRL('U'):
			input_pos = 0;
			input_len = 0;
			SPFRepaint(self->spf);
			break;
	}
}

void SYSDestroy(SPFPanel* self)
{
	(void) self;
	/* nothing for now */
}
