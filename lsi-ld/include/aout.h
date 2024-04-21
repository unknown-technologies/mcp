#ifndef __AOUT_H__
#define __AOUT_H__

#include "types.h"

typedef struct {
	u16		magic;
	u16		text;
	u16		data;
	u16		bss;
	u16		symbols;
	u16		entry;
	u16		unused;
	u16		flags;
} AOUT;

typedef struct {
	u16		module;
	u16		offset;
	u16		type;
	u16		value;
} SYM;

typedef struct {
	char		name[8];
	u16		type;
	u16		value;
} SHORTSYM;

typedef struct {
	u32		namelen;
	const char*	names;
	union {
		SYM*	sym;
		SHORTSYM* shortsym;
	};
	BOOL		is_shortsym;
} SYMTAB;

void*		AOUTGetText(AOUT* aout);
void*		AOUTGetData(AOUT* aout);
u16*		AOUTGetTextRels(AOUT* aout);
u16*		AOUTGetDataRels(AOUT* aout);

const char*	AOUTGetSymbol(u16 type, const char* name, char* t);
BOOL		AOUTIsShortSymtab(AOUT* aout);
void		AOUTGetSymtab(AOUT* aout, SYMTAB* symtab);
void*		AOUTGetSymbolTable(AOUT* aout);

#endif
