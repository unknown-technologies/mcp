#include <emulight/types.h>
#include <emulight/string.h>
#include <emulight/rad50.h>
#include <emulight/r4.h>
#include <emulight/r6.h>
#include <emulight/r7.h>
#include <emulight/mcp/mon.h>
#include <emulight/mcp/syslog.h>
#include <emulight/mcp/mem.h>
#include <emulight/mcp/program.h>
#include <emulight/mcp/fs.h>
#include <emulight/mcp/ds.h>
#include <emulight/mcp/cal.h>

static OSLibrary libraries[MAX_LIBRARIES];

/* symbols exported by the NUCLEUS itself to save space */
typedef struct {
	const char*	name;
	void*		value;
} NUCSymbol;

static NUCSymbol nucleus_symbols[] = {
	{ .name = "_CALIsLeapYear",	.value = CALIsLeapYear },
	{ .name = "_CALGetDaysOfMonth",	.value = CALGetDaysOfMonth },
	{ .name = "_MEMAlloc",		.value = MEMAlloc },
	{ .name = "_MEMAvailable",	.value = MEMAvailable },
	{ .name = "_OSCreateLibrary",	.value = OSCreateLibrary },
	{ .name = "_OSFreeLibrary",	.value = OSFreeLibrary },
	{ .name = "_OSLog",		.value = OSLog },
	{ .name = "_OSResolveImports",	.value = OSResolveImports },
	{ .name = "_R4DecodeString",	.value = R4DecodeString },
	{ .name = "_R4EncodeString",	.value = R4EncodeString },
	{ .name = "_R6Decode",		.value = R6Decode },
	{ .name = "_R6Encode",		.value = R6Encode },
	{ .name = "_R7Decode",		.value = R7Decode },
	{ .name = "_R7Encode",		.value = R7Encode },
	{ .name = "_RAD50Decode",	.value = RAD50Decode },
	{ .name = "_RAD50Encode",	.value = RAD50Encode },
	{ .name = "_RAD50DecodeString",	.value = RAD50DecodeString },
	{ .name = "_RAD50EncodeString",	.value = RAD50EncodeString },
	{ .name = "_VOLGetVolume",	.value = VOLGetVolume },
	{ .name = "_bzero",		.value = bzero },
	{ .name = "_csprintf",		.value = csprintf },
	{ .name = "_memcpy",		.value = memcpy },
	{ .name = "_memcmp",		.value = memcmp },
	{ .name = "_sprintf",		.value = sprintf },
	{ .name = "_strlen",		.value = strlen },
	{ .name = "_strcmp",		.value = strcmp },
	{ .name = "_vcsprintf",		.value = vcsprintf },
	{ .name = "_vsprintf",		.value = vsprintf },
	{ .name = NULL,			.value = NULL }
};

static const char* loaderror_program = "FAILED TO LOAD PROGRAM";

void* OSCreateLibrary(const char* name)
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

void OSFreeLibrary(OSLibrary* lib)
{
	MEMFree(lib->library);
	lib->library = NULL;
	lib->symbols = NULL;
}

void OSInitLibrary(void)
{
	bzero(libraries, sizeof(libraries));
}

void* OSGetSymbolFromLibrary(OSLibrary* lib, const char* name)
{
	OSSymbol* sym = lib->symbols;

	/* TODO: use binary search instead of linear search here */
	for(unsigned int j = 0; j < lib->library->symbols; j++, sym++) {
		if(!strcmp(sym->name, name)) {
			return sym->value;
		}
	}

	return NULL;
}

void* OSGetSymbol(const char* name)
{
	for(NUCSymbol* sym = nucleus_symbols; sym->name; sym++) {
		if(!strcmp(sym->name, name)) {
			return sym->value;
		}
	}

	for(unsigned int i = 0; i < MAX_LIBRARIES; i++) {
		OSLibrary* lib = &libraries[i];
		if(lib->library && lib->symbols) {
			void* value = OSGetSymbolFromLibrary(lib, name);
			if(value) {
				return value;
			}
		}
	}
	return NULL;
}

#ifdef DEBUG
#define	READ_ERROR() \
	OSLog(NUC_PROGRAM_READERR, "%s: READ ERROR [%u]", loaderror_program, __LINE__);
#else
static void READ_ERROR(void)
{
	OSLog(NUC_PROGRAM_READERR, "%s: READ ERROR", loaderror_program);
}
#endif

BOOL OSResolveImports(int ds, void* prog, void* base)
{
	char name_buffer[MAX_SYMBOL_LEN];

	unsigned int externals = 0;
	if(DSRead(ds, &externals, sizeof(unsigned int)) != sizeof(unsigned int)) {
		READ_ERROR();
		return FALSE;
	}

	for(unsigned int i = 0; i < externals; i++) {
		OSImportSymbol import;

		if(DSRead(ds, &import, sizeof(OSImportSymbol)) != sizeof(OSImportSymbol)) {
			READ_ERROR();
			return FALSE;
		}

		if(import.name > MAX_SYMBOL_LEN) {
			OSLog(NUC_PROGRAM_SYMLEN, "SYMBOL TOO LONG");
			return FALSE;
		}

		bzero(name_buffer, MAX_SYMBOL_LEN);
		if(DSRead(ds, name_buffer, import.name) != import.name) {
			READ_ERROR();
			return FALSE;
		}

		void* addr = OSGetSymbol(name_buffer);

#ifdef DEBUG
		OSLog(MSGID_NUC, 999, LOG_DEBUG, "IMPORT %u: %s (RELOCS: %u) = %04X [%d]", i, name_buffer, import.relocs, (unsigned int) addr, import.name);
#endif

		if(!addr) {
			OSLog(NUC_PROGRAM_NOSYM, "%s: SYMBOL %s NOT FOUND", loaderror_program, name_buffer);
			return FALSE;
		}

		for(unsigned int j = 0; j < import.relocs; j++) {
			unsigned int reloc;
			if(DSRead(ds, &reloc, sizeof(unsigned int)) != sizeof(unsigned int)) {
				READ_ERROR();
				return FALSE;
			}

			BOOL pcrel = reloc & 1;
			unsigned int rel = reloc & ~1;
			unsigned int* pc = rel + base;
			unsigned int value = (unsigned int) addr;

#ifdef DEBUG
			OSLog(MSGID_NUC, 999, LOG_DEBUG, "RELOC %u: %04X (%04X)", j, reloc, (unsigned int) pc);
#endif

			if(pcrel) {
				*pc += (value - (unsigned int) base);
			} else {
				*pc += value;
			}
		}
	}

	return TRUE;
}

BOOL OSLoadLibrary(const char* name, const char* dsn)
{
	void* loader = OSLoadModule("LOADLIB");
	if(loader) {
		BOOL (*OSiLoadLibrary)(const char*, const char*) = (BOOL (*)(const char*, const char*)) loader;
		BOOL result = OSiLoadLibrary(name, dsn);
		OSUnloadModule(loader);
		return result;
	} else {
		return FALSE;
	}
}

void* OSLoadProgram(const char* dsn)
{
	return OSiLoadProgram(dsn, FALSE);
}

static char dsnbuf[24];

void* OSLoadModule(const char* name)
{
	sprintf(dsnbuf, "SYS1.MOD.%s", name);
	return OSiLoadProgram(dsnbuf, TRUE);
}

void OSUnloadModule(void* module)
{
	MEMFreeBottom(module);
}

void* OSiLoadProgram(const char* dsn, BOOL bottom)
{
	void (*free)(void*) = bottom ? MEMFreeBottom : MEMFree;

	int ds = DSOpen(dsn);
	if(ds < 0) {
		OSLog(NUC_PROGRAM_NOT_FOUND, "PROGRAM NOT FOUND AT DSN %s", dsn);
		return NULL;
	}

#ifdef DEBUG
	OSLog(NUC_LOAD_PROGRAM, "LOADING PROGRAM FROM DSN %s", dsn);
#endif

	OSProgram header;
	if(DSRead(ds, &header, sizeof(OSProgram)) != sizeof(OSProgram)) {
		DSClose(ds);
		READ_ERROR();
		return NULL;
	}

	unsigned int size = header.image_size;

	void* prog = bottom ? MEMAllocBottom(size) : MEMAlloc(size);
	if(!prog) {
		OSLog(NUC_PROGRAM_NOMEM, "%s: NO MEMORY", loaderror_program);
		return NULL;
	}

	/* load code */
	if(DSRead(ds, prog, size) != size) {
		free(prog);
		DSClose(ds);
		READ_ERROR();
		return NULL;
	}

	/* load relocations one by one and apply them */
	DSSeek(ds, header.relocs);

	unsigned int relocs;
	if(DSRead(ds, &relocs, sizeof(unsigned int)) != sizeof(unsigned int)) {
		free(prog);
		DSClose(ds);
		READ_ERROR();
		return NULL;
	}

	void* base = prog - sizeof(OSProgram);
#ifdef DEBUG
	OSLog(NUC_PROGRAM_RELOCS, "%u RELOCS @ %04X", relocs, header.relocs);
#endif
	for(unsigned int i = 0; i < relocs; i++) {
		unsigned int reloc;

		if(DSRead(ds, &reloc, sizeof(unsigned int)) != sizeof(unsigned int)) {
			free(prog);
			DSClose(ds);
			READ_ERROR();
			return NULL;
		}

		unsigned int* p = base + reloc;

#ifdef DEBUG
		OSLog(MSGID_NUC, 999, LOG_DEBUG, "RELOC %u: %04X (%04X)", i, reloc, (unsigned int) p);
#endif

		*p += (unsigned int) base;
	}

	/* load import symbols and apply them */
	if(!OSResolveImports(ds, prog, base)) {
		free(prog);
		DSClose(ds);
		return FALSE;
	}

	DSClose(ds);

#ifdef DEBUG
	OSLog(NUC_PROGRAM_LOADED, "PROGRAM LOADED AT %04X",
			(unsigned int) prog);
#endif

	return prog;
}

void OSUnloadProgram(void* program)
{
	MEMFree(program);
}
