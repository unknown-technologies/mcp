#include <emulight/types.h>
#include <emulight/rad50.h>
#include <emulight/mcp/mon.h>
#include <emulight/mcp/syslog.h>
#include <emulight/mcp/sched.h>

extern void** __schedule;

static unsigned int taskid = 0;

static OSTask tasks[MAX_TASKS] = { 0 };

static void* OSSchedule(void* sp)
{
	tasks[taskid].sp = sp;
	for(unsigned int i = 0, j = (taskid + 1) % MAX_TASKS; i < MAX_TASKS;
			i++, j = (j + 1) % MAX_TASKS) {
		if(tasks[j].state == TASK_RUNNING) {
			taskid = j;
			break;
		}
	}
	return tasks[taskid].sp;
}

void OSSchedInit(void)
{
	taskid = 0;
	OSTask* idle = &tasks[taskid];
	RAD50EncodeString(idle->name, "IDLE", TASK_NAMELEN);
	idle->state = TASK_RUNNING;
	idle->sp = 0;
	idle->flags = 0;

	*__schedule = OSSchedule;
}

static inline void __OSTrampoline(void)
{
	OSTerminate(0);
}

int OSSpawn(const char* name, void (*pc)(void), void* sp, unsigned int flags)
{
	char taskname[TASK_NAMELEN * 3 + 1];

	for(unsigned int i = 0; i < MAX_TASKS; i++) {
		OSTask* task = &tasks[i];
		if(task->state == TASK_FREE) {
			void** stack = (void**) sp;
			*stack = (void*) __OSTrampoline;
			task->sp = MONSpawn(sp, pc);
			task->state = TASK_RUNNING;
			task->flags = flags;
			RAD50EncodeString(task->name, name, TASK_NAMELEN);
			RAD50DecodeString(taskname, task->name, TASK_NAMELEN);

			OSLog(NUC_TASK_START, "TASK %s STARTED", taskname);
			return i;
		}
	}

	return -1;
}

void OSTerminate(int status)
{
	OSTask* task = &tasks[taskid];
	char taskname[TASK_NAMELEN * 3 + 1];
	RAD50DecodeString(taskname, task->name, TASK_NAMELEN);
	OSLog(NUC_TASK_STOP, "TASK %s STOPPED WITH STATUS %d", taskname, status);

	if(task->flags & TASK_FLAG_NOWAIT) {
		task->state = TASK_FREE;
	} else {
		task->state = TASK_DEAD;
	}

	task->code = (u8) status;

	MONYield();
}

int OSJoin(int tid)
{
	if(tid < 0 || tid >= MAX_TASKS) {
		return -1;
	}

	volatile OSTask* task = (volatile OSTask*) &tasks[tid];
	while(1) {
		if(task->state == TASK_DEAD) {
			int code = task->code;
			task->state = TASK_FREE;
			return code;
		} else if(task->state == TASK_FREE) {
			return -1;
		}
		MONYield();
	}
}
