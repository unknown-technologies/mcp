#include <stdarg.h>
#include <emulight/types.h>
#include <emulight/string.h>
#include <emulight/vt.h>
#include <emulight/si.h>

/* the FAST option avoids slow sprintf calls;
 * terminal size is limited to 99 lines and 999 columns */
#define FAST

void VTInit(VT* vt)
{
	vt->width = 80;
	vt->height = 24;
	vt->state = VT_STATE_NORMAL;
}

/******************************************************************************/
/* VT input routines                                                          */
/******************************************************************************/
u16 VTiReceive(VT* vt, u8 c)
{
	switch(vt->state) {
		case VT_STATE_NORMAL:
			switch(c) {
				case 0x1B:
					vt->state = VT_STATE_ESC;
					return VT_KEY_NONE;
				case 0x9B:
					vt->state = VT_STATE_CSI;
					vt->value = 0;
					return VT_KEY_NONE;
				case 0x8F: /* SS3 */
					vt->state = VT_STATE_SS3;
					return VT_KEY_NONE;
				default:
					return c;
			}
			break;
		case VT_STATE_ESC:
			switch(c) {
				case '[':
					vt->state = VT_STATE_CSI;
					vt->value = 0;
					return VT_KEY_NONE;
				case 'O':
					vt->state = VT_STATE_SS3;
					return VT_KEY_NONE;
				case 0x1B: /* ESC */
					return VT_KEY_NONE;
				case 0x9B: /* CSI */
					vt->state = VT_STATE_CSI;
					vt->value = 0;
					return VT_KEY_NONE;
				default:
					vt->state = VT_STATE_NORMAL;
					return c;
			}
			break;
		case VT_STATE_CSI:
			switch(c) {
				case 0x1B: /* ESC */
					vt->state = VT_STATE_ESC;
					return VT_KEY_NONE;
				case 0x9B: /* CSI */
					vt->value = 0;
					return VT_KEY_NONE;
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					vt->value *= 10;
					vt->value += c - '0';
					return VT_KEY_NONE;
				case '~':
					vt->state = VT_STATE_NORMAL;
					switch(vt->value) {
						case 1:
							return VT_KEY_FIND;
						case 2:
							return VT_KEY_INSERT;
						case 3:
							return VT_KEY_REMOVE;
						case 4:
							return VT_KEY_SELECT;
						case 5:
							return VT_KEY_PSCREEN;
						case 6:
							return VT_KEY_NSCREEN;
						case 17:
							return VT_KEY_F6;
						case 18:
							return VT_KEY_F7;
						case 19:
							return VT_KEY_F8;
						case 20:
							return VT_KEY_F9;
						case 21:
							return VT_KEY_F10;
						case 23:
							return VT_KEY_F11;
						case 24:
							return VT_KEY_F12;
						case 25:
							return VT_KEY_F13;
						case 26:
							return VT_KEY_F14;
						case 28:
							return VT_KEY_F15;
						case 29:
							return VT_KEY_F16;
						case 31:
							return VT_KEY_F17;
						case 32:
							return VT_KEY_F18;
						case 33:
							return VT_KEY_F19;
						case 34:
							return VT_KEY_F20;
						default:
							return VT_KEY_NONE;
					}
					break;
				case 'A':
					vt->state = VT_STATE_NORMAL;
					return VT_KEY_UP;
				case 'B':
					vt->state = VT_STATE_NORMAL;
					return VT_KEY_DOWN;
				case 'C':
					vt->state = VT_STATE_NORMAL;
					return VT_KEY_RIGHT;
				case 'D':
					vt->state = VT_STATE_NORMAL;
					return VT_KEY_LEFT;
				default:
					vt->state = VT_STATE_NORMAL;
					return VT_KEY_NONE;
			}
			break;
		case VT_STATE_SS3:
			switch(c) {
				case 'A':
					vt->state = VT_STATE_NORMAL;
					return VT_KEY_UP;
				case 'B':
					vt->state = VT_STATE_NORMAL;
					return VT_KEY_DOWN;
				case 'C':
					vt->state = VT_STATE_NORMAL;
					return VT_KEY_RIGHT;
				case 'D':
					vt->state = VT_STATE_NORMAL;
					return VT_KEY_LEFT;
				case 'p':
				case 'q':
				case 'r':
				case 's':
				case 't':
				case 'u':
				case 'v':
				case 'w':
				case 'x':
				case 'y':
					vt->state = VT_STATE_NORMAL;
					return VT_KEY_KP_0 + (c - 'p');
				case 'm':
					vt->state = VT_STATE_NORMAL;
					return VT_KEY_KP_MINUS;
				case 'l':
					vt->state = VT_STATE_NORMAL;
					return VT_KEY_KP_COMMA;
				case 'n':
					vt->state = VT_STATE_NORMAL;
					return VT_KEY_KP_PERIOD;
				case 'M':
					vt->state = VT_STATE_NORMAL;
					return VT_KEY_KP_ENTER;
				case 'P':
				case 'Q':
				case 'R':
				case 'S':
					vt->state = VT_STATE_NORMAL;
					return VT_KEY_KP_PF1 + (c - 'P');
				default:
					vt->state = VT_STATE_NORMAL;
					return VT_KEY_NONE;
			}
			break;
	}

	return VT_KEY_NONE;
}

u16 VTReceive(VT* vt, u8* buf, u8* len)
{
	u8 c;

	if(buf && len) {
		*len = 0;
	}

	while(SIReceiveByte((u8*) &c)) {
		if(buf && len) {
			buf[(*len)++] = c;
		}
		u16 key = VTiReceive(vt, c);
		if(key != VT_KEY_NONE) {
			return key;
		}
	}

	return VT_KEY_NONE;
}

/******************************************************************************/
/* VT output routines                                                         */
/******************************************************************************/
void VTCSI(const char* s)
{
	size_t len = strlen(s);
	VTWrite(CSI, CSILEN);
	VTWrite(s, len);
}

void VTESC(const char* s)
{
	size_t len = strlen(s);
	VTWrite("\x1b", 1);
	VTWrite(s, len);
}

void VTHome(void)
{
	VTWrite(CSI "H", CSILEN + 1);
}

void VTGoto(int line, int column)
{
	char buf[16];
#ifdef FAST
	char* p = buf + CSILEN;
	memcpy(buf, CSI, CSILEN);
	*(p++) = (line / 10) + '0';
	*(p++) = (line % 10) + '0';
	if(column != 1) {
		*(p++) = ';';
		if(column > 99) {
			*(p++) = (column / 100) + '0';
			column %= 100;
		}
		*(p++) = (column / 10) + '0';
		*(p++) = (column % 10) + '0';
	}
	*(p++) = 'H';
	VTWrite(buf, p - buf);
#else
	int len = sprintf(buf, CSI "%d;%dH", line, column);
	VTWrite(buf, len);
#endif
}

void VTGotoLine(int line)
{
	char buf[16];
#ifdef FAST
	char* p = buf + CSILEN;
	memcpy(buf, CSI, CSILEN);
	p[0] = (line / 10) + '0';
	p[1] = (line % 10) + '0';
	p[2] = 'H';
	VTWrite(buf, CSILEN + 3);
#else
	int len = sprintf(buf, CSI "%dH", line);
	VTWrite(buf, len);
#endif
}

void VTCUU(int n)
{
	if(n == 1) {
		VTWrite(CSI "A", CSILEN + 1);
	} else {
		char buf[8];
		int len = sprintf(buf, CSI "%dA", n);
		VTWrite(buf, len);
	}
}

void VTCUD(int n)
{
	if(n == 1) {
		VTWrite(CSI "B", CSILEN + 1);
	} else {
		char buf[8];
		int len = sprintf(buf, CSI "%dB", n);
		VTWrite(buf, len);
	}
}

void VTCUF(int n)
{
	if(n == 1) {
		VTWrite(CSI "C", CSILEN + 1);
	} else {
		char buf[8];
		int len = sprintf(buf, CSI "%dC", n);
		VTWrite(buf, len);
	}
}

void VTCUB(int n)
{
	if(n == 1) {
		VTWrite(CSI "D", CSILEN + 1);
	} else {
		char buf[8];
		int len = sprintf(buf, CSI "%dD", n);
		VTWrite(buf, len);
	}
}

void VTEL(int x)
{
	char buf[8];
	int len = sprintf(buf, CSI "%dK", x);
	VTWrite(buf, len);
}

void VTEL2(void)
{
	VTWrite(CSI "2K", CSILEN + 2);
}

void VTED(int x)
{
	char buf[8];
	int len = sprintf(buf, CSI "%dJ", x);
	VTWrite(buf, len);
}

void VTED2(void)
{
	VTWrite(CSI "2J", CSILEN + 2);
}

void VTDCH(int n)
{
	char buf[8];
	int len = sprintf(buf, CSI "%dP", n);
	VTWrite(buf, len);
}

void VTECH(int n)
{
	char buf[8];
	int len = sprintf(buf, CSI "%dX", n);
	VTWrite(buf, len);
}

void VTIndex(void)
{
#ifdef USE_8BIT
	VTWrite("\x84", 1);
#else
	VTWrite("\x1b" "D", 2);
#endif
}

void VTReverseIndex(void)
{
#ifdef USE_8BIT
	VTWrite("\x8D", 1);
#else
	VTWrite("\x1bM", 2);
#endif
}

void VTSGR(int x)
{
	char buf[8];
	int len = sprintf(buf, CSI "%dm", x);
	VTWrite(buf, len);
}

void VTSetDblHt(void)
{
	VTWrite("\x1b#3", 3);
}

void VTSetDblHb(void)
{
	VTWrite("\x1b#4", 3);
}

void VTSetDblW(void)
{
	VTWrite("\x1b#6", 3);
}

void VTDecEn(int n)
{
	char buf[8];
	int len = sprintf(buf, CSI "?%dh", n);
	VTWrite(buf, len);
}

void VTDecDis(int n)
{
	char buf[8];
	int len = sprintf(buf, CSI "?%dl", n);
	VTWrite(buf, len);
}

void VTDecSC(void)
{
	VTWrite("\x1b" "7", 2);
}

void VTDecRC(void)
{
	VTWrite("\x1b" "8", 2);
}

void VTSTBM(int top, int bottom)
{
	char buf[16];
	int len = sprintf(buf, CSI "%d;%dr", top, bottom);
	VTWrite(buf, len);
}

void VTGraphics(void)
{
	VTWrite("\x1b(0", 3);
}

void VTAscii(void)
{
	VTWrite("\x1b(B", 3);
}

/******************************************************************************/
/* Sixel graphics                                                             */
/******************************************************************************/
void VTSixelBegin(int bg)
{
	char buf[16];
	int len = sprintf(buf, DCS "1;%dq", bg);
	VTWrite(buf, len);
}

void VTSixelEnd(void)
{
	VTWrite(ST, STLEN);
}

void VTSixel(u8 bits)
{
	VTPutc('?' + (bits & 0x3F));
}

void VTSixelCR(void)
{
	VTWrite("$", 1);
}

void VTSixelLF(void)
{
	VTWrite("-", 1);
}

/******************************************************************************/
/* General output/print routines                                              */
/******************************************************************************/
int VTPrint(const char* s)
{
	int len = strlen(s);
	VTWrite(s, len);
	return len;
}

int VTPrintf(const char* format, ...)
{
	va_list args;
	int length;
	va_start(args, format);
	length = vcsprintf(VTPutc, format, args);
	va_end(args);

	return length;
}
