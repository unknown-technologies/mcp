#include <emulight/types.h>
#include <emulight/string.h>
#include <emulight/tso.h>
#include <emulight/ds.h>

static const char* helpfile = "SYS1.TSO.HELP";

static char recbuf[256];
static char cmdbuf[16];

#define	TYPE_NONE	0
#define	TYPE_FUNC	1
#define	TYPE_SYNTAX	2

int print_info(void)
{
	DS info;
	int ds = DSOpen(helpfile);
	if(ds < 0) {
		TPrint("FAILED TO OPEN DS\n");
		return -1;
	}

	DSGetInfo(ds, &info);
	if(DSIsFB(&info)) {
		unsigned int lrecl = DSGetRecLen(&info);
		if(lrecl >= sizeof(recbuf)) {
			TPrint("LRECL TOO LONG\n");
			DSClose(ds);
			return 0;
		}

		BOOL info_found = FALSE;

		while(DSRead(ds, recbuf, 1) == 1) {
			recbuf[lrecl] = 0;
			/* trim whitespace at end of line */
			for(int i = lrecl - 1; i > 0; i--) {
				if(recbuf[i] == ' ') {
					recbuf[i] = 0;
				} else {
					break;
				}
			}

			if(!strcmp(recbuf, ".TOC")) {
				info_found = TRUE;
				continue;
			} else if(info_found && !strcmp(recbuf, ".END")) {
				break;
			}

			if(info_found) {
				TPrint(recbuf);
				TPutc('\n');
			}
		}
	} else {
		TPrint("NOT A FB DATA SET\n");
	}

	DSClose(ds);

	return 0;
}

int print_cmd(const char* cmd)
{
	for(unsigned int i = 0; i < 8; i++) {
		cmdbuf[i] = toupper(cmd[i]);
	}
	cmdbuf[8] = 0;

	if(cmdbuf[7] != 0) {
		TPrint("INVALID COMMAND\n");
		return -1;
	}

	DS info;
	int ds = DSOpen(helpfile);
	if(ds < 0) {
		TPrint("FAILED TO OPEN DS\n");
		return -1;
	}

	DSGetInfo(ds, &info);
	if(DSIsFB(&info)) {
		unsigned int lrecl = DSGetRecLen(&info);
		if(lrecl >= sizeof(recbuf)) {
			TPrint("LRECL TOO LONG\n");
			DSClose(ds);
			return 0;
		}

		BOOL cmd_found = FALSE;
		unsigned int type = TYPE_NONE;
		unsigned int last_type = TYPE_NONE;

		while(DSRead(ds, recbuf, 1) == 1) {
			recbuf[lrecl] = 0;
			/* trim whitespace at end of line */
			for(int i = lrecl - 1; i > 0; i--) {
				if(recbuf[i] == ' ') {
					recbuf[i] = 0;
				} else {
					break;
				}
			}

			if(!memcmp(recbuf, ".CMD ", 5)
					&& !strcmp(recbuf + 5, cmdbuf)) {
				cmd_found = TRUE;
				continue;
			} else if(cmd_found) {
				if(!strcmp(recbuf, ".END")) {
					break;
				} else if(!strcmp(recbuf, ".FUNCTION")) {
					type = TYPE_FUNC;
				} else if(!strcmp(recbuf, ".SYNTAX")) {
					type = TYPE_SYNTAX;
				} else if(recbuf[0] == '.') {
					TPrint("SYNTAX ERROR IN HELP FILE\n");
					break;
				}
			}

			/* print headings */
			if(type != last_type) {
				const char* name = NULL;
				switch(type) {
					case TYPE_FUNC:
						name = "FUNCTION";
						break;
					case TYPE_SYNTAX:
						name = "SYNTAX";
						break;
				}

				if(name) {
					TPutc('\n');
					TPrint(name);
					TPrint(" -\n");
				}
			}

			last_type = type;

			if(cmd_found && recbuf[0] != '.' && type != TYPE_NONE) {
				TPrint("  ");
				TPrint(recbuf);
				TPutc('\n');
			}
		}

		if(!cmd_found) {
			TPrint("UNKNOWN COMMAND\n");
		}
	} else {
		TPrint("NOT A FB DATA SET\n");
	}

	DSClose(ds);

	return 0;
}

int TMain(unsigned int argc, TSOArgument* argv)
{
	(void) argc;
	(void) argv;

	if(argc == 1) {
		return print_info();
	} else {
		return print_cmd(argv[1].value);
	}
}
