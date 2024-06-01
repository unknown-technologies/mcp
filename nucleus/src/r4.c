#include <emulight/r4.h>

#if 0
static inline BOOL R4CheckValid(const char c)
{
	if(c >= 'A' && c <= 'Z') {
		return TRUE;
	} else if(c >= 'a' && c <= 'z') {
		return TRUE;
	} else if(c >= '0' && c <= '9') {
		return TRUE;
	} else if(c == '$' || c == '#' || c == '@' || c == '.') {
		return TRUE;
	} else {
		return FALSE;
	}
}
#endif

static u8 r4_getidx(const char c)
{
	if(c == '.') {
		return 0;
	} else if(c == '#') {
		return 1;
	} else if(c == '@') {
		return 2;
	} else if(c == '$') {
		return 3;
	} else if(c >= '0' && c <= '9') {
		return c - '0' + 4;
	} else if(c >= 'A' && c <= 'Z') {
		return c - 'A' + 14;
	} else if(c >= 'a' && c <= 'z') {
		return c - 'a' + 14;
	} else {
		return 0;
	}
}

static char r4_decode(u8 value)
{
	if(value == 0) {
		return '.';
	} else if(value == 1) {
		return '#';
	} else if(value == 2) {
		return '@';
	} else if(value == 3) {
		return '$';
	} else if(value < 14) {
		return '0' + value - 4;
	} else {
		return 'A' + value - 14;
	}
}

static u8 rad50_getidx(const char c)
{
	if(c == ' ') {
		return 0;
	} else if(c >= 'A' && c <= 'Z') {
		return c - 'A' + 1;
	} else if(c >= 'a' && c <= 'z') {
		return c - 'a' + 1;
	} else if(c == '$') {
		return 27;
	} else if(c == '@') {
		return 28;
	} else if(c == '#') {
		return 29;
	} else if(c >= '0' && c <= '9') {
		return c - '0' + 30;
	} else {
		return 0;
	}
}

static char rad50_decode(u8 value)
{
	if(value == 0) {
		return ' ';
	} else if(value < 27) {
		return 'A' + value - 1;
	} else if(value == 27) {
		return '$';
	} else if(value == 28) {
		return '@';
	} else if(value == 29) {
	       return '#';
	} else {
		return '0' + value - 30;
	}
}

u16 R4iEncode(const char* s, BOOL rad50)
{
	char c1 = *s;
	char c2 = 0;
	char c3 = 0;

	if(c1) {
		c2 = s[1];

		if(c2) {
			c3 = s[2];
		}
	}

	if(rad50) {
		return ((rad50_getidx(c1) * 40) + rad50_getidx(c2)) * 40 + rad50_getidx(c3);
	} else {
		return ((r4_getidx(c3) * 40) + r4_getidx(c2)) * 40 + r4_getidx(c1);
	}
}

char* R4iDecode(char* out, u16 enc, BOOL rad50)
{
	u8 i1 = enc % 40;
	enc /= 40;
	u8 i2 = enc % 40;
	enc /= 40;
	u8 i3 = enc % 40;

	if(rad50) {
		out[0] = rad50_decode(i3);
		out[1] = rad50_decode(i2);
		out[2] = rad50_decode(i1);
	} else {
		out[0] = r4_decode(i1);
		out[1] = r4_decode(i2);
		out[2] = r4_decode(i3);
	}

	return out;
}

void R4iEncodeString(u16* out, const char* s, unsigned int len, BOOL rad50)
{
	while(len) {
		*(out++) = R4iEncode(s, rad50);
		if(s[0] && s[1] && s[2]) {
			s += 3;
			len--;
		} else {
			len--;
			break;
		}
	}

	while(len--) {
		*(out++) = 0;
	}
}

void R4iDecodeString(char* out, u16* in, unsigned int len, BOOL rad50)
{
	char* o = out;

	while(len--) {
		R4iDecode(out, *(in++), rad50);
		out += 3;
	}

	if(len > 0) {
		out--;
		while(out >= o && *out == '.') {
			*out = 0;
			out--;
		}
	}
}

u16 R4Encode(const char* s)
{
	return R4iEncode(s, FALSE);
}

char* R4Decode(char* out, u16 enc)
{
	return R4iDecode(out, enc, FALSE);
}

void R4EncodeString(u16* out, const char* s, unsigned int len)
{
	R4iEncodeString(out, s, len, FALSE);
}

void R4DecodeString(char* out, u16* in, unsigned int len)
{
	R4iDecodeString(out, in, len, FALSE);
}
