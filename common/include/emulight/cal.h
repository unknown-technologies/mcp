#ifndef __CAL_H__
#define __CAL_H__

#include <emulight/types.h>

BOOL		CALIsLeapYear(unsigned int year);
unsigned int	CALGetDaysOfMonth(unsigned int year, unsigned int month);
int		CALGetWeekday(int year, int month, int day);

#endif
