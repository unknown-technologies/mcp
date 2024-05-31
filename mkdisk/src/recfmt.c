#include <string.h>
#include <stdlib.h>

#include "types.h"
#include "recfmt.h"
#include "r6.h"
#include "r7.h"

void recfmt8(char* out, unsigned int lrecl, const char* src)
{
	char c;
	unsigned int i = 0;
	unsigned int ts = 8;
	char* buf = out;
	while((c = *(src++))) {
		if(c == '\n') {
			memset(&buf[i], 0x20, lrecl - i);
			buf += lrecl;
			i = 0;
		} else if(i < lrecl) {
			if(c == '\t') {
				/* process tab */
				if(i % ts == 0) {
					for(unsigned int n = 0; n < ts; n++) {
						if(i < lrecl) {
							buf[i++] = ' ';
						}
					}
				} else {
					while(i < lrecl && (i % ts) != 0) {
						if(i < lrecl) {
							buf[i++] = ' ';
						}
					}
				}
			} else if(c < 0x20) {
				buf[i++] = ' ';
			} else {
				buf[i++] = (char) c;
			}
		}
	}

	if(i != 0) {
		memset(&buf[i], 0x20, lrecl - i);
		i = 0;
	}
}

void recfmt7(u8* out, unsigned int lrecl, const char* src)
{
	char c;
	unsigned int i = 0;
	unsigned int ts = 8;
	unsigned int lrecl7 = (lrecl + 7) / 8 * 7;
	char* buf = (char*) malloc(lrecl + 1);
	while((c = *(src++))) {
		if(c == '\n') {
			memset(&buf[i], 0x20, lrecl - i);
			R7Encode(out, buf, lrecl7);
			out += lrecl7;
			i = 0;
		} else if(i < lrecl) {
			if(c == '\t') {
				/* process tab */
				if(i % ts == 0) {
					for(unsigned int n = 0; n < ts; n++) {
						if(i < lrecl) {
							buf[i++] = ' ';
						}
					}
				} else {
					while(i < lrecl && (i % ts) != 0) {
						if(i < lrecl) {
							buf[i++] = ' ';
						}
					}
				}
			} else if(c < 0x20) {
				buf[i++] = ' ';
			} else {
				buf[i++] = (char) c;
			}
		}
	}

	if(i != 0) {
		memset(&buf[i], 0x20, lrecl - i);
		R7Encode(out, buf, lrecl7);
		out += lrecl7;
		i = 0;
	}

	free(buf);
}

void recfmt6(u8* out, unsigned int lrecl, const char* src)
{
	char c;
	unsigned int i = 0;
	unsigned int ts = 8;
	unsigned int lrecl6 = (lrecl + 3) / 4 * 3;
	char* buf = (char*) malloc(lrecl + 1);
	while((c = *(src++))) {
		if(c == '\n') {
			memset(&buf[i], 0x20, lrecl - i);
			R6Encode(out, buf, lrecl6);
			out += lrecl6;
			i = 0;
		} else if(i < lrecl) {
			if(c == '\t') {
				/* process tab */
				if(i % ts == 0) {
					for(unsigned int n = 0; n < ts; n++) {
						if(i < lrecl) {
							buf[i++] = ' ';
						}
					}
				} else {
					while(i < lrecl && (i % ts) != 0) {
						if(i < lrecl) {
							buf[i++] = ' ';
						}
					}
				}
			} else if(c < 0x20) {
				buf[i++] = ' ';
			} else {
				buf[i++] = (char) c;
			}
		}
	}

	if(i != 0) {
		memset(&buf[i], 0x20, lrecl - i);
		R6Encode(out, buf, lrecl6);
		out += lrecl6;
		i = 0;
	}

	free(buf);
}
