#include <emulight/string.h>
#include <emulight/vt.h>
#include <emulight/tso/screen.h>

#define	MODE_OUTPUT	0
#define	MODE_INPUT	1

static char screen[24][80];
static char input_lines[24];

static unsigned int cursor_x;
static unsigned int cursor_y;
static unsigned int output_lines;

static char mode = 0;

static VT vt;

void TInit(void)
{
	VTInit(&vt);

	bzero(screen, sizeof(screen));
	bzero(input_lines, sizeof(input_lines));

	cursor_x = 0;
	cursor_y = 0;

	output_lines = 0;
}

void TStart(void)
{
	VTClearScreen();
	VTHome();
	VTShowCursor();
}

char* TGetScreen(unsigned int* lines, unsigned int* cols, char** input)
{
	if(lines) {
		*lines = VTGetHeight(&vt);
	}
	if(cols) {
		*cols = VTGetWidth(&vt);
	}
	if(input) {
		*input = input_lines;
	}
	return &screen[0][0];
}

void TScrollUp(void)
{
	VTIndex();
	unsigned int cols = VTGetWidth(&vt);
	unsigned int lines = VTGetHeight(&vt);
	for(unsigned int i = 0; i < lines - 1; i++) {
		memcpy(screen[i], screen[i + 1], cols);
		input_lines[i] = input_lines[i + 1];
	}
	bzero(screen[lines - 1], cols);
	input_lines[lines - 1] = 0;
	VTGotoLine(lines);
	VTEL2();
}

static void TSyncCursor(void)
{
	VTGoto(cursor_y + 1, cursor_x + 1);
	if(input_lines[cursor_y]) {
		VTSGR(1);
	} else {
		VTSGR(0);
	}
}

void TGetCursor(unsigned int* line, unsigned int* col)
{
	*line = cursor_y;
	*col = cursor_x;
}

void TNL(void)
{
	cursor_y++;
	cursor_x = 0;
	if(cursor_y >= VTGetHeight(&vt)) {
		/* scroll */
		cursor_y = VTGetHeight(&vt) - 1;
		TScrollUp();
		TSyncCursor();
	} else {
		TSyncCursor();
		VTEL2();
		bzero(screen[cursor_y], VTGetWidth(&vt));
	}
	input_lines[cursor_y] = mode;

	if(mode == MODE_OUTPUT) {
		output_lines++;
		if(output_lines >= VTGetHeight(&vt) - 1) {
			output_lines = 0;
			VTPrint("***");
			while(1) {
				u16 key = VTReceive(&vt, NULL, NULL);
				switch(key) {
					case VT_KEY_NONE:
						break;
					case VTGetCTRL('L'):
						TSync();
						VTPrint("***");
						break;
					case VT_KEY_F10:
						TClear();
						return;
					case VT_KEY_ENTER:
						output_lines = 0;
						VTEL2();
						VTPutc('\r');
						return;
				}
			}
		}
	}
}

static void TPutChar(char c)
{
	if(c == '\n') {
		TNL();
	} else {
		if(c == 0) {
			VTPutc(' ');
		} else {
			VTPutc(c);
		}
		if(c == 0x20) {
			c = 0;
		}
		screen[cursor_y][cursor_x] = c;
		input_lines[cursor_y] = mode;
		cursor_x++;
		if(cursor_x >= VTGetWidth(&vt)) {
			TNL();
		}
	}
}

void TPutc(char c)
{
	mode = MODE_OUTPUT;
	VTSGR(0);
	TPutChar(c);
}

void TPrint(const char* text)
{
	TSyncCursor();
	mode = MODE_OUTPUT;
	VTSGR(0);
	BOOL was_nl = TRUE;
	for(const char* s = text; *s; s++) {
		if(was_nl && cursor_x == 0) {
			TPutChar(0);
		}
		was_nl = *s == '\n';
		TPutChar(*s);
	}
}

void TSetOutput(void)
{
	mode = MODE_OUTPUT;
	output_lines = 0;
}

void TSetInput(void)
{
	mode = MODE_INPUT;
}

void TInput(char c)
{
	mode = MODE_INPUT;
	VTSGR(1);
	TPutChar(c);
}

void TClear(void)
{
	VTClearScreen();
	VTHome();

	output_lines = 0;
	cursor_x = 0;
	cursor_y = 0;

	bzero(screen, sizeof(screen));
	bzero(input_lines, sizeof(input_lines));
}

void TSync(void)
{
	VTClearScreen();
	VTHome();

	for(unsigned int y = 0; y < VTGetHeight(&vt); y++) {
		VTGoto(y + 1, 1);
		if(input_lines[y]) {
			VTSGR(1);
		} else {
			VTSGR(0);
		}

		/* optimization: skip space after last character on the line */
		unsigned int end = VTGetWidth(&vt);
		for(unsigned int x = VTGetWidth(&vt) - 1; x > 0; x--) {
			char c = screen[y][x];
			if(c != ' ' && c != 0) {
				end = x + 1;
				break;
			}
		}

		for(unsigned int x = 0; x < end; x++) {
			char c = screen[y][x];
			if(!c) {
				VTPutc(' ');
			} else {
				VTPutc(c);
			}
		}
	}

	TSyncCursor();
	VTShowCursor();
}

static void TCursorUp(void)
{
	if(cursor_y > 0) {
		cursor_y--;
		TSyncCursor();
	}
}

static void TCursorDown(void)
{
	if(cursor_y < VTGetHeight(&vt) - 1) {
		cursor_y++;
		TSyncCursor();
	}
}

static void TCursorLeft(void)
{
	if(cursor_x > 0) {
		cursor_x--;
		TSyncCursor();
	}
}

static void TCursorRight(void)
{
	if(cursor_x < VTGetWidth(&vt) - 1) {
		cursor_x++;
		TSyncCursor();
	}
}

static void TCursorPos1(void)
{
	cursor_x = 0;
	TSyncCursor();
}

static void TCursorEnd(void)
{
	cursor_x = 0;
	unsigned int cols = VTGetWidth(&vt);
	for(unsigned int i = 0; i < cols; i++) {
		if(screen[cursor_y][i] && screen[cursor_y][i] != 0x20) {
			cursor_x = i + 1;
			if(cursor_x >= cols) {
				cursor_x = cols - 1;
			}
		}
	}
	TSyncCursor();
}

static void TBS(void)
{
	if(cursor_x > 0) {
		/* same line */
		cursor_x--;
		screen[cursor_y][cursor_x] = 0;
		VTPrint("\b \b");
	} else if(cursor_y > 0) {
		/* across line */
		cursor_x = VTGetWidth(&vt) - 1;
		cursor_y--;
		screen[cursor_y][cursor_x] = 0;
		input_lines[cursor_y] = MODE_INPUT;
		TSyncCursor();
		VTPutc(' ');
		TSyncCursor();
	}
}

void TPrompt(TSO* tso)
{
	if(tso->prompt) {
		TSetOutput();
		if(cursor_x > 1) {
			TNL();
		}
		tso->prompt(tso);
		TNL();
		TSetInput();
	}
}

void TProcess(TSO* tso)
{
	u16 key = VTReceive(&vt, NULL, NULL);
	switch(key) {
		case VT_KEY_NONE:
			break;
		case VTGetCTRL('L'):
			TSync();
			break;
		case VT_KEY_F10:
			TClear();
			TPrompt(tso);
			break;
		case VT_KEY_UP:
			TCursorUp();
			break;
		case VT_KEY_DOWN:
			TCursorDown();
			break;
		case VT_KEY_LEFT:
			TCursorLeft();
			break;
		case VT_KEY_RIGHT:
			TCursorRight();
			break;
		case VTGetCTRL('A'):
		case VT_KEY_FIND:
			TCursorPos1();
			break;
		case VTGetCTRL('E'):
		case VT_KEY_SELECT:
			TCursorEnd();
			break;
		case VT_KEY_BS:
		case VT_KEY_DEL:
			TBS();
			break;
		case VT_KEY_ENTER:
			if(tso->enter) {
				tso->enter(tso, cursor_y, cursor_x);
			}
			break;
		default:
			if(key >= 0x20 && key < 0x7F) {
				TInput((char) key);
			}
			break;
	}
}
