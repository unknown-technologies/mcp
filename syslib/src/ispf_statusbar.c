#include <emulight/types.h>
#include <emulight/string.h>
#include <emulight/vt.h>
#include <emulight/ispf.h>

void SPFInitStatusBar(SPF* spf)
{
	bzero(&spf->clock, sizeof(OSDateTime));
	spf->status.draw = NULL;
	spf->status.update = NULL;
}

void SPFDrawStatusBar(SPF* spf)
{
	unsigned int pos = 1;

	OSDateTime* now = &spf->clock;
	OSGetDateTime(now);

	VTWrite(CSI "24H" CSI "0;7m", 2 * CSILEN + 7);

	while(pos < 64) {
		VTPutc(' ');
		pos++;
	}

	size_t len = sprintf(spf->buf, "%02d.%02d.%04d %02d:%02d ",
			now->date.day, now->date.month, now->date.year,
			now->time.hour, now->time.minute);
	VTWrite(spf->buf, len);

	if(spf->status.draw) {
		const char* msg = spf->status.draw(spf);

		if(msg) {
			VTWrite(CSI "24;2H", CSILEN + 5);
			VTPrint(msg);
		}
	}

	VTWrite(CSI "0m", CSILEN + 2);
}

void SPFUpdateDate(SPF* spf)
{
}

void SPFUpdateTime(SPF* spf)
{
	OSTime now;
	OSGetTime(&now);

	/* did anything change? */
	if(spf->clock.time.hour != now.hour
			|| spf->clock.time.minute != now.minute) {
		size_t len = sprintf(spf->buf, "%02d:%02d", now.hour,
				now.minute);

		VTPrint(ESC "7"		/* store cursor position */
			CSI "?25l"	/* hide cursor */
			CSI "24;75H"	/* go to (75, 24) */
			CSI "0;7m");	/* reverse video */
		VTWrite(spf->buf, len);
		VTPrint(CSI "0m"	/* normal video */
			ESC "8");	/* restore cursor position */

		spf->clock.time = now;
		SPFPanel* panel = SPFCurrentPanel(spf);
		if(panel && panel->show_cursor) {
			VTWrite(CSI "?25h", CSILEN + 4); /* show cursor */
		}
	}
}

void SPFUpdateStatusBar(SPF* spf)
{
	SPFUpdateDate(spf);
	SPFUpdateTime(spf);

	if(spf->status.update) {
		const char* msg = spf->status.update(spf);

		if(msg) {
			VTPrint(ESC "7"		/* store cursor position */
				CSI "?25l"	/* hide cursor */
				CSI "24;2H"	/* go to (2, 24) */
				CSI "0;7m");	/* reverse video */

			VTPrint(msg);

			VTPrint(CSI "0m"	/* normal video */
				ESC "8");	/* restore cursor position */

			SPFPanel* panel = SPFCurrentPanel(spf);
			if(panel && panel->show_cursor) {
				/* show cursor */
				VTWrite(CSI "?25h", CSILEN + 4);
			}
		}
	}
}

void SPFSetStatusBarDrawCallback(SPF* spf, SPFStatusBarHandler handler)
{
	spf->status.draw = handler;
}

void SPFSetStatusBarUpdateCallback(SPF* spf, SPFStatusBarHandler handler)
{
	spf->status.update = handler;
}
