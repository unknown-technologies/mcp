#include <emulight/cal.h>

/* compute weekday (0=Monday, 6=Sunday) for day/month/year
 * (day=1..31, month=1..12) */

int CALGetWeekday(int year, int month, int day)
{
	int m = month < 3 ? month + 12 : month;
	int Y = month < 3 ? year - 1 : year;

	int w = day + (13 * (m + 1)) / 5 + Y + Y / 4 - Y / 100 + Y / 400;

	return (w + 5) % 7;
}
