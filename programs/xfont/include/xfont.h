#ifndef __XFONT_H__
#define __XFONT_H__

#include <emulight/ispf.h>

typedef struct {
	u8	bits[10];
} GLYPH;

typedef struct {
	GLYPH	glyph[94];
	u8	current_glyph;
	u8	cursor_x;
	u8	cursor_y;
} XFONT;

extern const unsigned char vt220_font[2880];

void	UIInit(SPFPanel* self);
void	UIDraw(SPFPanel* self);
void	UIProcess(SPFPanel* self, u16 key);
void	UIDestroy(SPFPanel* self);

#endif
