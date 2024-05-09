#ifndef __SCHED_H__
#define __SCHED_H__

#include <emulight/types.h>
#include <emulight/mcp/fs.h>
#include <emulight/mcp/ds.h>

#define	MAX_TASKS		4
#define	TASK_NAMELEN		3

#define	TASK_FREE		0
#define	TASK_RUNNING		1
#define	TASK_DEAD		2

#define	TASK_FLAG_NOWAIT	_BV(0)

typedef struct {
	void*	sp;
	u16	name[TASK_NAMELEN];
	u8	state;
	u8	code;
	u8	flags;
	u8	pad;
	DS	datasets[MAX_DATASETS];
	u16	ds_mask;
} OSTask;

void	OSSchedInit(void);
int	OSSpawn(const char* name, void (*pc)(void), void* sp, unsigned int flags);
void	OSTerminate(int code);
int	OSJoin(int tid);
OSTask*	OSGetCurrentTask(void);

#endif
