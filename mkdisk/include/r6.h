#ifndef __R6_H__
#define __R6_H__

#include "types.h"

void R6Encode(u8* out, const char* in, unsigned int len);
void R6Decode(char* out, u8* in, unsigned int len);

#endif
