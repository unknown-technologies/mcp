#include <stdarg.h>

#include <emulight/types.h>
#include <emulight/string.h>

static char* __sprinth(char* result, unsigned int resultlen, unsigned int x)
{
	const char* letters = "0123456789ABCDEF";
	char* p = &result[resultlen - 1];
	*p = 0;
	if(x == 0) {
		*(--p) = '0';
		return p;
	}
	while(x != 0) {
		*(--p) = letters[x & 0x0F];
		x >>= 4;
	}
	return p;
}

static char* __sprinto(char* result, unsigned int resultlen, unsigned int x)
{
	char* p = &result[resultlen - 1];
	*p = 0;
	if(x == 0) {
		*(--p) = '0';
		return p;
	}
	while(x != 0) {
		*(--p) = (x & 0x07) + '0';
		x >>= 3;
	}
	return p;
}

static char* __sprintd(char* result, unsigned int resultlen, unsigned int x)
{
	int neg = 0;
	char* p = &result[resultlen - 1];
	*p = 0;
	if(x == 0) {
		*(--p) = '0';
		return p;
	}
	if((int) x < 0) {
		neg = 1;
		x = -x;
	}
	while(x != 0) {
		*(--p) = (x % 10) + 0x30;
		x /= 10;
	}
	if(neg)
		*(--p) = '-';
	return p;
}

static char* __sprintu(char* result, unsigned int resultlen, unsigned int x)
{
	char* p = &result[resultlen - 1];
	*p = 0;
	if(x == 0) {
		*(--p) = '0';
		return p;
	}
	while(x != 0) {
		*(--p) = (x % 10) + 0x30;
		x /= 10;
	}
	return p;
}

#ifdef LONG64
static char* __sprinth64(char* result, unsigned int resultlen, u64 x)
{
	const char* letters = "0123456789ABCDEF";
	char* p = &result[resultlen - 1];
	*p = 0;
	if(x == 0) {
		*(--p) = '0';
		return p;
	}
	while(x != 0) {
		*(--p) = letters[x & 0x0F];
		x >>= 4;
	}
	return p;
}

static char* __sprintd64(char* result, unsigned int resultlen, u64 x)
{
	int neg = 0;
	char* p = &result[resultlen - 1];
	*p = 0;
	if(x == 0) {
		*(--p) = '0';
		return p;
	}
	if((s64) x < 0) {
		neg = 1;
		x = -x;
	}
	while(x != 0) {
		*(--p) = (x % 10) + 0x30;
		x /= 10;
	}
	if(neg)
		*(--p) = '-';
	return p;
}

static char* __sprintu64(char* result, unsigned int resultlen, u64 x)
{
	char* p = &result[resultlen - 1];
	*p = 0;
	if(x == 0) {
		*(--p) = '0';
		return p;
	}
	while(x != 0) {
		*(--p) = (x % 10) + 0x30;
		x /= 10;
	}
	return p;
}
#elif defined(LONG32)
static char* __sprinth32(char* result, unsigned int resultlen, u32 x)
{
	const char* letters = "0123456789ABCDEF";
	char* p = &result[resultlen - 1];
	*p = 0;
	if(x == 0) {
		*(--p) = '0';
		return p;
	}
	while(x != 0) {
		*(--p) = letters[x & 0x0F];
		x >>= 4;
	}
	return p;
}

static char* __sprintd32(char* result, unsigned int resultlen, u32 x)
{
	u32 q, r;

	int neg = 0;
	char* p = &result[resultlen - 1];
	*p = 0;
	if(x == 0) {
		*(--p) = '0';
		return p;
	}
	if((s32) x < 0) {
		neg = 1;
		x = -x;
	}
	while(x != 0) {
#if 0
		ESLDiv32(&q, &r, x, 10);
		*(--p) = (u8) r + 0x30;
		x = q;
#else
		*(--p) = (x % 10LL) + 0x30;
		x /= 10LL;
#endif
	}
	if(neg)
		*(--p) = '-';
	return p;
}

static char* __sprintu32(char* result, unsigned int resultlen, u32 x)
{
	u32 q, r;

	char* p = &result[resultlen - 1];
	*p = 0;
	if(x == 0) {
		*(--p) = '0';
		return p;
	}
	while(x != 0) {
#if 0
		ESLDiv32(&q, &r, x, 10);
		*(--p) = (u8) r + 0x30;
		x = q;
#else
		*(--p) = (x % 10LL) + 0x30;
		x /= 10LL;
#endif
	}
	return p;
}
#else
static char* __sprintlh(char* result, unsigned int resultlen, unsigned long x)
{
	const char* letters = "0123456789ABCDEF";
	char* p = &result[resultlen - 1];
	*p = 0;
	if(x == 0) {
		*(--p) = '0';
		return p;
	}
	while(x != 0) {
		*(--p) = letters[x & 0x0F];
		x >>= 4;
	}
	return p;
}

static char* __sprintld(char* result, unsigned int resultlen, unsigned long x)
{
	int neg = 0;
	char* p = &result[resultlen - 1];
	*p = 0;
	if(x == 0) {
		*(--p) = '0';
		return p;
	}
	if((s64) x < 0) {
		neg = 1;
		x = -x;
	}
	while(x != 0) {
		*(--p) = (x % 10) + 0x30;
		x /= 10;
	}
	if(neg)
		*(--p) = '-';
	return p;
}

static char* __sprintlu(char* result, unsigned int resultlen, unsigned long x)
{
	char* p = &result[resultlen - 1];
	*p = 0;
	if(x == 0) {
		*(--p) = '0';
		return p;
	}
	while(x != 0) {
		*(--p) = (x % 10) + 0x30;
		x /= 10;
	}
	return p;
}
#endif

#define TMP_SIZE 32

#define	WR(x) { \
	if(buf) { \
		*(p++) = x; \
	} else { \
		p++; \
	} \
	if(cb) { \
		cb(x); \
	} \
}

int ivsprintf(char* buf, const char* s, va_list args, void (*cb)(char))
{
	char tmp[TMP_SIZE];
	char* c;
	char* p = buf;
	unsigned int n;
	char pad = 0;
	int t;
	for(; *s != 0; s++) {
		if(*s == '%') {
			pad = 0;
			n = 0;
			s++;
			if(*s == '0') {
				s++;
				pad = '0';
			}
			while((*s >= '0') && (*s <= '9')) {
				n *= 10;
				n += *s - '0';
				s++;
				if(!pad) {
					pad = ' ';
				}
			}
			if(*s == 'c' || *s == 'C') {
				int val = va_arg(args, int);
				WR(val);
			} else if(*s == 'd' || *s == 'D') {
				int val = va_arg(args, int);
				c = __sprintd(tmp, TMP_SIZE, val);
				t = n - strlen(c);
				if(pad) {
					for(; t > 0; t--) {
						WR(pad);
					}
				}
				for(; *c != 0; c++) {
					WR(*c);
				}
			} else if(*s == 'u' || *s == 'U') {
				unsigned int val = va_arg(args, unsigned int);
				c = __sprintu(tmp, TMP_SIZE, val);
				t = n - strlen(c);
				if(pad) {
					for(; t > 0; t--) {
						WR(pad);
					}
				}
				for(; *c != 0; c++) {
					WR(*c);
				}
			} else if(*s == 'x') {
				int val = va_arg(args, int);
				c = __sprinth(tmp, TMP_SIZE, val);
				t = n - strlen(c);
				if(pad) {
					for(; t > 0; t--) {
						WR(pad);
					}
				}
				for(; *c != 0; c++) {
					WR(*c | 0x20);
				}
			} else if(*s == 'X') {
				int val = va_arg(args, int);
				c = __sprinth(tmp, TMP_SIZE, val);
				t = n - strlen(c);
				if(pad) {
					for(; t > 0; t--) {
						WR(pad);
					}
				}
				for(; *c != 0; c++) {
					WR(*c);
				}
			} else if(*s == 'o' || *s == 'O') {
				int val = va_arg(args, int);
				c = __sprinto(tmp, TMP_SIZE, val);
				t = n - strlen(c);
				if(pad) {
					for(; t > 0; t--) {
						WR(pad);
					}
				}
				for(; *c != 0; c++) {
					WR(*c);
				}
			} else if(*s == 'l' || *s == 'L') {
				s++;
				if(*s == 'd' || *s == 'D') {
#ifdef LONG32
					u32 val = va_arg(args, u32);
					c = __sprintd32(tmp, TMP_SIZE, val);
#elif defined(LONG64)
					u64 val = va_arg(args, u64);
					c = __sprintd64(tmp, TMP_SIZE, val);
#else
					unsigned long val = va_arg(args, unsigned long);
					c = __sprintld(tmp, TMP_SIZE, val);
#endif
					t = n - strlen(c);
					if(pad) {
						for(; t > 0; t--) {
							WR(pad);
						}
					}
					for(; *c != 0; c++) {
						WR(*c);
					}
				} else if(*s == 'u' || *s == 'U') {
#ifdef LONG32
					u32 val = va_arg(args, u32);
					c = __sprintu32(tmp, TMP_SIZE, val);
#elif defined(LONG64)
					u64 val = va_arg(args, u64);
					c = __sprintu64(tmp, TMP_SIZE, val);
#else
					unsigned long val = va_arg(args, unsigned long);
					c = __sprintlu(tmp, TMP_SIZE, val);
#endif
					t = n - strlen(c);
					if(pad) {
						for(; t > 0; t--) {
							WR(pad);
						}
					}
					for(; *c != 0; c++) {
						WR(*c);
					}
				} else if(*s == 'x') {
#ifdef LONG32
					u32 val = va_arg(args, u32);
					c = __sprinth(tmp, TMP_SIZE, val);
#elif defined(LONG64)
					u64 val = va_arg(args, u64);
					c = __sprinth64(tmp, TMP_SIZE, val);
#else
					unsigned long val = va_arg(args, unsigned long);
					c = __sprintlh(tmp, TMP_SIZE, val);
#endif
					t = n - strlen(c);
					if(pad) {
						for(; t > 0; t--) {
							WR(pad);
						}
					}
					for(; *c != 0; c++) {
						WR(*c | 0x20);
					}
				} else if(*s == 'X') {
#ifdef LONG32
					u32 val = va_arg(args, u32);
					c = __sprinth32(tmp, TMP_SIZE, val);
#elif defined(LONG64)
					u64 val = va_arg(args, u64);
					c = __sprinth64(tmp, TMP_SIZE, val);
#else
					unsigned long val = va_arg(args, unsigned long);
					c = __sprintlh(tmp, TMP_SIZE, val);
#endif
					t = n - strlen(c);
					if(pad) {
						for(; t > 0; t--) {
							WR(pad);
						}
					}
					for(; *c != 0; c++) {
						WR(*c);
					}
				}
			} else if(*s == 's' || *s == 'S') {
				c = (char*)va_arg(args, char*);
				t = n - strlen(c);
				if(pad) {
					for(; t > 0; t--) {
						WR(' ');
					}
				}
				for(; *c != 0; c++) {
					WR(*c);
				}
			} else if(*s == '%') {
				WR('%');
			}
		} else {
			WR(*s);
		}
	}
	if(buf) {
		*(p++) = 0;
		return p - buf - 1;
	} else {
		return p - buf;
	}
}

int vsprintf(char* buf, const char* s, va_list args)
{
	return ivsprintf(buf, s, args, NULL);
}

int vcsprintf(void (*cb)(char), const char* s, va_list args)
{
	return ivsprintf(NULL, s, args, cb);
}

int sprintf(char* buf, const char* format, ...)
{
	va_list args;
	int length;
	va_start(args, format);
	length = vsprintf(buf, format, args);
	va_end(args);

	return length;
}

int csprintf(void (*cb)(char), const char* format, ...)
{
	va_list args;
	int length;
	va_start(args, format);
	length = vcsprintf(cb, format, args);
	va_end(args);

	return length;
}
