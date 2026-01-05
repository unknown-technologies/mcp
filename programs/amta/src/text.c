#include <emulight/ispf.h>
#include <emulight/string.h>

#include "game.h"

void UIPrintCenter(SPFPanel* self, unsigned int line, const char* text)
{
	unsigned int width = VTGetWidth(&self->spf->vt);

	unsigned int x = (width - strlen(text)) / 2;

	VTGoto(line, x + 1);
	VTPrint(text);
}

void UIPrintCenterDBL(SPFPanel* self, unsigned int line, const char* text)
{
	unsigned int width = VTGetWidth(&self->spf->vt);

	unsigned int dblwidth = width / 2;
	unsigned int x = (dblwidth - strlen(text)) / 2;

	VTGoto(line, x + 1);
	VTSetDblHt();
	VTPrint(text);

	VTGoto(line + 1, x + 1);
	VTSetDblHb();
	VTPrint(text);
}

static BOOL parse(const char* input, unsigned int* operand,
		const char** text)
{
	unsigned int value = 0;
	while(*input == ' ') {
		input++;
	}

	if(*input < '0' || *input > '9') {
		return FALSE;
	}

	while(*input >= '0' && *input <= '9') {
		value *= 10;
		value += *input - '0';
		input++;
	}

	if(*input != ' ') {
		return FALSE;
	}

	while(*input == ' ') {
		input++;
	}

	*operand = value;
	*text = input;

	return TRUE;
}

#define	CMD_NONE	0
#define	CMD_TITLE	1
#define	CMD_CENTER	2
#define	CMD_BC		3

BOOL UIMarkup(SPFPanel* self, const char* markup)
{
	const char* line = markup;
	while(*line) {
		while(*line == ' ' || *line == '\n') {
			line++;
		}
		if(!*line) {
			break;
		}

		unsigned int cmd = CMD_NONE;
		if(startswith(line, ".TITLE ")) {
			cmd = CMD_TITLE;
			line += 7;
		} else if(startswith(line, ".CENTER ")) {
			cmd = CMD_CENTER;
			line += 8;
		} else if(startswith(line, ".BC ")) {
			cmd = CMD_BC;
			line += 4;
		} else {
			/* unknown command */
			sprintf(rec_buf, "UNKNOWN COMMAND: %02X",
					(u8) *line);
			OSLog(MSG_MARKUP_ERR, rec_buf);
			return FALSE;
		}

		if(cmd != CMD_NONE) {
			/* draw title */
			unsigned int ln = 0;
			const char* text = NULL;
			if(!parse(line, &ln, &text)) {
				sprintf(rec_buf, "INVALID OPERANDS: \"%s\"",
						line);
				OSLog(MSG_MARKUP_ERR, rec_buf);
				return FALSE;
			}

			size_t i;
			for(i = 0; i < sizeof(rec_buf) && text[i]
					&& text[i] != '\n'; i++) {
				rec_buf[i] = text[i];
			}
			rec_buf[i] = 0;
			line = &text[i];

			switch(cmd) {
				case CMD_TITLE:
					VTSGR(1);
					UIPrintCenterDBL(self, ln,
							rec_buf);
					VTSGR(0);
					break;
				case CMD_CENTER:
					UIPrintCenter(self, ln, rec_buf);
					break;
				case CMD_BC:
					VTSGR(1);
					UIPrintCenter(self, ln, rec_buf);
					VTSGR(0);
					break;
				default:
					sprintf(rec_buf, "INVALID COMMAND: %u",
							cmd);
					OSLog(MSG_MARKUP_ERR, rec_buf);
					return FALSE;
			}
		}

		/* skip to next line */
		while(*line && *line != '\n') {
			line++;
		}
		while(*line == '\n') {
			line++;
		}
	}

	return TRUE;
}

BOOL startswith(const char* ref, const char* search)
{
	size_t len = strlen(search);

	if(strlen(ref) < len) {
		return FALSE;
	}

	return !memcmp(ref, search, len);
}

