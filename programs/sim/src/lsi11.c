#include <emulight/types.h>
#include <emulight/string.h>
#include "lsi11.h"

LSI11 lsi;

volatile int lsi_irq = 0;

int LSI11Interrupt(int n)
{
	if(n != 004) {
		LSIIRQ();
		if(lsi.bus.trap || lsi.bus.irq) {
			return 0;
		} else {
			lsi.bus.irq = n;
			lsi.bus.delay = 0;
			return 1;
		}
	} else {
		lsi.bus.trap = n;
		return 1;
	}
}

static unsigned int rng_state = 0;

static inline unsigned int rng(void)
{
	rng_state = (rng_state * 1103515245) + 12345;
	return rng_state;
}

static void LSI11QBUSStep()
{
	QBUS* bus = &lsi.bus;
	if(bus->delay >= QBUS_DELAY) {
		/* wait until last trap was serviced */
		if(!bus->trap) {
			bus->trap = bus->irq;
			bus->irq = 0;
			bus->delay = rng() % QBUS_DELAY_JITTER;
		}
	} else if(bus->irq) {
		bus->delay++;
	}
}

void LSI11Init(void)
{
	bzero(&lsi, sizeof(LSI11));

	KD11Init();
	lsi.bus.trap = 0;
	lsi.bus.nxm = 0;

	/* MSV11DInit */
	bzero(&lsi.msv11, sizeof(MSV11D));

	DLV11JInit();
}

void QBUSReset(void)
{
	/* clear pending interrupts */
	lsi_irq = 0;
	lsi.bus.trap = 0;
	lsi.bus.irq = 0;
	lsi.bus.delay = 0;

	DLV11JReset();
}

void LSI11Reset(void)
{
	KD11Reset();

	QBUSReset();
}

/* TODO: tune this properly */
#define STEPS	16

void LSI11Step(void)
{
	for(unsigned int i = 0; i < STEPS; i++) {
		LSI11QBUSStep();
		KD11Step();

		if(lsi.kd11.state == 0) {
			i += STEPS / 4;
		}

		if(lsi_irq || lsi.kd11.state != 1) {
			lsi_irq = 0;

			DLV11JStep();
		}
	}

	lsi_irq = 0;

	DLV11JStep();
}
