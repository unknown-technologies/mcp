#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include "linker.h"

void* LDLoadFile(const char* filename)
{
	struct stat buf;
	if(stat(filename, &buf) == -1) {
		printf("Failed to stat %s: %s\n", filename, strerror(errno));
		return NULL;
	}

	void* data = malloc(buf.st_size);
	if(!data) {
		printf("Failed to allocate memory: %s\n", strerror(errno));
		return NULL;
	}

	FILE* file = fopen(filename, "rb");
	if(!file) {
		printf("Failed to open file %s: %s\n", filename, strerror(errno));
		free(data);
		return NULL;
	}

	if(fread(data, buf.st_size, 1, file) != 1) {
		printf("Failed to read file %s: %s\n", filename, strerror(errno));
		fclose(file);
		free(data);
		return NULL;
	}

	return data;
}

void LDInit(LINKER* linker)
{
	memset(linker, 0, sizeof(LINKER));

	MAPInit(&linker->symbols);
}

void LDDestroy(LINKER* linker)
{
	MODULE* module = linker->modules;
	while(module) {
		MODULE* next = module->next;

		/* free module code */
		free(module->aout);

		/* free symbols */
		SYMBOL* symbol = module->symbols;
		while(symbol) {
			SYMBOL* sym = symbol->next;
			free(symbol);
			symbol = sym;
		}

		/* free module itself */
		free(module);
		module = next;
	}

	MAPDestroy(&linker->symbols);
}

BOOL LDLoad(LINKER* linker, const char* filename)
{
	void* file = LDLoadFile(filename);
	if(!file) {
		return FALSE;
	}

	AOUT* aout = (AOUT*) file;

	MODULE* module = (MODULE*) malloc(sizeof(MODULE));
	memset(module, 0, sizeof(MODULE));
	module->name = filename;
	module->aout = aout;

	if(linker->modules) {
		module->next = linker->modules;
		linker->modules = module;
	} else {
		linker->modules = module;
	}
	linker->module_count++;

	SYMTAB symtab;
	AOUTGetSymtab(aout, &symtab);

	SYM* sym = symtab.sym;
	for(unsigned int i = 0; i < U16L(aout->symbols); i += 8, sym++) {
		const char* symname = &symtab.names[sym->offset];

		if(sym->type != 000 && sym->type != 040 && sym->type != 037) {
			SYMBOL* symbol = (SYMBOL*) malloc(sizeof(SYMBOL));
			memset(symbol, 0, sizeof(SYMBOL));
			symbol->module = module;
			symbol->type = sym->type;
			symbol->value = sym->value;
			symbol->name = symname;

			symbol->next = module->symbols;
			module->symbols = symbol;

			if(sym->type > 4) {
				/* only add globl symbols */
				if(!MAPPut(&linker->symbols, symname, symbol)) {
					printf("Symbol '%s' redefined\n", symname);
					return FALSE;
				}
			}
		}
	}

	return TRUE;
}

void LDCollectRecursive(LINKER* linker, MODULE* module)
{
	if(module->visited) {
		return;
	}

	module->visited = TRUE;
	module->keep = TRUE;

	AOUT* aout = module->aout;
	SYMTAB symtab;
	AOUTGetSymtab(aout, &symtab);

	SYM* sym = symtab.sym;
	for(unsigned int i = 0; i < U16L(aout->symbols); i += 8, sym++) {
		const char* symname = &symtab.names[sym->offset];

		if(sym->type == 000 || sym->type == 040) {
			/* external symbol */
			SYMBOL* symbol = MAPGet(&linker->symbols, symname);
			if(symbol) {
				LDCollectRecursive(linker, symbol->module);
			} else if(strcmp(symname, "_reltab") && strcmp(symname, "_relcnt")) {
				linker->unresolved = TRUE;
				printf("Unresolved reference: %s required by %s\n",
						symname, module->name);
			}
		}
	}
}

BOOL LDCollect(LINKER* linker, const char* entry)
{
	linker->unresolved = FALSE;

	SYMBOL* sym = (SYMBOL*) MAPGet(&linker->symbols, entry);
	if(sym == NULL) {
		printf("Entrypoint %s not found\n", entry);
		linker->unresolved = TRUE;
		return FALSE;
	}

	for(MODULE* m = linker->modules; m; m = m->next) {
		m->keep = FALSE;
	}

	MODULE* main = sym->module;
	main->keep = TRUE;
	LDCollectRecursive(linker, main);

	if(linker->unresolved) {
		return FALSE;
	}

	for(MODULE* m = linker->modules; m; m = m->next) {
		if(!m->keep) {
			printf("Discarding module %s\n", m->name);
		}
	}

	linker->entry = sym;

	return TRUE;
}

MODULE* LDCreateRelocModule(LINKER* linker, size_t relocs)
{
	size_t reloc_size = (relocs + 1) * 2;
	size_t relocsym_size = sizeof(SYM) + 4 + strlen("_reltab") + 1;

	MODULE* relmod = (MODULE*) malloc(sizeof(MODULE));
	relmod->name = "<relocations>";
	relmod->next = NULL;
	relmod->keep = TRUE;
	relmod->visited = TRUE;
	relmod->symbols = (SYMBOL*) malloc(sizeof(SYMBOL));
	relmod->symbols->next = NULL;
	relmod->symbols->module = relmod;
	relmod->symbols->name = "_reltab";
	relmod->symbols->type = 043;
	relmod->symbols->value = 0;

	relmod->aout = (AOUT*) malloc(sizeof(AOUT) + reloc_size + relocsym_size);
	memset(relmod->aout, 0, sizeof(AOUT) + reloc_size + relocsym_size);
	relmod->aout->text = U16L(0);
	relmod->aout->data = U16L(reloc_size);
	relmod->aout->bss = U16L(0);
	relmod->aout->symbols = U16L(sizeof(SYM));
	relmod->aout->flags = U16L(1);

	SYM* sym = (SYM*) AOUTGetSymbolTable(relmod->aout);
	memset(sym, 0, sizeof(SYM));
	sym->module = 0;
	sym->offset = 4;
	sym->type = 043;
	sym->value = 0;

	size_t namelen = strlen("_reltab") + 1;
	u16* names = (u16*) ((uintptr_t) sym + U16L(relmod->aout->symbols));
	names[0] = U16L((u16) (namelen >> 16));
	names[1] = U16L((u16) namelen);
	strcpy((char*) &names[2], "_reltab");

	return relmod;
}

void* LDLink(LINKER* linker, size_t* size)
{
	size_t text_size = 0;
	size_t data_size = 0;
	size_t bss_size  = 0;

	if(!linker->entry) {
		printf("Entry point not defined, aborting\n");
		return NULL;
	}

	unsigned int relocs = 0;

	MODULE* last_module = NULL;
	for(MODULE* module = linker->modules; module; module = module->next) {
		last_module = module;
		if(!module->visited || !module->keep) {
			continue;
		}

		text_size += U16L(module->aout->text);
		data_size += U16L(module->aout->data);
		bss_size  += U16L(module->aout->bss);

		AOUT* aout = module->aout;

		if(module == linker->entry->module) {
			continue;
		}

		u16* textrels = AOUTGetTextRels(aout);
		u16* datarels = AOUTGetDataRels(aout);

		if(textrels) {
			for(unsigned int i = 0; i < U16L(aout->text) / 2; i++) {
				u16 rel = textrels[i];

				if(rel == 0) {
					continue;
				}

				BOOL pcrel = rel & 1;

				if(!pcrel) {
					relocs++;
				}
			}
		}

		if(datarels) {
			for(unsigned int i = 0; i < U16L(aout->data) / 2; i++) {
				u16 rel = datarels[i];

				if(rel == 0) {
					continue;
				}

				BOOL pcrel = rel & 1;

				if(!pcrel) {
					relocs++;
				}
			}
		}
	}

	size_t reloc_size = (relocs + 1) * 2;

	size_t total = text_size + data_size + bss_size + reloc_size;

	if(total > 0xFFFF) {
		printf("Image does not fit into memory\n");
		return NULL;
	}

	printf("text: %06lo, data: %06lo, bss: %06lo, total: %06lo; relocs: %d\n",
			text_size, data_size, bss_size, total, relocs);

	/* create synthetic reloc module */
	MODULE* relmod = LDCreateRelocModule(linker, relocs);
	last_module->next = relmod;
	relmod->next = NULL;
	MAPPut(&linker->symbols, relmod->symbols->name, relmod->symbols);

	data_size += U16L(relmod->aout->data);
	*size = text_size + data_size;

	u16 text = 0;
	u16 data = (u16) text_size;
	u16 bss = data + (u16) data_size;

	/* layout modules in memory, entry point at 0 */
	linker->entry->module->textptr = text;
	linker->entry->module->dataptr = data;
	linker->entry->module->bssptr  = bss;

	text += U16L(linker->entry->module->aout->text);
	data += U16L(linker->entry->module->aout->data);
	bss  += U16L(linker->entry->module->aout->bss);

	u16* reloc = (u16*) AOUTGetData(relmod->aout);

	*(reloc++) = (u16) relocs;

	/* compute size and store relocs */
	for(MODULE* module = linker->modules; module; module = module->next) {
		if(!module->visited || !module->keep) {
			continue;
		}

		/* skip entrypoint module */
		if(module == linker->entry->module) {
			continue;
		}

		module->textptr = text;
		module->dataptr = data;
		module->bssptr  = bss;

		AOUT* aout = module->aout;
		text += U16L(aout->text);
		data += U16L(aout->data);
		bss  += U16L(aout->bss);

		u16* textrels = AOUTGetTextRels(aout);
		u16* datarels = AOUTGetDataRels(aout);
		if(textrels) {
			for(unsigned int i = 0; i < U16L(aout->text) / 2; i++) {
				u16 pc = module->textptr + (i << 1);
				u16 rel = textrels[i];

				if(rel == 0) {
					continue;
				}

				BOOL pcrel = rel & 1;

				if(!pcrel) {
					*(reloc++) = pc;
				}
			}
		}

		if(datarels) {
			for(unsigned int i = 0; i < U16L(aout->data) / 2; i++) {
				u16 pc = module->dataptr + (i << 1);
				u16 rel = datarels[i];

				if(rel == 0) {
					continue;
				}

				BOOL pcrel = rel & 1;

				if(!pcrel) {
					*(reloc++) = pc;
				}
			}
		}
	}

	/* update symbols */
	for(MODULE* module = linker->modules; module; module = module->next) {
		if(!module->visited || !module->keep) {
			continue;
		}

		AOUT* aout = module->aout;

		/* update linked symbols */
		for(SYMBOL* sym = module->symbols; sym; sym = sym->next) {
			switch(sym->type) {
				case 001:	/* absolute symbol */
				case 041:	/* absolute external symbol */
					break;
				case 002:	/* text segment symbol */
				case 042:	/* text segment external symbol */
					sym->value += module->textptr;
					break;
				case 003:	/* data segment symbol */
				case 043:	/* data segment external symbol */
					sym->value += module->dataptr - U16L(aout->text);
					break;
				case 004:	/* bss segment symbol */
				case 044:	/* bss segment external symbol */
					sym->value += module->bssptr - U16L(aout->text) - U16L(aout->data);
					break;
			}
		}
	}

	/* resolve externals */
	for(MODULE* module = linker->modules; module; module = module->next) {
		if(!module->visited || !module->keep) {
			continue;
		}

		AOUT* aout = module->aout;
		SYMTAB symtab;
		AOUTGetSymtab(aout, &symtab);

		SYM* sym = symtab.sym;
		for(unsigned int i = 0; i < U16L(aout->symbols); i += 8, sym++) {
			const char* symname = &symtab.names[sym->offset];
			if(sym->type == 000 || sym->type == 040) {
				SYMBOL* s = MAPGet(&linker->symbols, symname);
				if(!s) {
					printf("Error: failed to get external symbol '%s', this should not happen\n", symname);
					return NULL;
				}
				sym->value = s->value;
			}
		}
	}

	u8* image = (u8*) malloc(text_size + data_size);
	if(!image) {
		printf("Failed to allocate memory: %s\n", strerror(errno));
		return NULL;
	}

	/* copy code and data to memory and process relocations */
	for(MODULE* module = linker->modules; module; module = module->next) {
		if(!module->visited || !module->keep) {
			continue;
		}

		AOUT* aout = module->aout;

		u16* textptr = (u16*) (image + module->textptr);
		u16* dataptr = (u16*) (image + module->dataptr);

		if(U16L(aout->text)) {
			memcpy(textptr, AOUTGetText(aout), U16L(aout->text));
		}

		if(U16L(aout->data)) {
			memcpy(dataptr, AOUTGetData(aout), U16L(aout->data));
		}

		u16* textrels = AOUTGetTextRels(aout);
		u16* datarels = AOUTGetDataRels(aout);

		if(!textrels || !datarels) {
			continue;
		}

		SYMTAB symtab;
		AOUTGetSymtab(aout, &symtab);

		SYM* syms = symtab.sym;

#ifdef _DEBUG
		printf("MODULE: %s TEXT=%06o@%06o DATA=%06o@%06o\n", module->name,
				U16L(aout->text), module->textptr,
				U16L(aout->data), module->dataptr);
#endif

		for(unsigned int i = 0; i < U16L(aout->text) / 2; i++) {
			u16* text = &textptr[i];
			u16 rel = textrels[i];

			if(rel == 0) {
				continue;
			}

			BOOL pcrel = rel & 1;
			u16 type = (rel >> 1) & 7;
			u16 id = rel >> 4;

			SYM* sym = &syms[id];

#ifdef _DEBUG
			u16 pc = module->textptr + (i << 1);
			const char* symname = &symtab.names[sym->offset];
			char t = '?';
			switch(type) {
				case 0:
					t = 'A';
					break;
				case 1:
					t = 'T';
					break;
				case 2:
					t = 'D';
					break;
				case 3:
					t = 'B';
					break;
				case 4:
					t = 'U';
					break;
			}

			printf("PC=%06o [%06o] REL=%06o %06o => %c %s = %06o\n", pc, i << 1, rel, *text, t, symname, sym->value);
#endif

			/* perform relocation */
			switch(type) {
				case 0: /* absolute */
					*text = sym->value;
					break;
				case 1: /* text */
					*text += module->textptr;
					break;
				case 2: /* data */
					*text += module->dataptr - U16L(aout->text);
					break;
				case 3: /* bss */
					*text += module->bssptr - U16L(aout->text) - U16L(aout->data);
					break;
				case 4: /* external */
					*text += sym->value;
					break;
			}

			if(pcrel) {
				*text -= module->textptr;
			}
		}

	}

	return image;
}

static int symsort(const void* a, const void* b)
{
	SYMBOL* sym_a = *(SYMBOL**) a;
	SYMBOL* sym_b = *(SYMBOL**) b;
	return (int) sym_a->value - (int) sym_b->value;
}

void LDPrintMap(LINKER* linker, FILE* out)
{
	/* print symbols */
	unsigned int symcnt = 0;
	for(MODULE* module = linker->modules; module; module = module->next) {
		if(!module->visited || !module->keep) {
			continue;
		}

		for(SYMBOL* sym = module->symbols; sym; sym = sym->next) {
			symcnt++;
		}
	}

	SYMBOL** symbols = (SYMBOL**) malloc(symcnt * sizeof(SYMBOL*));
	unsigned int i = 0;
	for(MODULE* module = linker->modules; module; module = module->next) {
		if(!module->visited || !module->keep) {
			continue;
		}

		for(SYMBOL* sym = module->symbols; sym; sym = sym->next) {
			symbols[i++] = sym;
		}
	}

	qsort(symbols, symcnt, sizeof(SYMBOL*), symsort);

	for(unsigned int i = 0; i < symcnt; i++) {
		SYMBOL* sym = symbols[i];
		char t;
		switch(sym->type) {
			case 000:	/* undefined symbol */
				t = 'U';
				break;
			case 037:	/* file name symbol (produced by ld) */
				t = 'F';
				break;
			case 040:	/* undefined external (.globl) symbol */
				t = 'u';
				break;
			case 002:	/* text segment symbol */
				t = 'T';
				break;
			case 001:	/* absolute symbol */
				t = 'A';
				break;
			case 041:	/* absolute external symbol */
				t = 'a';
				break;
			case 042:	/* text segment external symbol */
				t = 't';
				break;
			case 003:	/* data segment symbol */
				t = 'D';
				break;
			case 004:	/* bss segment symbol */
				t = 'B';
				break;
			case 043:	/* data segment external symbol */
				t = 'd';
				break;
			case 044:	/* bss segment external symbol */
				t = 'b';
				break;
			default:
				t = '?';
				break;
		}
		fprintf(out, "%06o %c %s\n", sym->value, t, sym->name);
	}
	free(symbols);
}
