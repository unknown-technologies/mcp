#ifndef __PROGRAM_H__
#define __PROGRAM_H__

#include <emulight/types.h>

#define	MAX_LIBRARIES	4
#define	MAX_SYMBOL_LEN	32

typedef struct {
	unsigned int	magic;
	unsigned int	image_size;
	unsigned int	symbols;
	unsigned int	relocs;
} OSProgram;

typedef struct {
	const char*	name;
	unsigned int	type;
	void*		value;
} OSSymbol;

typedef struct {
	unsigned int	name;
	unsigned int	relocs;
} OSImportSymbol;

typedef struct {
	u16		name[3];
	u16		refcnt;
	OSProgram*	library;
	OSSymbol*	symbols;
} OSLibrary;

void	OSInitLibrary(void);
BOOL	OSLoadLibrary(const char* name, const char* dsn);
void	OSFreeLibrary(OSLibrary* lib);
void*	OSLoadProgram(const char* dsn);
void	OSUnloadProgram(void* program);
void*	OSLoadModule(const char* name);
void	OSUnloadModule(void* module);

OSLibrary* OSGetLibrary(const char* name);
OSLibrary* OSCreateLibrary(const char* name);
void	OSFreeLibrary(OSLibrary* lib);
BOOL	OSResolveImports(int ds, void* prog, void* base);
void*	OSiLoadProgram(const char* dsn, OSLibrary* lib, BOOL bottom);

#endif
