#include <emulight/string.h>
#include <emulight/vt.h>

#include "tso.h"
#include "screen.h"

VT vt;

void tso_prompt(void)
{
	screen_print("READY\n");
}

void tso_execute(void)
{
	size_t len = 0;
	const char* cmd = screen_get_command(&len);

	screen_nl();

	if(cmd) {
		/* execute command */
	}

	tso_prompt();
}

void tso_enter(void)
{
	if(screen_line_cont()) {
		screen_nl();
	} else {
		tso_execute();
	}
}

void tso(void)
{
	VTInit(&vt);

	VTClearScreen();
	VTHome();
	VTShowCursor();

	tso_prompt();

	while(1) {
		u16 key = VTReceive(&vt, NULL, NULL);
		switch(key) {
			case VT_KEY_NONE:
				break;
			case VTGetCTRL('L'):
				screen_repaint();
				break;
			case VT_KEY_F10:
				screen_clear();
				tso_prompt();
				break;
			case VT_KEY_F12:
				return;
			case VT_KEY_UP:
				screen_cursor_up();
				break;
			case VT_KEY_DOWN:
				screen_cursor_down();
				break;
			case VT_KEY_LEFT:
				screen_cursor_left();
				break;
			case VT_KEY_RIGHT:
				screen_cursor_right();
				break;
			case VTGetCTRL('A'):
			case VT_KEY_FIND:
				screen_cursor_pos1();
				break;
			case VTGetCTRL('E'):
			case VT_KEY_SELECT:
				screen_cursor_end();
				break;
			case VT_KEY_BS:
			case VT_KEY_DEL:
				screen_bs();
				break;
			case VT_KEY_ENTER:
				tso_enter();
				break;
			default:
				if(key >= 0x20 && key < 0x7F) {
					screen_input((char) key);
				}
				break;
		}
	}
}
