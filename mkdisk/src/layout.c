#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>

#include "types.h"
#include "efs.h"
#include "layout.h"
#include "recfmt.h"

BOOL readline(FILE* file, char* buf, size_t bufsz)
{
	BOOL start = TRUE;
	for(size_t i = 0; i < bufsz; i++) {
		int c = fgetc(file);
		if(start && (c == ' ' || c == '\t' || c == '\n')) {
			i--;
			continue;
		} else {
			start = FALSE;
		}
		if(c == EOF) {
			buf[i] = 0;
			if(i == 0) {
				return FALSE;
			} else {
				return TRUE;
			}
		} else if(c == '\n') {
			buf[i] = 0;
			return TRUE;
		} else {
			buf[i] = (char) c;
		}
	}

	buf[bufsz - 1] = 0;
	return TRUE;
}

size_t count_lines(const char* s)
{
	size_t linecount = 1;
	for(; *s; s++) {
		if(*s == '\n') {
			linecount++;
		}
	}
	return linecount;
}

BOOL add_file(EFS* efs, const char* dsn, const char* path, unsigned int flags)
{
	struct stat buf;
	if(stat(path, &buf) == -1) {
		printf("Failed to stat %s: %s\n", path, strerror(errno));
		return FALSE;
	}

	if(buf.st_size == 0) {
		printf("%s is empty, skipping\n", dsn);
		return TRUE;
	}

	void* data = malloc(buf.st_size);
	if(!data) {
		printf("Failed to allocated %lu bytes\n", buf.st_size);
		return FALSE;
	}

	FILE* fd = fopen(path, "rb");
	if(!fd) {
		printf("Failed to open file %s: %s\n", path, strerror(errno));
		free(data);
		return FALSE;
	}

	if(fread(data, buf.st_size, 1, fd) != 1) {
		printf("Failed to read %s\n", path);
		free(data);
		fclose(fd);
		return FALSE;
	}

	fclose(fd);

	if(flags & 0xFFF) {
		char* c = (char*) data;
		size_t lc = count_lines(c);
		u16 type = flags & 0xC000;

		if(c[buf.st_size - 1] == '\n') {
			c[buf.st_size - 1] = 0;
			lc--;
		}

		if(lc > 0) {
			size_t lrecl = (flags & 0xFFF) + 1;
			size_t sz = lc * lrecl;
			char* lines = (char*) malloc(sz);
			if(!lines) {
				printf("Failed to allocate %lu bytes\n", sz);
				free(data);
				return FALSE;
			}

			memset(lines, 0, sz);

			if(type == EFS_DSCB_DS_TYPE_TXT6B) {
				size_t lrecl6 = (lrecl + 3) / 4 * 3;
				sz = lc * lrecl6;
				printf("TXT6: LRECL=%lu LRECL6=%lu\n", lrecl, lrecl6);
				recfmt6((u8*) lines, lrecl, c);
			} else if(type == EFS_DSCB_DS_TYPE_TXT7B) {
				size_t lrecl7 = (lrecl + 7) / 8 * 7;
				sz = lc * lrecl7;
				printf("TXT7: LRECL=%lu LRECL7=%lu\n", lrecl, lrecl7);
				recfmt7((u8*) lines, lrecl, c);
			} else {
				recfmt8(lines, lrecl, c);
			}

			EFSStoreSequential(efs, dsn, sz, lines, flags);

			free(lines);
		} else {
			printf("No lines in %s, skipping\n", dsn);
			free(data);
			return TRUE;
		}
	} else {
		EFSStoreSequential(efs, dsn, buf.st_size, data, flags);
	}

	free(data);

	return TRUE;
}

void LAYOUTProcess(EFS* efs, const char* layout_filename)
{
	char buf[256];

	FILE* file = fopen(layout_filename, "rt");
	if(!file) {
		printf("Failed to open layout file: %s\n", strerror(errno));
		return;
	}

	while(readline(file, buf, sizeof(buf))) {
		if(buf[1] != ' ' || buf[4] != ' ') {
			printf("Syntax error\n");
			break;
		}

		buf[4] = 0;

		char type = buf[0];
		unsigned int lrecl = 0;
		if(buf[2] != '*') {
			lrecl = atoi(&buf[2]);
		}

		char* dsn = &buf[5];
		for(char* c = &buf[5]; *c; c++) {
			if(*c == ' ' || *c == '\t') {
				*c = 0;
				break;
			}
		}

		char* path = dsn + strlen(dsn) + 1;
		for(; *path; path++) {
			if(*path != '\t' && *path != ' ') {
				break;
			}
		}

		for(char* c = path; *c; c++) {
			if(*c == ' ' || *c == '\t' || *c == '\n') {
				*c = 0;
				break;
			}
		}

		unsigned int flags = 0;
		switch(type) {
			case 'b':
			case 'B':
				flags = EFS_DSCB_DS_TYPE_BIN;
				break;
			case 'x':
			case 'X':
				flags = EFS_DSCB_DS_TYPE_EXEC;
				break;
			case '6':
				flags = EFS_DSCB_DS_TYPE_TXT6B;
				break;
			case '7':
				flags = EFS_DSCB_DS_TYPE_TXT7B;
				break;
			case '8':
				flags = EFS_DSCB_DS_TYPE_BIN;
				break;
			default:
				printf("Unknown type '%c'\n", type);
				fclose(file);
				return;
		}

		if(lrecl > 0) {
			flags |= (lrecl - 1) & 0x0FFF;
		}

		if(!add_file(efs, dsn, path, flags)) {
			break;
		}
	}

	fclose(file);
}
