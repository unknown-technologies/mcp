#include <emulight/types.h>
#include <emulight/mcp/rl02.h>

static unsigned int rl_ca;
unsigned int rl_boot_unit;

void RLInit(void)
{
	rl_ca = 0;
	u16 csr = RL_RLCS;
	rl_boot_unit = RL_CSR_UNIT(csr);
}

void RLReadHeader(void)
{
}
