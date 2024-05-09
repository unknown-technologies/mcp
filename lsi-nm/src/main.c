#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include "types.h"
#include "mcp.h"

void* NMLoadFile(const char* filename)
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

void NMPrintSymbols(PROGRAM* prog)
{
	if(U16B(prog->magic) != 0x0301 && prog->magic != 0) {
		printf("Not an MCP program\n");
		return;
	}

	unsigned int cnt = U16L(prog->symbols);
	unsigned int off = sizeof(PROGRAM) + U16L(prog->image_size);
	SYM* sym = (SYM*) ((uintptr_t) prog + off);
	const char* names = (const char*) sym;

	for(unsigned int i = 0; i < cnt; i++, sym++) {
		const char* name = &names[U16L(sym->name)];
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

		printf("%06o %c %s\n", U16L(sym->value), t, name);
	}
}

int main(int argc, char** argv)
{
	if(argc != 2) {
		printf("Usage: %s program\n", *argv);
		return 1;
	}

	PROGRAM* prog = NMLoadFile(argv[1]);
	if(!prog) {
		return 1;
	}

	NMPrintSymbols(prog);

	free(prog);

	return 0;
}
