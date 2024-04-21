#ifndef __MCP_H__
#define __MCP_H__

#include "types.h"

typedef struct {
	u16	magic;
	u16	image_size;
	u16	symbols;
	u16	relocs;
} PROGRAM;

typedef struct {
	u16	name;
	u16	type;
	u16	value;
} SYM;

#endif
