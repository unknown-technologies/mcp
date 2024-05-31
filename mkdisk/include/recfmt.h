#ifndef __RECFMT_H__
#define __RECFMT_H__

void recfmt6(u8* out, unsigned int lrecl, const char* src);
void recfmt7(u8* out, unsigned int lrecl, const char* src);
void recfmt8(char* out, unsigned int lrecl, const char* src);

#endif
