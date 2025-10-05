#ifndef __CALC_H__
#define __CALC_H__

#include <emulight/types.h>
#include <emulight/ispf.h>

#define	INPUT_SZ	128
#define	VAR_CNT		16
#define	VAR_NAMESZ	12
#define	CONST_CNT	2

#define	TOKEN_EOF	0
#define	TOKEN_NUMBER	1
#define	TOKEN_DECIMAL	2
#define	TOKEN_ADD	3
#define	TOKEN_SUB	4
#define	TOKEN_MUL	5
#define	TOKEN_DIV	6
#define	TOKEN_LPAR	7
#define	TOKEN_RPAR	8
#define	TOKEN_IDENT	9
#define	TOKEN_EQ	10
#define	TOKEN_ERROR	11

typedef struct {
	int	type;
	char*	value;
	int	len;
} TOKEN;

typedef struct {
	u16	bits[2];
} FLOAT;

typedef union {
	u16	bits[2];
	u32	ival;
} I32;

typedef struct {
	char	name[VAR_NAMESZ];
	FLOAT	value;
} VAR;

typedef struct {
	char*	name;
	u16	hi;
	u16	lo;
} CONST;

typedef struct {
	char*	input;
	TOKEN	t;
	TOKEN	la;
	FLOAT	result;
	VAR	variable[VAR_CNT];
} CALC;

void	FAdd(FLOAT* a, FLOAT* b);
void	FSub(FLOAT* a, FLOAT* b);
void	FMul(FLOAT* a, FLOAT* b);
void	FDiv(FLOAT* a, FLOAT* b);
void	I2F(FLOAT* out, long in);
void	F2I(long* out, FLOAT* in);
void	F2A(char* out, FLOAT* f);
void	IMul10(long* x);
void	IAdd(long* a, unsigned int b);

void	CALCInit(SPFPanel* self);
void	CALCDraw(SPFPanel* self);
void	CALCProcess(SPFPanel* self, u16 key);

void	CALCInitContext(void);
BOOL	CALCEvaluate(char* input);
void	CALCGetResult(FLOAT* out);

#endif
