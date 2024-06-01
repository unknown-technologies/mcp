#ifndef __R4_H__
#define __R4_H__

#include <emulight/types.h>

u16	R4iEncode(const char* s, BOOL rad50);
char*	R4iDecode(char* out, u16 enc, BOOL rad50);
void	R4iEncodeString(u16* out, const char* s, unsigned int len, BOOL rad50);
void	R4iDecodeString(char* out, u16* in, unsigned int len, BOOL rad50);

u16	R4Encode(const char* s);
char*	R4Decode(char* out, u16 enc);
void	R4EncodeString(u16* out, const char* s, unsigned int len);
void	R4DecodeString(char* out, u16* in, unsigned int len);

#endif
