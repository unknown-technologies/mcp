#ifndef __LINKER_H__
#define __LINKER_H__

#include "types.h"
#include "aout.h"
#include "map.h"

#define	LD_TYPE_EXEC	0
#define	LD_TYPE_LIB	1
#define	LD_TYPE_DYN	2

#define	PROG_MAGIC	0x0301

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
	u16		magic;
	u16		image_size;
	u16		symbols;
	u16		relocs;
} PROGRAM;

typedef struct {
	u16		name;
	u16		type;
	u16		value;
} PROGSYM;

typedef struct {
	PROGSYM*	symptr;
	char*		strptr;
	unsigned int	stroff;
	unsigned int	symbols;
	unsigned int	namelen;
} SYMINFO;

typedef struct {
	u16		name;
	u16		relocs;
} IMPORT;

typedef struct {
	MAP		undefined;
	MAP		defined;
	IMPORT*		import;
	void*		ptr;
} IMPORTTAB;

typedef struct {
	MAP		symbols;
	MODULE*		modules;
	SYMBOL*		entry;
	unsigned int	module_count;
	unsigned int	type;
	BOOL		unresolved;
	IMPORTTAB	import;
} LINKER;

void*	LDLoadFile(const char* filename);

void	LDInit(LINKER* linker, unsigned int type);
void	LDDestroy(LINKER* linker);

BOOL	LDLoad(LINKER* linker, const char* filename);
BOOL	LDCollect(LINKER* linker, const char* entry);
BOOL	LDCollectLibrary(LINKER* linker);
void*	LDLink(LINKER* linker, size_t* size);
void*	LDLinkBinary(LINKER* linker, size_t* size);
void	LDPrintMap(LINKER* linker, FILE* out);

#endif
