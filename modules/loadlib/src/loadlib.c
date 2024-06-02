#include <emulight/types.h>
#include <emulight/string.h>
#include <emulight/ds.h>
#include <emulight/mcp/mon.h>
#include <emulight/mcp/syslog.h>
#include <emulight/mcp/mem.h>
#include <emulight/mcp/program.h>

static const char* loaderror_library = "FAILED TO LOAD LIBRARY";

void OSRelocateSymbols(OSLibrary* lib)
{
	OSSymbol* sym = lib->symbols;
	const char* names = (const char*) sym;
	unsigned int cnt = lib->library->symbols;
	void* base = lib->library;
	for(unsigned int i = 0; i < cnt; i++) {
		sym->name += (uintptr_t) names;
		sym->value += (uintptr_t) base;
#ifdef DEBUG
		OSLog(NUC_LIBRARY_SYMBOL, "SYMBOL: %s = %04X", sym->name,
				(unsigned int) sym->value);
#endif
		sym++;
	}
}


BOOL OSLoadLibrary(const char* name, const char* dsn)
{
	OSProgram header;

	int ds = DSOpen(dsn);
	if(ds < 0) {
		OSLog(NUC_LIBRARY_NOT_FOUND, "LIBRARY %s NOT FOUND AT DSN %s",
				name, dsn);
		return FALSE;
	}

#ifdef DEBUG
	OSLog(NUC_LOAD_LIBRARY, "LOADING LIBRARY %s FROM DSN %s", name, dsn);
#endif

	if(DSRead(ds, &header, sizeof(OSProgram)) != sizeof(OSProgram)) {
		OSLog(NUC_LIBRARY_READERR, "%s %s: READ ERROR",
				loaderror_library, name);
		DSClose(ds);
		return FALSE;
	}

	unsigned int size = header.relocs;

	OSLibrary* oslib = OSCreateLibrary(name);
	if(!oslib) {
		OSLog(NUC_LIBRARY_NOSPC, "%s %s: NO SLOT LEFT",
				loaderror_library, name);
		return FALSE;
	}

	void* lib = MEMAlloc(size);
	if(!lib) {
		OSLog(NUC_LIBRARY_NOMEM, "%s %s: NO MEMORY", loaderror_library,
				name);
		return FALSE;
	}

	oslib->library = (OSProgram*) lib;
	oslib->symbols = (OSSymbol*) (lib + header.image_size
			+ sizeof(OSProgram));

	memcpy(oslib->library, &header, sizeof(OSProgram));

	/* load code and symbols */
	size_t rdsz = size - sizeof(OSProgram);
	if(DSRead(ds, oslib->library + 1, rdsz) != rdsz) {
		OSFreeLibrary(lib);
		DSClose(ds);
		OSLog(NUC_LIBRARY_READERR, "%s %s: READ ERROR",
				loaderror_library, name);
		return FALSE;
	}

	/* load relocations one by one and apply them */
	unsigned int relocs;
	if(DSRead(ds, &relocs, sizeof(unsigned int)) != sizeof(unsigned int)) {
		OSFreeLibrary(oslib);
		DSClose(ds);
		OSLog(NUC_LIBRARY_READERR, "%s %s: READ ERROR",
				loaderror_library, name);
		return FALSE;
	}

#ifdef DEBUG
	OSLog(NUC_LIBRARY_RELOCS, "%u RELOCS", relocs);
#endif
	for(unsigned int i = 0; i < relocs; i++) {
		unsigned int reloc;

		if(DSRead(ds, &reloc, sizeof(unsigned int))
				!= sizeof(unsigned int)) {
			OSFreeLibrary(oslib);
			DSClose(ds);
			OSLog(NUC_LIBRARY_READERR, "%s %s: READ ERROR",
					loaderror_library, name);
			return FALSE;
		}

		unsigned int* p = lib + reloc;
		*p += (unsigned int) lib;
	}

	/* relocate symbols */
	OSRelocateSymbols(oslib);

	/* load import symbols and apply them */
	if(!OSResolveImports(ds, lib, lib)) {
		OSFreeLibrary(oslib);
		DSClose(ds);
		return FALSE;
	}

	DSClose(ds);

#ifdef DEBUG
	OSLog(NUC_LIBRARY_LOADED, "LIBRARY %s LOADED AT %04X WITH %u SYMBOLS",
			name, (unsigned int) oslib->library,
			oslib->library->symbols);
#endif

	return TRUE;
}
