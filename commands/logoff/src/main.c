#include <emulight/tso.h>

int TMain(unsigned int argc, TSOArgument* argv)
{
	(void) argc;

	if(argc != 1) {
		TPrint("SYNTAX ERROR\n");
		return -1;
	}

	TSOExit(env->tso);

	return 0;
}
