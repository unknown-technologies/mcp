#include <emulight/r7.h>

/* testing in python:
 * "".join(f'{ord(x):07b}' for x in "Hello world")
 * " ".join(hex(int('10010001100101110110011011001101111010000011101111101111111001011011001100100'[x*8:x*8+8], 2))[2:] for x in range(9))
 */
void R7Encode(u8* out, const char* in, unsigned int len)
{
	u16 accumulator = 0;

	unsigned int bit = 0;
	unsigned int j = 0;
	for(unsigned int i = 0; j < len && *in; i++, in++) {
		if(bit) {
			accumulator <<= 7;
			accumulator |= *in & 0x7F;
			u8 result = accumulator >> (7 - bit);
			*(out++) = result;
			j++;
		} else {
			accumulator = *in & 0x7F;
		}

		bit++;
		if(bit == 8) {
			bit = 0;
		}
	}

	if(bit != 0) {
		accumulator <<= 7;
		u8 result = accumulator >> (7 - bit);
		if(j < len) {
			*(out++) = result;
		}
	}
}

void R7Decode(char* out, u8* in, unsigned int len)
{
	u16 accumulator = 0;

	unsigned int bit = 0;
	for(unsigned int i = 0, j = 0; j < len; i++, out++) {
		char result;

		if(bit != 7) {
			bit++;

			accumulator <<= 8;
			accumulator |= *in;
			result = (accumulator >> bit) & 0x7F;

			in++;
			j++;
		} else {
			result = accumulator & 0x7F;
			bit = 0;
		}

		*out = result;
		if(!result) {
			break;
		}
	}
}
