#include <emulight/types.h>
#include <emulight/vt.h>
#include <emulight/string.h>
#include <emulight/ds.h>
#include "lsi11.h"

#define	DSN_LEN	32
static char dsn[DSN_LEN + 1];

static void load(const char* dsn)
{
	int ds = DSOpen(dsn);
	if(ds < 0) {
		VTPrint("FAILED TO LOAD\r\n");
		return;
	}

	DS info;
	DSGetInfo(ds, &info);
	if(DSIsFB(&info)) {
		VTPrint("UNSUPPORTED DS FORMAT\r\n");
		DSClose(ds);
		return;
	}

	while(1) {
		u8 c = 0;
		while(DSRead(ds, &c, 1) == 1) {
			if(c == 1) {
				break;
			}
		}

		if(c != 1) {
			break;
		}

		if(DSRead(ds, &c, 1) != 1) {
			VTPrint("UNEXPECTED EOF\r\n");
			goto error;
		}

		if(c != 0) {
			VTPrint("UNEXPECTED SIGNATURE\r\n");
			goto error;
		}

		u16 len;
		u16 addr;

		if(DSRead(ds, &len, 2) != 2) {
			VTPrint("UNEXPECTED EOF\r\n");
			goto error;
		}

		if(DSRead(ds, &addr, 2) != 2) {
			VTPrint("UNEXPECTED EOF\r\n");
			goto error;
		}

		VTPrintf("LOAD: %u @ %06o\r\n", len - 6, addr);

		if(len > 6) {
			if(addr < MSV11D_SIZE && (addr + len - 6) < MSV11D_SIZE) {
				u8* mem = (u8*) lsi.msv11.data;
				if(DSRead(ds, &mem[addr], len - 6) != len - 6) {
					VTPrint("UNEXPECTED EOF\r\n");
					goto error;
				}
			} else {
				VTPrint("INVALID ADDR/LEN\r\n");
				goto error;
			}
		}

		u8 cksum;
		if(DSRead(ds, &cksum, 1) != 1) {
			VTPrint("UNEXPECTED EOF\r\n");
			goto error;
		}

		if(len == 6) {
			if((addr & 1) == 0) {
				lsi.kd11.r[7] = addr;
				lsi.kd11.state = 1;
			} else {
				lsi.kd11.r[7] = 0200;
				lsi.kd11.state = 1;
			}
			lsi.kd11.odt.state = 0;
			break;
		}
	}

error:
	DSClose(ds);
}

BOOL menu(void* _vt, u8* input_buf)
{
	VT* vt = (VT*) _vt;

	int pos = 0;
	bzero(dsn, sizeof(dsn));

	VTPrint(CSI "2J" CSI "HLOAD> ");
	while(1) {
		u8 len = 0;
		u16 key = VTReceive(vt, input_buf, &len);
		switch(key) {
			case VT_KEY_F10:
				return FALSE;
			case VT_KEY_F11:
				VTClearScreen();
				VTHome();
				return TRUE;
			case VT_KEY_ENTER:
				/* process it */
				VTPrint("\r\n");
				VTPrintf("LOADING '%s' ...\r\n", dsn);
				load(dsn);
				return TRUE;
			case VT_KEY_BS:
			case VT_KEY_DEL:
				if(pos > 0) {
					VTPrint("\b \b");
					dsn[--pos] = 0;
				}
				break;
			default:
				if(pos < DSN_LEN) {
					if((key >= 'A' && key <= 'Z')
							|| (key >= '0' && key <= '9')
							|| key == '$'
							|| key == '#'
							|| key == '@'
							|| key == '.') {
						dsn[pos++] = key;
						dsn[pos] = 0;
						VTPutc(dsn[pos - 1]);
					} else if(key >= 'a' && key <= 'z') {
						dsn[pos++] = key & ~0x20;
						dsn[pos] = 0;
						VTPutc(dsn[pos - 1]);
					}
				}
				break;
		}
	}
}

