#ifndef __GAME_H__
#define __GAME_H__

#include <emulight/types.h>
#include <emulight/ispf.h>

#include "msgid.h"

#define	MAX_STATES	256
#define	MAX_TITLE	80
#define	MAX_MESSAGE	1920
#define	MAX_BTNMSG	64
#define	MAX_BTN		8
#define	MAX_NAME	80

typedef struct {
	unsigned int	target;
	char		name[MAX_BTNMSG];
} BUTTON;

typedef struct {
	int		ds;
	size_t		lrecl;
	unsigned int	start;
	unsigned int	state;
	unsigned int	btncnt;
	unsigned int	selection;
	BOOL		markup;
	unsigned int	offsets[MAX_STATES];
	char		name[MAX_NAME];
	char		title[MAX_TITLE];
	char		message[MAX_MESSAGE];
	BUTTON		buttons[MAX_BTN];
} GAME;

extern char rec_buf[96];

/* game logic */
BOOL	GAMEInit(GAME* game, const char* dsn);
void	GAMEDestroy(GAME* game);
BOOL	GAMEGotoState(GAME* game, unsigned int state);

/* user interface */
void	UIInit(SPFPanel* self);
void	UIDraw(SPFPanel* self);
void	UIProcess(SPFPanel* self, u16 key);
void	UIDestroy(SPFPanel* self);

void	UIPrintCenter(SPFPanel* self, unsigned int line,
		const char* text);
void	UIPrintCenterDBL(SPFPanel* self, unsigned int line,
		const char* text);
BOOL	UIMarkup(SPFPanel* self, const char* markup);

BOOL	startswith(const char* ref, const char* search);

void	print(const char* s);
void	printn(const char* s, unsigned int n);
void	printindent(SPFPanel* self, const char* s, unsigned int border);

#endif
