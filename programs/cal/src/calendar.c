#include <emulight/ispf.h>
#include <emulight/cal.h>
#include <emulight/string.h>

#include "calendar.h"

static void CALAction(SPF* spf, const SPFAction* action);

static const char* month_names[12] = {
	"January", "February", "March", "April", "May", "June", "July",
	"August", "September", "October", "November", "December"
};

static const SPFAction actions[] = {
	{ .key = VT_KEY_F11, .name = "Previous Month", .action = CALAction },
	{ .key = VT_KEY_F12, .name = "Next Month",     .action = CALAction },
	{ .key = 0, .name = NULL, .action = NULL }
};

static int cal_offset;

void CALInit(SPFPanel* self)
{
	SPFSetPanelTitle(self, "Calendar");
	SPFSetActions(self, actions);

	cal_offset = 0;
}

static void CALAction(SPF* spf, const SPFAction* action)
{
	switch(action->key) {
		case VT_KEY_F11:
			cal_offset--;
			SPFRepaint(spf);
			break;
		case VT_KEY_F12:
			cal_offset++;
			SPFRepaint(spf);
			break;
	}
}

void CALDrawCalendar(SPFPanel* self, OSDate* date, u16 x, u16 y)
{
	const char* name = month_names[date->month - 1];
	int len = strlen(name) + 5;
	int cx = (19 - len) / 2;

	VTGoto(y, x + cx);
	VTPrintf(CSI "0;1m%s %04d" CSI "0m", name, date->year);

	VTGoto(y + 1, x);
	VTPrint("Mo Tu We Th Fr Sa Su");

	int wday = CALGetWeekday(date->year, date->month, 1);
	unsigned int days = CALGetDaysOfMonth(date->year, date->month);

	int pos_x = x + 3 * wday;
	int pos_y = y + 2;

	unsigned int day = wday;
	for(unsigned int i = 0; i < days; i++) {
		BOOL highlight = i == date->day - 1;

		VTGoto(pos_y, pos_x);
		if(highlight) {
			VTWrite(CSI "0;1;7m", CSILEN + 6);
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
}

void CALDraw(SPFPanel* self)
{
	u16 width = VTGetWidth(&self->spf->vt);

	OSDate now;
	OSGetDate(&now);

	/* title line */
	const char* title = "Six Month Calendar";
	u16 center = (width - strlen(title)) / 2;
	VTGoto(3, center + 1);
	VTPrint(title);

	for(int i = 0; i < 6; i++) {
		int x = 7 + (i % 3) * 25;
		int y = 5 + (i / 3) * 9;

		OSDate date;
		memcpy(&date, &now, sizeof(OSDate));

		int month = (int) date.month + i - 1 + cal_offset;
		while(month > 12) {
			month -= 12;
			date.year++;
		}
		while(month < 1) {
			date.year--;
			month += 12;
		}
		date.month = (u8) month;

		if(date.month != now.month || date.year != now.year) {
			date.day = -1;
		}

		CALDrawCalendar(self, &date, x, y);
	}
}

void CALProcess(SPFPanel* self, u16 key)
{
}
