#include <emulight/types.h>
#include <emulight/mcp/mem.h>
#include <emulight/mcp/syslog.h>

static void* mem_start;
static void* mem_end;
static void* mem_alloc_top;
static void* mem_alloc_bottom;

void MEMInit(void* start, void* end)
{
	mem_start = start;
	mem_end = end;
	mem_alloc_bottom = start;
	mem_alloc_top = end;
	size_t size = (size_t) (end - start);
	OSLog(NUC_MEMORY, "MEMORY REGION: %04X-%04X (%u BYTES)", (unsigned int) start, (unsigned int) end, (unsigned int) size);
}

void* MEMAlloc(size_t size)
{
#ifdef DEBUG
	OSLog(NUC_MALLOC, "ALLOCATING %u BYTES", (unsigned int) size);
#endif

	if(size & 1) {
		size++;
	}

	size_t sz = size + 2;
	if(mem_alloc_bottom + sz <= mem_alloc_top) {
		mem_alloc_top -= sz;
		unsigned int* area = (unsigned int*) mem_alloc_top;
		*area = sz;
		return (void*) (area + 1);
	} else {
		OSLog(NUC_MALLOC_FAIL, "FAILED TO ALLOCATE %u BYTES", (unsigned int) size);
		return NULL;
	}
}

void MEMFree(void* ptr)
{
#ifdef DEBUG
	OSLog(NUC_MFREE, "FREEING MEMORY AT %04X", (unsigned int) ptr);
#endif

	unsigned int* area = (unsigned int*) ptr - 1;
	unsigned int size = *area;
	if(area == mem_alloc_top) {
		mem_alloc_top += size;
	} else {
		OSLog(NUC_MFREE_FAIL, "FAILED TO FREE %u BYTES", size);
	}
}

void* MEMAllocBottom(size_t size)
{
#ifdef DEBUG
	OSLog(NUC_MALLOC, "ALLOCATING %u BYTES", (unsigned int) size);
#endif

	if(size & 1) {
		size++;
	}

	size_t sz = size + 2;
	if(mem_alloc_bottom + sz <= mem_alloc_top) {
		unsigned int* area = (unsigned int*) mem_alloc_bottom;
		mem_alloc_bottom += sz;
		*area = sz;
		return (void*) (area + 1);
	} else {
		OSLog(NUC_MALLOC_FAIL, "FAILED TO ALLOCATE %u BYTES", (unsigned int) size);
		return NULL;
	}
}

void MEMFreeBottom(void* ptr)
{
#ifdef DEBUG
	OSLog(NUC_MFREE, "FREEING MEMORY AT %04X", (unsigned int) ptr);
#endif

	unsigned int* area = (unsigned int*) ptr - 1;
	unsigned int size = *area;
	if((void*) area + size == mem_alloc_bottom) {
		mem_alloc_bottom -= size;
	} else {
		OSLog(NUC_MFREE_FAIL, "FAILED TO FREE %u BYTES", size);
	}
}

size_t MEMAvailable(void)
{
	return (size_t) (mem_alloc_top - mem_alloc_bottom);
}
