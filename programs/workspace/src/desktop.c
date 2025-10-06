#include <emulight/types.h>
#include <emulight/ispf.h>
#include <emulight/os.h>
#include <emulight/cal.h>
#include <emulight/string.h>

#include "workspace.h"

#define	CAL_Y		6
#define	INPUT_MAX	(80 - 14)

static const char* month_names[12] = {
	"January", "February", "March", "April", "May", "June", "July",
	"August", "September", "October", "November", "December"
};

const MENU main_menu[] = {
	{
		.name = "Settings",
		.description = "Terminal and user parameters",
		.program = NULL
	}, {
		.name = "View",
		.description = "Display source data or listings",
		.program = NULL,
	}, {
		.name = "Edit",
		.description = "Create or change source data",
		.program = NULL,
	}, {
		.name = "Utilities",
		.description = "Perform utility functions",
		.program = NULL,
	}, {
		.name = "Notes",
		.description = "Process notes and messages",
		.program = NULL,
	}, {
		.name = "Calculator",
		.description = "Perform floating point processing",
		.program = "SYS1.ISPF.CALC"
	}, {
		.name = "Clock",
		.description = "Full screen clock",
		.program = "SYS1.ISPF.CLOCK"
	}, {
		.name = NULL,
		.description = NULL,
		.program = NULL
	}
};

int main_menu_max;

static char input_buf[INPUT_MAX + 1];
static int input_pos;
static int input_len;

void WSInit(SPFPanel* self)
{
	SPFShowCursor(self);
	SPFSetPanelTitle(self, "Workplace");

	for(unsigned int i = 0; main_menu[i].name; i++) {
		main_menu_max = i;
	}

	bzero(input_buf, sizeof(input_buf));
	input_pos = 0;
	input_len = 0;
}

void WSDraw(SPFPanel* self)
{
	u16 width = VTGetWidth(&self->spf->vt);
	u16 height = VTGetHeight(&self->spf->vt);

	/* title line */
	const char* title = "MCP Primary Option Menu";
	u16 center = (width - strlen(title)) / 2;
	VTGoto(3, center + 1);
	VTPrint(title);

	/* status line */
	VTGoto(height - 1, 2);
	VTPrint("F10=Menu   CTRL-L=Refresh");

	/* main menu */
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

	/* termination hint */
	VTGoto(21, 7);
	VTPrint("Enter " CSI "1mX" CSI "0m to Terminate using defaults");

	/* calendar widget */
	u16 x = width - 22;
	OSDateTime now;
	OSGetDateTime(&now);

	VTGoto(CAL_Y, x);
	VTPutc('<');
	VTGoto(CAL_Y, x + 6);
	VTPrint("Calendar");
	VTGoto(CAL_Y, x + 19);
	VTPutc('>');

	VTGoto(CAL_Y + 1, x + 3);
	VTPrint(month_names[now.date.month - 1]);
	VTGoto(CAL_Y + 1, x + 13);
	VTPrintf("%04d", now.date.year);

	VTGoto(CAL_Y + 2, x);
	VTPrint("Mo Tu We Th Fr Sa Su");

	int wday = CALGetWeekday(now.date.year, now.date.month, 1);
	unsigned int days = CALGetDaysOfMonth(now.date.year,
			now.date.month);

	unsigned int day_of_year = now.date.day;
	for(unsigned int i = 1; i < now.date.month; i++) {
		day_of_year += CALGetDaysOfMonth(now.date.year, i);
	}

	int pos_x = x + 3 * wday;
	int pos_y = CAL_Y + 3;

	unsigned int day = wday;
	for(unsigned int i = 0; i < days; i++) {
		BOOL highlight = i == now.date.day - 1;

		VTGoto(pos_y, pos_x);
		if(highlight) {
			VTWrite(CSI "0;7m", CSILEN + 4);
		}
		VTPrintf("%2d", i + 1);
		if(highlight) {
			VTWrite(CSI "0m", CSILEN + 2);
		}

		if(day < 6) {
			day++;
			pos_x += 3;
		} else {
			pos_x = x;
			pos_y++;
			day = 0;
		}
	}

	pos_y += 2;
	VTGoto(pos_y, x);
	VTPrint("Time . . . . : ");
	VTPrintf("%02d:%02d", now.time.hour, now.time.minute);

	pos_y++;
	VTGoto(pos_y, x);
	VTPrint("Day of year. :   ");
	VTPrintf("%3d", day_of_year);

	/* output option input line */
	VTGoto(4, 14);
	VTSGR(4);
	VTWrite(input_buf, input_len);
	VTSGR(0);

	WSSetCursor(self);
}

void WSSetCursor(SPFPanel* self)
{
	VTGoto(4, 14 + input_pos);
}

void WSProcess(SPFPanel* self, u16 key)
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

				if(WSExecuteCommand(self, input_buf)) {
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
			WSSetCursor(self);
			break;
		case VTGetCTRL('E'):
			input_pos = input_len;
			WSSetCursor(self);
			break;
		case VTGetCTRL('U'):
			input_pos = 0;
			input_len = 0;
			SPFRepaint(self->spf);
			break;
	}
}

void WSDestroy(SPFPanel* self)
{
	(void) self;
	/* nothing for now */
}
