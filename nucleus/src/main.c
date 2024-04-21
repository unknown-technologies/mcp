#include <emulight/string.h>
#include <emulight/mcp/mon.h>
#include <emulight/mcp/si.h>
#include <emulight/mcp/syslog.h>
#include <emulight/mcp/rl02.h>
#include <emulight/mcp/sched.h>

void print_readme(void)
{
	unsigned int vtocix;
	unsigned int block;
	unsigned int size;

	if(MONLocate("SYS1.README", &vtocix, &block, &size)) {
		OSLog(MSGID_NUC, 10, LOG_INFO, "SYS1.README HAS VTOCIX=%d, BLOCK=%d, SIZE=%d", vtocix, block, size);

		for(unsigned int i = 0; i < size; i++) {
			MONReadBlock(__sector_buffer, block + i);
			SITransmit(__sector_buffer, strlen((const char*) __sector_buffer));
		}
		SITransmit("\r\n", 2);
	}
}

static u16 print_stack[128];
static u16 io_stack[128];

static void io_task(void)
{
	while(1) {
		char c;
		if(MONTTI(&c, 1)) {
			OSLog(MSGID_NUC, 11, LOG_INFO, "RX: %02X", (unsigned int) (unsigned char) c);

			if(c == 'q' || c == 'Q') {
				return;
			}
		} else {
			MONYield();
		}
	}
}

void nucleus(void)
{
	OSLogInit();
	OSSchedInit();
	RLInit();

	OSSpawn("PRINT", print_readme, &print_stack[127], TASK_FLAG_NOWAIT);
	int iotid = OSSpawn("IO", io_task, &io_stack[127], 0);

	OSJoin(iotid);

	OSLog(MSGID_NUC, 500, LOG_INFO, "NUCLEUS EXITED");

	MONHalt();

	asm("HALT");
}
