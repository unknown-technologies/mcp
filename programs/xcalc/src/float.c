#include <emulight/types.h>
#include <emulight/string.h>
#include <emulight/vt.h>
#include "float.h"

static void FiNeg(FLOAT* f);
static void FiInc(FLOAT* f, BOOL trunc);
static void FiAdd(FLOAT* out, FLOAT* a, FLOAT* b, unsigned int carry_in);
static void FiSub(FLOAT* out, FLOAT* a, FLOAT* b);

static FLOAT f_one = {
	.bits = { 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	.exp = 0,
	.sign = BCDFLOAT_POS
};

#define	STATE_DEC	0
#define	STATE_FRAC	1
#define	STATE_EXP	2
#define	STATE_ERR	3

static inline unsigned int HI(unsigned int x)
{
	x >>= 4;
	return x;
}

/******************************************************************************/
BOOL S2F(FLOAT* f, const char* s, size_t len)
{
	bzero(f, sizeof(FLOAT));
	unsigned int state = STATE_DEC;
	unsigned int i = 0;
	size_t read = 0;
	int exp = 0;
	BOOL negexp = FALSE;
	BOOL zero = TRUE;
	f->exp = -1;
	if(*s == '-') {
		f->sign = BCDFLOAT_NEG;
		s++;
		read++;
	} else {
		f->sign = BCDFLOAT_POS;
	}
	for(const char* c = s; *c && state < STATE_ERR && read < len;
			c++, read++) {
		switch(state) {
			/* decimal part */
			case STATE_DEC:
				if(*c >= '0' && *c <= '9') {
					unsigned int digit = *c - '0';
					if(digit > 0) {
						zero = FALSE;
					}
					if(i == 2 * BCDFLOAT_LEN) {
						/* round up */
						if(digit >= 5) {
							FiInc(f, FALSE);
						}
						if(!zero) {
							i++;
						} else {
							f->exp--;
						}
						continue;
					} else if(i >= 2 * BCDFLOAT_LEN) {
						continue;
					}
					if(i & 1) {
						f->bits[i / 2] |= digit;
					} else {
						f->bits[i / 2] = (digit << 4);
					}
					if(!zero) {
						i++;
						f->exp++;
					}
				} else if(*c == '.') {
					state = STATE_FRAC;
				} else if(*c == 'e' || *c == 'E') {
					state = STATE_EXP;
				} else {
					state = STATE_ERR;
				}
				break;
			/* fractional part */
			case STATE_FRAC:
				if(*c >= '0' && *c <= '9') {
					unsigned int digit = *c - '0';
					if(digit > 0) {
						zero = FALSE;
					}
					if(i == 2 * BCDFLOAT_LEN) {
						/* round up */
						if(digit >= 5) {
							FiInc(f, FALSE);
						}
						if(!zero) {
							i++;
						} else {
							f->exp--;
						}
						continue;
					} else if(i >= 2 * BCDFLOAT_LEN) {
						continue;
					}
					if(i & 1) {
						f->bits[i / 2] |= digit;
					} else {
						f->bits[i / 2] = (digit << 4);
					}
					if(!zero) {
						i++;
					} else {
						f->exp--;
					}
				} else if(*c == 'e' || *c == 'E') {
					state = STATE_EXP;
				} else {
					state = STATE_ERR;
				}
				break;
			/* exponent part */
			case STATE_EXP:
				if(*c == '-') {
					negexp = TRUE;
				} else if(*c >= '0' && *c <= '9') {
					BOOL oldsign = exp < 0;
					exp *= 10;
					exp += *c - '0';
					BOOL newsign = exp < 0;
					if(oldsign != newsign) {
						state = STATE_ERR;
					}
				} else {
					state = STATE_ERR;
				}
				break;
			default:
				state = STATE_ERR;
				break;
		}
	}

	if(state == STATE_EXP) {
		if(negexp) {
			f->exp -= exp;
		} else {
			f->exp += exp;
		}
	}

	return state < STATE_ERR;
}

/******************************************************************************/
/* ten's complement */
static void FiNeg(FLOAT* f)
{
	for(unsigned int i = 0; i < BCDFLOAT_LEN; i++) {
		unsigned int bits = f->bits[i];
		f->bits[i] = (u8) (0x99 - bits);
	}

	FiInc(f, TRUE);
}

static void FiInc(FLOAT* f, BOOL trunc)
{
	unsigned int carry = 1;
	for(int i = BCDFLOAT_LEN - 1; i >= 0 && carry != 0; i--) {
		unsigned int bits = f->bits[i];
		unsigned int result = bits + carry;
		unsigned int half_carry = ((bits & 0x0F) + carry) > 9;
		if(half_carry) {
			result += 6;
		}

		if((HI(result)) > 9) {
			result += 0x60;
			carry = 1;
		} else {
			carry = 0;
		}

		f->bits[i] = (u8) result;
	}

	/* add carry and normalize */
	if(carry && !trunc) {
		/* move everything by one nibble */
		for(int i = BCDFLOAT_LEN - 1; i > 0; i--) {
			unsigned int bits = f->bits[i - 1] & 0x0F;
			f->bits[i] = HI(f->bits[i])
				| (bits << 4);
		}

		/* add most significant digit */
		f->bits[0] = HI(f->bits[0]) | 0x10;
		f->exp++;
	}
}

/* this function interprets a and b as positive numbers,
 * computes out = a + b + carry_in */
static void FiAdd(FLOAT* out, FLOAT* a, FLOAT* b, unsigned int carry_in)
{
	if(a->exp == b->exp) {
		/* no shifting needed */
		out->exp = a->exp;
		unsigned int carry = carry_in;
		for(int i = BCDFLOAT_LEN - 1; i >= 0; i--) {
			unsigned int da = a->bits[i] + carry;
			unsigned int db = b->bits[i];
			unsigned int result = da + db;
			unsigned int half_carry = ((da & 0x0F)
					+ (db & 0x0F)) > 9;
			if(half_carry) {
				result += 6;
			}
			if(HI(result) > 9) {
				result += 0x60;
				carry = 1;
			} else {
				carry = 0;
			}

			out->bits[i] = (u8) result;
		}

		/* add carry and normalize */
		if(carry) {
			/* move everything by one nibble */
			for(int i = BCDFLOAT_LEN - 1; i > 0; i--) {
				unsigned int bits = out->bits[i - 1] & 0x0F;
				out->bits[i] = HI(out->bits[i])
					| (bits << 4);
			}

			/* add most significant digit */
			out->bits[0] = HI(out->bits[0]) | 0x10;
			out->exp++;
		}
	} else {
		/* we need to align both numbers first */
		int diff = a->exp - b->exp;
		if(diff < 0) {
			diff = -diff;
		}
		if(diff >= 2 * BCDFLOAT_LEN) {
			/* the difference is too big to matter, return the
			 * bigger number */
			if(a->exp > b->exp) {
				memcpy(out, a, sizeof(FLOAT));
			} else {
				memcpy(out, b, sizeof(FLOAT));
			}

			out->sign = BCDFLOAT_POS;

			return;
		}

		FLOAT* x = a;
		FLOAT* y = b;
		if(a->exp < b->exp) {
			/* the first one has a smaller exponent than the
			 * second one */
			x = b;
			y = a;
		}

		/* perform the addition with x > y */
		out->exp = x->exp;

		/* We start at the smallest significant digit */
		int x_digit = 2 * BCDFLOAT_LEN - 1;
		int y_digit = 2 * BCDFLOAT_LEN - 1 - diff;

		unsigned int carry = 0;

		/* rounding of the truncated part */
		unsigned int next;
		if(y_digit & 1) {
			next = HI(y->bits[(y_digit + 1) / 2]);
		} else {
			next = y->bits[y_digit / 2] & 0x0F;
		}

		if(next >= 5) {
			carry = 1;
		}

		while(x_digit >= 0) {
			int xi = x_digit / 2;
			int yi = y_digit / 2;

			unsigned int da = x->bits[xi];
			unsigned int db;

			if(y_digit >= 0) {
				db = y->bits[yi];
			} else {
				db = 0;
			}

			if(x_digit & 1) {
				da &= 0x0F;
			} else {
				da >>= 4;
			}

			if(y_digit & 1) {
				db &= 0x0F;
			} else {
				db >>= 4;
			}

			da += carry;

			/* da and db now hold exactly one digit */

			unsigned int result = da + db;
			carry = result > 9;
			if(carry) {
				result += 6;
			}

			result &= 0x0F;

			if(x_digit & 1) {
				out->bits[xi] = result;
			} else {
				out->bits[xi] |= result << 4;
			}

			x_digit--;
			y_digit--;
		}

		/* add carry and normalize */
		if(carry) {
			/* move everything by one nibble */
			for(int i = BCDFLOAT_LEN - 1; i > 0; i--) {
				unsigned int bits = out->bits[i - 1] & 0x0F;
				out->bits[i] = HI(out->bits[i])
					| (bits << 4);
			}

			/* add most significant digit */
			out->bits[0] = HI(out->bits[0]) | 0x10;
			out->exp++;
		}
	}

	out->sign = BCDFLOAT_POS;
}

void FAdd(FLOAT* out, FLOAT* a, FLOAT* b)
{
	unsigned int cases = (a->sign ? 0x02 : 0x00)
		| (b->sign ? 0x01 : 0x00);
	switch(cases) {
		case 0x00: /* a + b */
			FiAdd(out, a, b, 0);
			break;
		case 0x01: /* a + -b = a - b */
			FiSub(out, a, b);
			break;
		case 0x02: /* -a + b = b - a */
			FiSub(out, b, a);
			break;
		case 0x03: /* -a + -b = -a - b = -(a + b) */
			FiAdd(out, a, b, 0);
			out->sign = BCDFLOAT_NEG;
			break;
	}
}

/* this function interprets a and b as positive numbers,
 * computes out = a - b */
void FiSub(FLOAT* out, FLOAT* a, FLOAT* b)
{
	if(a->exp == b->exp) {
		/* no shifting needed */
		out->exp = a->exp;
		unsigned int carry = 1;
		for(int i = BCDFLOAT_LEN - 1; i >= 0; i--) {
			unsigned int da = a->bits[i] + carry;
			unsigned int db = 0x99 - b->bits[i];
			unsigned int result = da + db;
			unsigned int half_carry = ((da & 0x0F)
					+ (db & 0x0F)) > 9;
			if(half_carry) {
				result += 6;
			}
			if(HI(result) > 9) {
				result += 0x60;
				carry = 1;
			} else {
				carry = 0;
			}

			out->bits[i] = (u8) result;
		}

		if(carry) {
			out->sign = BCDFLOAT_POS;
		} else {
			out->sign = BCDFLOAT_NEG;
			FiNeg(out);
		}

		/* normalize */
		unsigned int shift = 0;
		for(unsigned int i = 0; i < BCDFLOAT_LEN; i++) {
			unsigned int d = out->bits[i];
			if(d & 0xF0) {
				shift = 2 * i;
				break;
			} else if(d & 0x0F) {
				shift = 2 * i + 1;
				break;
			}
		}

		/* now shift everything */
		if(shift) {
			for(unsigned int i = 0; i < 2 * BCDFLOAT_LEN; i++) {
				unsigned int idx = i + shift;
				unsigned int digit = 0;
				if(idx < 2 * BCDFLOAT_LEN) {
					digit = out->bits[idx / 2];
				}

				if(idx & 1) {
					digit &= 0x0F;
				} else {
					digit >>= 4;
				}

				unsigned int current = out->bits[i / 2];
				if(i & 1) {
					out->bits[i / 2] = (current & 0xF0)
						| digit;
				} else {
					out->bits[i / 2] = (current & 0x0F)
						| (digit << 4);
				}
			}

			out->exp -= shift;
		}
	} else {
		/* we need to align both numbers first */
		int diff = a->exp - b->exp;
		if(diff < 0) {
			diff = -diff;
		}
		if(diff >= 2 * BCDFLOAT_LEN) {
			/* the difference is too big to matter, return the
			 * bigger number */
			if(a->exp > b->exp) {
				memcpy(out, a, sizeof(FLOAT));
			} else {
				memcpy(out, b, sizeof(FLOAT));
				out->sign = !out->sign;
			}

			return;
		}

		FLOAT* x = a;
		FLOAT* y = b;
		BOOL swap = a->exp < b->exp;
		if(swap) {
			/* the first one has a smaller exponent than the
			 * second one */
			x = b;
			y = a;
		}

		/* perform the addition with x > y */
		out->exp = x->exp;

		/* We start at the smallest significant digit */
		int x_digit = 2 * BCDFLOAT_LEN - 1;
		int y_digit = 2 * BCDFLOAT_LEN - 1 - diff;

		unsigned int carry = 1;

		/* TODO: how to round the truncated part? */

		while(x_digit >= 0) {
			int xi = x_digit / 2;
			int yi = y_digit / 2;

			unsigned int da = x->bits[xi];
			unsigned int db;

			if(y_digit >= 0) {
				db = y->bits[yi];
			} else {
				db = 0;
			}

			if(x_digit & 1) {
				da &= 0x0F;
			} else {
				da >>= 4;
			}

			if(y_digit & 1) {
				db &= 0x0F;
			} else {
				db >>= 4;
			}

			if(swap) {
				da = 9 - da;
			} else {
				db = 9 - db;
			}

			da += carry;

			/* da and db now hold exactly one digit */

			unsigned int result = da + db;
			carry = result > 9;
			if(carry) {
				result += 6;
			}

			result &= 0x0F;

			if(x_digit & 1) {
				out->bits[xi] = result;
			} else {
				out->bits[xi] |= result << 4;
			}

			x_digit--;
			y_digit--;
		}

		if(carry) {
			out->sign = BCDFLOAT_POS;
		} else {
			out->sign = BCDFLOAT_NEG;
			FiNeg(out);
		}

		/* normalize */
		unsigned int shift = 0;
		for(unsigned int i = 0; i < BCDFLOAT_LEN; i++) {
			unsigned int d = out->bits[i];
			if(d & 0xF0) {
				shift = 2 * i;
				break;
			} else if(d & 0x0F) {
				shift = 2 * i + 1;
				break;
			}
		}

		/* now shift everything */
		if(shift) {
			for(unsigned int i = 0; i < 2 * BCDFLOAT_LEN; i++) {
				unsigned int idx = i + shift;
				unsigned int digit = 0;
				if(idx < 2 * BCDFLOAT_LEN) {
					digit = out->bits[idx / 2];
				}

				if(idx & 1) {
					digit &= 0x0F;
				} else {
					digit >>= 4;
				}

				unsigned int current = out->bits[i / 2];
				if(i & 1) {
					out->bits[i / 2] = (current & 0xF0)
						| digit;
				} else {
					out->bits[i / 2] = (current & 0x0F)
						| (digit << 4);
				}
			}

			out->exp -= shift;
		}
	}
}

void FSub(FLOAT* out, FLOAT* a, FLOAT* b)
{
	unsigned int cases = (a->sign ? 0x02 : 0x00)
		| (b->sign ? 0x01 : 0x00);
	switch(cases) {
		case 0x00: /* a - b */
			FiSub(out, a, b);
			break;
		case 0x01: /* a - -b = a + b */
			FiAdd(out, a, b, 0);
			out->sign = BCDFLOAT_POS;
			break;
		case 0x02: /* -a - b = -(a + b) */
			FiAdd(out, a, b, 0);
			out->sign = BCDFLOAT_NEG;
			break;
		case 0x03: /* -a - -b = -a + b = b - a */
			FiSub(out, b, a);
			break;
	}
}

#if 0
 999 * 9
---------------
  81
 81
81
---------------
8991

 779 * 7
---------------
  63
 49
49
11-------------
5453
#endif

/******************************************************************************/
static unsigned int FiRevIdx(unsigned int idx)
{
	return 2 * BCDFLOAT_LEN - idx - 1;
}

static unsigned int FiRevIdx2(unsigned int idx)
{
	return 4 * BCDFLOAT_LEN - idx - 1;
}

static unsigned int FiGetDigit(u8* bits, unsigned int digit)
{
	unsigned int result = bits[digit / 2];
	if(digit & 1) {
		return result & 0x0F;
	} else {
		return HI(result);
	}
}

static void FiSetDigit(u8* bits, unsigned int digit,
		unsigned int value)
{
	if(digit & 1) {
		bits[digit / 2] = (bits[digit / 2] & 0xF0) | value;
	} else {
		bits[digit / 2] = (bits[digit / 2] & 0x0F) | (value << 4);
	}
}

static unsigned int FiGetDigit2(u8* a, u8* b, unsigned int digit)
{
	if(digit < 2 * BCDFLOAT_LEN) {
		return FiGetDigit(a, digit);
	} else {
		return FiGetDigit(b, digit - 2 * BCDFLOAT_LEN);
	}
}

static void FiSetDigit2(u8* a, u8* b, unsigned int digit,
		unsigned int value)
{
	if(digit < 2 * BCDFLOAT_LEN) {
		FiSetDigit(a, digit, value);
	} else {
		FiSetDigit(b, digit - 2 * BCDFLOAT_LEN, value);
	}
}

void FMul(FLOAT* out, FLOAT* a, FLOAT* b)
{
	out->sign = a->sign != b->sign;
	out->exp = a->exp + b->exp;

	u8 tmp[BCDFLOAT_LEN];
	bzero(tmp, BCDFLOAT_LEN);
	bzero(out->bits, BCDFLOAT_LEN);

	for(int b_i = 0; b_i < 2 * BCDFLOAT_LEN; b_i++) {
		unsigned int carry = 0;

		unsigned int db = FiGetDigit(b->bits, FiRevIdx(b_i));

		if(!db) {
			continue;
		}

		for(int a_i = 0; a_i < 2 * BCDFLOAT_LEN; a_i++) {
			unsigned int da = FiGetDigit(a->bits, FiRevIdx(a_i));

			const int idx = a_i + b_i;
			unsigned int dsum = FiGetDigit2(out->bits, tmp,
					FiRevIdx2(idx));

			if(!da && !carry) {
				continue;
			}

			unsigned int sum = dsum + da * db + carry;


			FiSetDigit2(out->bits, tmp, FiRevIdx2(idx),
					sum % 10);
			carry = sum / 10;
		}

		FiSetDigit2(out->bits, tmp,
				FiRevIdx2(b_i + 2 * BCDFLOAT_LEN), carry);
	}

	/* normalize */
	if(!(out->bits[0] & 0xF0)) {
		/* shift everything by one digit */
		for(unsigned int i = 0; i < 2 * BCDFLOAT_LEN - 1; i++) {
			unsigned int digit = FiGetDigit(out->bits, i + 1);
			FiSetDigit(out->bits, i, digit);
		}
		out->bits[BCDFLOAT_LEN - 1] =
			(out->bits[BCDFLOAT_LEN - 1] & 0xF0) |
			HI(tmp[0]);

#if 0
		/* round */
		if((tmp[0] & 0x0F) > 5) {
			FiInc(out, FALSE);
		}
#endif
	} else {
		out->exp++;

#if 0
		/* round */
		if((tmp[0] & 0xF0) > 0x50) {
			FiInc(out, FALSE);
		}
#endif
	}
}

/* Theory of Newton-Raphson Division:
 *
 * We need the following constants:
 * 140/33 = 4.242424242424...
 *  64/11 = 5.818181818181...
 * 256/99 = 2.585858585858...
 *
 * We want to compute Q = N/D by computing 1/D. We then just do:
 *
 *   Q = 1/D * N
 *
 * When D is in the range [0.5, 1], the initial guess is:
 * 
 *   X = 140/33 - 64/11 D + 256/99 D^2
 *
 * The final iteration is as follows:
 *
 *   X = X + X * (1 - D' * X)
 */

static const u8 scaling_factors[10] = {
	/* 00 */ 0x00,
	/* 01 */ 0x75,
	/* 02 */ 0x37,
	/* 03 */ 0x25,
	/* 04 */ 0x19,
	/* 05 */ 0x15,
	/* 06 */ 0x12,
	/* 07 */ 0x11,
	/* 08 */ 0x09,
	/* 09 */ 0x08
};

/* this function computes out = n / d */
void FDiv(FLOAT* out, FLOAT* n, FLOAT* d)
{
	FLOAT x;
	FLOAT d_;
	FLOAT n_;
	FLOAT tmp;

	memcpy(&d_, d, sizeof(FLOAT));
	memcpy(&n_, n, sizeof(FLOAT));

	bzero(out, sizeof(FLOAT));

	/* select multiplication factor */
	unsigned int msd = HI(d->bits[0]);
	if(msd < 5 || d->exp != -1) {
		out->bits[0] = scaling_factors[msd];
		out->sign = d->sign;

		if(d->exp == 0) {
			out->exp = -1;
		}
		if(d->exp > 0) {
			out->exp = -d->exp - 1;
		} else if(d->exp < 0) {
			out->exp = -d->exp - 1;
		}

		FMul(&d_, d, out);
		FMul(&n_, n, out);
	}

	/* Initial estimate: X = 140/33 - 64/11 D + 256/99 D^2 */
	/* Simplify: X = 140/33 - (64/11 D - 256/99 D^2) */

	/* generate out = 256/99 */
	out->bits[0] = 0x25;
	out->exp = 0;
	out->sign = BCDFLOAT_POS;
	for(unsigned int i = 1; i < BCDFLOAT_LEN; i++) {
		out->bits[i] = 0x85;
	}
	FMul(&x, &d_, &d_);
	FMul(&tmp, out, &x);		/* tmp = 256/99 D^2 */

	/* generate out = 64/11 */
	out->bits[0] = 0x58;
	for(unsigned int i = 1; i < BCDFLOAT_LEN; i++) {
		out->bits[i] = 0x18;
	}
	FMul(&x, out, &d_);		/* x = 64/11 D */
	FSub(out, &x, &tmp);		/* out = 64/11 D - 256/99 D^2 */

	/* generate tmp = 140/33 */
	tmp.exp = 0;
	tmp.sign = BCDFLOAT_POS;
	for(unsigned int i = 0; i < BCDFLOAT_LEN; i++) {
		tmp.bits[i] = 0x42;
	}
	FSub(&x, &tmp, out);

	for(unsigned int i = 0; i < 5; i++) {
		/* Iteration: X = X + X * (1 - D' * X) */
		FMul(out, &d_, &x);	/* out = D' x X */

		/* check if no change */
		BOOL zero = out->bits[0] == 0x10 && out->exp == 0;
		for(unsigned int j = 1; zero && j < BCDFLOAT_LEN; j++) {
			if(out->bits[j] != 0) {
				zero = FALSE;
				break;
			}
		}
		if(zero) {
			break;
		}

		FSub(&tmp, &f_one, out);/* tmp = 1 - D' x X */
		FMul(out, &tmp, &x);	/* out = X x (1 - D' x X) */

		memcpy(&tmp, &x, sizeof(FLOAT));
		FAdd(&x, &tmp, out);	/* X = X + X x (1 - D' x X) */
	}

	FMul(out, &x, &n_);		/* Q = N' * X */
}

/******************************************************************************/
void FNeg(FLOAT* flt)
{
	flt->sign = !flt->sign;
}

unsigned int FGetDigits(FLOAT* flt)
{
	/* compute length */
	unsigned int len = 0;
	for(int i = BCDFLOAT_LEN - 1; i >= 0; i--) {
		unsigned int bits = flt->bits[i];
		if(bits & 0x0F) {
			len = 2 * i + 1;
			break;
		} else if(bits & 0xF0) {
			len = 2 * i;
			break;
		}
	}
	return len + 1;
}

BOOL FIsZero(FLOAT* flt)
{
	for(unsigned int i = 0; i < BCDFLOAT_LEN; i++) {
		if(flt->bits[i]) {
			return FALSE;
		}
	}

	return TRUE;
}

/******************************************************************************/
static FLOAT f_buf;

void FAdd2(FLOAT* a, FLOAT* b)
{
	memcpy(&f_buf, a, sizeof(FLOAT));
	FAdd(a, &f_buf, b);
}

void FSub2(FLOAT* a, FLOAT* b)
{
	memcpy(&f_buf, a, sizeof(FLOAT));
	FSub(a, &f_buf, b);
}

void FMul2(FLOAT* a, FLOAT* b)
{
	memcpy(&f_buf, a, sizeof(FLOAT));
	FMul(a, &f_buf, b);
}

void FDiv2(FLOAT* a, FLOAT* b)
{
	memcpy(&f_buf, a, sizeof(FLOAT));
	FDiv(a, &f_buf, b);
}

/******************************************************************************/
void FPrintSci(FLOAT* flt)
{
	if(FIsZero(flt)) {
		VTPrint("0");
		return;
	}

	if(flt->sign != BCDFLOAT_POS) {
		VTPutc('-');
	}

	unsigned int len = FGetDigits(flt);

	if(len == 1) {
		VTPrintf("%X", (unsigned int) HI(flt->bits[0]));
	} else {
		VTPrintf("%X.%X", (unsigned int) HI(flt->bits[0]),
				(unsigned int) (flt->bits[0] & 0x0F));
		for(unsigned int i = 1; i < BCDFLOAT_LEN; i++) {
			unsigned int pos = i * 2;
			if(pos >= len) {
				break;
			} else if(pos + 1 >= len) {
				VTPrintf("%X", (unsigned int) HI(flt->bits[i]));
				break;
			} else {
				VTPrintf("%02X", (unsigned int) flt->bits[i]);
			}
		}
	}
	if(flt->exp) {
		VTPrintf("E%d", (int) flt->exp);
	}
}

void FPrintEng(FLOAT* flt)
{
	if(FIsZero(flt)) {
		VTPrint("0");
		return;
	}

	if(flt->sign != BCDFLOAT_POS) {
		VTPutc('-');
	}

	int si;
	int off;

	if(flt->exp >= 0) {
		si = flt->exp / 3;
		off = flt->exp - si * 3;
	} else {
		si = (flt->exp - 2) / 3;
		off = flt->exp - si * 3;
	}

	unsigned int len = FGetDigits(flt);
	if(len < off + 1) {
		len = off + 1;
	}

	if(len == 1) {
		VTPrintf("%X", (unsigned int) HI(flt->bits[0]));
	} else {
		for(unsigned int i = 0; i < 2 * BCDFLOAT_LEN && i < len; i++) {
			unsigned int digit = flt->bits[i / 2];
			if(i & 1) {
				digit &= 0xF;
			} else {
				digit >>= 4;
			}

			VTPrintf("%X", digit);

			if(i == off && i != 2 * BCDFLOAT_LEN - 1
					&& i != len - 1) {
				VTPutc('.');
			}
		}
	}

	if(si) {
		VTPrintf("E%d", 3 * si);
	}
}

void FPrintStd(FLOAT* flt)
{
	if(FIsZero(flt)) {
		VTPrint("0");
	} else if(flt->exp >= -3 && flt->exp < BCDFLOAT_DGT) {
		if(flt->sign != BCDFLOAT_POS) {
			VTPutc('-');
		}

		unsigned int len = FGetDigits(flt);
		int off = flt->exp;

		if(flt->exp < 0) {
			int cnt = -flt->exp - 1;
			VTPrint("0.");
			for(int i = 0; i < cnt; i++) {
				VTPutc('0');
			}
		} else if(flt->exp + 1 > len) {
			len = flt->exp + 1;
		}

		if(len == 1) {
			VTPrintf("%X", (unsigned int) HI(flt->bits[0]));
		} else {
			for(unsigned int i = 0; i < 2 * BCDFLOAT_LEN
					&& i < len; i++) {
				unsigned int digit = flt->bits[i / 2];
				if(i & 1) {
					digit &= 0xF;
				} else {
					digit >>= 4;
				}

				VTPrintf("%X", digit);

				if(i == off && i != 2 * BCDFLOAT_LEN - 1
						&& i != len - 1) {
					VTPutc('.');
				}
			}
		}
	} else {
		FPrintSci(flt);
	}
}
