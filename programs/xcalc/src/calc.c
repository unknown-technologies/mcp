#include <emulight/types.h>
#include <emulight/string.h>

#include "calc.h"

static CALC ctx;

char*	CALCNextToken(TOKEN* tok, char* input);
BOOL	CALCScan(CALC* calc);
BOOL	CALCPeek(CALC* calc, TOKEN* t);
BOOL	CALCParse(CALC* calc);
BOOL	CALCParseSum(CALC* calc, FLOAT* out);
BOOL	CALCParseProd(CALC* calc, FLOAT* out);
BOOL	CALCParseFactor(CALC* calc, FLOAT* out);
BOOL	CALCParseNumber(CALC* calc, FLOAT* out);

static const CONST constants[CONST_CNT] = {
	{ .name = "pi", .value = {
		.bits = { 0x31, 0x41, 0x59, 0x26, 0x53, 0x58, 0x97, 0x93,
			  0x23, 0x84, 0x62, 0x64, 0x33, 0x83, 0x27, 0x95 },
		.sign = BCDFLOAT_POS,
		.exp = 0
	} },
	{ .name = "e", .value = {
		.bits = { 0x27, 0x18, 0x28, 0x18, 0x28, 0x45, 0x90, 0x45,
			  0x23, 0x53, 0x60, 0x28, 0x74, 0x71, 0x35, 0x27 },
		.sign = BCDFLOAT_POS,
		.exp = 0
	} }
};

BOOL CALCIsNull(char* name)
{
	for(unsigned int i = 0; i < VAR_NAMESZ; i++) {
		if(name[i]) {
			return FALSE;
		}
	}

	return TRUE;
}

BOOL CALCIsEqual(const char* name, const char* varname, int len)
{
	for(unsigned int j = 0; j < VAR_NAMESZ; j++) {
		if(j < len && name[j] != varname[j]) {
			return FALSE;
		}
	}
	return TRUE;
}

VAR* CALCFindAssign(CALC* calc, const char* name, int len)
{
	VAR* unused = NULL;
	for(unsigned int i = 0; i < VAR_CNT; i++) {
		VAR* var = &calc->variable[i];
		if(CALCIsEqual(name, var->name, len)) {
			return var;
		} else if(!unused && CALCIsNull(var->name)) {
			unused = var;
		}
	}

	return unused;
}

VAR* CALCFind(CALC* calc, const char* name, int len)
{
	for(unsigned int i = 0; i < VAR_CNT; i++) {
		VAR* var = &calc->variable[i];
		if(CALCIsEqual(name, var->name, len)) {
			return var;
		}
	}

	return NULL;
}

BOOL CALCIsIdentStart(char c)
{
	if(c >= 'a' && c <= 'z') {
		return TRUE;
	}
	if(c >= 'A' && c <= 'Z') {
		return TRUE;
	}
	if(c == '_') {
		return TRUE;
	}

	return FALSE;
}

BOOL CALCIsIdent(char c)
{
	if(CALCIsIdentStart(c)) {
		return TRUE;
	}
	if(c >= '0' && c <= '9') {
		return TRUE;
	}

	return FALSE;
}

char* CALCNextToken(TOKEN* tok, char* input)
{
	char* c = input;
	while(*c == ' ') {
		c++;
	}

	tok->value = NULL;
	tok->len = 0;

	if(!*c) {
		tok->type = TOKEN_EOF;
		return NULL;
	}

	switch(*c) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			tok->type = TOKEN_NUMBER;
			tok->value = c;
			while(*c >= '0' && *c <= '9') {
				c++;
			}
			if(*c == '.') {
				c++;
			}
			if(*c == 'e' || *c == 'E') {
				c++;
			}
			if(*c == '-') {
				c++;
			}
			while(*c >= '0' && *c <= '9') {
				c++;
			}
			tok->len = (int) (c - tok->value);
			return c;
		case '.':
			tok->type = TOKEN_DECIMAL;
			return c + 1;
		case '+':
			tok->type = TOKEN_ADD;
			return c + 1;
		case '-':
			tok->type = TOKEN_SUB;
			return c + 1;
		case '*':
			tok->type = TOKEN_MUL;
			return c + 1;
		case '/':
			tok->type = TOKEN_DIV;
			return c + 1;
		case '(':
			tok->type = TOKEN_LPAR;
			return c + 1;
		case ')':
			tok->type = TOKEN_RPAR;
			return c + 1;
		case '=':
			tok->type = TOKEN_EQ;
			return c + 1;
		default:
			if(CALCIsIdentStart(*c)) {
				tok->type = TOKEN_IDENT;
				tok->value = c;
				while(CALCIsIdent(*c)) {
					c++;
				}
				tok->len = (int) (c - tok->value);
				return c;
			} else {
				tok->type = TOKEN_ERROR;
				tok->value = c;
				return NULL;
			}
	}
}

BOOL CALCScan(CALC* calc)
{
	if(calc->input) {
		memcpy(&calc->t, &calc->la, sizeof(TOKEN));
		calc->input = CALCNextToken(&calc->la, calc->input);
		return calc->input != NULL;
	} else {
		return FALSE;
	}
}

BOOL CALCPeek(CALC* calc, TOKEN* t)
{
	if(calc->input) {
		return CALCNextToken(t, calc->input) != NULL;
	} else {
		return FALSE;
	}
}

BOOL CALCExpect(CALC* calc, int type)
{
	return calc->la.type == type;
}

BOOL CALCParse(CALC* calc)
{
	switch(calc->la.type) {
		case TOKEN_ADD:
		case TOKEN_SUB:
		case TOKEN_NUMBER:
		case TOKEN_DECIMAL:
		case TOKEN_LPAR:
			if(!CALCParseSum(calc, &calc->result)) {
				return FALSE;
			}
			break;
		case TOKEN_IDENT: {
			TOKEN la;
			CALCPeek(calc, &la);
			if(la.type == TOKEN_EQ) {
				/* assignment */
				CALCScan(calc);

				/* allocate variable */
				VAR* var = CALCFindAssign(calc,
						calc->t.value, calc->t.len);
				if(!var) {
					return FALSE;
				}

				if(calc->t.len > VAR_NAMESZ) {
					return FALSE;
				}

				bzero(var->name, sizeof(VAR_NAMESZ));
				memcpy(var->name, calc->t.value, calc->t.len);

				CALCScan(calc);

				if(!CALCParseSum(calc, &calc->result)) {
					return FALSE;
				}

				memcpy(&var->value, &calc->result,
						sizeof(FLOAT));
			} else if(!CALCParseSum(calc, &calc->result)) {
				return FALSE;
			}
			break;
		}
	}

	return CALCExpect(calc, TOKEN_EOF);
}

BOOL CALCParseSum(CALC* calc, FLOAT* out)
{
	FLOAT temp;

	if(!CALCParseProd(calc, out)) {
		return FALSE;
	}

	while(calc->la.type == TOKEN_ADD || calc->la.type == TOKEN_SUB) {
		switch(calc->la.type) {
			case TOKEN_ADD:
				CALCScan(calc);

				if(!CALCParseProd(calc, &temp)) {
					return FALSE;
				}

				FAdd2(out, &temp);
				break;
			case TOKEN_SUB:
				CALCScan(calc);

				if(!CALCParseProd(calc, &temp)) {
					return FALSE;
				}

				FSub2(out, &temp);
				break;
		}
	}

	return TRUE;
}

BOOL CALCParseProd(CALC* calc, FLOAT* out)
{
	FLOAT temp;

	if(!CALCParseFactor(calc, out)) {
		return FALSE;
	}

	while(calc->la.type == TOKEN_MUL || calc->la.type == TOKEN_DIV) {
		switch(calc->la.type) {
			case TOKEN_MUL:
				CALCScan(calc);

				if(!CALCParseFactor(calc, &temp)) {
					return FALSE;
				}

				FMul2(out, &temp);
				break;
			case TOKEN_DIV:
				CALCScan(calc);

				if(!CALCParseFactor(calc, &temp)) {
					return FALSE;
				}

				if(FIsZero(&temp)) {
					return FALSE;
				}

				FDiv2(out, &temp);
				break;
		}
	}

	return TRUE;
}

BOOL CALCParseFactor(CALC* calc, FLOAT* out)
{
	switch(calc->la.type) {
		case TOKEN_ADD:
		case TOKEN_SUB:
		case TOKEN_NUMBER:
			return CALCParseNumber(calc, out);
		case TOKEN_LPAR:
			CALCScan(calc);
			if(!CALCParseSum(calc, out)) {
				return FALSE;
			}
			if(!CALCExpect(calc, TOKEN_RPAR)) {
				return FALSE;
			}
			CALCScan(calc);
			return TRUE;
		case TOKEN_IDENT: {
			CALCScan(calc);
			VAR* var = CALCFind(calc, calc->t.value,
					calc->t.len);
			if(!var) {
				return FALSE;
			}
			memcpy(out, &var->value, sizeof(FLOAT));
			return TRUE;
		}
		default:
			return FALSE;
	}
}

BOOL CALCParseNumber(CALC* calc, FLOAT* out)
{
	BOOL negative = FALSE;
	switch(calc->la.type) {
		case TOKEN_ADD:
			negative = FALSE;
			CALCScan(calc);
			break;
		case TOKEN_SUB:
			negative = TRUE;
			CALCScan(calc);
			break;
	}

	if(calc->la.type == TOKEN_NUMBER) {
		CALCScan(calc);
		if(!S2F(out, calc->t.value, calc->t.len)) {
			return FALSE;
		}
	} else {
		return FALSE;
	}

	if(negative) {
		FNeg(out);
	}

	return TRUE;
}

void CALCInitContext(void)
{
	bzero(&ctx, sizeof(CALC));

	/* set up constants */
	for(unsigned int i = 0; i < CONST_CNT; i++) {
		const CONST* c = &constants[i];
		VAR* var = &ctx.variable[i];
		bzero(var->name, VAR_NAMESZ);
		memcpy(var->name, c->name, strlen(c->name));
		memcpy(&var->value, &c->value, sizeof(FLOAT));
	}
}

BOOL CALCEvaluate(char* input)
{
	ctx.input = input;
	CALCScan(&ctx);
	return CALCParse(&ctx);
}

void CALCGetResult(FLOAT* out)
{
	memcpy(out, &ctx.result, sizeof(FLOAT));
}
