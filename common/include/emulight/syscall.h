#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#define	SYS_HALT		0
#define	SYS_TTO			1
#define	SYS_TTI			2
#define	SYS_READBLOCK		3
#define	SYS_LOCATE		4
#define	SYS_SPAWN		5
#define	SYS_YIELD		6
#define	SYS_MTXLK		7
#define	SYS_MTXUL		8
#define	SYS_GETTIME		9
#define	SYS_SETTIME		10
#define	SYS_GETTICK		11
#define	SYS_GETDATE		12
#define	SYS_SETDATE		13
#define	SYS_OSLOG		14
#define	SYS_OSLOADPROG		15
#define	SYS_OSULPROG		16
#define	SYS_OSSPAWN		17
#define	SYS_OSJOIN		18
#define	SYS_OSEXIT		19
#define	SYS_DSOPEN		20
#define	SYS_DSGETINFO		21
#define	SYS_DSREAD		22
#define	SYS_DSWRITE		23
#define	SYS_DSSEEK		24
#define	SYS_DSCLOSE		25
#define	SYS_FSLIST		26
#define	SYS_VOLGETLABEL		27
#define	SYS_VOLREADBLOCK	28
#define	SYS_VOLWRITEBLOCK	29
#define	SYS_VOLISACTIVE		30
#define	SYS_OSGETLOG		31

#define	__SCNUM(x)	#x
#define	__SC(x)		__SCNUM(x)

#endif
