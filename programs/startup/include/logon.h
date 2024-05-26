#ifndef __LOGON_H__
#define __LOGON_H__

#include <emulight/types.h>

#define	MSGID_LOGON	046546
#define	MSG_NOT_FOUND	MSGID_LOGON, 1, LOG_CRIT

BOOL logon(void);

#endif
