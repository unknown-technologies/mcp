#include <emulight/types.h>
#include <emulight/ispf.h>
#include <emulight/os.h>
#include <emulight/ds.h>
#include <emulight/cal.h>
#include <emulight/string.h>

#include "workspace.h"

#define	CAL_Y		6
#define	INPUT_MAX	(80 - 14)

static const char* month_names[12] = {
	"January", "February", "March", "April", "May", "June", "July",
	"August", "September", "October", "November", "December"
};

static const char* menu_dsn = "SYS1.ISPF.MENU";

static void* memory = NULL;
MENU* main_menu = NULL;
int main_menu_cnt;

static char input_buf[INPUT_MAX + 1];
static int input_pos;
static int input_len;

static char rec_buf[96];

static int get_len(char* s, int len)
{
	for(int i = len - 1; i > 0; i--) {
		if(s[i - 1] != 0 && s[i - 1] != ' ') {
			s[i] = 0;
			return i;
		}
	}

	return 0;
}

void WSInit(SPFPanel* self)
{
	SPFShowCursor(self);
	SPFSetPanelTitle(self, "Workplace");

	int ds = DSOpen(menu_dsn);
	if(ds < 0) {
		SPFDestroyPanel(self);
		SPFTerminate(self->spf);
		return;
	}

	DS info;
	DSGetInfo(ds, &info);
	size_t lrecl = DSGetRecLen(&info);
	if(lrecl > sizeof(rec_buf)) {
		DSClose(ds);
		SPFDestroyPanel(self);
		SPFTerminate(self->spf);
		return;
	}

	size_t string_size = 0;
	while(DSRead(ds, rec_buf, 1) == 1) {
		rec_buf[sizeof(rec_buf) - 1] = 0;
		if(!rec_buf[0] || rec_buf[0] == ' ') {
			break;
		} else {
			char* dsn = &rec_buf[0];
			char* name = &rec_buf[24];
			char* description = &rec_buf[40];
			string_size += get_len(dsn, 24);
			string_size += get_len(name, 16);
			string_size += get_len(description, lrecl - 40);
			string_size += 3; /* NUL characters */
			main_menu_cnt++;
		}
	}

	/* allocate memory */
	size_t memory_size = string_size + main_menu_cnt * sizeof(MENU);
	memory = MEMAlloc(memory_size);
	if(!memory) {
		DSClose(ds);
		SPFDestroyPanel(self);
		SPFTerminate(self->spf);
		return;
	}

	bzero(memory, memory_size);

	char* str_ptr = memory + main_menu_cnt * sizeof(MENU);
	MENU* menu = (MENU*) memory;
	main_menu = menu;

	/* rewind and load menu data */
	DSSeek(ds, 0);
	while(DSRead(ds, rec_buf, 1) == 1) {
		rec_buf[sizeof(rec_buf) - 1] = 0;
		if(!rec_buf[0] || rec_buf[0] == ' ') {
			break;
		} else {
			char* dsn = &rec_buf[0];
			char* name = &rec_buf[24];
			char* description = &rec_buf[40];
			dsn[23] = 0;
			name[15] = 0;
			size_t dsn_len = get_len(dsn, 24);
			size_t name_len = get_len(name, 16);
			size_t description_len = get_len(description,
					lrecl - 40);

			menu->name = str_ptr;
			memcpy(str_ptr, name, name_len + 1);
			str_ptr += name_len + 1;

			menu->description = str_ptr;
			memcpy(str_ptr, description, description_len + 1);
			str_ptr += description_len + 1;

			menu->program = str_ptr;
			memcpy(str_ptr, dsn, dsn_len + 1);
			str_ptr += dsn_len + 1;

			/* '-' is the "no program associated" magic value */
			if(*dsn == '-') {
				menu->program = NULL;
			}

			menu++;
		}
	}

	DSClose(ds);

	bzero(input_buf, sizeof(input_buf));
	input_pos = 0;
	input_len = 0;
}

void WSDraw(SPFPanel* self)
{
	u16 width = VTGetWidth(&self->spf->vt);

	/* title line */
	const char* title = "MCP Primary Option Menu";
	u16 center = (width - strlen(title)) / 2;
	VTGoto(3, center + 1);
	VTPrint(title);

	/* main menu */
	VTGoto(4, 2);
	VTPrint("Option ===> ");
	VTSGR(4);
	for(unsigned int i = 14; i < width; i++) {
		VTPutc(' ');
	}
	VTSGR(0);

	for(int i = 0; i < main_menu_cnt; i++) {
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
	if(memory) {
		MEMFree(memory);
		memory = NULL;
	}
}
