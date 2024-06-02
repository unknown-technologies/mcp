#include <emulight/string.h>
#include <emulight/os.h>
#include <emulight/vt.h>
#include <emulight/ds.h>

#include "logon.h"

#define	SOFTWARE_VERSION	"r" REVISION "~" COMMIT

#define	INPUT_LEN		76

static VT vt;

static char input_buf[INPUT_LEN + 1];
static char command[INPUT_LEN + 1];
static unsigned int input_pos;

const char* logon_ds = NULL;
const char* error_msg = NULL;

static char applid_buf[96];

static void VTDrawText(int line, int column, const char* s)
{
	unsigned int space = 0;
	VTGoto(line++, column);
	for(; *s; s++) {
		if(*s == '\n') {
			space = 0;
			VTGoto(line++, column);
		} else if(*s == ' ') {
			space++;
		} else {
			if(space >= 5) {
				VTCUF(space);
			} else {
				while(space--) {
					VTPutc(' ');
				}
			}
			space = 0;
			VTPutc(*s);
		}
	}
}

static void VTDrawCenter(int line, const char* s)
{
	size_t len = strlen(s);
	unsigned int center = (80 - len) / 2;

	VTGoto(line, center + 1);
	VTPrint(s);
}

void update_time(void)
{
	char buf[16];
	OSDateTime now;

	OSGetDateTime(&now);

	/* print date */
	sprintf(buf, "%02d.%02d.%04d", now.date.day, now.date.month,
			now.date.year);
	VTGoto(2, 2);
	VTPrint(buf);

	/* print time */
	sprintf(buf, "%02d:%02d:%02d", now.time.hour, now.time.minute,
			now.time.second);
	VTGoto(2, 72);
	VTPrint(buf);
}

void draw_mcp(void)
{
	VTShowCursor();

	VTSTBM(1, 24);

	VTClearScreen();

	update_time();

	VTDrawCenter(2, "W E L C O M E  T O");

	VTGoto(5, 37);
	VTPrint("EMULIGHT");

	VTDrawText(7, 18,
		"MMM         MMM     CCCCCC     PPPPPPPPPP\n"
		"MMMM       MMMM   CCCCCCCCCC   PPPPPPPPPPP\n"
		"MMMMM     MMMMM  CCCC     CCC  PPP      PPP\n"
		"MMMMMM   MMMMMM  CCC           PPP      PPP\n"
		"MMM MMM MMM MMM  CCC           PPPPPPPPPPP\n"
		"MMM  MMMMM  MMM  CCC           PPPPPPPPPP\n"
		"MMM   MMM   MMM  CCC           PPP\n"
		"MMM         MMM  CCCC     CCC  PPP\n"
		"MMM         MMM   CCCCCCCCCC   PPP\n"
		"MMM         MMM     CCCCCC     PPP");

	VTGoto(18, 23);
	VTPrint("[?] unknown technologies (c) 2024");

	VTGoto(20, 2);
	VTPrint("===> Enter \"L\" followed by the APPLID you wish to logon to.");
	VTGoto(21, 7);
	VTPrint("Example \"L TSO\" for TSO or \"L ISPF\" for ISPF application.");

	VTGoto(23, 3);
	VTPrint(input_buf);

	VTGoto(23, 3 + input_pos);
}

void handle_bs(void)
{
	if(input_pos > 0) {
		input_buf[--input_pos] = 0;

		VTGoto(23, 3 + input_pos);
		VTPutc(' ');
		VTCUB(1);
	}
}

void handle_input(char c)
{
	if(input_pos < INPUT_LEN) {
		VTGoto(23, 3 + input_pos);
		VTPutc(c);

		input_buf[input_pos++] = c;
	} else {
		VTGoto(23, 3 + input_pos - 1);
		VTPutc(c);

		input_buf[input_pos - 1] = c;
	}
}

char toupper(char c)
{
	if(c >= 'a' && c <= 'z') {
		return c - 32;
	} else {
		return c;
	}
}

BOOL handle_enter(void)
{
	if(input_buf[0] == 0) {
		return FALSE;
	}

	/* convert to upper case */
	for(unsigned int i = 0; i < INPUT_LEN; i++) {
		command[i] = toupper(input_buf[i]);
	}
	command[INPUT_LEN] = 0;

	/* trim space at the end */
	for(int i = INPUT_LEN - 1; i >= 0; i--) {
		if(command[i] == ' ') {
			command[i] = 0;
		} else if(command[i]) {
			break;
		}
	}

	/* parse command */
	if(!strcmp(command, "ISPF")) {
		logon_ds = "SYS1.ISPF.CORE";
		return TRUE;
	}

	if(command[0] == 'L' && command[1] == ' ') {
		char* s = &command[2];
		while(*s == ' ') {
			s++;
		}

		/* open SYS1.APPLID dataset and read it to find the DSN of the
		 * requested application */
		int ds = DSOpen("SYS1.APPLID");
		if(ds < 0) {
			error_msg = "Failed to open APPLID dataset";
			return FALSE;
		}
		DS info;
		DSGetInfo(ds, &info);
		size_t lrecl = DSGetRecLen(&info);

		if(lrecl >= 96) {
			error_msg = "Invalid APPLID dataset";
			DSClose(ds);
			return FALSE;
		}

		while(DSRead(ds, applid_buf, 1) == 1) {
			applid_buf[sizeof(applid_buf) - 1] = 0;

			char* name = applid_buf;
			for(char* c = name; *c; c++) {
				if(*c == ' ') {
					*c = 0;
					break;
				}
			}

			if(*name == 0) {
				/* empty line */
				continue;
			}

			if(!strcmp(s, name)) {
				/* we found the APPLID */
				char* dsn = &applid_buf[8];
				while(*dsn == ' ') {
					dsn++;
				}

				for(char* c = dsn; *c; c++) {
					if(*c == ' ') {
						*c = 0;
						break;
					}
				}

				DSClose(ds);

				logon_ds = dsn;
				return TRUE;
			}
		}

		DSClose(ds);

		error_msg = "Invalid application";
	} else {
		error_msg = "Invalid command";
	}

	return FALSE;
}

void logon(void)
{
	VTInit(&vt);

	bzero(input_buf, INPUT_LEN);
	input_pos = 0;

	draw_mcp();

	while(1) {
		error_msg = NULL;

		u16 key = VTReceive(&vt, NULL, NULL);
		switch(key) {
			case VT_KEY_NONE:
				break;
			case VTGetCTRL('L'):
				draw_mcp();
				break;
			case VTGetCTRL('U'):
				bzero(input_buf, INPUT_LEN);
				input_pos = 0;

				VTGoto(23, 3);
				VTEL2();
				break;
			case VT_KEY_ENTER:
				if(handle_enter()) {
					return;
				} else {
					update_time();
					if(error_msg) {
						VTGoto(24, 2);
						VTEL2();
						VTPrint(error_msg);
					}
					VTGoto(23, 3 + input_pos);
				}
				break;
			case VT_KEY_BS:
			case VT_KEY_DEL:
				handle_bs();
				break;
			case VT_KEY_LEFT:
				if(input_pos > 0) {
					input_pos--;
					VTGoto(23, 3 + input_pos);
				}
				break;
			case VT_KEY_RIGHT:
				if(input_pos < INPUT_LEN) {
					if(!input_buf[input_pos]) {
						input_buf[input_pos] = ' ';
					}
					input_pos++;
					VTGoto(23, 3 + input_pos);
				}
				break;
			case VTGetCTRL('A'):
			case VT_KEY_FIND:
				input_pos = 0;
				VTGoto(23, 3);
				break;
			case VTGetCTRL('E'):
			case VT_KEY_SELECT:
				for(input_pos = 0; input_buf[input_pos]
						&& input_pos < INPUT_LEN;
						input_pos++);
				VTGoto(23, 3 + input_pos);
				break;
			default:
				if(key >= 0x20 && key < 0x7F) {
					handle_input((char) key);
				}
				break;
		}
	}
}
