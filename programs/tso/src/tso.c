#include <emulight/string.h>
#include <emulight/vt.h>

#include "tso.h"
#include "screen.h"

#define	STATE_SPACE		0
#define	STATE_ARG		1
#define	STATE_LPAR		2
#define	STATE_KWARG		3
#define	STATE_RPAR		4

VT vt;

static BOOL exit_tso = FALSE;
static char cmdbuf[CMD_MAXLEN];

static TSOArgument cmd_args[CMD_MAXARGS];

char toupper(char c)
{
	if(c >= 'a' && c <= 'z') {
		return c - 32;
	} else {
		return c;
	}
}

void tso_prompt(void)
{
	screen_print("READY\n");
}

int tso_logoff(unsigned int argc, TSOArgument* argv)
{
	(void) argc;

	if(argc != 1) {
		screen_print("SYNTAX ERROR\n");
		return -1;
	}

	exit_tso = TRUE;

	return 0;
}

void tso_execute(void)
{
	size_t len = 0;
	char* cmd = screen_get_command(&len);

	if(cmd) {
		if(len >= CMD_MAXLEN) {
			screen_nl();
			screen_print("COMMAND TOO LONG\n");
			tso_prompt();
			return;
		}

		memcpy(cmdbuf, cmd, len);
		cmd = cmdbuf;

		/* parse arguments */
		unsigned int slot = 0;
		unsigned int argid = 0;
		unsigned int level = 0;
		unsigned int state = STATE_SPACE;
		TSOArgument* arg = NULL;

		/* TODO: filter continuation character */
		for(size_t i = 0; i < len; i++) {
			char c = cmd[i];
			switch(state) {
				case STATE_SPACE:
					if(c == '(') {
						screen_nl();
						screen_print("SYNTAX ERROR\n");
						tso_prompt();
						return;
					} else if(c > 0x20 && c < 0x7F) {
						if(slot >= CMD_MAXARGS) {
							screen_nl();
							screen_print("TOO MANY ARGUMENTS\n");
							tso_prompt();
							return;
						}
						arg = &cmd_args[slot++];
						arg->name = NULL;
						arg->value = &cmd[i];
						arg->index = argid;
						state = STATE_ARG;
					}
					/* everything else is a space */
					break;
				case STATE_ARG:
					if(c == ' ' || c == 0) {
						/* space */
						argid++;
						cmd[i] = 0;
						state = STATE_SPACE;
					} else if(c == '(') {
						/* keyword argument */
						arg->name = arg->value;
						arg->value = NULL;
						arg->index = -1;
						state = STATE_LPAR;
						for(char* ch = (char*) arg->name;
								*ch; ch++) {
							*ch = toupper(*ch);
						}
					}
					break;
				case STATE_LPAR:
					if(c == ')') {
						state = STATE_RPAR;
					} else {
						arg->value = &cmd[i];
						state = STATE_KWARG;
					}
					break;
				case STATE_KWARG:
					if(c == ')') {
						if(level == 0) {
							cmd[i] = 0;
							state = STATE_RPAR;
						} else {
							level--;
						}
					} else if(c == '(') {
						level++;
					} else if(c == 0) {
						cmd[i] = ' ';
					} else {
						cmd[i] = toupper(c);
					}
					break;
				case STATE_RPAR:
					if(c == ' ' || c == 0) {
						state = STATE_SPACE;
					} else {
						screen_nl();
						screen_print("SYNTAX ERROR\n");
						tso_prompt();
						return;
					}
					break;
			}
		}

		if(state != STATE_SPACE || cmd_args[0].index != 0
				|| cmd_args[0].name != NULL) {
			screen_nl();
			screen_print("SYNTAX ERROR\n");
			tso_prompt();
			return;
		}

		screen_nl();

		if(slot > 0) {
			for(char* c = (char*) cmd_args[0].value; *c; c++) {
				*c = toupper(*c);
			}
		}

		tso_execute_command(slot, cmd_args);
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
