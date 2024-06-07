#ifndef __TSO_SCREEN_H__
#define __TSO_SCREEN_H__

#include <emulight/types.h>
#include <emulight/tso.h>

void	TInit(void);
void	TStart(void);
char*	TGetScreen(unsigned int* lines, unsigned int* cols, char** input_lines);
void	TGetCursor(unsigned int* line, unsigned int* col);

void	TNL(void);
void	TPutc(char c);
void	TPrint(const char* text);
void	TInput(char c);
void	TClear(void);
void	TSync(void);

void	TGetCursor(unsigned int* line, unsigned int* col);

void	TSetOutput(void);
void	TSetInput(void);
void	TPrompt(TSO* tso);

void	TProcess(TSO* tso);

#endif
