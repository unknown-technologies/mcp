#include <emulight/types.h>
#include <emulight/string.h>
#include <emulight/vt.h>
#include <emulight/ispf.h>

static void SPFiShowMessage(SPF* spf, const char* title, const char* message,
		const char* sgr)
{
	size_t len = strlen(message);
	size_t titlelen = strlen(title);

	size_t maxlen = (len > titlelen ? len : titlelen) + 2;

	unsigned int width = VTGetWidth(&spf->vt);
	unsigned int height = VTGetHeight(&spf->vt);

	unsigned int start_x = (width - maxlen) / 2;
	unsigned int start_y = (height - 3) / 2 + 1;

	/* always disable the cursor, it will be reset by next repaint */
	VTDecDis(DECTCEM);

	if(sgr) {
		VTPrint(sgr);
	}

	/* first line: border with title */
	VTGoto(start_y, start_x);
	VTWrite(ESC "(0l", 4);
	if(titlelen > len) {
		VTWrite("q " ESC "(B", 5);
		VTPrint(title);
		VTWrite(ESC "(0 q", 5);
	} else {
		/* pad left and right */
		unsigned int pad = len - titlelen;
		unsigned int i;
		for(i = 0; i < pad / 2; i++) {
			VTPutc('q');
		}
		VTWrite(" " ESC "(B", 4);
		VTPrint(title);
		VTWrite(ESC "(0 ", 4);
		for(; i < pad; i++) {
			VTPutc('q');
		}
	}
	VTPutc('k');

	/* second line: message */
	VTGoto(start_y + 1, start_x);
	VTWrite("x " ESC "(B", 5);
	if(len > titlelen) {
		VTPrint(message);
	} else {
		/* pad left and right */
		unsigned int pad = titlelen - len + 2;
		unsigned int i;
		for(i = 0; i < pad / 2; i++) {
			VTPutc(' ');
		}
		VTPrint(message);
		for(; i < pad; i++) {
			VTPutc(' ');
		}
	}
	VTWrite(ESC "(0 x", 5);

	/* third line: border */
	VTGoto(start_y + 2, start_x);
	VTPutc('m');
	if(len > titlelen) {
		for(unsigned int i = 0; i < maxlen; i++) {
			VTPutc('q');
		}
	} else {
		for(unsigned int i = 0; i < maxlen + 2; i++) {
			VTPutc('q');
		}
	}
	VTWrite("j" ESC "(B", 4);

	if(sgr) {
		VTWrite(CSI "0m", CSILEN + 2);
	}

	spf->error_active = TRUE;
	spf->dirty = FALSE;
}

void SPFShowMessage(SPF* spf, const char* title, const char* message)
{
	SPFiShowMessage(spf, title, message, CSI "1m");
}

void SPFShowError(SPF* spf, const char* message)
{
	SPFiShowMessage(spf, "Error", message, CSI "1m");
}
