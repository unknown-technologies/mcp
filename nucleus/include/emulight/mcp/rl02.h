#ifndef __RL02_H__
#define __RL02_H__

#include <emulight/types.h>

#define	RL_RLCS			*((volatile unsigned int*) 0174400)
#define	RL_RLBA			*((volatile unsigned int*) 0174402)
#define	RL_RLDA			*((volatile unsigned int*) 0174404)
#define	RL_RLMP			*((volatile unsigned int*) 0174406)
#define	RL_RLBAE		*((volatile unsigned int*) 0174410)

#define	RL_VECTOR		0160

#define	RL_DRDY			_BV(0)
#define	RL_FUNCTION_MASK	(_BV(1) | _BV(2) | _BV(3))
#define	RL_BA16			_BV(4)
#define	RL_BA17			_BV(5)
#define	RL_IE			_BV(6)
#define	RL_CRDY			_BV(7)
#define	RL_DS0			_BV(8)
#define	RL_DS1			_BV(9)
#define	RL_E0			_BV(10)
#define	RL_E1			_BV(11)
#define	RL_E2			_BV(12)
#define	RL_E3			_BV(13)
#define	RL_DE			_BV(14)
#define	RL_ERR			_BV(15)

#define RL_CSR_UNIT_MASK	(RL_DS0 | RL_DS1)
#define RL_CSR_UNIT(csr)	(((csr) & RL_CSR_UNIT_MASK) >> 8)

#define	RL_DAR_SEEK		_BV(1)
#define	RL_DAR_SEEK_DIR		_BV(2)
#define	RL_DAR_SEEK_HS		_BV(4)

#define	RL_MAINTENANCE		(00 << 1)
#define	RL_WRITE_CHECK		(01 << 1)
#define	RL_GET_STATUS		(02 << 1)
#define	RL_SEEK			(03 << 1)
#define	RL_READ_HEADER		(04 << 1)
#define	RL_WRITE_DATA		(05 << 1)
#define	RL_READ_DATA		(06 << 1)
#define	RL_READ_DATA_NOCHECK	(07 << 1)

#define	RL_ERR_OPI		(01 << 10)
#define	RL_ERR_DCRC		(02 << 10)
#define	RL_ERR_WCE		(02 << 10)
#define	RL_ERR_HCRC		(03 << 10)
#define	RL_ERR_DLT		(04 << 10)
#define	RL_ERR_HNF		(05 << 10)
#define	RL_ERR_NXM		(10 << 10)
#define	RL_ERR_MPE		(11 << 10)

void RLInit(void);

extern unsigned int rl_boot_unit;

static inline unsigned int RLGetBootUnit(void)
{
	return rl_boot_unit;
}

#endif
