#ifndef __AMTA_H__
#define __AMTA_H__

#include <emulight/vt.h>

extern VT vt;

void	amta(void);

void	putc(char c);
void	print(const char* s);
void	printn(const char* s, unsigned int n);
void	printindent(const char* s, unsigned int border);

#endif
