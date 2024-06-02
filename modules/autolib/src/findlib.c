#include <emulight/types.h>
#include <emulight/string.h>
#include <emulight/rad50.h>
#include <emulight/ds.h>
#include <emulight/mcp/syslog.h>
#include <emulight/mcp/mem.h>
#include <emulight/mcp/program.h>

static char recbuf[96];

BOOL prefix_match(const char* dsn, const char* entry)
{
	while(*dsn && *entry) {
		char c = *dsn;
		if(c >= 'a' && c <= 'z') {
			c -= 0x20;
		}

		if(c != *entry) {
			if(c == '.' && *entry == ' ') {
				return TRUE;
			} else {
				return FALSE;
			}
		} else {
			dsn++;
			entry++;
		}
	}

	return *dsn == '.' || !*dsn;
}

BOOL OSAutoLoadLibrary(const char* dsn, OSLibrary** ref)
{
	int ds = DSOpen("SYS1.LIBS");
	if(ds < 0) {
		OSLog(NUC_LIBRARY_IDX_NOTF, "LIBRARY INDEX NOT FOUND");
		return TRUE;
	}

	DS info;
	DSGetInfo(ds, &info);
	if(!DSIsFB(&info)) {
		DSClose(ds);
		OSLog(NUC_LIBRARY_IDX_INVAL, "LIBRARY INDEX INVALID");
		return FALSE;
	}

	unsigned int lrecl = DSGetRecLen(&info);
	if(lrecl >= 96) {
		DSClose(ds);
		OSLog(NUC_LIBRARY_IDX_INVAL,
				"LIBRARY INDEX INVALID: LRECL TO LONG");
		return FALSE;
	}

	while(DSRead(ds, recbuf, 1) == 1) {
		if(recbuf[0] == ' ') {
			continue;
		}

		if(!prefix_match(dsn, recbuf)) {
			continue;
		}

		/* find library DSN */
		char* libdsn = recbuf;
		/* skip DSN prefix */
		while(*libdsn && *libdsn != ' ') {
			libdsn++;
		}
		/* skip space */
		while(*libdsn == ' ') {
			libdsn++;
		}

		/* terminate at space */
		for(char* c = libdsn; *c; c++) {
			if(*c == ' ') {
				*c = 0;
				break;
			}
		}

		/* find final level = library name */
		char* libname = libdsn;
		while(*libname) {
			libname++;
		}
		while(libname > libdsn && *libname != '.') {
			libname--;
		}
		if(*libname == '.') {
			libname++;
		}

		/* now load this library if it is not in memory already */
		DSClose(ds);

		OSLibrary* lib = OSGetLibrary(libname);
		if(!lib) {
			if(OSLoadLibrary(libname, libdsn)) {
				lib = OSGetLibrary(libname);
			} else {
				return FALSE;
			}
		}

		*ref = lib;

		return TRUE;
	}

	/* nothing was found */

	DSClose(ds);

	return TRUE;
}
