#ifndef __LSI_11_H__
#define __LSI_11_H__

#include <emulight/types.h>

#ifndef _BV
#define	_BV(x)	(1 << (x))
#endif

/* Main memory size: 4kW / 8kB */
#define	MSV11D_SIZE		8192

/* QBUS interrupt request delay */
#define	QBUS_DELAY		20
#define	QBUS_DELAY_JITTER	10

extern volatile int lsi_irq;

typedef struct {
	u16	addr;
	u16	val;
	u8	input;
	u8	state;
	u8	next;
	u8	buf[16];
	u8	buf_r;
	u8	buf_sz;
} KD11ODT;

typedef struct {
	u16	r[8];
	u16	psw;
	KD11ODT	odt;
	u8	state;
	u16	trap;
} KD11;

typedef struct QBUS QBUS;
typedef struct LSI11 LSI11;

struct QBUS {
	u16	trap;
	u16	delay;
	u16	irq;
	u16	nxm;
};

/* peripherals */
typedef struct {
	u16	data[MSV11D_SIZE / 2];
} MSV11D;

typedef struct {
	u16	irq;

	u16	rcsr0;
	u16	rbuf0;
	u16	xcsr0;
	u16	xbuf0;

	u16	rcsr1;
	u16	rbuf1;
	u16	xcsr1;
	u16	xbuf1;

	u16	rcsr2;
	u16	rbuf2;
	u16	xcsr2;
	u16	xbuf2;

	u16	rcsr3;
	u16	rbuf3;
	u16	xcsr3;
	u16	xbuf3;
} DLV11J;

/* global LSI-11 state */
struct LSI11 {
	QBUS	bus;
	KD11	kd11;
	MSV11D	msv11;
	DLV11J	dlv11;
};

extern LSI11 lsi;

BOOL	menu(void* vt, u8* input_buf);

/* peripheral subroutines */

void	MSV11DInit(void);

void	DLV11JInit(void);
void	DLV11JReset(void);
void	DLV11JSend(unsigned char c);
u16	DLV11JRead(u16 address);
void	DLV11JWrite(u16 address, u16 value);
void	DLV11JStep(void);

void	SYSReceive(unsigned char c);

/* KD11 subroutines */
void	KD11Init(void);
void	KD11Reset(void);
void	KD11Step(void);
void	KD11Trap(int n);

/* LSI-11 subroutines */
void	LSI11Init(void);
void	LSI11Reset(void);
void	LSI11Restart(void);
void	LSI11Step(void);

int	LSI11Interrupt(int n);
void	QBUSReset(void);

/* small routines */
static inline void LSI11CancelInterrupt(int n)
{
	if(lsi.bus.irq == n) {
		lsi.bus.irq = 0;
	}

	if(lsi.bus.trap == n) {
		lsi.bus.trap = 0;
	}
}

#include <emulight/vt.h>
/* performance critical memory access routines */
static inline u16 LSI11ReadDMA(u16 address, BOOL* nxm)
{
	*nxm = 0;
	address &= 0xFFFE;

	if(address < MSV11D_SIZE) {
		return lsi.msv11.data[address >> 1];
	} else if(address >= 0177560 && address <= 0177566) {
		return DLV11JRead(address);
	}

	*nxm = 1;
	return 0;
}

static inline u16 LSI11WriteDMA(u16 address, u16 value)
{
	address &= 0xFFFE;

	if(address < MSV11D_SIZE) {
		lsi.msv11.data[address >> 1] = value;
	} else if(address >= 0177560 && address <= 0177566) {
		DLV11JWrite(address, value);
	} else {
		return 1;
	}

	return 0;
}

static inline u16 LSI11Read(u16 address)
{
	BOOL nxm;

	u16 data = LSI11ReadDMA(address, &nxm);

	if(nxm) {
		lsi.bus.nxm = 1;
	}

	return data;
}

static inline void LSI11Write(u16 address, u16 value)
{
	if(LSI11WriteDMA(address, value)) {
		lsi.bus.nxm = 1;
	}
}

static inline void LSIIRQ(void)
{
	lsi_irq = 1;
}

#endif
