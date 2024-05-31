#ifndef __R4_H__
#define __R4_H__

#include "types.h"

BOOL	R4CheckValid(const char c);
u16	R4Encode(const char* s);
char*	R4Decode(char* out, u16 enc);
void	R4EncodeString(u16* out, const char* s, unsigned int len);
void	R4DecodeString(char* out, u16* in, unsigned int len);

#endif
