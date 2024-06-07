#ifndef __STRING_H__
#define __STRING_H__

#include <stdarg.h>
#include <stddef.h>

#include <emulight/types.h>

/* ========================================================================== */
/* String functions to avoid huge libc dependencies                           */
/* ========================================================================== */

void	bzero(void* s, size_t n);
void*	memcpy(void* __restrict dst, const void* __restrict src, size_t len);
int	memcmp(const void* s1, const void* s2, size_t n);
void*	memset(void* m, int c, size_t n);
void*	memchr(const void* src, int c, size_t length);
void*	memmem(const void* haystack, size_t hs_len, const void* needle, size_t ne_len);

size_t	strlen(const char* s);
char*	strcat(char* __restrict s1, const char* __restrict s2);
char*	strcpy(char* dst, const char* src);
int	strcmp(const char* s1, const char* s2);
char*	strchr(const char* s1, int i);
char*	strrchr(const char* s, int i);
char*	strstr(const char* haystack, const char* needle);

size_t	strnlen(const char* str, size_t n);
char*	strncat(char* __restrict s1, const char* __restrict s2, size_t n);
char*	strncpy(char* __restrict dst, const char* __restrict src, size_t count);
int	strncmp(const char* s1, const char* s2, size_t n);
char*	strnstr(const char* haystack, const char* needle, size_t haystack_len);

int	sprintf(char* buf, const char* fmt, ...);
int	vsprintf(char* buf, const char* fmt, va_list args);
int	csprintf(void (*cb)(char), const char* fmt, ...);
int	vcsprintf(void (*cb)(char), const char* fmt, va_list args);

char	toupper(char c);

#endif
