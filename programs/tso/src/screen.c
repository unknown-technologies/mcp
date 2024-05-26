#include <emulight/string.h>
#include <emulight/vt.h>

#include "tso.h"

#define	MODE_OUTPUT	0
#define	MODE_INPUT	1

static char screen[24][80];
static char input_lines[24];

static unsigned int cursor_x;
static unsigned int cursor_y;

static char mode = 0;

void screen_init(void)
{
	bzero(screen, sizeof(screen));
	bzero(input_lines, sizeof(input_lines));

	cursor_x = 0;
	cursor_y = 0;
}

void scroll_up(void)
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
	VTGoto(lines, cols);
	VTEL2();
}

static inline void screen_sync_cursor(void)
{
	VTGoto(cursor_y + 1, cursor_x + 1);
	if(input_lines[cursor_y]) {
		VTSGR(1);
	} else {
		VTSGR(0);
	}
}

void screen_nl(void)
{
	cursor_y++;
	cursor_x = 0;
	if(cursor_y >= VTGetHeight(&vt)) {
		/* scroll */
		cursor_y = VTGetHeight(&vt) - 1;
		scroll_up();
		screen_sync_cursor();
	} else {
		screen_sync_cursor();
		VTEL2();
		bzero(screen[cursor_y], VTGetWidth(&vt));
	}
}

static void screen_putchar(char c)
{
	if(c == '\n') {
		screen_nl();
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
			screen_nl();
		}
	}
}

void screen_putc(char c)
{
	mode = MODE_OUTPUT;
	VTSGR(0);
	screen_putchar(c);
}

void screen_print(const char* text)
{
	screen_sync_cursor();
	mode = MODE_OUTPUT;
	VTSGR(0);
	if(cursor_x == 0) {
		screen_putchar(0);
	}
	for(const char* s = text; *s; s++) {
		screen_putchar(*s);
	}
}

void screen_input(char c)
{
	mode = MODE_INPUT;
	VTSGR(1);
	screen_putchar(c);
}

void screen_clear(void)
{
	VTClearScreen();
	VTHome();

	cursor_x = 0;
	cursor_y = 0;

	bzero(screen, sizeof(screen));
	bzero(input_lines, sizeof(input_lines));
}

void screen_repaint(void)
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
		for(unsigned int x = 0; x < VTGetWidth(&vt); x++) {
			char c = screen[y][x];
			if(!c) {
				VTPutc(' ');
			} else {
				VTPutc(c);
			}
		}
	}

	screen_sync_cursor();
}

void screen_cursor_up(void)
{
	if(cursor_y > 0) {
		cursor_y--;
		screen_sync_cursor();
	}
}

void screen_cursor_down(void)
{
	if(cursor_y < VTGetHeight(&vt) - 1) {
		cursor_y++;
		screen_sync_cursor();
	}
}

void screen_cursor_left(void)
{
	if(cursor_x > 0) {
		cursor_x--;
		screen_sync_cursor();
	}
}

void screen_cursor_right(void)
{
	if(cursor_x < VTGetWidth(&vt) - 1) {
		cursor_x++;
		screen_sync_cursor();
	}
}

void screen_cursor_pos1(void)
{
	cursor_x = 0;
	screen_sync_cursor();
}

void screen_cursor_end(void)
{
	cursor_x = 0;
	for(unsigned int i = 0; i < VTGetWidth(&vt); i++) {
		if(screen[cursor_y][i] && screen[cursor_y][i] != 0x20) {
			cursor_x = i;
		}
	}
	screen_sync_cursor();
}

void screen_bs(void)
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
		screen_sync_cursor();
		VTPutc(' ');
		screen_sync_cursor();
	}
}

BOOL screen_line_cont_internal(unsigned int line)
{
	/* check if the last character is a "+" */
	for(int x = VTGetWidth(&vt) - 1; x >= 0; x--) {
		char c = screen[line][x];
		if(c == ' ' || c == 0) {
			continue;
		} else if(c == '+') {
			/* continuation found */
			return TRUE;
		} else {
			return FALSE;
		}
	}

	return FALSE;
}

BOOL screen_line_cont(void)
{
	return screen_line_cont_internal(cursor_y);
}

const char* screen_get_command(size_t* len)
{
	unsigned int lines = 1;

	const char* command = screen[cursor_y];

	if(cursor_y > 0) {
		/* check if previous line ended with a "+" continuation */
		for(int y = cursor_y - 1; y >= 0; y--) {
			if(!input_lines[y]) {
				/* this is not an input line */
				break;
			}

			if(screen_line_cont_internal(y)) {
				lines++;
				command = screen[y];
			} else {
				break;
			}
		}
	}

	*len = lines * VTGetWidth(&vt);
	return command;
}
