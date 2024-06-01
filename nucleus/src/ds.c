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
			size_t lrecl_phy = DSGetPhysicalRecLen(dataset);
			void (*decode)(char*, u8*, unsigned int) = NULL;

			u16 type = dataset->info.flags & 0xC000;
			if(type == EFS_DSCB_DS_TYPE_TXT6B) {
				/* 6bit text encoding */
				decode = R6Decode;
			} else if(type == EFS_DSCB_DS_TYPE_TXT7B) {
				/* 7bit text encoding */
				decode = R7Decode;
			}
			if(decode) {
				size_t off = lrecl - lrecl_phy;
				char* p = (char*) dst;
				size_t read = 0;
				for(size_t i = 0; i < size; i++) {
					u8* rd = (u8*) (p + off);
					size_t rcnt = DSRead(dataset, rd, lrecl_phy);
					decode(p, rd, lrecl_phy);
					if(rcnt != lrecl_phy) {
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
