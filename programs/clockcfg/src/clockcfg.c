#include <emulight/string.h>
#include <emulight/os.h>
#include <emulight/ispf.h>

#include "clockcfg.h"

/******************************************************************************/
/* Date / Time configuration                                                  */
/******************************************************************************/

#define	UI_CLOCK_OK		0
#define	UI_CLOCK_CANCEL		1

static Config cfg;

void CFGInit(SPFPanel* self)
{
	SPFSetPanelTitle(self, "Clock Configuration");

	/* prepare data */
	OSGetDateTime(&cfg.clock);

	cfg.position = 0;
	cfg.field = 0;
	cfg.action = UI_CLOCK_OK;

	sprintf(cfg.datebuf, "%02d.%02d.%04d", cfg.clock.date.day,
			cfg.clock.date.month, cfg.clock.date.year);
	sprintf(cfg.timebuf, "%02d:%02d:%02d", cfg.clock.time.hour,
			cfg.clock.time.minute, cfg.clock.time.second);

	SPFShowCursor(self);
}

void CFGDraw(SPFPanel* self)
{
	u16 width = VTGetWidth(&self->spf->vt);
	u16 height = VTGetHeight(&self->spf->vt);

	u16 startX = (width - 23) / 2 + 1;
	u16 startY = (height - 7) / 2 + 1;

	VTGoto(startY, startX);
	VTPrint(ESC "(0l " ESC "(BClock Configuration " ESC "(0k");

	VTGoto(startY + 1, startX);
	VTPrint("x" CSI "21Cx");

	VTGoto(startY + 2, startX);
	VTPrintf(ESC "(0x  " ESC "(BDate:  " CSI "%sm%s"
			CSI "0m  " ESC "(0x",
			cfg.field == 0 ? "4;7" : "4", cfg.datebuf);

	VTGoto(startY + 3, startX);
	VTPrintf("x  " ESC "(BTime:  " CSI "%sm%s  "
			CSI "0m  " ESC "(0x",
			cfg.field == 1 ? "4;7" : "4", cfg.timebuf);

	VTGoto(startY + 4, startX);
	VTPrint("x" CSI "21Cx");

	VTGoto(startY + 5, startX);
	VTPrint("x    " ESC "(B");

	if(cfg.action == UI_CLOCK_OK) {
		VTPrint(CSI "7m[OK]" CSI "0m");
	} else {
		VTPrint("[OK]");
	}

	VTPrint(" ");

	if(cfg.action == UI_CLOCK_CANCEL) {
		VTPrint(CSI "7m[CANCEL]" CSI "0m");
	} else {
		VTPrint("[CANCEL]");
	}

	VTPrint("    " ESC "(0x");

	VTGoto(startY + 6, startX);
	VTPrint(ESC "(0mqqqqqqqqqqqqqqqqqqqqqj" ESC "(B");

	if(cfg.field == 0 || cfg.field == 1) {
		VTGoto(startY + 2 + cfg.field, startX + 10 + cfg.position);
		VTPrint(CSI "0;4;7m");
	} else {
		if(cfg.action == UI_CLOCK_OK) {
			VTGoto(startY + 5, startX + 6);
		} else {
			VTGoto(startY + 5, startX + 11);
		}
	}
}

void CFGProcess(SPFPanel* self, u16 key)
{
	u16 width = VTGetWidth(&self->spf->vt);
	u16 height = VTGetHeight(&self->spf->vt);

	u16 startX = (width - 23) / 2 + 1;
	u16 startY = (height - 7) / 2 + 1;

	/* key handling */
	switch(key) {
		case VT_KEY_ENTER:
			if(cfg.action == UI_CLOCK_CANCEL) {
				SPFDestroyPanel(self);
				return;
			}

			/* parse date */
			u8 day = (cfg.datebuf[0] - '0') * 10
				+ (cfg.datebuf[1] - '0');
			u8 month = (cfg.datebuf[3] - '0') * 10
				+ (cfg.datebuf[4] - '0');
			u16 year = (cfg.datebuf[6] - '0') * (u16) 1000
				+ (cfg.datebuf[7] - '0') * (u16) 100
				+ (cfg.datebuf[8] - '0') * (u16) 10
				+ (cfg.datebuf[9] - '0');

			/* parse time */
			u8 hour = (cfg.timebuf[0] - '0') * 10
				+ (cfg.timebuf[1] - '0');
			u8 minute = (cfg.timebuf[3] - '0') * 10
				+ (cfg.timebuf[4] - '0');
			u8 second = (cfg.timebuf[6] - '0') * 10
				+ (cfg.timebuf[7] - '0');

			if(day == 0 || day > 31 || month == 0 || month > 12) {
				/* invalid date */
				cfg.field = 0;
				cfg.position = 0;

				SPFRepaint(self->spf);
				VTGoto(startY + 2, startX + 20);
				VTPrint(CSI "0m!" CSI "0;4;7m");
				VTGoto(startY + 2, startX + 10);

				return;
			}

			if(hour > 23 || minute > 59 || second > 59) {
				/* invalid time */
				cfg.field = 0;
				cfg.position = 0;

				SPFRepaint(self->spf);
				VTGoto(startY + 3, startX + 20);
				VTPrint(CSI "0m!" CSI "0;4;7m");
				VTGoto(startY + 3, startX + 10);

				return;
			}

			/* update RTC */
			OSDateTime dt = {
				.date = {
					.day = day,
					.month = month,
					.year = year
				},
				.time = {
					.hour = hour,
					.minute = minute,
					.second = second
				}
			};
			OSSetDateTime(&dt);

			SPFDestroyPanel(self);
			return;
		case VT_KEY_DOWN:
			if(cfg.field == 0) {
				cfg.field = 1;

				/* unfocus date field */
				VTDecDis(DECTCEM);
				VTGoto(startY + 2, startX + 10);
				VTPrintf(CSI "0;4m%s", cfg.datebuf);

				/* focus time field */
				VTGoto(startY + 3, startX + 10);
				VTPrintf(CSI "0;4;7m%s  ", cfg.timebuf);

				/* limit cursor position */
				if(cfg.position > 7) {
					cfg.position = 7;
				}

				/* set cursor */
				VTGoto(startY + 3, startX + 10 + cfg.position);
				VTDecEn(DECTCEM);
			} else if(cfg.field == 1) {
				cfg.field = 2;

				/* unfocus time field */
				VTDecDis(DECTCEM);
				VTGoto(startY + 3, startX + 10);
				VTPrintf(CSI "0;4m%s  ", cfg.timebuf);

				VTPrint(CSI "0;7m");

				if(cfg.action == UI_CLOCK_OK) {
					VTGoto(startY + 5, startX + 6);
				} else {
					VTGoto(startY + 5, startX + 11);
				}
				VTDecEn(DECTCEM);
			}
			break;
		case VT_KEY_UP:
			if(cfg.field == 1) {
				cfg.field = 0;

				/* unfocus time field */
				VTDecDis(DECTCEM);
				VTGoto(startY + 3, startX + 10);
				VTPrintf(CSI "0;4m%s  ", cfg.timebuf);

				/* focus date field */
				VTGoto(startY + 2, startX + 10);
				VTPrintf(CSI "0;4;7m%s", cfg.datebuf);

				/* set cursor */
				VTGoto(startY + 2, startX + 10 + cfg.position);
				VTDecEn(DECTCEM);
			} else if(cfg.field == 2) {
				cfg.field = 1;

				/* focus time field */
				VTDecDis(DECTCEM);
				VTGoto(startY + 3, startX + 10);
				VTPrintf(CSI "0;4;7m%s  ", cfg.timebuf);

				/* set cursor */
				VTGoto(startY + 3, startX + 10 + cfg.position);
				VTDecEn(DECTCEM);
			}
			break;
		case 0x08:
		case 0x7F:
		case VT_KEY_LEFT:
			if(cfg.field == 0 || cfg.field == 1) {
				if(cfg.position > 0) {
					cfg.position--;

					VTGoto(startY + 2 + cfg.field, startX
							+ 10 + cfg.position);
				}
			} else {
				if(cfg.action == UI_CLOCK_CANCEL) {
					cfg.action = UI_CLOCK_OK;

					/* focus OK, unfocus CANCEL */
					VTDecDis(DECTCEM);
					VTGoto(startY + 5, startX + 5);
					VTPrint(CSI "7m[OK]" CSI "0m [CANCEL]");

					/* focus OK */
					VTGoto(startY + 5, startX + 6);
					VTDecEn(DECTCEM);
				}
			}
			break;
		case VT_KEY_RIGHT:
			if(cfg.field == 0) {
				if(cfg.position < 9) {
					cfg.position++;

					VTGoto(startY + 2 + cfg.field,
							startX + 10
							+ cfg.position);
				}
			} else if(cfg.field == 1) {
				if(cfg.position < 7) {
					cfg.position++;

					VTGoto(startY + 2 + cfg.field,
							startX + 10
							+ cfg.position);
				}
			} else {
				if(cfg.action == UI_CLOCK_OK) {
					cfg.action = UI_CLOCK_CANCEL;

					/* unfocus OK, focus CANCEL */
					VTDecDis(DECTCEM);
					VTGoto(startY + 5, startX + 5);
					VTPrint(CSI "0m[OK] " CSI "7m[CANCEL]"
							CSI "0m");

					/* focus CANCEL */
					VTGoto(startY + 5, startX + 11);
					VTDecEn(DECTCEM);
				}
			}
			break;
		case VT_KEY_TAB:
			if(cfg.field == 0) {
				cfg.field = 1;
				cfg.position = 0;

				/* unfocus date field */
				VTDecDis(DECTCEM);
				VTGoto(startY + 2, startX + 10);
				VTPrintf(CSI "0;4m%s", cfg.datebuf);

				/* focus time field */
				VTGoto(startY + 3, startX + 10);
				VTPrintf(CSI "0;4;7m%s  ", cfg.timebuf);

				/* set cursor */
				VTGoto(startY + 3, startX + 10 + cfg.position);
				VTDecEn(DECTCEM);
			} else if(cfg.field == 1) {
				cfg.field = 2;

				/* unfocus time field */
				VTDecDis(DECTCEM);
				VTGoto(startY + 3, startX + 10);
				VTPrintf(CSI "0;4m%s  ", cfg.timebuf);

				VTPrint(CSI "0;7m");

				if(cfg.action == UI_CLOCK_OK) {
					VTGoto(startY + 5, startX + 6);
				} else {
					VTGoto(startY + 5, startX + 11);
				}
				VTDecEn(DECTCEM);
			} else if(cfg.field == 2) {
				cfg.field = 0;

				/* focus date field */
				VTDecDis(DECTCEM);
				VTGoto(startY + 2, startX + 10);
				VTPrintf(CSI "0;4;7m%s", cfg.datebuf);

				/* set cursor */
				VTGoto(startY + 2, startX + 10 + cfg.position);
				VTDecEn(DECTCEM);
			}
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			if(cfg.field == 0) {
				switch(cfg.position) {
					case 2:
					case 5:
						VTPutc('.');
						cfg.position++;
						/* fallthrough! */
					case 0:
					case 1:
					case 3:
					case 4:
					case 6:
					case 7:
					case 8:
					case 9:
						cfg.datebuf[cfg.position] = key;

						if(cfg.position < 9) {
							cfg.position++;
							VTPutc(key);
						} else {
							VTPutc(key);
							VTPutc(0x08);
						}
						break;
				}
			} else if(cfg.field == 1) {
				switch(cfg.position) {
					case 2:
					case 5:
						VTPutc(':');
						cfg.position++;
						/* fallthrough! */
					case 0:
					case 1:
					case 3:
					case 4:
					case 6:
					case 7:
						cfg.timebuf[cfg.position] = key;

						if(cfg.position < 7) {
							cfg.position++;
							VTPutc(key);
						} else {
							VTPutc(key);
							VTPutc(0x08);
						}
						break;
				}
			}
			break;
		case '.':
			if(cfg.field == 0) {
				switch(cfg.position) {
					case 2:
					case 5:
						VTPutc('.');
						cfg.position++;
						break;
					case 1:
					case 4:
						VTPrint("0.");
						cfg.datebuf[cfg.position] = '0';
						cfg.position += 2;
						break;
					case 0:
					case 3:
						VTPrint("00.");
						cfg.datebuf[cfg.position] = '0';
						cfg.datebuf[cfg.position + 1]
							= '0';
						cfg.position += 3;
						break;
				}
			}
			break;
		case ':':
			if(cfg.field == 1) {
				switch(cfg.position) {
					case 2:
					case 5:
						VTPutc(':');
						cfg.position++;
						break;
					case 1:
					case 4:
						VTPrint("0:");
						cfg.timebuf[cfg.position] = '0';
						cfg.position += 2;
						break;
					case 0:
					case 3:
						VTPrint("00:");
						cfg.timebuf[cfg.position] = '0';
						cfg.timebuf[cfg.position + 1]
							= '0';
						cfg.position += 3;
						break;
				}
			}
			break;
	}
}
