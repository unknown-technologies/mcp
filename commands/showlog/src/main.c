#include <emulight/string.h>
#include <emulight/os.h>
#include <emulight/tso.h>

static void logmsg(const char* msg, void* user)
{
	(void) user;

	TPrint(msg);
	TPutc('\n');
}

int TMain(unsigned int argc, TSOArgument* argv)
{
	(void) argv;

	if(argc != 1) {
		TPrint("SYNTAX ERROR\n");
		return -1;
	}

	OSGetLog(logmsg, NULL);

	return 0;
}
