#include <emulight/string.h>
#include <emulight/os.h>
#include <emulight/tso.h>

#define	SOFTWARE_VERSION	"r" REVISION "~" COMMIT

static char buf[32];

int TMain(unsigned int argc, TSOArgument* argv)
{
	(void) argv;

	if(argc != 1) {
		TPrint("SYNTAX ERROR\n");
		return -1;
	}

	TPrint("MCP VERSION " SOFTWARE_VERSION " ");

	sprintf(buf, "MEM=%u\n", MEMAvailable());
	TPrint(buf);

	return 0;
}
