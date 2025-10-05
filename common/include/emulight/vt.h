#ifndef __VT_H__
#define __VT_H__

#include <emulight/types.h>
#include <emulight/si.h>

/* #define USE_8BIT */

#ifdef USE_8BIT
#define	CSI	"\x9b"
#define	CSILEN	1
#define	DCS	"\x90"
#define	DCSLEN	1
#define	ST	"\x9C"
#define	STLEN	1
#else
#define	CSI	"\x1b["
#define	CSILEN	2
#define	DCS	"\x1bP"
#define	DCSLEN	2
#define	ST	"\x1b\\"
#define	STLEN	2
#endif

#define ESC	"\x1b"

#define	SIXEL_BG_DEFAULT	0
#define	SIXEL_BG_CURRENT	1
#define	SIXEL_BG_SET		2

#define	KAM	2
#define	IRM	4
#define	SRM	12
#define	LNM	20

#define	DECCKM	1
#define	DECANM	2
#define	DECCOLM	3
#define	DECSCLM	4
#define	DECSCNM	5
#define	DECOM	6
#define	DECAWM	7
#define	DECARM	8
#define	DECPFF	18
#define	DECPEX	19
#define	DECTCEM	25
#define	DECTEK	38
#define	DECNRCM	42
#define	DECGEPM	43
#define	DECGPCM	44
#define	DECGPCS	45
#define	DECGPBM	46
#define	DECGRPM	47

#define	VT_STATE_NORMAL	0
#define	VT_STATE_ESC	1
#define	VT_STATE_CSI	2
#define	VT_STATE_SS3	3

#define	VT_KEY_ENTER	0x0D
#define	VT_KEY_TAB	'\t'
#define	VT_KEY_SPACE	' '
#define	VT_KEY_BS	'\b'
#define	VT_KEY_DEL	0x7F
#define	VT_KEY_F1	0x0100
#define	VT_KEY_F2	0x0101
#define	VT_KEY_F3	0x0102
#define	VT_KEY_F4	0x0103
#define	VT_KEY_F5	0x0104
#define	VT_KEY_F6	0x0105
#define	VT_KEY_F7	0x0106
#define	VT_KEY_F8	0x0107
#define	VT_KEY_F9	0x0108
#define	VT_KEY_F10	0x0109
#define	VT_KEY_F11	0x010A	/* ESC */
#define	VT_KEY_F12	0x010B	/* BS */
#define	VT_KEY_F13	0x010C	/* LF */
#define	VT_KEY_F14	0x010D
#define	VT_KEY_F15	0x010E	/* Help */
#define	VT_KEY_F16	0x010F	/* Do */
#define	VT_KEY_F17	0x0120
#define	VT_KEY_F18	0x0121
#define	VT_KEY_F19	0x0122
#define	VT_KEY_F20	0x0123
#define	VT_KEY_LEFT	0x0130
#define	VT_KEY_RIGHT	0x0131
#define	VT_KEY_UP	0x0132
#define	VT_KEY_DOWN	0x0133
#define	VT_KEY_FIND	0x0140
#define	VT_KEY_INSERT	0x0141
#define	VT_KEY_REMOVE	0x0142
#define	VT_KEY_SELECT	0x0143
#define	VT_KEY_PSCREEN	0x0144
#define	VT_KEY_NSCREEN	0x0145
#define	VT_KEY_KP_0	0x0150
#define	VT_KEY_KP_1	0x0151
#define	VT_KEY_KP_2	0x0152
#define	VT_KEY_KP_3	0x0153
#define	VT_KEY_KP_4	0x0154
#define	VT_KEY_KP_5	0x0155
#define	VT_KEY_KP_6	0x0156
#define	VT_KEY_KP_7	0x0157
#define	VT_KEY_KP_8	0x0158
#define	VT_KEY_KP_9	0x0159
#define	VT_KEY_KP_MINUS	0x015A
#define	VT_KEY_KP_COMMA	0x015B
#define	VT_KEY_KP_PERIOD 0x015C
#define	VT_KEY_KP_ENTER	0x015D
#define	VT_KEY_KP_PF1	0x0160
#define	VT_KEY_KP_PF2	0x0161
#define	VT_KEY_KP_PF3	0x0162
#define	VT_KEY_KP_PF4	0x0163

#define	VT_KEY_NONE	0xFFFF

typedef struct {
	u16	width;
	u16	height;
	u8	state;
	u8	value;
} VT;

#define	VTGetWidth(vt)		((vt)->width)
#define	VTGetHeight(vt)		((vt)->height)
#define	VTSetWidth(vt, w)	((vt)->width = (w))
#define	VTSetHeight(vt, h)	((vt)->height = (h))
#define	VTWrite(x, n)		SITransmit(x, n)
#define	VTClear			VTED
#define	VTClearScreen		VTED2

#define	VTShowCursor()		VTWrite(CSI "?25h", CSILEN + 4)
#define	VTHideCursor()		VTWrite(CSI "?25l", CSILEN + 4)

#define	VTGetCTRL(c)		(((c) & ~0x20) - 'A' + 1)

void	VTInit(VT* vt);

u16	VTReceive(VT* vt, u8* buf, u8* len);

void	VTCSI(const char* s);
void	VTESC(const char* s);
void	VTHome(void);
void	VTGoto(int line, int column);
void	VTGotoLine(int line);
void	VTCUU(int n);
void	VTCUD(int n);
void	VTCUF(int n);
void	VTCUB(int n);
void	VTEL(int x);
void	VTEL2(void);
void	VTED(int x);
void	VTED2(void);
void	VTDCH(int n);
void	VTICH(int n);
void	VTECH(int n);
void	VTIndex(void);
void	VTReverseIndex(void);
void	VTNextLine(void);
void	VTSGR(int x);
void	VTSetDblHt(void);
void	VTSetDblHb(void);
void	VTSetDblW(void);
void	VTDecEn(int n);
void	VTDecDis(int n);
void	VTDecSC(void);
void	VTDecRC(void);
void	VTSTBM(int top, int bottom);
void	VTGraphics(void);
void	VTAscii(void);

void	VTSixelBegin(int bg);
void	VTSixelEnd(void);
void	VTSixel(u8 bits);
void	VTSixelCR(void);
void	VTSixelLF(void);

static inline void VTDecOM(BOOL enable)
{
	if(enable) {
		VTDecEn(6);
	} else {
		VTDecDis(6);
	}
}

static inline void VTPutc(const char c)
{
	VTWrite(&c, 1);
}

int	VTPrint(const char* s);
int	VTPrintf(const char* format, ...);

#endif
