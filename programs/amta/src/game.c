#include <emulight/types.h>
#include <emulight/string.h>
#include <emulight/ds.h>

#include "game.h"

#define	FSM_SCREEN	0
#define	FSM_DATA	1
#define	FSM_MSG		2
#define	FSM_MARKUP	3

char rec_buf[96];

static void trim(char* c)
{
	size_t len = strlen(c);
	for(char* ch = &c[len - 1]; *ch == ' ' && ch >= c; ch--) {
		*ch = 0;
	}
}

BOOL GAMEInit(GAME* game, const char* dsn)
{
	bzero(game, sizeof(GAME));

	game->ds = -1;

	int ds = DSOpen(dsn);
	if(ds < 0) {
		return FALSE;
	}

	DS info;
	DSGetInfo(ds, &info);
	size_t lrecl = DSGetRecLen(&info);
	if(lrecl > sizeof(rec_buf)) {
		DSClose(ds);
		return FALSE;
	}

	unsigned int line = 0;
	unsigned int last_state = 0;
	while(DSRead(ds, rec_buf, 1) == 1) {
		rec_buf[lrecl] = 0;

		if(startswith(rec_buf, ".SCREEN ")) {
			int state = -1;
			for(int i = 8; i < sizeof(rec_buf); i++) {
				if(rec_buf[i] != ' ') {
					/* this is the state */
					state = 0;
					while(i < sizeof(rec_buf) &&
							rec_buf[i] != ' ') {
						char c = rec_buf[i++];
						if(c < '0' || c > '9') {
							sprintf(rec_buf, "INVALID CHAR: %c (LN %u)",
									c, line);
							OSLog(MSG_NUMBER, rec_buf);
							goto error;
						}

						state *= 10;
						state += c - '0';
					}
					break;
				}
			}

			if(state == -1) {
				sprintf(rec_buf, "NO STATE ID (LN %u)",
						line);
				OSLog(MSG_NO_STATE, rec_buf);
				goto error;
			}

			if(state >= MAX_STATES) {
				sprintf(rec_buf, "MAX STATE EXCEEDED (LN %u)",
						line);
				OSLog(MSG_MAX_STATE, rec_buf);
				goto error;
			}

			game->offsets[state] = line;
			last_state = state;
		} else if(startswith(rec_buf, ".GAME ")) {
			char* c = rec_buf + 6;
			while(*c == ' ') {
				c++;
			}
			trim(c);
			size_t len = strlen(c);
			if(len < MAX_NAME) {
				memcpy(game->name, c, len);
				game->name[len] = 0;
			} else {
				sprintf(rec_buf, "NAME TOO LONG (LN %u)",
						line);
				OSLog(MSG_NAME_LEN, rec_buf);
				goto error;
			}
		} else if(!strcmp(rec_buf, ".STARTUP")) {
			game->start = last_state;
		}

		line++;
	}

	game->ds = ds;
	game->lrecl = lrecl;

	return GAMEGotoState(game, game->start);

error:
	DSClose(ds);

	return FALSE;
}

void GAMEDestroy(GAME* game)
{
	if(game->ds != -1) {
		DSClose(game->ds);
		game->ds = -1;
	}
}

BOOL GAMEGotoState(GAME* game, unsigned int state)
{
	if(state >= MAX_STATES) {
		sprintf(rec_buf, "UNKNOWN STATE %u (ST %u)",
				state, game->state);
		OSLog(MSG_UNK_STATE, rec_buf);
		return FALSE;
	}

	unsigned int lastst = game->state;

	game->state = state;
	game->selection = 0;

	unsigned int off = game->offsets[state];
	DSSeek(game->ds, off);

	/* parse screen */
	unsigned int fsm = 0;

	unsigned int line = off;
	char* msgptr = game->message;
	unsigned int btnid = 0;
	while(DSRead(game->ds, rec_buf, 1) == 1) {
		rec_buf[game->lrecl] = 0;
		trim(rec_buf);

		switch(fsm) {
			case FSM_SCREEN: {
				if(!startswith(rec_buf, ".SCREEN ")) {
					OSLog(MSG_STATE_MISM, rec_buf);
					sprintf(rec_buf, "SCREEN COMMAND MISMATCH %u (ST %u, LN %u)",
							state, lastst, line);
					OSLog(MSG_STATE_MISM, rec_buf);
					return FALSE;
				}

				unsigned int st = 0;
				char* c = &rec_buf[8];
				while(*c == ' ') {
					c++;
				}

				if(*c < '0' || *c > '9') {
					sprintf(rec_buf, "INVALID CHAR: %c (LN %u)",
							*c, line);
					OSLog(MSG_NUMBER, rec_buf);
					return FALSE;
				}

				while(*c >= '0' && *c <= '9') {
					st *= 10;
					st += *c - '0';
					c++;
				}

				if(*c != ' ' && *c != 0) {
					sprintf(rec_buf, "INVALID CHAR: %c (LN %u)",
							*c, line);
					OSLog(MSG_NUMBER, rec_buf);
					return FALSE;
				}

				if(st != state) {
					sprintf(rec_buf, "STATE MISMATCH: %u VS %u (LN %u)",
							st, state, line);
					OSLog(MSG_INVALID_ST, rec_buf);
					return FALSE;
				}

				while(*c == ' ') {
					c++;
				}

				/* the rest is the screen title */
				size_t len = strlen(c);
				if(len >= MAX_TITLE) {
					sprintf(rec_buf, "TITLE TOO LONG (ST %u, LN %u)",
							state, line);
					OSLog(MSG_TITLE_LEN, rec_buf);
					return FALSE;
				}

				memcpy(game->title, c, len);
				game->title[len] = 0;

				fsm = FSM_DATA;
				break;
			}
			case FSM_DATA:
				if(startswith(rec_buf, ".SCREEN ")) {
					/* end of screen reached */
					game->btncnt = btnid;
					return TRUE;
				} else if(!strcmp(rec_buf, ".MSG")) {
					msgptr = game->message;
					fsm = FSM_MSG;
					game->markup = FALSE;
				} else if(!strcmp(rec_buf, ".MARKUP")) {
					msgptr = game->message;
					fsm = FSM_MARKUP;
					game->markup = TRUE;
				} else if(startswith(rec_buf, ".BTN ")) {
					if(btnid >= MAX_BTN) {
						sprintf(rec_buf, "TOO MANY BUTTONS (ST %u, LN %u)",
								state, line);
						OSLog(MSG_BTNCNT, rec_buf);
						return FALSE;
					}

					unsigned int target = 0;
					char* c = &rec_buf[5];
					while(*c == ' ') {
						c++;
					}

					if(*c < '0' || *c > '9') {
						sprintf(rec_buf, "INVALID CHAR: %c (LN %u)",
								*c, line);
						OSLog(MSG_NUMBER, rec_buf);
						return FALSE;
					}

					while(*c >= '0' && *c <= '9') {
						target *= 10;
						target += *c - '0';
						c++;
					}

					if(*c != ' ') {
						sprintf(rec_buf, "INVALID CHAR: %c (LN %u)",
								*c, line);
						OSLog(MSG_NUMBER, rec_buf);
						return FALSE;
					}

					while(*c == ' ') {
						c++;
					}

					/* the rest is the button message */
					size_t len = strlen(c);
					if(len >= MAX_BTNMSG) {
						sprintf(rec_buf, "BUTTON NAME TOO LONG (ST %u, LN %u)",
								state, line);
						OSLog(MSG_BTNMSG_LEN, rec_buf);
						return FALSE;
					}

					BUTTON* btn = &game->buttons[btnid++];
					memcpy(btn->name, c, len);
					btn->name[len] = 0;
					btn->target = target;
				}
				break;
			case FSM_MSG:
				if(!rec_buf[0] || !strcmp(rec_buf, ".P")) {
					/* empty line = paragraph */
					if(msgptr > game->message) {
						size_t ln = msgptr - game->message;
						if(ln + 1 >= MAX_MESSAGE) {
							sprintf(rec_buf, "MESSAGE TOO LONG (ST %u, LN %u)", state, line);
							OSLog(MSG_MESSAGE_LEN, rec_buf);
							return FALSE;
						}
						msgptr[-1] = '\n';
						*(msgptr++) = '\n';
						*msgptr = 0;
					}
				} else if(!strcmp(rec_buf, ".ENDMSG")) {
					/* remove trailing space */
					if(msgptr > game->message) {
						msgptr[-1] = 0;
					}
					fsm = FSM_DATA;
				} else if(!strcmp(rec_buf, ".br")) {
					/* line break */
					if(msgptr > game->message) {
						msgptr[-1] = '\n';
						*msgptr = 0;
					}
				} else {
					size_t len = strlen(rec_buf);
					size_t ln = msgptr - game->message;
					ln += len + 1;
					if(ln >= MAX_MESSAGE) {
						sprintf(rec_buf, "MESSAGE TOO LONG (ST %u, LN %u)", state, line);
						OSLog(MSG_MESSAGE_LEN, rec_buf);
						return FALSE;
					}

					memcpy(msgptr, rec_buf, len);
					msgptr += len;
					*(msgptr++) = ' ';
					*msgptr = 0;
				}
				break;
			case FSM_MARKUP:
				if(!rec_buf[0]) {
					/* empty line = ignored */
					continue;
				} else if(!strcmp(rec_buf, ".ENDMARKUP")) {
					/* remove trailing newline */
					if(msgptr > game->message) {
						msgptr[-1] = 0;
					}
					fsm = FSM_DATA;
				} else {
					size_t len = strlen(rec_buf);
					size_t ln = msgptr - game->message;
					ln += len + 1;
					if(ln >= MAX_MESSAGE) {
						sprintf(rec_buf, "MESSAGE TOO LONG (ST %u, LN %u)", state, line);
						OSLog(MSG_MESSAGE_LEN, rec_buf);
						return FALSE;
					}

					memcpy(msgptr, rec_buf, len);
					msgptr += len;
					*(msgptr++) = '\n';
					*msgptr = 0;
				}
				break;
		}

		line++;
	}

	/* this only happens if the target screen is the last one in the 
	 * story dataset */
	game->btncnt = btnid;
	return TRUE;
}
