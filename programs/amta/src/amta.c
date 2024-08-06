#include "amta.h"

void amta(void)
{
	while(1) {
		u16 key = VTReceive(&vt, NULL, NULL);
		switch(key) {
			case VT_KEY_F10:
				return;
		}
	}
}
