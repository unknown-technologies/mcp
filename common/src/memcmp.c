#include <emulight/string.h>

/* TODO: make sure the 16bit path and the 8bit path give identical results */
int memcmp(const void* s1, const void* s2, size_t n)
{
	if((((uintptr_t) s1) & 1) == 0 && (((uintptr_t) s2) & 1) == 0
			&& (n & 1) == 0) {
		const u16* a = (const u16*) s1;
		const u16* b = (const u16*) s2;

		n >>= 1;

		while(n--) {
			if(*a != *b) {
				if(*a > *b) {
					return 1;
				} else {
					return -1;
				}
			} else {
				a++;
				b++;
			}
		}

		return 0;
	} else {
		const u8* a = (const u8*) s1;
		const u8* b = (const u8*) s2;

		while(n--) {
			if(*a != *b) {
				return *b - *a;
			} else {
				a++;
				b++;
			}
		}

		return 0;
	}
}
