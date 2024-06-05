#include <emulight/string.h>
#include <emulight/tso.h>

#include "tso.h"

#define	STATE_SPACE		0
#define	STATE_ARG		1
#define	STATE_LPAR		2
#define	STATE_KWARG		3
#define	STATE_RPAR		4

static char cmdbuf[CMD_MAXLEN];

static TSOArgument cmd_args[CMD_MAXARGS];

static TSO tso;

char toupper(char c)
{
	if(c >= 'a' && c <= 'z') {
		return c - 32;
	} else {
		return c;
	}
}

void tso_prompt(TSO* tso)
{
	(void) tso;
	TPrint("READY");
}

int tso_logoff(unsigned int argc, TSOArgument* argv)
{
	(void) argc;

	if(argc != 1) {
		TPrint("SYNTAX ERROR\n");
		return -1;
	}

	TSOExit(&tso);

	return 0;
}

void tso_execute(void)
{
	size_t len = 0;
	char* cmd = TGetCommand(&len);

	if(cmd) {
		if(len >= CMD_MAXLEN) {
			TPrint("\nCOMMAND TOO LONG\n");
			return;
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
						return;
					} else if(c > 0x20 && c < 0x7F) {
						if(slot >= CMD_MAXARGS) {
							TPrint("TOO MANY ARGUMENTS\n");
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
						return;
					}
					break;
			}
		}

		if(state != STATE_SPACE || cmd_args[0].index != 0
				|| cmd_args[0].name != NULL) {
			TPrint("SYNTAX ERROR\n");
			return;
		}

		if(slot > 0) {
			for(char* c = (char*) cmd_args[0].value; *c; c++) {
				*c = toupper(*c);
			}
		}

		tso_execute_command(slot, cmd_args);
	}
}

void tso_enter(TSO* tso)
{
	if(TLineCont()) {
		TPutc('\n');
	} else {
		tso_execute();
		TPrompt(tso);
		TSetInput();
	}
}

void tso_main(void)
{
	TSOInit(&tso);

	TSOSetPrompt(&tso, tso_prompt);
	TSOSetEnter(&tso, tso_enter);

	TSOStart(&tso);

	while(!TSOIsExit(&tso)) {
		TSOProcess(&tso);
	}
}
