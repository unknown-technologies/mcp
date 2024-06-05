#ifndef __TSO_H__
#define __TSO_H__

#include <emulight/types.h>

struct TSO;
typedef struct TSO TSO;

typedef struct {
	const char*	name;
	const char*	value;
	int		index;
} TSOArgument;

struct TSO {
	void		(*prompt)(TSO* tso);
	void		(*enter)(TSO* tso);

	BOOL		exit;
};

typedef struct {
	TSO*		tso;
	TSOArgument*	argv;
	unsigned int	argc;
} TENV;

#include <emulight/tso/screen.h>

void	TSOInit(TSO* tso);
void	TSOExit(TSO* tso);
void	TSOStart(TSO* tso);
void	TSOProcess(TSO* tso);

static inline void TSOSetPrompt(TSO* tso, void (*prompt)(TSO*))
{
	tso->prompt = prompt;
}

static inline void TSOSetEnter(TSO* tso, void (*enter)(TSO*))
{
	tso->enter = enter;
}

static inline BOOL TSOIsExit(TSO* tso)
{
	return tso->exit;
}

#endif
