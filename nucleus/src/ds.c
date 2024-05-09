#include <emulight/types.h>
#include <emulight/mcp/fs.h>
#include <emulight/mcp/ds.h>
#include <emulight/mcp/sched.h>

int __DSOpen(const char* dsn)
{
	OSTask* task = OSGetCurrentTask();
	for(unsigned int i = 0; i < MAX_DATASETS; i++) {
		if(~task->ds_mask & (1 << i)) {
			DS* ds = &task->datasets[i];
			if(DSOpen(ds, dsn)) {
				task->ds_mask |= 1 << i;
				return i;
			} else {
				return -1;
			}
		}
	}
	return -1;
}

size_t __DSRead(int ds, void* dst, size_t size)
{
	OSTask* task = OSGetCurrentTask();
	if(~task->ds_mask & (1 << ds)) {
		return 0;
	}

	DS* dataset = &task->datasets[ds];
	return DSRead(dataset, dst, size);
}

void __DSSeek(int ds, size_t pos)
{
	OSTask* task = OSGetCurrentTask();
	if(~task->ds_mask & (1 << ds)) {
		return;
	}

	DS* dataset = &task->datasets[ds];
	DSSeek(dataset, pos);
}

void __DSClose(int ds)
{
	OSTask* task = OSGetCurrentTask();
	task->ds_mask &= ~(1 << ds);
}
