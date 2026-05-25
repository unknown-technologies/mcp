#include <emulight/string.h>
#include <emulight/ispf.h>
#include "xfont.h"

static XFONT* xfont = NULL;

#define	EDITOR_X	40
#define	EDITOR_Y	6

static void UIReload(SPF* spf, const SPFAction* action);
static void UILoadGlyph(SPF* spf, const SPFAction* action);
static void UIShow(SPF* spf, const SPFAction* action);

static const SPFAction actions[] = {
	{ .key = VT_KEY_F9,  .name = "Reload",         .action = UIReload },
	{ .key = VT_KEY_F11, .name = "Download Glyph", .action = UILoadGlyph },
	{ .key = VT_KEY_F12, .name = "Show",           .action = UIShow },
	{ .key = 0, .name = NULL, .action = NULL }
};

static void UIReload(SPF* spf, const SPFAction* action)
{
	VTDECDLD(0x21, TRUE, VT_MATRIX_7X10, VT_WIDTH_80, FALSE,
			xfont->glyph[0].bits, 94);
}

static void UILoadGlyph(SPF* spf, const SPFAction* action)
{
	GLYPH* glyph = &xfont->glyph[xfont->current_glyph];

	VTDECDLD(xfont->current_glyph + 0x21, FALSE, VT_MATRIX_7X10,
			VT_WIDTH_80, FALSE, glyph->bits, 1);
}

static void UIShowDECDLD(unsigned char pcn, BOOL erase, unsigned int matrix,
	unsigned int width, BOOL full_cell, u8* bits,
	unsigned int count)
{
	char sixels[20];
	bzero(sixels, sizeof(sixels));
	sixels[8] = '/';

	char* sixels_top = sixels;
	char* sixels_bot = &sixels[9];

	/* start download */
	VTPrintf(CSI "7mDCS" CSI "0m1;%d;%d;%d;%d;%d{ @", pcn - 0x20,
			erase ? 0 : 1, matrix, width, full_cell ? 2 : 1);

	for(unsigned int i = 0; i < count; i++) {
		u8* glyph = &bits[i * 10];
		bzero(sixels_top, 8);
		bzero(sixels_bot, 8);

		/* compute sixel patterns */
		for(int y = 5; y >= 0; y--) {
			u8 line = glyph[y];
			for(unsigned int x = 0; x < 8; x++) {
				BOOL bit = (line << x) & 0x80;
				sixels_top[x] <<= 1;
				if(bit) {
					sixels_top[x] |= 1;
				}
			}
		}

		for(int y = 9; y > 5; y--) {
			u8 line = glyph[y];
			for(unsigned int x = 0; x < 8; x++) {
				BOOL bit = (line << x) & 0x80;
				sixels_bot[x] <<= 1;
				if(bit) {
					sixels_bot[x] |= 1;
				}
			}
		}

		for(unsigned int i = 0; i < 8; i++) {
			sixels_bot[i] += 077;
			sixels_top[i] += 077;
		}

		VTPrint(sixels);

		/* separator between glyphs */
		if(i + 1 < count) {
			VTPutc(';');
		}
	}

	VTWrite(CSI "7mST" CSI "0m", 2 * CSILEN + 6);
}

static void UIShow(SPF* spf, const SPFAction* action)
{
	GLYPH* glyph = &xfont->glyph[xfont->current_glyph];

	VTGoto(21, 32);
	VTEL(0);

	UIShowDECDLD(xfont->current_glyph + 0x21, FALSE, VT_MATRIX_7X10,
			VT_WIDTH_80, FALSE, glyph->bits, 1);
}

void UIInit(SPFPanel* self)
{
	SPFHideCursor(self);
	SPFSetPanelTitle(self, "XFONT");

	xfont = MEMAlloc(sizeof(XFONT));
	if(!xfont) {
		SPFDestroyPanel(self);
		return;
	}

	bzero(xfont, sizeof(XFONT));

	memcpy(xfont->glyph, &vt220_font[0x21 * 10], 94 * 10);

	SPFSetActions(self, actions);

	/* initial font download */
	VTDECDLD(0x21, TRUE, VT_MATRIX_7X10, VT_WIDTH_80, FALSE,
			xfont->glyph[0].bits, 94);
}

void UIDrawBit(SPFPanel* self, unsigned int x, unsigned int y)
{
	GLYPH* glyph = &xfont->glyph[xfont->current_glyph];
	u8 bits = glyph->bits[y];

	BOOL bit = (bits << x) & 0x80;

	BOOL cursor = xfont->cursor_y == y &&
		xfont->cursor_x == x;

	VTGoto(EDITOR_Y + 2 + y, EDITOR_X + 2 + 2 * x);

	if(cursor) {
		VTWrite(CSI "7m", CSILEN + 2);
	}

	if(bit) {
		VTPutc('*');
	} else {
		/* alternatively: '.' */
		VTPutc('\xB7');
	}

	if(cursor) {
		VTWrite(CSI "0m", CSILEN + 2);
	}
}

void UIUpdateBit(SPFPanel* self, unsigned int x, unsigned int y)
{
	UIDrawBit(self, x, y);

	GLYPH* glyph = &xfont->glyph[xfont->current_glyph];
	u8 bits = glyph->bits[y];

	u8 stretched_bits = bits | (bits >> 1);

	VTGoto(EDITOR_Y + 2 + y, EDITOR_X  + 2 + 16 + 6 + x);

	for(int col = x; col <= (x + 1) && col < 8; col++) {
		BOOL bit = (stretched_bits << col) & 0x80;

		if(bit) {
			VTPutc('*');
		} else {
			VTPutc(' ');
		}
	}

	if(x == 7) {
		for(int col = 8; col < 10; col++) {
			BOOL bit = bits & 1;

			if(bit) {
				VTPutc('*');
			} else {
				VTPutc(' ');
			}
		}
	}
}

void UIUpdateGlyphGrid(SPFPanel* self, unsigned char c)
{
	unsigned int line = c / 10;
	unsigned int col = c % 10;
	VTGoto(2 + 2 * line + 1, 2 + 2 * col);

	if(c == xfont->current_glyph) {
		VTWrite(CSI "7m", CSILEN + 2);
	}
	if(c >= 94) {
		VTPutc(' ');
	} else {
		VTDRCS(c + 0x21);
	}
	if(c == xfont->current_glyph) {
		VTWrite(CSI "0m", CSILEN + 2);
	}
}

void UIUpdateGlyphEditor(SPFPanel* self)
{
	GLYPH* glyph = &xfont->glyph[xfont->current_glyph];

	VTGoto(4, 35);
	VTPrintf("%02X", (unsigned int) (xfont->current_glyph + 0x21));
	VTGoto(5, 35);
	VTPutc(xfont->current_glyph + 0x21);

	for(int line = 0; line < 10; line++) {
		VTGoto(EDITOR_Y + 2 + line, EDITOR_X);
		VTPutc('0' + line);

		VTGraphics();
		VTPutc('x');
		VTAscii();

		u8 bits = glyph->bits[line];
		for(int col = 0; col < 8; col++) {
			BOOL bit = (bits << col) & 0x80;

			BOOL cursor = xfont->cursor_y == line &&
				xfont->cursor_x == col;

			if(cursor) {
				VTWrite(CSI "7m", CSILEN + 2);
			}

			if(bit) {
				VTPutc('*');
			} else {
				/* alternatively: '.' */
				VTPutc('\xB7');
			}

			if(cursor) {
				VTWrite(CSI "0m", CSILEN + 2);
			}

			if(col < 7) {
				VTPutc(' ');
			}
		}

		VTGraphics();
		VTPutc('x');
		VTPutc('0' + line);
		VTWrite("    x", 5);
		VTAscii();

		u8 stretched_bits = bits | (bits >> 1);
		for(int col = 0; col < 8; col++) {
			BOOL bit = (stretched_bits << col) & 0x80;

			if(bit) {
				VTPutc('*');
			} else {
				VTPutc(' ');
			}
		}

		for(int col = 8; col < 10; col++) {
			BOOL bit = bits & 1;

			if(bit) {
				VTPutc('*');
			} else {
				/* preview only shows space here */
				VTPutc(' ');
			}
		}

		VTGraphics();
		VTPutc('x');
		VTAscii();
	}

	UIShow(self->spf, NULL);
}

void UIDraw(SPFPanel* self)
{
	VTGoto(4, 24);
	VTWrite("Char Code:", 10);
	VTGoto(5, 24);
	VTWrite("ASCII:", 6);
	VTGoto(21, 24);
	VTWrite("DECDLD:", 7);

	for(int line = 0; line < 10; line++) {
		VTGoto(2 + 2 * line, 1);
		VTGraphics();
		if(line == 0) {
			VTWrite("lqwqwqwqwqwqwqwqwqwqk", 21);
		} else {
			VTWrite("tqnqnqnqnqnqnqnqnqnqu", 21);
		}
		VTGoto(2 + 2 * line + 1, 1);
		VTPutc('x');
		VTAscii();

		for(int col = 0; col < 10; col++) {
			unsigned char c = line * 10 + col;
			if(c == xfont->current_glyph) {
				VTWrite(CSI "7m", CSILEN + 2);
			}
			if(c >= 94) {
				VTPutc(' ');
			} else {
				VTDRCS(c + 0x21);
			}
			if(c == xfont->current_glyph) {
				VTWrite(CSI "0m", CSILEN + 2);
			}

			VTGraphics();
			VTPutc('x');
			VTAscii();
		}
	}
	VTGoto(2 + 20, 1);
	VTGraphics();
	VTWrite("mqvqvqvqvqvqvqvqvqvqj", 21);
	VTAscii();

	VTGoto(EDITOR_Y, EDITOR_X + 2);
	for(int col = 0; col < 8; col++) {
		VTPutc('0' + col);
		VTPutc(' ');
	}

	VTGoto(EDITOR_Y, EDITOR_X + 2 + 16 + 6 + (10 - 7) / 2);
	VTWrite("Preview", 7);

	VTGoto(EDITOR_Y + 1, EDITOR_X + 1);
	VTGraphics();
	VTWrite("lqqqqqqqqqqqqqqqk     lqqqqqqqqqqk", 34);
	VTGoto(EDITOR_Y + 12, EDITOR_X + 1);
	VTWrite("mqqqqqqqqqqqqqqqj     mqqqqqqqqqqj", 34);
	VTAscii();

	UIUpdateGlyphEditor(self);

	VTGoto(EDITOR_Y + 13, EDITOR_X + 2);
	for(int col = 0; col < 8; col++) {
		VTPutc('0' + col);
		VTPutc(' ');
	}
}

void UIProcess(SPFPanel* self, u16 key)
{
	switch(key) {
		case VT_KEY_LEFT:
			if(xfont->cursor_x > 0) {
				xfont->cursor_x--;
				UIDrawBit(self, xfont->cursor_x + 1,
						xfont->cursor_y);
				UIDrawBit(self, xfont->cursor_x,
						xfont->cursor_y);
			}
			break;
		case VT_KEY_RIGHT:
			if(xfont->cursor_x < 7) {
				xfont->cursor_x++;
				UIDrawBit(self, xfont->cursor_x - 1,
						xfont->cursor_y);
				UIDrawBit(self, xfont->cursor_x,
						xfont->cursor_y);
			}
			break;
		case VT_KEY_UP:
			if(xfont->cursor_y > 0) {
				xfont->cursor_y--;
				UIDrawBit(self, xfont->cursor_x,
						xfont->cursor_y + 1);
				UIDrawBit(self, xfont->cursor_x,
						xfont->cursor_y);
			}
			break;
		case VT_KEY_DOWN:
			if(xfont->cursor_y < 9) {
				xfont->cursor_y++;
				UIDrawBit(self, xfont->cursor_x,
						xfont->cursor_y - 1);
				UIDrawBit(self, xfont->cursor_x,
						xfont->cursor_y);
			}
			break;
		case VT_KEY_SPACE:
		case VT_KEY_ENTER:
		case VT_KEY_KP_ENTER: {
			GLYPH* glyph = &xfont->glyph[xfont->current_glyph];
			u8 bit = 1 << (7 - xfont->cursor_x);
			glyph->bits[xfont->cursor_y] ^= bit;
			UIUpdateBit(self, xfont->cursor_x, xfont->cursor_y);
			break;
		}
		case 'a':
		case 'A':
			if(xfont->current_glyph > 0) {
				xfont->current_glyph--;
				UIUpdateGlyphGrid(self,
						xfont->current_glyph + 1);
				UIUpdateGlyphGrid(self,
						xfont->current_glyph);
				UIUpdateGlyphEditor(self);
			}
			break;
		case 'd':
		case 'D':
			if(xfont->current_glyph < 93) {
				xfont->current_glyph++;
				UIUpdateGlyphGrid(self,
						xfont->current_glyph - 1);
				UIUpdateGlyphGrid(self,
						xfont->current_glyph);
				UIUpdateGlyphEditor(self);
			}
			break;
		case 'w':
		case 'W':
			if(xfont->current_glyph >= 10) {
				xfont->current_glyph -= 10;
				UIUpdateGlyphGrid(self,
						xfont->current_glyph + 10);
				UIUpdateGlyphGrid(self,
						xfont->current_glyph);
				UIUpdateGlyphEditor(self);
			}
			break;
		case 's':
		case 'S':
			if(xfont->current_glyph < 84) {
				xfont->current_glyph += 10;
				UIUpdateGlyphGrid(self,
						xfont->current_glyph - 10);
				UIUpdateGlyphGrid(self,
						xfont->current_glyph);
				UIUpdateGlyphEditor(self);
			}
			break;
	}
}

void UIDestroy(SPFPanel* self)
{
	if(xfont) {
		MEMFree(xfont);
		xfont = NULL;
	}
}
