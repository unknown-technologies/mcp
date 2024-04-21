#include <emulight/string.h>

size_t strlen(const char* s)
{
	const char* c;
	for(c = s; *c; c++);
	return (size_t) (c - s);
}
