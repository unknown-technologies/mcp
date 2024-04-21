#include <emulight/rad50.h>

static u8 getidx(const char c)
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

static char decode(u8 value)
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

u16 RAD50Encode(const char* s)
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

	return ((getidx(c1) * 40) + getidx(c2)) * 40 + getidx(c3);
}

char* RAD50Decode(char* out, u16 enc)
{
	u8 i1 = enc % 40;
	enc /= 40;
	u8 i2 = enc % 40;
	enc /= 40;
	u8 i3 = enc % 40;

	out[0] = decode(i3);
	out[1] = decode(i2);
	out[2] = decode(i1);

	return out;
}

void RAD50EncodeString(u16* out, const char* s, unsigned int len)
{
	while(len) {
		*(out++) = RAD50Encode(s);
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

void RAD50DecodeString(char* out, u16* in, unsigned int len)
{
	char* o = out;

	while(len--) {
		RAD50Decode(out, *(in++));
		out += 3;
	}

	if(len > 0) {
		out--;
		while(out >= o && *out == ' ') {
			*out = 0;
			out--;
		}
	}
}
