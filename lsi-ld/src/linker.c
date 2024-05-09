#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include "linker.h"

static const char* symtab_name = "___symtab";
static const char* reltab_name = "___reltab";

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

void LDInit(LINKER* linker, unsigned int type)
{
	memset(linker, 0, sizeof(LINKER));
	linker->type = type;

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

	BOOL allow_unresolved = linker->type != LD_TYPE_EXEC;

	SYM* sym = symtab.sym;
	for(unsigned int i = 0; i < U16L(aout->symbols); i += 8, sym++) {
		const char* symname = &symtab.names[sym->offset];

		if(sym->type == 000 || sym->type == 040) {
			/* external symbol */
			SYMBOL* symbol = MAPGet(&linker->symbols, symname);
			if(symbol) {
				LDCollectRecursive(linker, symbol->module);
			} else if(strcmp(symname, reltab_name) && strcmp(symname, symtab_name)) {
				linker->unresolved = TRUE;
				if(!allow_unresolved) {
					printf("Unresolved reference: %s required by %s\n",
							symname, module->name);
				}
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

	if(linker->unresolved && linker->type == LD_TYPE_EXEC) {
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

BOOL LDCollectLibrary(LINKER* linker)
{
	linker->unresolved = FALSE;

	for(MODULE* module = linker->modules; module; module = module->next) {
		module->keep = TRUE;
		module->visited = TRUE;

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
				} else {
					linker->unresolved = TRUE;
				}
			}
		}
	}

	return TRUE;
}

MODULE* LDCreateRelocModule(LINKER* linker, size_t relocs)
{
	size_t reloc_size = (relocs + 1) * 2;
	size_t relocsym_size = sizeof(SYM) + 4 + strlen(reltab_name) + 1;
	if(relocsym_size & 1) {
		relocsym_size++;
	}

	MODULE* relmod = (MODULE*) malloc(sizeof(MODULE));
	memset(relmod, 0, sizeof(MODULE));
	relmod->name = "<relocations>";
	relmod->next = NULL;
	relmod->keep = TRUE;
	relmod->visited = TRUE;
	relmod->symbols = (SYMBOL*) malloc(sizeof(SYMBOL));
	relmod->symbols->next = NULL;
	relmod->symbols->module = relmod;
	relmod->symbols->name = reltab_name;
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

	size_t namelen = strlen(reltab_name) + 1;
	u16* names = (u16*) ((uintptr_t) sym + U16L(relmod->aout->symbols));
	names[0] = U16L((u16) (namelen >> 16));
	names[1] = U16L((u16) namelen);
	strcpy((char*) &names[2], reltab_name);

	return relmod;
}

static void symtab_export_counter(const char* key, void* value, void* attachment)
{
	SYMINFO* info = (SYMINFO*) attachment;
	SYMBOL* sym = (SYMBOL*) value;
	if(sym->type > 040 && strcmp(key, reltab_name) && strcmp(key, symtab_name)) {
		info->symbols++;
		info->namelen += strlen(key) + 1;
	}
}

static void symtab_export_collector(const char* key, void* value, void* attachment)
{
	SYMINFO* info = (SYMINFO*) attachment;
	SYMBOL* sym = (SYMBOL*) value;
	if(sym->type > 040 && strcmp(key, reltab_name) && strcmp(key, symtab_name)) {
		info->symptr->name = info->stroff;
		info->symptr->type = sym->type;
		info->symptr->value = sym->value;
		strcpy(info->strptr, key);
		info->strptr += strlen(key) + 1;
		info->stroff += strlen(key) + 1;
		info->symptr++;
	}
}

MODULE* LDCreateSymtabModule(LINKER* linker, size_t* symcnt)
{
	SYMINFO exports = { 0 };

	MAPIterate(&linker->symbols, symtab_export_counter, &exports);

	size_t size = exports.symbols * sizeof(PROGSYM) + exports.namelen;
	size_t sym_size = sizeof(SYM) + 4 + strlen(symtab_name) + 1;

	if(size & 1) {
		size++;
	}
	if(sym_size & 1) {
		sym_size++;
	}

	MODULE* mod = (MODULE*) malloc(sizeof(MODULE));
	memset(mod, 0, sizeof(MODULE));
	mod->name = "<symbols>";
	mod->next = NULL;
	mod->keep = TRUE;
	mod->visited = TRUE;
	mod->symbols = NULL;

	mod->aout = (AOUT*) malloc(sizeof(AOUT) + size + sym_size);
	memset(mod->aout, 0, sizeof(AOUT) + size + sym_size);
	mod->aout->text = U16L(0);
	mod->aout->data = U16L(size);
	mod->aout->bss = U16L(0);
	mod->aout->symbols = U16L(sizeof(SYM));
	mod->aout->flags = U16L(1);

	mod->symbols = (SYMBOL*) malloc(sizeof(SYMBOL));
	mod->symbols->next = NULL;
	mod->symbols->module = mod;
	mod->symbols->name = symtab_name;
	mod->symbols->type = 043;
	mod->symbols->value = 0;

	exports.symptr = AOUTGetData(mod->aout);
	exports.stroff = exports.symbols * sizeof(PROGSYM);
	exports.strptr = (char*) (((uintptr_t) exports.symptr)
			+ exports.stroff);
	MAPIterate(&linker->symbols, symtab_export_collector, &exports);

	SYM* sym = (SYM*) AOUTGetSymbolTable(mod->aout);
	memset(sym, 0, sizeof(SYM));
	sym->module = 0;
	sym->offset = 4;
	sym->type = 043;
	sym->value = 0;

	size_t namelen = strlen(symtab_name) + 1;
	u16* names = (u16*) ((uintptr_t) sym + U16L(mod->aout->symbols));
	names[0] = U16L((u16) (namelen >> 16));
	names[1] = U16L((u16) namelen);
	strcpy((char*) &names[2], symtab_name);

	*symcnt = exports.symbols;

	return mod;
}

int export_symtab_sorter(const void* a, const void* b, void* arg)
{
	const PROGSYM* sym_a = (const PROGSYM*) a;
	const PROGSYM* sym_b = (const PROGSYM*) b;
	const char* names = (const char*) arg;

	const char* name_a = names + sym_a->name;
	const char* name_b = names + sym_b->name;

	return strcmp(name_a, name_b);
}

void LDUpdateSymtabModule(LINKER* linker, MODULE* mod, size_t symcnt)
{
	SYMINFO exports = { 0 };

	exports.symptr = AOUTGetData(mod->aout);
	exports.stroff = symcnt * sizeof(PROGSYM);
	exports.strptr = (char*) (((uintptr_t) exports.symptr)
			+ exports.stroff);
	MAPIterate(&linker->symbols, symtab_export_collector, &exports);

	/* sort table */
	qsort_r(AOUTGetData(mod->aout), symcnt, sizeof(PROGSYM),
			export_symtab_sorter, AOUTGetData(mod->aout));
}

static void imptab_add_collector(const char* key, void* value, void* attachment)
{
	LINKER* linker = (LINKER*) attachment;
	IMPORTTAB* tab = &linker->import;

	unsigned int relocs = (unsigned int) ((uintptr_t) value) - 1;
	size_t namelen = strlen(key) + 1;
	if(namelen & 1) {
		namelen++;
	}

	void* ptr = tab->ptr;
	IMPORT* imp = (IMPORT*) ptr;
	imp->name = (u16) namelen;
	imp->relocs = (u16) relocs;

	size_t size = sizeof(IMPORT) + namelen + relocs * sizeof(u16);

	tab->ptr += size;

	strcpy((char*) (imp + 1), key);
	ptr = ptr + sizeof(IMPORT) + namelen;

	MAPPut(&tab->defined, key, ptr);
}

static void LDUpdateImport(LINKER* linker, const char* symname, u16 pc, BOOL pcrel)
{
	IMPORTTAB* tab = &linker->import;
	u16* imp = MAPGet(&tab->defined, symname);

	if(imp) {
		/* add import */
		u16 value = pc;
		if(pcrel) {
			value |= 1;
		}
		*imp = value;
		MAPPut(&tab->defined, symname, imp + 1);
	} else {
		printf("Error: import not found: %s\n", symname);
	}
}

MODULE* LDCreateImportModule(LINKER* linker)
{
	IMPORTTAB* tab = &linker->import;
	MAP* undefined = &tab->undefined;
	MAPInit(undefined);

	size_t symbols = 0;
	size_t strings = 0;
	size_t relocs = 0;

	/* find undefined symbols and count relocations against them */
	for(MODULE* module = linker->modules; module; module = module->next) {
		if(!module->visited || !module->keep) {
			continue;
		}

		AOUT* aout = module->aout;

		u16* textrels = AOUTGetTextRels(aout);
		u16* datarels = AOUTGetDataRels(aout);

		SYMTAB symtab;
		AOUTGetSymtab(aout, &symtab);

		SYM* sym = symtab.sym;
		for(unsigned int i = 0; i < U16L(aout->symbols); i += 8, sym++) {
			const char* symname = &symtab.names[sym->offset];
			if(sym->type == 000 || sym->type == 040) {
				SYMBOL* s = MAPGet(&linker->symbols, symname);
				if(!s) {
					void* val = MAPGet(undefined, symname);
					if(!val) {
						MAPPut(undefined, symname, (void*) 1);
						size_t len = strlen(symname) + 1;
						if(len & 1) {
							len++;
						}
						symbols++;
						strings += len;
					}
				}
			}
		}

		SYM* syms = symtab.sym;

		if(textrels) {
			for(unsigned int i = 0; i < U16L(aout->text) / 2; i++) {
				u16 rel = textrels[i];

				if(rel == 0) {
					continue;
				}

				u16 type = (rel >> 1) & 7;
				u16 id = rel >> 4;

				SYM* sym = &syms[id];
				const char* symname = &symtab.names[sym->offset];

				BOOL unresolved = type == 4
					&& (sym->type == 000 || sym->type == 040)
					&& MAPGet(&linker->symbols, symname) == NULL;

				if(unresolved) {
					void* val = MAPGet(undefined, symname);
					val++;
					MAPPut(undefined, symname, val);
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

				u16 type = (rel >> 1) & 7;
				u16 id = rel >> 4;

				SYM* sym = &syms[id];
				const char* symname = &symtab.names[sym->offset];

				BOOL unresolved = type == 4
					&& (sym->type == 000 || sym->type == 040)
					&& MAPGet(&linker->symbols, symname) == NULL;

				if(unresolved) {
					void* val = MAPGet(undefined, symname);
					val++;
					MAPPut(undefined, symname, val);
					relocs++;
				}
			}
		}
	}

	size_t size = symbols * sizeof(IMPORT) + strings + relocs * sizeof(u16) + sizeof(u16);
	if(size & 1) {
		size++;
	}

	MODULE* impmod = (MODULE*) malloc(sizeof(MODULE));
	memset(impmod, 0, sizeof(MODULE));
	impmod->name = "<imports>";
	impmod->next = NULL;
	impmod->keep = TRUE;
	impmod->visited = TRUE;
	impmod->symbols = NULL;

	impmod->aout = (AOUT*) malloc(sizeof(AOUT) + size);
	memset(impmod->aout, 0, sizeof(AOUT) + size);
	impmod->aout->text = U16L(0);
	impmod->aout->data = U16L((u16) size);
	impmod->aout->bss = U16L(0);
	impmod->aout->symbols = U16L(0);
	impmod->aout->flags = U16L(1);

	/* build import table */
	u16* data = AOUTGetData(impmod->aout);
	*data = symbols;
	tab->import = (IMPORT*) (data + 1);
	tab->ptr = tab->import;
	MAPInit(&tab->defined);

	/* layout import symbols */
	MAPIterate(&tab->undefined, imptab_add_collector, linker);

	return impmod;
}

void LDUpdateImportModule(LINKER* linker)
{
	for(MODULE* module = linker->modules; module; module = module->next) {
		if(!module->visited || !module->keep) {
			continue;
		}

		AOUT* aout = module->aout;

		u16* textrels = AOUTGetTextRels(aout);
		u16* datarels = AOUTGetDataRels(aout);

		SYMTAB symtab;
		AOUTGetSymtab(aout, &symtab);

		SYM* syms = symtab.sym;

		if(textrels) {
			for(unsigned int i = 0; i < U16L(aout->text) / 2; i++) {
				u16 rel = textrels[i];

				if(rel == 0) {
					continue;
				}

				BOOL pcrel = rel & 1;
				u16 type = (rel >> 1) & 7;
				u16 id = rel >> 4;

				SYM* sym = &syms[id];
				const char* symname = &symtab.names[sym->offset];
				u16 pc = module->textptr + (i << 1);

				BOOL unresolved = type == 4
					&& (sym->type == 000 || sym->type == 040)
					&& MAPGet(&linker->symbols, symname) == NULL;

				if(unresolved) {
					LDUpdateImport(linker, symname, pc, pcrel);
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
				u16 type = (rel >> 1) & 7;
				u16 id = rel >> 4;

				SYM* sym = &syms[id];
				const char* symname = &symtab.names[sym->offset];
				u16 pc = module->dataptr + (i << 1);

				BOOL unresolved = type == 4
					&& (sym->type == 000 || sym->type == 040)
					&& MAPGet(&linker->symbols, symname) == NULL;

				if(unresolved) {
					LDUpdateImport(linker, symname, pc, pcrel);
				}
			}
		}
	}

	MAPDestroy(&linker->import.defined);
	MAPDestroy(&linker->import.undefined);
}

void* LDLinkBinary(LINKER* linker, size_t* size)
{
	size_t text_size = 0;
	size_t data_size = 0;
	size_t bss_size  = 0;

	BOOL is_library = linker->type == LD_TYPE_LIB;

	if(!is_library && !linker->entry) {
		printf("Entry point not defined, aborting\n");
		return NULL;
	}

	BOOL zero_magic = FALSE;
	if(linker->entry) {
		/* is this a text symbol or a data symbol? */
		BOOL data;
		switch(linker->entry->type) {
			case 003:	/* data segment symbol */
			case 043:	/* data segment external symbol */
				data = TRUE;
				break;
			default:
				data = FALSE;
				break;
		}

		AOUT* aout = linker->entry->module->aout;
		if(data && U16L(aout->text) == 0) {
			/* if this is in data and sizeof(.text) = 0:
			 * swap .text and .data
			 */

			aout->text = aout->data;
			aout->data = 0;
			linker->entry->type--;
			zero_magic = TRUE;

			SYMTAB symtab;
			AOUTGetSymtab(aout, &symtab);

			SYM* sym = symtab.sym;
			for(unsigned int i = 0; i < U16L(aout->symbols); i += 8, sym++) {
				const char* symname = &symtab.names[sym->offset];
				if(!strcmp(symname, linker->entry->name)) {
					sym->type--;
					break;
				}
			}
		}
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

		u16* textrels = AOUTGetTextRels(aout);
		u16* datarels = AOUTGetDataRels(aout);

		SYMTAB symtab;
		AOUTGetSymtab(aout, &symtab);

		SYM* syms = symtab.sym;

		if(textrels) {
			for(unsigned int i = 0; i < U16L(aout->text) / 2; i++) {
				u16 rel = textrels[i];

				if(rel == 0) {
					continue;
				}

				BOOL pcrel = rel & 1;
				u16 type = (rel >> 1) & 7;
				u16 id = rel >> 4;

				SYM* sym = &syms[id];
				const char* symname = &symtab.names[sym->offset];

				BOOL unresolved = type == 4
					&& (sym->type == 000 || sym->type == 040)
					&& MAPGet(&linker->symbols, symname) == NULL;

				if(!unresolved && !pcrel) {
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
				u16 type = (rel >> 1) & 7;
				u16 id = rel >> 4;

				SYM* sym = &syms[id];
				const char* symname = &symtab.names[sym->offset];

				BOOL unresolved = type == 4
					&& (sym->type == 000 || sym->type == 040)
					&& MAPGet(&linker->symbols, symname) == NULL;

				if(!unresolved && !pcrel) {
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

	/* create synthetic symbol table module */
	size_t symcnt = 0;
	MODULE* symmod = NULL;
	if(linker->type != LD_TYPE_EXEC) {
		symmod = LDCreateSymtabModule(linker, &symcnt);
		last_module->next = symmod;
		symmod->next = NULL;
		last_module = symmod;
		data_size += U16L(symmod->aout->data);
		MAPPut(&linker->symbols, symmod->symbols->name, symmod->symbols);
	}

	/* create synthetic reloc module */
	MODULE* relmod = LDCreateRelocModule(linker, relocs);
	last_module->next = relmod;
	relmod->next = NULL;
	last_module = relmod;
	MAPPut(&linker->symbols, relmod->symbols->name, relmod->symbols);
	data_size += U16L(relmod->aout->data);

	/* create import module */
	if(linker->type != LD_TYPE_EXEC) {
		MODULE* impmod = LDCreateImportModule(linker);
		last_module->next = impmod;
		impmod->next = NULL;
		last_module = impmod;
		data_size += U16L(impmod->aout->data);
	}

	*size = sizeof(PROGRAM) + text_size + data_size;

	u16 text = sizeof(PROGRAM);
	u16 data = text + (u16) text_size;
	u16 bss = data + (u16) data_size;

	/* layout modules in memory, entry point at 0 */
	if(!is_library) {
		linker->entry->module->textptr = text;
		linker->entry->module->dataptr = data;
		linker->entry->module->bssptr  = bss;

		text += U16L(linker->entry->module->aout->text);
		data += U16L(linker->entry->module->aout->data);
		bss  += U16L(linker->entry->module->aout->bss);
	}

	u16* reloc = (u16*) AOUTGetData(relmod->aout);

	*(reloc++) = (u16) relocs;

	/* compute size and store relocs */
	for(MODULE* module = linker->modules; module; module = module->next) {
		if(!module->visited || !module->keep) {
			continue;
		}

		AOUT* aout = module->aout;

		/* skip entrypoint module for layouting */
		if(!is_library && module == linker->entry->module) {
			module->textptr = sizeof(PROGRAM);
			module->dataptr = module->textptr + text_size;
			module->bssptr  = module->dataptr + data_size;
		} else {
			module->textptr = text;
			module->dataptr = data;
			module->bssptr  = bss;

			text += U16L(aout->text);
			data += U16L(aout->data);
			bss  += U16L(aout->bss);
		}

		u16* textrels = AOUTGetTextRels(aout);
		u16* datarels = AOUTGetDataRels(aout);

		SYMTAB symtab;
		AOUTGetSymtab(aout, &symtab);

		SYM* syms = symtab.sym;

		if(textrels) {
			for(unsigned int i = 0; i < U16L(aout->text) / 2; i++) {
				u16 pc = module->textptr + (i << 1);
				u16 rel = textrels[i];

				if(rel == 0) {
					continue;
				}

				BOOL pcrel = rel & 1;
				u16 type = (rel >> 1) & 7;
				u16 id = rel >> 4;

				SYM* sym = &syms[id];
				const char* symname = &symtab.names[sym->offset];

				BOOL unresolved = type == 4
					&& (sym->type == 000 || sym->type == 040)
					&& MAPGet(&linker->symbols, symname) == NULL;

				if(!unresolved && !pcrel) {
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
				u16 type = (rel >> 1) & 7;
				u16 id = rel >> 4;

				SYM* sym = &syms[id];
				const char* symname = &symtab.names[sym->offset];

				BOOL unresolved = type == 4
					&& (sym->type == 000 || sym->type == 040)
					&& MAPGet(&linker->symbols, symname) == NULL;

				if(!unresolved && !pcrel) {
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
					if(linker->type == LD_TYPE_EXEC) {
						printf("Error: failed to get external symbol '%s', this should not happen\n", symname);
						return NULL;
					} else {
						sym->value = 0;
					}
				} else {
					sym->value = s->value;
				}
			}
		}
	}

	/* update export symbol table with new symbol values */
	if(symmod) {
		LDUpdateSymtabModule(linker, symmod, symcnt);
	}

	LDUpdateImportModule(linker);

	u8* image = (u8*) malloc(*size);
	if(!image) {
		printf("Failed to allocate memory: %s\n", strerror(errno));
		return NULL;
	}

	PROGRAM* prog = (PROGRAM*) image;
	if(zero_magic) {
		prog->magic = 0;
	} else {
		prog->magic = U16B(PROG_MAGIC);
	}
	if(symmod) {
		prog->image_size = U16L(symmod->dataptr) - sizeof(PROGRAM);
	} else {
		/* prog->image_size = U16L(text_size + data_size); */
		prog->image_size = U16L(relmod->dataptr) - sizeof(PROGRAM);
	}
	prog->symbols = U16L((u16) symcnt);
	prog->relocs = U16L(relmod->dataptr);

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

		for(unsigned int i = 0; i < U16L(aout->data) / 2; i++) {
			u16* text = &dataptr[i];
			u16 rel = datarels[i];

			if(rel == 0) {
				continue;
			}

			BOOL pcrel = rel & 1;
			u16 type = (rel >> 1) & 7;
			u16 id = rel >> 4;

			SYM* sym = &syms[id];

#ifdef _DEBUG
			u16 pc = module->dataptr + (i << 1);
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

void* LDLink(LINKER* linker, size_t* size)
{
	switch(linker->type) {
		case LD_TYPE_EXEC:
		case LD_TYPE_DYN:
			if(!LDCollect(linker, "_start")) {
				return NULL;
			}
			return LDLinkBinary(linker, size);
		case LD_TYPE_LIB:
			if(!LDCollectLibrary(linker)) {
				return NULL;
			}
			return LDLinkBinary(linker, size);
		default:
			return NULL;
	}
}

static int symsort(const void* a, const void* b)
{
	SYMBOL* sym_a = *(SYMBOL**) a;
	SYMBOL* sym_b = *(SYMBOL**) b;
	return (int) sym_a->value - (int) sym_b->value;
}

void LDPrintMap(LINKER* linker, FILE* out)
{
	BOOL gc;
	switch(linker->type) {
		case LD_TYPE_EXEC:
		case LD_TYPE_DYN:
			gc = TRUE;
			break;
		case LD_TYPE_LIB:
		default:
			gc = FALSE;
	}

	/* print symbols */
	unsigned int symcnt = 0;
	for(MODULE* module = linker->modules; module; module = module->next) {
		if(gc && (!module->visited || !module->keep)) {
			continue;
		}

		for(SYMBOL* sym = module->symbols; sym; sym = sym->next) {
			symcnt++;
		}
	}

	SYMBOL** symbols = (SYMBOL**) malloc(symcnt * sizeof(SYMBOL*));
	unsigned int i = 0;
	for(MODULE* module = linker->modules; module; module = module->next) {
		if(gc && (!module->visited || !module->keep)) {
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
