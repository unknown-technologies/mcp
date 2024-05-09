#include <emulight/types.h>
#include <emulight/mcp/mon.h>
#include <emulight/mcp/cal.h>

extern void** __nextday;

static OSDate date = {
	.day = 1,
	.month = 1,
	.year = 2000
};

void OSInitCalendar(void)
{
	*__nextday = OSiNextDay;
}

void OSiNextDay(void)
{
	unsigned int days = CALGetDaysOfMonth(date.year, date.month);
	if(date.month >= days) {
		date.day = 1;
		date.month++;
		if(date.month >= 12) {
			date.month = 1;
			date.year++;
		}
	}
}

void OSGetDate(OSDate* now)
{
	*now = date;
}

void OSSetDate(OSDate* now)
{
	date = *now;
}

BOOL CALIsLeapYear(unsigned int year)
{
	if((year % 400) == 0) {
		return TRUE;
	} else if((year % 100) == 0) {
		return FALSE;
	} else {
		return (year % 4) == 0;
	}
}

unsigned int CALGetDaysOfMonth(unsigned int year, unsigned int month)
{
	switch(month) {
		case 1: /* JAN */
			return 31;
		case 2: /* FEB */
			return 28 + (CALIsLeapYear(year) ? 1 : 0);
		case 3: /* MAR */
			return 31;
		case 4: /* APR */
			return 30;
		case 5: /* MAY */
			return 31;
		case 6: /* JUN */
			return 30;
		case 7: /* JUL */
			return 31;
		case 8: /* AUG */
			return 31;
		case 9: /* SEP */
			return 30;
		case 10: /* OCT */
			return 31;
		case 11: /* NOV */
			return 30;
		case 12: /* DEC */
			return 31;
		default:
			return 30;
	}
}
