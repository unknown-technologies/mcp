#include <emulight/types.h>
#include <emulight/string.h>
#include "calc.h"

void I2F(FLOAT* out, long in)
{
	u16 nlz = 32;
	u16 msb;
	u16 lsb;

	u16 sign;
	u32 value;

	if(((int16_t) (in >> 16)) < 0) {
		sign = 0x8000;
		value = (u32) -in;
	} else {
		sign = 0;
		value = (u32) in;
	}

	I32 ival;
	ival.ival = value;

	msb = ival.bits[0];
	lsb = ival.bits[1];

	/* the value is zero */
	if(!msb && !lsb) {
		bzero(out, sizeof(FLOAT));
		return;
	}


	s16 tmp = msb;
	for(int i = 0; i < 16; i++) {
		if(tmp < 0) {
			nlz = i;
			break;
		} else {
			tmp <<= 1;
		}
	}

	if(nlz == 32) {
		tmp = lsb;

		for(int i = 0; i < 16; i++) {
			if(tmp < 0) {
				nlz = i + 16;
				break;
			} else {
				tmp <<= 1;
			}
		}
	}

	unsigned int bits = 32 - nlz;
	unsigned int bias = 0200 + bits;

	u32 shifted = value << (24 - bits);
	ival.ival = shifted;

	out->bits[0] = sign | (bias << 7) | (ival.bits[0] & 0x7F);
	out->bits[1] = ival.bits[1];
}

void F2I(long* out, FLOAT* f)
{
	int bias = (f->bits[0] >> 7) & 0xFF;
	int exponent = bias - 0200;

	I32 ival;
	ival.bits[0] = (f->bits[0] & 0x7F) | 0x80;
	ival.bits[1] = f->bits[1];

	u32 shifted = ival.ival >> (24 - exponent);

	*out = shifted;
}

void F2A(char* out, FLOAT* f)
{
#if 0
	FLOAT ten;
	FLOAT tmp;

	I2F(&ten, 10);
	memcpy(&tmp, f, sizeof(FLOAT));

	char* c = out;
	BOOL sign = ((s16) f->bits[0]) < 0;

	if(sign) {
		*(c++) = '-';
	}

	*(c++) = '1';
	*(c++) = '.';

	for(unsigned int i = 0; i < 7; i++) {
		BOOL bit = tmp.bits[0] & (1 << (6 - i));
		*(c++) = bit ? '1' : '0';
	}

	for(unsigned int i = 0; i < 16; i++) {
		BOOL bit = tmp.bits[1] & (1 << (15 - i));
		*(c++) = bit ? '1' : '0';
	}

	*(c++) = 'E';
	int bias = (tmp.bits[0] >> 7) & 0xFF;
	sprintf(c, "%d", bias - 0200 - 1);
#else
	FLOAT tmp;
	FLOAT ten;

	memcpy(&tmp, f, sizeof(FLOAT));
	tmp.bits[0] &= 0x7FFF;

	I2F(&ten, 10L);

	char* c = out;
	if(((s16) f->bits[0]) < 0) {
		*(c++) = '-';
	}

	long ival;
	F2I(&ival, f);
	c += sprintf(c, "%lu", (unsigned long) ival);

	/* remove integer part */
	FLOAT sub;
	I2F(&sub, ival);
	FSub(&tmp, &sub);

	if((tmp.bits[0] >> 7) & 0xFF) {
		/* now get the decimals */
		*(c++) = '.';

		/* print the digits until there are no more left */
		int n = 8;
		while((tmp.bits[0] >> 7) & 0xFF) {
			if(!n--) {
				break;
			}
			/* multiply by 10 to get the next digit */
			FMul(&tmp, &ten);

			/* convert to integer and print */
			F2I(&ival, &tmp);
			*(c++) = (unsigned int) ival + '0';

			/* remove integer part */
			I2F(&sub, ival);
			FSub(&tmp, &sub);
		}
	}

	*(c++) = 0;
#endif
}
