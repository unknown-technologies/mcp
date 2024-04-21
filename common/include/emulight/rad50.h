#ifndef __RAD50_H__
#define __RAD50_H__

#include <emulight/types.h>

u16     RAD50Encode(const char* s);
char*   RAD50Decode(char* out, u16 enc);

void    RAD50EncodeString(u16* out, const char* s, unsigned int len);
void    RAD50DecodeString(char* out, u16* in, unsigned int len);

static inline BOOL RAD50CheckValid(const char c)
{
	if(c >= 'A' && c <= 'Z') {
		return TRUE;
	} else if(c >= 'a' && c <= 'z') {
		return TRUE;
	} else if(c >= '0' && c <= '9') {
		return TRUE;
	} else if(c == '$' || c == '#' || c == '@' || c == ' ') {
		return TRUE;
	} else {
		return FALSE;
	}
}

#endif
