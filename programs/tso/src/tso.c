#include <emulight/string.h>
#include <emulight/os.h>
#include <emulight/tso.h>

#include "tso.h"

static char cmdbuf[CMD_MAXLEN];

static TSOArgument cmd_args[CMD_MAXARGS];

static TSO tso;

static char dsnbuf[32];

static TENV tso_env;

BOOL TLineCont(unsigned int line, unsigned int col)
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

void tso_prompt(TSO* tso)
{
	(void) tso;
	TPrint("READY");
}

void tso_execute(void)
{
	int result = -1;
	void* prog = OSLoadProgram("SYS1.TSO.EXEC");
	if(prog) {
		char* (*exec)(TENV*, char*) = (char* (*)(TENV*, char*)) prog;
		char* dsn = exec(&tso_env, cmdbuf);
		if(dsn) {
			bzero(dsnbuf, sizeof(dsnbuf));
			strcpy(dsnbuf, dsn);
		}
		OSUnloadProgram(prog);

		if(dsn) {
			void* cmdprog = OSLoadProgram(dsnbuf);
			if(cmdprog) {
				TSetOutput();
				int (*main)(TENV*) = (int (*)(TENV*)) cmdprog;
				result = main(&tso_env);
				OSUnloadProgram(cmdprog);
			}
		}
	}

	/* TODO: store status code somewhere */
	(void) result;
}

void tso_enter(TSO* tso, unsigned int line, unsigned int col)
{
	if(TLineCont(line, col)) {
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

	tso_env.tso = &tso;
	tso_env.argv = cmd_args;

	TSOStart(&tso);

	while(!TSOIsExit(&tso)) {
		TSOProcess(&tso);
	}
}
