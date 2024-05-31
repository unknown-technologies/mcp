#include <emulight/types.h>
#include <emulight/r6.h>
#include <emulight/r7.h>
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

static DS* get_ds(int ds)
{
	OSTask* task = OSGetCurrentTask();
	if(~task->ds_mask & (1 << ds)) {
		return NULL;
	}

	return &task->datasets[ds];
}

void __DSGetInfo(int ds, DSInfo* info)
{
	DS* dataset = get_ds(ds);
	if(dataset) {
		DSGetInfo(dataset, info);
	}
}

size_t __DSRead(int ds, void* dst, size_t size)
{
	DS* dataset = get_ds(ds);
	if(dataset) {
		if(DSIsFB(dataset)) {
			size_t lrecl = DSGetRecLen(dataset);
			u16 type = dataset->info.flags & 0xC000;
			if(type == EFS_DSCB_DS_TYPE_TXT6B) {
				/* 6bit text encoding */
				size_t lrecl6 = (lrecl + 3) / 4 * 3;
				size_t off = lrecl - lrecl6;
				char* p = (char*) dst;
				size_t read = 0;
				for(size_t i = 0; i < size; i++) {
					u8* rd = (u8*) (p + off);
					size_t rcnt = DSRead(dataset, rd, lrecl6);
					R6Decode(p, rd, lrecl6);
					if(rcnt != lrecl6) {
						break;
					} else {
						read++;
						p += lrecl;
					}
				}
				return read;
			} else if(type == EFS_DSCB_DS_TYPE_TXT7B) {
				/* 7bit text encoding */
				size_t lrecl7 = (lrecl + 7) / 8 * 7;
				size_t off = lrecl - lrecl7;
				char* p = (char*) dst;
				size_t read = 0;
				for(size_t i = 0; i < size; i++) {
					u8* rd = (u8*) (p + off);
					size_t rcnt = DSRead(dataset, rd, lrecl7);
					R7Decode(p, rd, lrecl7);
					if(rcnt != lrecl7) {
						break;
					} else {
						read++;
						p += lrecl;
					}
				}
				return read;
			} else {
				size_t sz = lrecl * size;
				size_t rd = DSRead(dataset, dst, sz);
				return (rd + lrecl - 1) / lrecl;
			}
		} else {
			return DSRead(dataset, dst, size);
		}
	} else {
		return 0;
	}
}

size_t __DSWrite(int ds, void* src, size_t size)
{
	return 0;
}

void __DSSeek(int ds, size_t pos)
{
	DS* dataset = get_ds(ds);
	if(dataset) {
		if(DSIsFB(dataset)) {
			size_t p = DSGetRecLen(dataset) * pos;
			DSSeek(dataset, p);
		} else {
			DSSeek(dataset, pos);
		}
	}
}

void __DSClose(int ds)
{
	OSTask* task = OSGetCurrentTask();
	task->ds_mask &= ~(1 << ds);
}
