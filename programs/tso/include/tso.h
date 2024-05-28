#ifndef __TSO_H__
#define __TSO_H__

#include <emulight/vt.h>

#define	CMD_MAXLEN		(80 * 16)
#define	CMD_MAXARGS		16

typedef struct {
	const char*	name;
	const char*	value;
	int		index;
} TSOArgument;

extern VT vt;

void tso(void);
void tso_exit(void);
void tso_execute_command(unsigned int argc, TSOArgument* argv);

/* commands */
int tso_logoff(unsigned int argc, TSOArgument* argv);
int tso_showmem(unsigned int argc, TSOArgument* argv);
int tso_showver(unsigned int argc, TSOArgument* argv);
int tso_write(unsigned int argc, TSOArgument* argv);
int tso_showlog(unsigned int argc, TSOArgument* argv);
int tso_listcat(unsigned int argc, TSOArgument* argv);

#endif
