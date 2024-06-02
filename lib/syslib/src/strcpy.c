#include <emulight/string.h>

char* strcpy(char* dst, const char* src)
{
	char* d = dst;

	while(*src) {
		*(dst++) = *(src++);
	}

	*dst = 0;

	return d;
}
