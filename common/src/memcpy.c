#include <emulight/string.h>

void* memcpy(void* __restrict dst, const void* __restrict src, size_t len)
{
	if((((uintptr_t) dst) & 1) == 0 && (((uintptr_t) src) & 1) == 0
			&& (len & 1) == 0) {
		u16* dst16 = (u16*) dst;
		const u16* src16 = (const u16*) src;
		len >>= 1;
		while(len--) {
			*dst16++ = *src16++;
		}
	} else {
		u8* dst8 = (u8*) dst;
		const u8* src8 = (const u8*) src;
		while(len--) {
			*dst8++ = *src8++;
		}
	}
	return dst;
}
