#include <emulight/types.h>
#include <emulight/ispf.h>
#include <emulight/os.h>
#include <emulight/string.h>

#include "calc.h"

static char input[INPUT_SZ];
static char outbuf[48];
static int line;
static int pos;
static int len;

void CALCClearInput(SPFPanel* self)
{
	bzero(input, sizeof(input));
	pos = 0;
	len = 0;
}

void CALCInit(SPFPanel* self)
{
	SPFShowCursor(self);
	SPFSetPanelTitle(self, "Calculator");

	line = 0;

	CALCInitContext();

	CALCClearInput(self);
}

void CALCGetCursor(SPFPanel* self, int* pos_y, int* pos_x)
{
	u16 width = VTGetWidth(&self->spf->vt);
	u16 height = VTGetHeight(&self->spf->vt);

	int y = line + 2;
	int x = pos + 2;

	int lines = x / width;
	int columns = x % width;
	y += lines;
	x = columns;

	if(y > height - 2) {
		y = height - 2;
	}

	*pos_y = y - 1;
	*pos_x = x;
}

void CALCSetCursor(SPFPanel* self)
{
	int x;
	int y;

	CALCGetCursor(self, &y, &x);

	VTGoto(y + 1, x + 1);
}

void CALCPrompt(SPFPanel* self)
{
	/* command prompt */
	VTGoto(line + 2, 1);
	VTSGR(1);
	VTPrint("> ");

	VTWrite(input, len);
	VTSGR(0);

	CALCSetCursor(self);
}

void CALCDraw(SPFPanel* self)
{
	u16 height = VTGetHeight(&self->spf->vt);

	/* status line */
	VTGoto(height - 1, 2);
	VTPrint("F10=Menu   CTRL-L=Refresh");
	VTSTBM(2, 22);

	CALCPrompt(self);
}

void CALCProcess(SPFPanel* self, u16 key)
{
	u16 width = VTGetWidth(&self->spf->vt);
	u16 height = VTGetHeight(&self->spf->vt);

	if(key >= 0x20 && key <= 0x7E) {
		/* text input */
		if(len < INPUT_SZ - 1) {
			/* The limit is len instead of len - 1 because one
			 * character is inserted, therefore the real length
			 * is one more than len */
			for(int i = len; i > pos; i--) {
				input[i] = input[i - 1];
			}

			input[pos++] = (char) key;
			len++;

			VTICH(1);
			VTSGR(1);
			VTPutc((char) key);
			VTSGR(0);

			CALCSetCursor(self);
		}
	}

	switch(key) {
		case VT_KEY_ENTER:
			input[len] = 0;

			VTNextLine();

			if(!CALCEvaluate(input)) {
				VTPrint("Syntax error");
				VTNextLine();
			} else {
				FLOAT result;
				CALCGetResult(&result);
				F2A(outbuf, &result);
				VTPrint(outbuf);
				VTNextLine();
			}

			if(len + 2 > width) {
				int lines = ((len + 2) / width) + 1;
				line += lines;
			} else {
				line += 2;
			}

			if(line > height - 4) {
				line = height - 4;
			}

			CALCClearInput(self);

			CALCPrompt(self);
			break;
		case VT_KEY_LEFT:
			if(pos > 0) {
				int last_x;
				int last_y;
				CALCGetCursor(self, &last_y, &last_x);

				pos--;

				int current_x;
				int current_y;
				CALCGetCursor(self, &current_y, &current_x);

				if(last_y == current_y) {
					VTCUB(1);
				} else {
					CALCSetCursor(self);
				}
			}
			break;
		case VT_KEY_RIGHT:
			if(pos < len) {
				int last_x;
				int last_y;
				CALCGetCursor(self, &last_y, &last_x);

				pos++;

				int current_x;
				int current_y;
				CALCGetCursor(self, &current_y, &current_x);

				if(last_y == current_y) {
					VTCUF(1);
				} else {
					CALCSetCursor(self);
				}
			}
			break;
		case VT_KEY_BS:
		case VT_KEY_DEL:
			if(pos > 0) {
				int last_x;
				int last_y;
				CALCGetCursor(self, &last_y, &last_x);

				pos--;
				len--;

				int current_x;
				int current_y;
				CALCGetCursor(self, &current_y, &current_x);

				for(int i = pos; i < len; i++) {
					input[i] = input[i + 1];
				}

				if(last_y == current_y) {
					VTCUB(1);
					VTDCH(1);
				} else {
					SPFRepaint(self->spf);
				}
			}
			break;
		case VTGetCTRL('A'):
			pos = 0;
			CALCSetCursor(self);
			break;
		case VTGetCTRL('E'):
			pos = len;
			CALCSetCursor(self);
			break;
		case VTGetCTRL('U'):
			CALCClearInput(self);
			SPFRepaint(self->spf);
			break;
		case VT_KEY_NONE:
			break;
	}
}
