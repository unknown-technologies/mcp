#include <emulight/types.h>
#include <emulight/vt.h>
#include "lsi11.h"

static VT vt;
static u8 input_buf[16];

void start(void)
{
	VTInit(&vt);

	LSI11Init();
	LSI11Reset();

	lsi.kd11.state = 0;

	while(1) {
		u8 len = 0;
		u16 key = VTReceive(&vt, input_buf, &len);
		if(key == VT_KEY_F10) {
			return;
		} else if(key == VT_KEY_F12) {
			lsi.kd11.state = 0;
			/* lsi.kd11.odt.state = 0; */
		} else if(len && key != VT_KEY_NONE) {
			for(unsigned int i = 0; i < len; i++) {
				SYSReceive(input_buf[i]);
			}
		}

		LSI11Step();
	}
}
