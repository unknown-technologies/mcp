#include <stdint.h>

uint32_t __udivmodsi4(uint32_t num, uint32_t den, int modwanted)
{
	uint32_t bit = 1;
	uint32_t res = 0;

	while(den < num && bit && ((int16_t) (den >> 16) >= 0)) {
		den <<=1;
		bit <<=1;
	}

	while(bit) {
		if(num >= den) {
			num -= den;
			res |= bit;
		}
		bit >>=1;
		den >>=1;
	}

	if(modwanted)
		return num;

	return res;
}
