#ifndef __RX02_H__
#define __RX02_H__

#include "types.h"

#define	TOTAL_BLOCKS	988

#define	RX02_SIZE	(77 * 26 * 256)

size_t	get_offset(u16 track, u16 sector);
size_t	get_sector(u32 lsn);
u8*	get_block(u8* out, u8* floppy, u16 block);
void	set_block(u8* floppy, u16 block, u8* data);

void	rad50dec(u8* out, u16 rad50);
u16	rad50enc(const char* in);

#endif
