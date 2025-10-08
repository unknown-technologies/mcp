#include <emulight/types.h>
#include <emulight/string.h>
#include <emulight/os.h>
#include <emulight/ispf.h>

#include "clock.h"

static Clock clock;
static char buf[12];

/**************************************************************************/
/* Fullscreen clock                                                       */
/**************************************************************************/

/* width = 6, height = 7; digits 0-9 + ':' */
static const u8 clock_font[77] = { 0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E,
	0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E, 0x0E, 0x11, 0x01, 0x02, 0x04,
	0x08, 0x1F, 0x1F, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0E, 0x02, 0x06, 0x0A,
	0x12, 0x1F, 0x02, 0x02, 0x1F, 0x10, 0x10, 0x1E, 0x01, 0x01, 0x1E, 0x0E,
	0x10, 0x10, 0x1E, 0x11, 0x11, 0x0E, 0x1F, 0x01, 0x01, 0x02, 0x04, 0x04,
	0x04, 0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E, 0x0E, 0x11, 0x11, 0x0F,
	0x01, 0x01, 0x0E, 0x00, 0x0C, 0x0C, 0x00, 0x0C, 0x0C, 0x00 };

static void CLKDrawChar(int x, int y, int c)
{
	/* draw character */
	BOOL last = FALSE;
	for(int line = 0; line < 7; line++) {
		VTGoto(y + line, x);
		for(int bit = 0, test = 1 << 5; bit < 6; bit++, test >>= 1) {
			BOOL current = clock_font[c * 7 + line] & test;
			if(current && !last) {
				VTWrite(CSI "7m " , CSILEN + 3);
				last = TRUE;
			} else if(!current && last) {
				VTWrite(CSI "0m " , CSILEN + 3);
				last = FALSE;
			} else {
				VTPutc(' ');
			}
		}
	}
	if(last) {
		VTWrite(CSI "0m" , CSILEN + 2);
	}
}

void CLKInit(SPFPanel* self)
{
	SPFSetPanelTitle(self, "Clock");
	SPFHideActions(self);
	OSGetTime(&clock.time);
	bzero(clock.timebuf, 12);
}

BOOL CLKUpdate(SPFPanel* self)
{
	OSTime* time = &clock.time;

	u16 width = VTGetWidth(&self->spf->vt);
	u16 height = VTGetHeight(&self->spf->vt);

	u16 startX = (width - (6 * 8)) / 2 + 1;
	u16 startY = (height - 7) / 2 + 1;

	BOOL updated = FALSE;

	sprintf(buf, "%02d:%02d:%02d", time->hour, time->minute, time->second);

	for(int i = 0; i < 8; i++) {
		if(buf[i] != clock.timebuf[i]) {
			updated = TRUE;
			int c = buf[i] - '0';
			if(buf[i] == ':') {
				c = 10;
			}

			CLKDrawChar(startX + i * 6, startY, c);
		}
	}

	memcpy(clock.timebuf, buf, sizeof(buf));

	return updated;
}

void CLKDraw(SPFPanel* self)
{
	bzero(clock.timebuf, 12);
	CLKUpdate(self);
}

void CLKProcess(SPFPanel* self, u16 key)
{
	OSTime* clk = &clock.time;
	OSTime now;

	OSGetTime(&now);

	if(now.hour != clk->hour || now.minute != clk->minute
			|| now.second != clk->second) {
		clk->hour = now.hour;
		clk->minute = now.minute;
		clk->second = now.second;
	}

	switch(key) {
		case VT_KEY_ENTER:
			SPFDestroyPanel(self);
			break;
		case VTGetCTRL('L'):
			bzero(clock.timebuf, 12);
			break;
	}
}
