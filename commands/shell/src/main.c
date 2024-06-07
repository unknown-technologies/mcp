#include <emulight/types.h>
#include <emulight/string.h>
#include <emulight/tso.h>

#include "exec.h"

#define	STATE_SPACE		0
#define	STATE_ARG		1
#define	STATE_LPAR		2
#define	STATE_KWARG		3
#define	STATE_RPAR		4

static BOOL screen_line_cont_internal(unsigned int line)
{
	unsigned int lines;
	unsigned int cols;
	char* screen = TGetScreen(&lines, &cols, NULL);

	/* check if the last character is a "+" */
	for(int x = cols - 1; x >= 0; x--) {
		char c = screen[line * cols + x];
		if(c == ' ' || c == 0) {
			continue;
		} else if(c == '+') {
			/* continuation found */
			return TRUE;
		} else {
			return FALSE;
		}
	}

	return FALSE;
}

char* TGetCommand(size_t* len)
{
	unsigned int width;
	unsigned int height;
	char* input_lines;

	char* screen = TGetScreen(&height, &width, &input_lines);

	unsigned int cursor_y;
	unsigned int cursor_x;
	TGetCursor(&cursor_y, &cursor_x);

	unsigned int lines = 1;

	char* command = &screen[cursor_y * width];

	/* check if current line ends with "+" continuation */
	for(unsigned int i = cursor_y; i < height; i++) {
		if(screen_line_cont_internal(i)) {
			lines++;
		} else {
			break;
		}
	}

	if(cursor_y > 0) {
		/* check if previous line ended with a "+" continuation */
		for(int y = cursor_y - 1; y >= 0; y--) {
			if(!input_lines[y]) {
				/* this is not an input line */
				break;
			}

			if(screen_line_cont_internal(y)) {
				lines++;
				command = &screen[y * height];
			} else {
				break;
			}
		}
	}

	*len = lines * width;
	return command;
}


char* TExec(TENV* env, char* cmdbuf)
{
	size_t len = 0;
	char* cmd = TGetCommand(&len);

	TSOArgument* cmd_args = env->argv;

	if(cmd) {
		if(len >= CMD_MAXLEN) {
			TPrint("\nCOMMAND TOO LONG\n");
			return NULL;
		}

		memcpy(cmdbuf, cmd, len);
		cmd = cmdbuf;

		TSetOutput();
		TPutc('\n');

		/* parse arguments */
		unsigned int slot = 0;
		unsigned int argid = 0;
		unsigned int level = 0;
		unsigned int state = STATE_SPACE;
		TSOArgument* arg = NULL;

		bzero(cmd_args, sizeof(cmd_args));

		/* TODO: filter continuation character */
		for(size_t i = 0; i < len; i++) {
			char c = cmd[i];
			switch(state) {
				case STATE_SPACE:
					if(c == '(') {
						TPrint("SYNTAX ERROR\n");
						return NULL;
					} else if(c > 0x20 && c < 0x7F) {
						if(slot >= CMD_MAXARGS) {
							TPrint("TOO MANY ARGUMENTS\n");
							return NULL;
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
						cmd[i] = 0;
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
					}
					break;
				case STATE_RPAR:
					if(c == ' ' || c == 0) {
						state = STATE_SPACE;
					} else {
						TPrint("SYNTAX ERROR\n");
						return NULL;
					}
					break;
			}
		}

		if(state != STATE_SPACE || cmd_args[0].index != 0
				|| cmd_args[0].name != NULL) {
			TPrint("SYNTAX ERROR\n");
			return NULL;
		}

		if(slot > 0) {
			for(char* c = (char*) cmd_args[0].value; *c; c++) {
				*c = toupper(*c);
			}
		}

		env->argc = slot;
		return TLocate(env);
	}

	return NULL;
}
