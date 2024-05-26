#ifndef __SCREEN_H__
#define __SCREEN_H__

void screen_init(void);
void screen_nl(void);
void screen_putc(char c);
void screen_print(const char* text);
void screen_input(char c);
void screen_clear(void);
void screen_repaint(void);

void screen_cursor_up(void);
void screen_cursor_down(void);
void screen_cursor_left(void);
void screen_cursor_right(void);

void screen_cursor_pos1(void);
void screen_cursor_end(void);

void screen_bs(void);

BOOL screen_line_cont(void);
char* screen_get_command(size_t* len);

#endif
