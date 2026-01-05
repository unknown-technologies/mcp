#include <emulight/ispf.h>

#include "game.h"

void print(const char* s)
{
	for(; *s; s++) {
		if(*s == '\n') {
			VTWrite("\r\n", 2);
		} else {
			VTPutc(*s);
		}
	}
}

void printn(const char* s, unsigned int n)
{
	while(n && *s) {
		if(*s == '\n') {
			VTWrite("\r\n", 2);
		} else {
			VTPutc(*s);
		}
		n--;
		s++;
	}
}

void printindent(SPFPanel* self, const char* s, unsigned int border)
{
	const char* start = s;
	const char* space = s;
	unsigned int count = 0;

	unsigned int width = VTGetWidth(&self->spf->vt);

	while(*s) {
		if(count == (width - 2 * border)) {
			/* print indent */
			for(unsigned int i = 0; i < border; i++) {
				VTPutc(' ');
			}

			if(*s == ' ') {
				s++;
				printn(start, s - start);
			} else {
				printn(start, space - start);
				s = space;
			}

			while(*s == ' ') {
				s++;
			}

			start = s;
			VTWrite("\r\n", 2);
			count = 0;
			space = start;
		}

		if(*s == ' ') {
			if(space < s - 1) {
				space = s;
			}
			s++;
			count++;
		} else {
			if(*s == '\n') {
				for(unsigned int i = 0; i < border; i++) {
					VTPutc(' ');
				}
				printn(start, s - start);
				VTWrite("\r\n", 2);
				s++;
				count = 0;
				start = s;
				space = s;
			} else {
				s++;
				count++;
			}
		}
	}

	if(count) {
		for(unsigned int i = 0; i < border; i++) {
			VTPutc(' ');
		}
		print(start);
	}
}
