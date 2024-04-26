#include <emulight/types.h>
#include <emulight/string.h>
#include <emulight/rad50.h>
#include <emulight/mcp/mon.h>
#include <emulight/mcp/syslog.h>
#include <emulight/mcp/mem.h>
#include <emulight/mcp/program.h>
#include <emulight/mcp/fs.h>

static OSLibrary libraries[MAX_LIBRARIES];

extern void* __relbase;

static void* OSCreateLibrary(const char* name)
{
	for(unsigned int i = 0; i < MAX_LIBRARIES; i++) {
		OSLibrary* lib = &libraries[i];
		if(lib->library == NULL) {
			RAD50EncodeString(lib->name, name, 3);
			return lib;
		}
	}

	return NULL;
}

void OSInitLibrary(void)
{
	bzero(libraries, sizeof(libraries));

	OSLibrary* self = OSCreateLibrary("NUCLEUS");

	self->library = (OSProgram*) __relbase;
	self->symbols = (OSSymbol*) ((uintptr_t) self->library + self->library->image_size + sizeof(OSProgram));

	OSLog(NUC_SYMBOLS, "NUCLEUS CONTAINS %u SYMBOLS", self->library->symbols);

	/* relocate symbols */
	OSSymbol* sym = self->symbols;
	const char* names = (const char*) sym;
	for(unsigned int i = 0; i < self->library->symbols; i++) {
		sym->name += (uintptr_t) names;
		sym->value += (uintptr_t) self->library;
		OSLog(NUC_LIBRARY_SYMBOL, "SYMBOL: %s", sym->name);
		sym++;
	}
}

BOOL OSLoadLibrary(const char* name, const char* dsn)
{
	OSProgram header;

	DS ds;
	if(!DSOpen(&ds, dsn)) {
		OSLog(NUC_LIBRARY_NOT_FOUND, "LIBRARY %s NOT FOUND AT DSN %s", name, dsn);
		return FALSE;
	}

	OSLog(NUC_LOAD_LIBRARY, "LOADING LIBRARY %s FROM DSN %s", name, dsn);

	if(DSRead(&ds, &header, sizeof(OSProgram)) != sizeof(OSProgram)) {
		OSLog(NUC_LIBRARY_READERR, "LOADING LIBRARY %s FAILED: READ ERROR", name);
		return FALSE;
	}

	unsigned int size = header.relocs;

	OSLibrary* oslib = OSCreateLibrary(name);
	if(!oslib) {
		OSLog(NUC_LIBRARY_NOSPC, "FAILED TO LOAD LIBRARY %s: NO SLOT LEFT", name);
		return FALSE;
	}

	void* lib = MEMAlloc(size);
	if(!lib) {
		OSLog(NUC_LIBRARY_NOMEM, "FAILED TO LOAD LIBRARY %s: NO MEMORY", name);
		return FALSE;
	}

	oslib->library = (OSProgram*) lib;
	oslib->symbols = (OSSymbol*) (lib + header.image_size + sizeof(OSProgram));

	memcpy(oslib->library, &header, sizeof(OSProgram));

	/* load code and symbols */
	size_t rdsz = size - sizeof(OSProgram);
	if(DSRead(&ds, oslib->library + 1, rdsz) != rdsz) {
		MEMFree(lib);
		OSLog(NUC_LIBRARY_READERR, "LOADING LIBRARY %s FAILED: READ ERROR", name);
		return FALSE;
	}

	/* load relocations one by one and apply them */
	unsigned int relocs;
	if(DSRead(&ds, &relocs, sizeof(unsigned int)) != sizeof(unsigned int)) {
		MEMFree(lib);
		OSLog(NUC_LIBRARY_READERR, "LOADING LIBRARY %s FAILED: READ ERROR", name);
		return FALSE;
	}

	OSLog(NUC_LIBRARY_RELOCS, "%u RELOCS", relocs);
	for(unsigned int i = 0; i < relocs; i++) {
		unsigned int reloc;

		if(DSRead(&ds, &reloc, sizeof(unsigned int)) != sizeof(unsigned int)) {
			MEMFree(lib);
			OSLog(NUC_LIBRARY_READERR, "LOADING LIBRARY %s FAILED: READ ERROR", name);
			return FALSE;
		}

		unsigned int* p = lib + reloc;
		*p += (unsigned int) lib;
	}

	/* relocate symbols */
	OSSymbol* sym = oslib->symbols;
	const char* names = (const char*) sym;
	for(unsigned int i = 0; i < header.symbols; i++) {
		sym->name += (uintptr_t) names;
		sym->value += (uintptr_t) lib;
		OSLog(NUC_LIBRARY_SYMBOL, "SYMBOL: %s = %04X", sym->name, (unsigned int) sym->value);
		sym++;
	}

	OSLog(NUC_LIBRARY_LOADED, "LIBRARY %s LOADED AT %04X WITH %u SYMBOLS", name,
			(unsigned int) oslib->library, oslib->library->symbols);

	return TRUE;
}

void* OSLoadProgram(const char* name, const char* dsn)
{
	DS ds;
	if(!DSOpen(&ds, dsn)) {
		OSLog(NUC_PROGRAM_NOT_FOUND, "PROGRAM %s NOT FOUND AT DSN %s", name, dsn);
		return NULL;
	}

	OSLog(NUC_LOAD_PROGRAM, "LOADING PROGRAM %s FROM DSN %s", name, dsn);

	OSProgram header;
	if(DSRead(&ds, &header, sizeof(OSProgram)) != sizeof(OSProgram)) {
		OSLog(NUC_PROGRAM_READERR, "LOADING PROGRAM %s FAILED: READ ERROR", name);
		return NULL;
	}

	unsigned int size = header.image_size;

	void* prog = MEMAlloc(size);
	if(!prog) {
		OSLog(NUC_PROGRAM_NOMEM, "FAILED TO LOAD PROGRAM %s: NO MEMORY", name);
		return NULL;
	}

	/* load code */
	if(DSRead(&ds, prog, size) != size) {
		MEMFree(prog);
		OSLog(NUC_PROGRAM_READERR, "LOADING PROGRAM %s FAILED: READ ERROR", name);
		return NULL;
	}

	/* load relocations one by one and apply them */
	DSSeek(&ds, header.relocs);

	unsigned int relocs;
	if(DSRead(&ds, &relocs, sizeof(unsigned int)) != sizeof(unsigned int)) {
		MEMFree(prog);
		OSLog(NUC_PROGRAM_READERR, "LOADING PROGRAM %s FAILED: READ ERROR", name);
		return NULL;
	}

	void* base = prog - sizeof(OSProgram);
	OSLog(NUC_PROGRAM_RELOCS, "%u RELOCS @ %04X", relocs, header.relocs);
	for(unsigned int i = 0; i < relocs; i++) {
		unsigned int reloc;

		if(DSRead(&ds, &reloc, sizeof(unsigned int)) != sizeof(unsigned int)) {
			MEMFree(prog);
			OSLog(NUC_PROGRAM_READERR, "LOADING PROGRAM %s FAILED: READ ERROR", name);
			return NULL;
		}

		unsigned int* p = base + reloc;

		OSLog(MSGID_NUC, 999, LOG_DEBUG, "RELOC %u: %04X (%04X)", i, reloc, (unsigned int) p);

		*p += (unsigned int) base;
	}

	/* load import symbols and apply them */
	unsigned int externals = 0;
	if(DSRead(&ds, &externals, sizeof(unsigned int)) != sizeof(unsigned int)) {
		MEMFree(prog);
		OSLog(NUC_PROGRAM_READERR, "LOADING PROGRAM %s FAILED: READ ERROR", name);
		return NULL;
	}

	for(unsigned int i = 0; i < externals; i++) {
		OSImportSymbol import;

		if(DSRead(&ds, &import, sizeof(OSImportSymbol)) != sizeof(OSImportSymbol)) {
			MEMFree(prog);
			OSLog(NUC_PROGRAM_READERR, "LOADING PROGRAM %s FAILED: READ ERROR", name);
			return NULL;
		}

		const char* name = import.name;
		OSLog(MSGID_NUC, 999, LOG_DEBUG, "IMPORT %u: %04X (RELOCS: %u)", i, name, import.reloc_cnt);
	}

	OSLog(NUC_PROGRAM_LOADED, "PROGRAM %s LOADED AT %04X WITH %u EXTERNAL SYMBOLS",
			name, (unsigned int) prog, externals);

	return NULL;

	return prog;
}
