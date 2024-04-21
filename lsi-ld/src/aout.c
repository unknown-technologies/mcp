#include <string.h>
#include <stdio.h>

#include "aout.h"

static inline const char* strip_underline(const char* name)
{
	if(*name == '_' || *name == '~') {
		return name + 1;
	} else {
		return name;
	}
}

const char* AOUTGetSymbol(u16 type, const char* name, char* t)
{
	switch(type) {
		case 000:		/* undefined symbol */
			*t = 'U';
			return strip_underline(name);
		case 037:		/* file name symbol (produced by ld) */
			*t = 'F';
			return name;
		case 040:		/* undefined external (.globl) symbol */
			*t = 'u';
			return strip_underline(name);
		default:
			*t = '?';
			return name;

		case 002:		/* text segment symbol */
			*t = 'T';
			if(*name == '/') {
				return name + 1;
			} else if(name[strlen(name) - 2] == '.'
					&& name[strlen(name) - 1] == 'o') {
				return name;
			} else {
				return strip_underline(name);
			}
		case 001:		/* absolute symbol */
			*t = 'A';
			return strip_underline(name);
		case 041:		/* absolute external symbol */
			*t = 'a';
			return strip_underline(name);
		case 042:		/* text segment external symbol */
			*t = 't';
			return strip_underline(name);
		case 003:		/* data segment symbol */
			*t = 'D';
			return strip_underline(name);
		case 004:		/* bss segment symbol */
			*t = 'B';
			return strip_underline(name);
		case 043:		/* data segment external symbol */
			*t = 'd';
			return strip_underline(name);
		case 044:		/* bss segment external symbol */
			*t = 'b';
			return strip_underline(name);
	}
}

void* AOUTGetSymbolTable(AOUT* aout)
{
	unsigned int offset = sizeof(AOUT) + U16L(aout->text)
	       + U16L(aout->data);

	/* reloc info? */
	if(!(U16L(aout->flags) & 1)) {
		offset += U16L(aout->text) + U16L(aout->data);
	}

	return (void*) (((u8*) aout) + offset);
}

void* AOUTGetText(AOUT* aout)
{
	unsigned int offset = sizeof(AOUT);

	return (void*) (((u8*) aout) + offset);
}

void* AOUTGetData(AOUT* aout)
{
	unsigned int offset = sizeof(AOUT) + U16L(aout->text);

	return (void*) (((u8*) aout) + offset);
}

u16* AOUTGetTextRels(AOUT* aout)
{
	unsigned int offset = sizeof(AOUT) + U16L(aout->text)
		+ U16L(aout->data);

	if(!(U16L(aout->flags) & 1)) {
		return (void*) (((u8*) aout) + offset);
	} else {
		return NULL;
	}
}

u16* AOUTGetDataRels(AOUT* aout)
{
	unsigned int offset = sizeof(AOUT) + U16L(aout->text)
		+ U16L(aout->data) + U16L(aout->text);

	if(!(U16L(aout->flags) & 1)) {
		return (void*) (((u8*) aout) + offset);
	} else {
		return NULL;
	}
}


BOOL AOUTIsShortSymtab(AOUT* aout)
{
	void* symtab = AOUTGetSymbolTable(aout);
	return *((u8*) symtab) != 0;
}

void AOUTGetSymtab(AOUT* aout, SYMTAB* symtab)
{
	if(U16L(aout->symbols) == 0) {
		symtab->namelen = 0;
		symtab->names = NULL;
		symtab->sym = NULL;
		symtab->is_shortsym = FALSE;
		return;
	}

	memset(symtab, 0, sizeof(SYMTAB));
	void* tab = AOUTGetSymbolTable(aout);
	symtab->is_shortsym = *((u8*) symtab) != 0;
	if(!symtab->is_shortsym) {
		u16* names = (u16*) ((uintptr_t) tab + U16L(aout->symbols));
		symtab->namelen = U16L(names[0]) << 16 | U16L(names[1]);
		symtab->names = (const char*) names;
		symtab->sym = (SYM*) tab;
	} else {
		symtab->shortsym = (SHORTSYM*) tab;
	}
}

void* AOUTFindSymbol(const char* name)
{
	return NULL;
}
