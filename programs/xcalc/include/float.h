#ifndef __BCDFLOAT_H__
#define __BCDFLOAT_H__

#include <emulight/types.h>

#define	BCDFLOAT_LEN	16
#define	BCDFLOAT_POS	0
#define	BCDFLOAT_NEG	1

#define	BCDFLOAT_DGT	(2 * BCDFLOAT_LEN)

typedef struct {
	u8	bits[BCDFLOAT_LEN];
	s16	exp;
	s16	sign;
} FLOAT;

BOOL S2F(FLOAT* f, const char* s, size_t len);

void FNeg(FLOAT* f);
void FAdd(FLOAT* out, FLOAT* a, FLOAT* b);
void FSub(FLOAT* out, FLOAT* a, FLOAT* b);
void FMul(FLOAT* out, FLOAT* a, FLOAT* b);
void FDiv(FLOAT* out, FLOAT* a, FLOAT* b);

void FAdd2(FLOAT* a, FLOAT* b);
void FSub2(FLOAT* a, FLOAT* b);
void FMul2(FLOAT* a, FLOAT* b);
void FDiv2(FLOAT* a, FLOAT* b);

BOOL FIsZero(FLOAT* a);
unsigned int FGetDigits(FLOAT* flt);

void FPrintSci(FLOAT* flt);
void FPrintEng(FLOAT* flt);
void FPrintStd(FLOAT* flt);

#endif
