#ifndef __R6_H__
#define __R6_H__

#include <emulight/types.h>

/* Encoding packs 4x 6bit text into 3x 8bit binary */
/* len for R6Encode is the character count,
 * for R6Decode it is the byte count */

void	R6Encode(u8* out, const char* in, unsigned int len);
void	R6Decode(char* out, u8* in, unsigned int len);

#endif
