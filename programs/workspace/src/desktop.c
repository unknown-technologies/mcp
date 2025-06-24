#include <emulight/types.h>
#include <emulight/ispf.h>
#include <emulight/os.h>
#include <emulight/cal.h>

#include "workspace.h"

static const char* month_names[12] = {
	"January", "February", "March", "April", "May", "June", "July",
	"August", "September", "October", "November", "December"
};

void WSInit(SPFPanel* self)
{
	SPFHideCursor(self);
	SPFSetPanelTitle(self, "Workplace");
}

void WSDraw(SPFPanel* self)
{
	u16 width = VTGetWidth(&self->spf->vt);
	u16 height = VTGetHeight(&self->spf->vt);

	/* status line */
	VTGoto(height - 1, 2);
	VTPrint("F10=Menu   CTRL-L=Refresh");

	/* calendar widget */
	u16 x = width - 22;
	OSDateTime now;
	OSGetDateTime(&now);

	VTGoto(5, x);
	VTPutc('<');
	VTGoto(5, x + 6);
	VTPrint("Calendar");
	VTGoto(5, x + 19);
	VTPutc('>');

	VTGoto(6, x + 3);
	VTPrint(month_names[now.date.month - 1]);
	VTGoto(6, x + 13);
	VTPrintf("%04d", now.date.year);

	VTGoto(7, x);
	VTPrint("Mo Tu We Th Fr Sa Su");

	int wday = CALGetWeekday(now.date.year, now.date.month, 1);
	unsigned int days = CALGetDaysOfMonth(now.date.year,
			now.date.month);

	unsigned int day_of_year = now.date.day;
	for(unsigned int i = 1; i < now.date.month; i++) {
		day_of_year += CALGetDaysOfMonth(now.date.year, i);
	}

	int pos_x = x + 3 * wday;
	int pos_y = 8;

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
}
