#ifndef __TSO_PRIVATE_H__
#define __TSO_PRIVATE_H__

#include <emulight/tso.h>

#define	CMD_MAXLEN		(80 * 16)
#define	CMD_MAXARGS		16

char toupper(char c);

void tso_main(void);
void tso_execute_command(unsigned int argc, TSOArgument* argv);

/* commands */
int tso_logoff(unsigned int argc, TSOArgument* argv);
int tso_showmem(unsigned int argc, TSOArgument* argv);
int tso_showver(unsigned int argc, TSOArgument* argv);
int tso_write(unsigned int argc, TSOArgument* argv);
int tso_showlog(unsigned int argc, TSOArgument* argv);
int tso_listcat(unsigned int argc, TSOArgument* argv);
int tso_listds(unsigned int argc, TSOArgument* argv);
int tso_printds(unsigned int argc, TSOArgument* argv);

#endif
