#include <emulight/rad50.h>
#include <emulight/r4.h>

u16 RAD50Encode(const char* s)
{
	return R4iEncode(s, TRUE);
}

char* RAD50Decode(char* out, u16 enc)
{
	return R4iDecode(out, enc, TRUE);
}

void RAD50EncodeString(u16* out, const char* s, unsigned int len)
{
	R4iEncodeString(out, s, len, TRUE);
}

void RAD50DecodeString(char* out, u16* in, unsigned int len)
{
	R4iDecodeString(out, in, len, TRUE);
}
