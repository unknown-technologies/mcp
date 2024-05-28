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

static u8 getidx(const char c)
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

static char decode(u8 value)
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

u16 R4Encode(const char* s)
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

	return ((getidx(c3) * 40) + getidx(c2)) * 40 + getidx(c1);
}

char* R4Decode(char* out, u16 enc)
{
	u8 i1 = enc % 40;
	enc /= 40;
	u8 i2 = enc % 40;
	enc /= 40;
	u8 i3 = enc % 40;

	out[0] = decode(i1);
	out[1] = decode(i2);
	out[2] = decode(i3);

	return out;
}

void R4EncodeString(u16* out, const char* s, unsigned int len)
{
	while(len) {
		*(out++) = R4Encode(s);
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

void R4DecodeString(char* out, u16* in, unsigned int len)
{
	char* o = out;

	while(len--) {
		R4Decode(out, *(in++));
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
