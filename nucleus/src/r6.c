#include <emulight/r6.h>

static u8 encode(const char c)
{
	if(c >= 'a' && c <= 'z') {
		return (c - 0x40) & 0x3F;
	} else if(c < 0x20 || c >= 0x60) {
		return 0;
	} else {
		return (c - 0x20) & 0x3F;
	}
}

static char decode(u8 c) {
	return (char) (c + 0x20);
}

void R6Encode(u8* out, const char* in, unsigned int len)
{
	unsigned int accumulator = 0;

	unsigned int cnt = 0;
	for(unsigned int i = 0; i < len && *in; i++, in++) {
		unsigned int c = encode(*in);
		switch(cnt) {
			case 0:
				/* A = 000000__ */
				accumulator = c << 2;
				break;
			case 1:
				/* WR = 00000011 */
				*(out++) = accumulator | (c >> 4);
				/* A = 1111____ */
				accumulator = c << 4;
				break;
			case 2:
				/* WR = 11112222 */
				*(out++) = accumulator | (c >> 2);
				/* A = 22______ */
				accumulator = c << 6;
				break;
			case 3:
				/* WR = 22333333 */
				*(out++) = accumulator | c;
				break;

		}

		cnt++;
		if(cnt == 4) {
			cnt = 0;
		}
	}

	if(len && cnt) {
		*(out++) = accumulator;
	}
}

void R6Decode(char* out, u8* in, unsigned int len)
{
	char* ptr = out;
	unsigned int accumulator = 0;

	unsigned int tmp;
	unsigned int cnt = 0;
	for(unsigned int i = 0; i < len; i++) {
		switch(cnt) {
			case 0:
				/* RD = 00000011 */
				tmp = *(in++);
				/* WR = 000000__ */
				*(out++) = decode(tmp >> 2);
				/* AC = __11____ */
				accumulator = (tmp << 4) & 0x3F;
				break;
			case 1:
				/* RD = 11112222 */
				tmp = *(in++);
				/* WR = __111111 */
				*(out++) = decode(accumulator | (tmp >> 4));
				/* AC = __2222__ */
				accumulator = (tmp << 2) & 0x3F;
				break;
			case 2:
				/* RD = 22333333 */
				tmp = *(in++);
				/* WR = __222222 */
				*(out++) = decode(accumulator | (tmp >> 6));
				/* WR = __333333 */
				if(i != len - 1) {
					*(out++) = decode(tmp & 0x3F);
				}
				break;
		}

		cnt++;
		if(cnt == 3) {
			cnt = 0;
		}
	}

	/* trim trailing spaces */
	for(char* p = out - 1; p >= ptr; p--) {
		if(*p == ' ') {
			*p = 0;
		} else {
			break;
		}
	}

	*(out++) = 0;
}
