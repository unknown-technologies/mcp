#include <emulight/types.h>
#include <emulight/string.h>
#include <emulight/ds.h>
#include <emulight/tso.h>

static char buf[8];
static char recbuf[256];

int TMain(unsigned int argc, TSOArgument* argv)
{
	if(argc != 2) {
		TPrint("SYNTAX ERROR\n");
		return -1;
	}

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
		return -1;
	}

	DSGetInfo(ds, &info);
	if(DSIsFB(&info)) {
		unsigned int i = 1;
		unsigned int lrecl = DSGetRecLen(&info);
		if(lrecl >= sizeof(recbuf)) {
			TPrint("LRECL TOO LONG\n");
			DSClose(ds);
			return 0;
		}

		while(DSRead(ds, recbuf, 1) == 1) {
			sprintf(buf, "%05d ", i++);
			TPrint(buf);
			recbuf[lrecl] = 0;
			for(int i = lrecl - 1; i > 0; i--) {
				if(recbuf[i] == ' ') {
					recbuf[i] = 0;
				} else {
					break;
				}
			}
			TPrint(recbuf);
			TPutc('\n');
		}

		TPrint("*** EOF ***\n");
	} else {
		TPrint("NOT A FB DATA SET\n");
	}

	DSClose(ds);

	return 0;
}
