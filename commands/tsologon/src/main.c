#include <emulight/types.h>
#include <emulight/string.h>
#include <emulight/ds.h>
#include <emulight/tso.h>

static char recbuf[256];

int TMain(unsigned int argc, TSOArgument* argv)
{
	DS info;
	int ds = DSOpen("SYS1.TSO.WELCOME");
	if(ds < 0) {
		return 0;
	}

	DSGetInfo(ds, &info);
	if(DSIsFB(&info)) {
		unsigned int lrecl = DSGetRecLen(&info);
		if(lrecl >= sizeof(recbuf)) {
			DSClose(ds);
			return 0;
		}

		while(DSRead(ds, recbuf, 1) == 1) {
			if(recbuf[0] != '*') {
				break;
			}

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
	}

	DSClose(ds);

	return 0;
}
