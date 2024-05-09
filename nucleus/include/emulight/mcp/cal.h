#ifndef __CAL_H__
#define __CAL_H__

#include <emulight/types.h>

typedef struct {
	u8	day;
	u8	month;
	u16	year;
} OSDate;

void		OSInitCalendar(void);
void		OSiNextDay(void);

BOOL		CALIsLeapYear(unsigned int year);
unsigned int	CALGetDaysOfMonth(unsigned int year, unsigned int month);

#endif
