#ifndef __R7_H__
#define __R7_H__

#include <emulight/types.h>

/* Encoding packs 8x 7bit ASCII into 7x 8bit binary */

void	R7Encode(u8* out, const char* in, unsigned int len);
void	R7Decode(char* out, u8* in, unsigned int len);

#endif
