#include <emulight/types.h>
#include <emulight/mcp/si.h>
#include <emulight/mcp/mon.h>

void SITransmitBlocking8(const char c)
{
	while(!(DL11_XCSR & DL11_XCSR_READY));
	DL11_XBUF = c;
}

void SITransmitBlocking(const void* s, unsigned int len)
{
	const char* p = s;
	while(len--) {
		SITransmitBlocking8(*p++);
	}
}

BOOL SIReceiveByte(u8* c)
{
	if(!(DL11_RCSR & DL11_RCSR_DONE)) {
		return FALSE;
	} else {
		*c = DL11_RBUF;
		return TRUE;
	}
}
