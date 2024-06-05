#include <emulight/types.h>
#include <emulight/string.h>
#include <emulight/tso.h>

void TSOInit(TSO* tso)
{
	bzero(tso, sizeof(TSO));
	TInit();
}

void TSOStart(TSO* tso)
{
	TPrompt(tso);
}

void TSOExit(TSO* tso)
{
	tso->exit = TRUE;
}

void TSOProcess(TSO* tso)
{
	TProcess(tso);
}
