#ifndef __LINKER_H__
#define __LINKER_H__

#include "types.h"
#include "aout.h"
#include "map.h"

typedef struct MODULE MODULE;
typedef struct SYMBOL SYMBOL;

struct MODULE {
	MODULE*		next;
	SYMBOL*		symbols;
	const char*	name;
	AOUT*		aout;
	u16		textptr;
	u16		dataptr;
	u16		bssptr;
	BOOL		visited;
	BOOL		keep;
};

struct SYMBOL {
	SYMBOL*		next;
	const char*	name;
	MODULE*		module;
	u16		type;
	u16		value;
};

typedef struct {
	MAP		symbols;
	MODULE*		modules;
	SYMBOL*		entry;
	unsigned int	module_count;
	BOOL		unresolved;
} LINKER;

void*	LDLoadFile(const char* filename);

void	LDInit(LINKER* linker);
void	LDDestroy(LINKER* linker);

BOOL	LDLoad(LINKER* linker, const char* filename);
BOOL	LDCollect(LINKER* linker, const char* entry);
void*	LDLink(LINKER* linker, size_t* size);
void	LDPrintMap(LINKER* linker, FILE* out);

#endif
