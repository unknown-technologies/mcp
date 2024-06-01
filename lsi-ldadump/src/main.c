#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "types.h"

#define CHECKSUM8(x)	((u8) (x) + (u8) ((x) >> 8))

u8 checksum8(u8* memory, size_t size)
{
	u8 sum = 0;
	u8* p = memory;
	while(size--) {
		sum += *(p++);
	}
	return sum;
}

#define	CHECK(x)	\
	if((x) != 1) { \
		printf("error: unexpected EOF (%u)\n", __LINE__); \
		return 0; \
	}

static BOOL quiet = FALSE;

u16 load_binary(u8* memory, FILE* f)
{
	u16 len;
	u16 addr;
	u8 cksum;
	u16 end = 0;
	size_t bytes = 0;

	memset(memory, 0, 65536);
	while(1) {
		int c;
		while((c = fgetc(f)) != EOF) {
			if(c == 1) {
				break;
			}
		}
		if(c == 1) {
			c = fgetc(f);
		}
		if(c == EOF) {
			printf("error: unexpected EOF\n");
			return 0;
		} else if(c != 0) {
			printf("error: invalid signature! [%02x]\n", c);
			return 0;
		}

		CHECK(fread(&len, 2, 1, f));
		CHECK(fread(&addr, 2, 1, f));
		bytes += len;
		if(!quiet) {
			printf("%06o: %d bytes\n", addr, len - 6);
		}
		if(end < (addr + len - 6)) {
			end = addr + len - 6;
		}

		if(len - 6 > 0) {
			CHECK(fread(&memory[addr], len - 6, 1, f));
		}
		CHECK(fread(&cksum, 1, 1, f));
		cksum += checksum8(&memory[addr], len - 6);
		cksum += CHECKSUM8(len) + CHECKSUM8(addr) + 1;
		if(cksum) {
			printf("invalid checksum: %03o\n", (unsigned int) cksum);
			return 0;
		}
		if(len == 6) {
			if(((addr & 1) == 0) && (addr != 0)) {
				printf("invalid transfer address %06o\n", addr);
				return 0;
			}
			break;
		}
	}
	return end;
}


int main(int argc, char** argv)
{
	if(argc != 3 && argc != 4) {
		printf("Usage: %s [-q] in.lda out.bin\n", *argv);
		return 1;
	}

	if(argc == 4) {
		if(!strcmp(argv[1], "-q")) {
			quiet = TRUE;
			argv++;
			argc--;
		} else {
			printf("Syntax error\n");
			return 1;
		}
	}

	const char* in_filename = argv[1];
	const char* out_filename = argv[2];

	FILE* binary = fopen(in_filename, "rb");
	if(!binary) {
		printf("Failed to open LDA file: %s\n", strerror(errno));
		return 1;
	}

	void* memory = malloc(65536);
	if(!memory) {
		printf("Failed to allocate memory\n");
		fclose(binary);
		return 1;
	}

	size_t binsize = load_binary(memory, binary);
	fclose(binary);
	if(binsize == 0) {
		printf("Failed to load binary\n");
		free(memory);
		return 1;
	}

	FILE* output = fopen(out_filename, "wb");
	if(!output) {
		printf("Failed to open output file: %s\n", strerror(errno));
		free(memory);
		return 1;
	}

	fwrite(memory, binsize, 1, output);
	fclose(output);
	free(memory);

	return 0;
}
