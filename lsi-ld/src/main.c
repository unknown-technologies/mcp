#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include "types.h"
#include "aout.h"
#include "linker.h"

void print_help(const char* self)
{
	printf("Usage: %s [OPTIONS] FILE ...\n"
			"\n"
			"OPTIONS\n"
			"  -h              Show this help text\n"
			"  -m symbol.map   Print symbol map to file, - is stdout\n"
			"  -o output.bin   Write output to file\n"
			"  -e              Link EXEC program (statically linked)\n"
			"  -d              Link DYN program (dynamically linked)\n"
			"  -l              Link shared library\n"
			"  -no             Output octal addresses in symbol map\n"
			"\n"
			"The last argument FILE is a list of object files to process\n", self);
}

int print_symbol_list(char** ofiles, int ofilecnt)
{
	for(int i = 0; i < ofilecnt; i++) {
		char* ofilename = ofiles[i];
		printf("Loading %s\n", ofilename);
		void* ofile = LDLoadFile(ofilename);
		if(!ofile) {
			return 1;
		}

		AOUT* aout = (AOUT*) ofile;
		SYMTAB symtab;
		AOUTGetSymtab(aout, &symtab);

		SYM* sym = symtab.sym;
		printf("MODULE  OFFSET VALUE  (HEXVAL) TY TYP NAME\n");
		for(unsigned int j = 0; j < U16L(aout->symbols); j += 8, sym++) {
			char type;
			const char* name = AOUTGetSymbol(U16L(sym->type),
					&symtab.names[sym->offset], &type);
			printf("%07o %6d %06o (0x%04X) %02o [%c] %s\n",
					U16L(sym->module), U16L(sym->offset),
					U16L(sym->value), U16L(sym->value),
					U16L(sym->type), type, name);
		}

		free(ofile);
	}

	return 0;
}

int link(char* outfile, char** ofiles, int ofilecnt, FILE* symbols_out, int type, BOOL oct)
{
	LINKER ld;

	LDInit(&ld, type);

	for(int i = 0; i < ofilecnt; i++) {
		if(!LDLoad(&ld, ofiles[i])) {
			LDDestroy(&ld);
			return 1;
		}
	}

	size_t memsz;
	void* out = LDLink(&ld, &memsz);

	if(out) {
		FILE* f = fopen(outfile, "wb");
		if(!f) {
			printf("Error opening %s: %s\n", outfile, strerror(errno));
			free(out);
			LDDestroy(&ld);
			return 1;
		}

		if(fwrite(out, memsz, 1, f) != 1) {
			printf("Error writing %s: %s\n", outfile, strerror(errno));
			fclose(f);
			free(out);
			LDDestroy(&ld);
			return 1;
		}

		fclose(f);
		free(out);

		if(symbols_out) {
			LDPrintMap(&ld, symbols_out, oct);
		}
	} else {
		LDDestroy(&ld);
		return 1;
	}

	LDDestroy(&ld);

	return 0;
}

int main(int argc, char** argv)
{
	char* self = *argv;

	int print_symbols = 0;
	char* outfile = NULL;
	char* mapfile = NULL;
	unsigned int type = LD_TYPE_EXEC;
	BOOL oct = FALSE;

	argc--;
	argv++;

	for(; argc; argc--, argv++) {
		if(!strcmp(*argv, "-h") || !strcmp(*argv, "--help")) {
			print_help(self);
			return 0;
		} else if(!strcmp(*argv, "-p")) {
			print_symbols = 1;
		} else if(!strcmp(*argv, "-e")) {
			type = LD_TYPE_EXEC;
		} else if(!strcmp(*argv, "-l")) {
			type = LD_TYPE_LIB;
		} else if(!strcmp(*argv, "-d")) {
			type = LD_TYPE_DYN;
		} else if(!strcmp(*argv, "-no")) {
			oct = TRUE;
		} else if(!strcmp(*argv, "-o") && argc > 1) {
			outfile = argv[1];
			argv++;
			argc--;
		} else if(!strcmp(*argv, "-m") && argc > 1) {
			mapfile = argv[1];
			argv++;
			argc--;
		} else if(*argv[0] == '-') {
			printf("Unrecognized option: %s\n", *argv);
			return 1;
		} else {
			break;
		}
	}

	int ofilecnt = argc;
	char** ofiles = argv;

	if(outfile) {
		FILE* symbols_out = NULL;
		if(print_symbols) {
			symbols_out = stdout;
		}
		if(mapfile) {
			if(!strcmp(mapfile, "-")) {
				symbols_out = stdout;
			} else {
				symbols_out = fopen(mapfile, "wt");
				if(!symbols_out) {
					printf("Failed to open map file %s: %s\n", mapfile, strerror(errno));
					return 1;
				}
			}
		}
		int result = link(outfile, ofiles, ofilecnt, symbols_out, type, oct);
		if(symbols_out) {
			fclose(symbols_out);
		}
		return result;
	}

	if(print_symbols) {
		return print_symbol_list(ofiles, ofilecnt);
	}


	printf("ERROR: No operation\n");

	return 0;
}
