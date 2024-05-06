#ifndef __PROGRAM_H__
#define __PROGRAM_H__

#include <emulight/types.h>

#define	MAX_LIBRARIES	4

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
	OSProgram*	library;
	OSSymbol*	symbols;
} OSLibrary;

void	OSInitLibrary(void);
BOOL	OSLoadLibrary(const char* name, const char* dsn);
void*	OSLoadProgram(const char* name, const char* dsn);

#endif
