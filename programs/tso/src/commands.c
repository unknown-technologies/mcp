#include <emulight/string.h>
#include <emulight/os.h>

#include "tso.h"
#include "screen.h"

#define	SOFTWARE_VERSION	"r" REVISION "~" COMMIT

typedef struct {
	const char*	name;
	int		(*main)(unsigned int argc, TSOArgument* argv);
} TSOCommand;

static TSOCommand tso_commands[] = {
	{ .name = "LOGOFF",	.main = tso_logoff },
	{ .name = "SHOWMEM",	.main = tso_showmem },
	{ .name = "SHOWVER",	.main = tso_showver },
	{ .name = "SHOWLOG",	.main = tso_showlog },
	{ .name = "WRITE",	.main = tso_write },
	{ .name = NULL,		.main = NULL }
};

static int last_status = 0;

BOOL startswith(const char* a, const char* b)
{
	while(*a == *b) {
		if(!*a) {
			return TRUE;
		}
		a++;
		b++;
	}

	return !*b;
}

void tso_execute_command(unsigned int argc, TSOArgument* argv)
{
	if(argc == 0) {
		return;
	}

	const char* name = argv->value;

	for(TSOCommand* cmd = tso_commands; cmd->name; cmd++) {
		if(!strcmp(cmd->name, name)) {
			last_status = cmd->main(argc, argv);
			return;
		}
	}

	/* no command found, but now check if it's unique */
	TSOCommand* found = NULL;
	for(TSOCommand* cmd = tso_commands; cmd->name; cmd++) {
		if(startswith(cmd->name, name)) {
			if(found) {
				last_status = -1;
				screen_print("AMBIGUOUS COMMAND\n");
				return;
			}
			found = cmd;
		}
	}

	if(found) {
		last_status = found->main(argc, argv);
		return;
	}

	last_status = -1;
	screen_print("NOT FOUND\n");
}

int tso_showmem(unsigned int argc, TSOArgument* argv)
{
	(void) argv;

	if(argc != 1) {
		screen_print("SYNTAX ERROR\n");
		return -1;
	}

	char buf[24];
	sprintf(buf, "MEM=%u\n", MEMAvailable());
	screen_print(buf);

	return 0;
}

int tso_showver(unsigned int argc, TSOArgument* argv)
{
	(void) argv;

	if(argc != 1) {
		screen_print("SYNTAX ERROR\n");
		return -1;
	}

	screen_print("MCP VERSION " SOFTWARE_VERSION "\n");

	return 0;
}

int tso_write(unsigned int argc, TSOArgument* argv)
{
	BOOL space = FALSE;
	for(unsigned int i = 1; i < argc; i++) {
		if(space) {
			screen_putc(' ');
		} else {
			space = TRUE;
		}
		if(argv[i].name != NULL) {
			screen_print(argv[i].name);
			screen_print("(");
			screen_print(argv[i].value);
			screen_print(")");
		} else {
			screen_print(argv[i].value);
		}
	}

	screen_nl();

	return 0;
}

static void tso_logmsg(const char* msg, void* user)
{
	(void) user;

	screen_print(msg);
	screen_nl();
}

int tso_showlog(unsigned int argc, TSOArgument* argv)
{
	(void) argv;

	if(argc != 1) {
		screen_print("SYNTAX ERROR\n");
		return -1;
	}

	OSGetLog(tso_logmsg, NULL);

	return 0;
}
