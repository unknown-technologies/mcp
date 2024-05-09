#include <emulight/string.h>

void bzero(void* s, size_t n)
{
	if((((uintptr_t) s) & 1) == 0 && (n & 1) == 0) {
		u16* dst = (u16*) s;
		n >>= 1;
		while(n--) {
			*dst++ = 0;
		}
	} else {
		u8* dst = (u8*) s;
		while(n--) {
			*dst++ = 0;
		}
	}
}
