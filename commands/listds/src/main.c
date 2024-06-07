#include <emulight/types.h>
#include <emulight/string.h>
#include <emulight/ds.h>
#include <emulight/tso.h>

static char buf[16];

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

	return 0;
}
