#include <emulight/string.h>
#include <emulight/vt.h>
#include <emulight/os.h>

#include "tso.h"
#include "screen.h"

#define	SOFTWARE_VERSION	"r" REVISION "~" COMMIT
#define	CMD_MAXLEN		(80 * 16)

VT vt;

static BOOL exit_tso = FALSE;
static char cmdbuf[CMD_MAXLEN];

void tso_prompt(void)
{
	screen_print("READY\n");
}

void tso_execute(void)
{
	size_t len = 0;
	char* cmd = screen_get_command(&len);

	if(cmd) {
		if(len >= CMD_MAXLEN) {
			screen_print("\nCOMMAND TOO LONG\n");
			tso_prompt();
			return;
		}

		memcpy(cmdbuf, cmd, len);
		cmd = cmdbuf;

		/* trim leading space */
		while(len > 0 && (*cmd == ' ' || *cmd == 0)) {
			len--;
			cmd++;
		}

		/* find end of command */
		char* end = NULL;
		for(size_t i = 0; i < len; i++) {
			if(cmd[i] == ' ') {
				cmd[i] = 0;
				end = &cmd[i];
				break;
			} else if(!cmd[i]) {
				end = &cmd[i];
				break;
			}
		}

		if(!end) {
			end = &cmdbuf[len];
			*end = 0;
		}

		screen_nl();

		if(*cmd) {
			if(!strcmp(cmd, "logoff")) {
				exit_tso = TRUE;
			} else if(!strcmp(cmd, "showver")) {
				screen_print("MCP VERSION " SOFTWARE_VERSION "\n");
			} else if(!strcmp(cmd, "showmem")) {
				size_t mem = MEMAvailable();
				char buf[24];
				sprintf(buf, "MEM=%u\n", mem);
				screen_print(buf);
			} else {
				screen_print("UNKNOWN COMMAND: ");
				for(size_t i = 0; i < len && cmd[i]; i++) {
					screen_putc(cmd[i]);
				}
				screen_nl();
			}
		}
	} else {
		screen_nl();
	}

	tso_prompt();
}

void tso_enter(void)
{
	if(screen_line_cont()) {
		screen_nl();
	} else {
		tso_execute();
	}
}

void tso(void)
{
	VTInit(&vt);

	VTClearScreen();
	VTHome();
	VTShowCursor();

	tso_prompt();

	while(!exit_tso) {
		u16 key = VTReceive(&vt, NULL, NULL);
		switch(key) {
			case VT_KEY_NONE:
				break;
			case VTGetCTRL('L'):
				screen_repaint();
				break;
			case VT_KEY_F10:
				screen_clear();
				tso_prompt();
				break;
			case VT_KEY_UP:
				screen_cursor_up();
				break;
			case VT_KEY_DOWN:
				screen_cursor_down();
				break;
			case VT_KEY_LEFT:
				screen_cursor_left();
				break;
			case VT_KEY_RIGHT:
				screen_cursor_right();
				break;
			case VTGetCTRL('A'):
			case VT_KEY_FIND:
				screen_cursor_pos1();
				break;
			case VTGetCTRL('E'):
			case VT_KEY_SELECT:
				screen_cursor_end();
				break;
			case VT_KEY_BS:
			case VT_KEY_DEL:
				screen_bs();
				break;
			case VT_KEY_ENTER:
				tso_enter();
				break;
			default:
				if(key >= 0x20 && key < 0x7F) {
					screen_input((char) key);
				}
				break;
		}
	}
}
