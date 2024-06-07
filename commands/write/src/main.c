#include <emulight/types.h>
#include <emulight/tso.h>

int TMain(unsigned int argc, TSOArgument* argv)
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
