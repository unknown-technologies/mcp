#ifndef __SI_H__
#define __SI_H__

#include <emulight/types.h>
#include <emulight/mcp/mon.h>

#define	DL11_RCSR	*((volatile unsigned int*)  0177560)
#define	DL11_RCSR_DONE	0x80
#define	DL11_RBUF	*((volatile unsigned char*) 0177562)
#define	DL11_XCSR	*((volatile unsigned int*)  0177564)
#define	DL11_XCSR_READY	0x80
#define	DL11_XBUF	*((volatile unsigned char*) 0177566)

void	SITransmitBlocking(const void* s, unsigned int len);
void	SITransmitBlocking8(const char c);

BOOL	SIReceiveByte(u8* c);

static inline void SITransmit(const void* s, unsigned int len)
{
	MONTTO(s, len);
}

#endif
