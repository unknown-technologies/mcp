#include <emulight/types.h>
#include <emulight/string.h>
#include <emulight/ds.h>
#include <emulight/tso.h>

static BOOL callback(const char* dsn, void* user)
{
	(void) user;

	TPrint(dsn);
	TPutc('\n');

	return TRUE;
}

int TMain(unsigned int argc, TSOArgument* argv)
{
	if(argc == 1) {
		FSList(NULL, callback, NULL);
	} else if(argc == 2) {
		char* prefix = (char*) argv[1].value; /* evil, but anyway */
		for(char* c = prefix; *c; c++) {
			*c = toupper(*c);
			if(*c == '*') {
				*c = 0;
				break;
			}
		}
		FSList(prefix, callback, NULL);
	} else {
		TPrint("SYNTAX ERROR\n");
		return -1;
	}

	return 0;
}
