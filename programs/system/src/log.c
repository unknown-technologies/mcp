#include <emulight/types.h>
#include <emulight/ispf.h>
#include <emulight/os.h>
#include <emulight/string.h>

#include "system.h"

static char line_buf[80 * 48];

static int lines;

static void SYSLogGetLogMsg(const char* msg, void* user)
{
	/* TODO: implement ring buffer to always get the end of the log */
	if(lines >= 48) {
		return;
	}

	char* line = &line_buf[80 * lines];
	lines++;

	size_t len = strlen(msg);
	if(len > 80) {
		len = 80;
	}

	memcpy(line, msg, len);
}

static void SYSLogGetLog(void)
{
	bzero(line_buf, sizeof(line_buf));
	lines = 0;

	OSGetLog(SYSLogGetLogMsg, NULL);
}

static void SYSLogInit(SPFPanel* self)
{
	SPFHideCursor(self);
	SPFHideActions(self); /* we want to use all 22 lines for logs */
	SPFSetPanelTitle(self, "System Log");

	SYSLogGetLog();
}

static void SYSLogDraw(SPFPanel* self)
{
	char* line = line_buf;
	int cnt = lines;

	if(cnt > 22) {
		line = &line_buf[80 * (lines - 22)];
		cnt = 22;
	}

	for(int i = 0; i < cnt; i++) {
		VTGoto(2 + i, 1);

		for(int j = 0; j < 80; j++) {
			if(line[j] == 0) {
				break;
			}
			VTPutc(line[j]);
		}

		line += 80;
	}
}

static void SYSLogProcess(SPFPanel* self, u16 key)
{
	if(key == VT_KEY_ENTER) {
		SPFDestroyPanel(self);
	}
}

const SPFPanelDefinition syslog_panel = {
	.init = SYSLogInit,
	.draw = SYSLogDraw,
	.update = NULL,
	.process = SYSLogProcess,
	.destroy = NULL
};
