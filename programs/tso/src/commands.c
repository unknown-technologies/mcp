#include <emulight/string.h>
#include <emulight/os.h>
#include <emulight/ds.h>
#include <emulight/tso.h>

#include "tso.h"

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
	{ .name = "LISTCAT",	.main = tso_listcat },
	{ .name = "LISTDS",	.main = tso_listds },
	{ .name = "PRINTDS",	.main = tso_printds },
	{ .name = NULL,		.main = NULL }
};

static int last_status = 0;

static char recbuf[134];

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
				TPrint("AMBIGUOUS COMMAND\n");
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
	TPrint("NOT FOUND\n");
}

int tso_showmem(unsigned int argc, TSOArgument* argv)
{
	(void) argv;

	if(argc != 1) {
		TPrint("SYNTAX ERROR\n");
		return -1;
	}

	char buf[24];
	sprintf(buf, "MEM=%u\n", MEMAvailable());
	TPrint(buf);

	return 0;
}

int tso_showver(unsigned int argc, TSOArgument* argv)
{
	(void) argv;

	if(argc != 1) {
		TPrint("SYNTAX ERROR\n");
		return -1;
	}

	TPrint("MCP VERSION " SOFTWARE_VERSION "\n");

	return 0;
}

int tso_write(unsigned int argc, TSOArgument* argv)
{
	BOOL space = FALSE;
	for(unsigned int i = 1; i < argc; i++) {
		if(space) {
			TPutc(' ');
		} else {
			space = TRUE;
		}
		if(argv[i].name != NULL) {
			TPrint(argv[i].name);
			TPrint("(");
			TPrint(argv[i].value);
			TPrint(")");
		} else {
			TPrint(argv[i].value);
		}
	}

	TPutc('\n');

	return 0;
}

static void tso_logmsg(const char* msg, void* user)
{
	(void) user;

	TPrint(msg);
	TPutc('\n');
}

int tso_showlog(unsigned int argc, TSOArgument* argv)
{
	(void) argv;

	if(argc != 1) {
		TPrint("SYNTAX ERROR\n");
		return -1;
	}

	OSGetLog(tso_logmsg, NULL);

	return 0;
}

static void tso_listcat_cb(const char* dsn, void* user)
{
	(void) user;

	TPrint(dsn);
	TPutc('\n');
}

int tso_listcat(unsigned int argc, TSOArgument* argv)
{
	if(argc == 1) {
		FSList(NULL, tso_listcat_cb, NULL);
	} else if(argc == 2) {
		char* prefix = (char*) argv[1].value; /* evil, but anyway */
		for(char* c = prefix; *c; c++) {
			*c = toupper(*c);
			if(*c == '*') {
				*c = 0;
				break;
			}
		}
		FSList(prefix, tso_listcat_cb, NULL);
	} else {
		TPrint("SYNTAX ERROR\n");
		return -1;
	}

	return 0;
}

int tso_listds(unsigned int argc, TSOArgument* argv)
{
	char buf[16];
	if(argc == 2) {
		char* dsn = (char*) argv[1].value; /* evil, but anyway */
		for(char* c = dsn; *c; c++) {
			*c = toupper(*c);
		}

		TPrint(dsn);
		TPutc('\n');

		DS info;
		int ds = DSOpen(argv[1].value);
		if(ds < 0) {
			TPrint("FAILED TO OPEN DS\n");
			return 1;
		} else {
			DSGetInfo(ds, &info);
			DSClose(ds);

			TPrint("--RECFM-LRECL-BLKSIZE-DSORG\n");
			TPrint("  ");
			if(DSIsFB(&info)) {
				TPrint("FB    ");
			} else {
				TPrint("U     ");
			}

			size_t len;
			if(DSIsFB(&info)) {
				unsigned int lrecl = DSGetRecLen(&info);
				len = sprintf(buf, "%u", lrecl);
			} else {
				buf[0] = '*';
				buf[1] = '*';
				len = 2;
			}
			for(size_t i = len; i < 6; i++) {
				buf[i] = ' ';
			}
			buf[6] = 0;
			TPrint(buf);

			len = sprintf(buf, "%u ", info.size);
			for(size_t i = len; i < 8; i++) {
				buf[i] = ' ';
			}
			buf[8] = 0;
			TPrint(buf);

			TPrint("PS\n");
			TPrint("--VOLUMES--\n");
			TPrint("  ");

			bzero(buf, sizeof(buf));
			VOLGetLabel(info.volume, buf);
			TPrint(buf);
			TPutc('\n');
		}
	} else {
		TPrint("SYNTAX ERROR\n");
		return -1;
	}

	return 0;
}

int tso_printds(unsigned int argc, TSOArgument* argv)
{
	if(argc == 2) {
		char* dsn = (char*) argv[1].value; /* evil, but anyway */
		for(char* c = dsn; *c; c++) {
			*c = toupper(*c);
		}

		TPrint(dsn);
		TPutc('\n');

		DS info;
		int ds = DSOpen(argv[1].value);
		if(ds < 0) {
			TPrint("FAILED TO OPEN DS\n");
		} else {
			DSGetInfo(ds, &info);
			if(DSIsFB(&info)) {
				char buf[8];
				unsigned int i = 1;
				unsigned int lrecl = DSGetRecLen(&info);
				while(DSRead(ds, recbuf, 1) == 1) {
					sprintf(buf, "%05d ", i++);
					TPrint(buf);
					recbuf[lrecl] = 0;
					TPrint(recbuf);
					TPutc('\n');
				}
				TPrint("*** EOF ***\n");
			} else {
				TPrint("NOT A FB DATA SET\n");
			}
			DSClose(ds);
		}
	} else {
		TPrint("SYNTAX ERROR\n");
		return -1;
	}

	return 0;
}
