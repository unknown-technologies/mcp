;# GNU C99 (GCC) version 13.2.1 20230801 (pdp11-aout)
;#	compiled by GNU C version 13.2.1 20230801, GMP version 6.3.0, MPFR version 4.2.1, MPC version 1.3.1, isl version isl-0.26-GMP

;# GGC heuristics: --param ggc-min-expand=100 --param ggc-min-heapsize=131072
;# options passed: -m40 -msoft-float -mno-split -Os -std=c99 -fwrapv -fwrapv-pointer -fzero-initialized-in-bss -fPIC -ffreestanding
	.text
	.data
LC_0:
	.byte 033,0133,077,062,065,0150,0
LC_1:
	.byte 0111,0123,0120,0106,040,0123,0124,0101,0122,0124,0105,0104,0
	.text
	.even
	.globl	_start
_start:
	mov	r2,-(sp)	;#,
	mov	r3,-(sp)	;#,
;# /home/daniel/git/mcp/programs/ispf/src/main.c:13: 	ISPFInit();
	jsr	pc,_ISPFInit	;# ISPFInit
;# /home/daniel/git/mcp/programs/ispf/src/main.c:14: 	VTGoto(1, 1);
	mov	$01,-(sp)	;#,
	mov	$01,-(sp)	;#,
	jsr	pc,_VTGoto	;# VTGoto
;# /home/daniel/git/mcp/programs/ispf/../../common/include/emulight/si.h:8: 	register const char* r0 asm("r0") = buf;
	mov	$LC_0,r0	;#, r0
;# /home/daniel/git/mcp/programs/ispf/../../common/include/emulight/si.h:9: 	register unsigned int r1 asm("r1") = size;
	mov	$06,r1	;#, r1
;# /home/daniel/git/mcp/programs/ispf/../../common/include/emulight/si.h:10: 	asm volatile ("emt 1" : "=r"(r0) : "r"(r0), "r"(r1) : "memory");
;# 10 "/home/daniel/git/mcp/programs/ispf/../../common/include/emulight/si.h" 1
	emt 1	
;# 0 "" 2
;# /home/daniel/git/mcp/programs/ispf/src/main.c:17: 	VTClearScreen();
	jsr	pc,_VTED2	;# VTED2
;# /home/daniel/git/mcp/programs/ispf/src/main.c:18: 	VTHome();
	jsr	pc,_VTHome	;# VTHome
;# /home/daniel/git/mcp/programs/ispf/../../common/include/emulight/os.h:48: 	register unsigned int r0 asm("r0") = sys;
	mov	$074506,r0	;#, r0
;# /home/daniel/git/mcp/programs/ispf/../../common/include/emulight/os.h:49: 	register unsigned int r1 asm("r1") = code;
	mov	$0144,r1	;#, r1
;# /home/daniel/git/mcp/programs/ispf/../../common/include/emulight/os.h:50: 	register unsigned int r2 asm("r2") = severity;
	mov	$01,r2	;#, r2
;# /home/daniel/git/mcp/programs/ispf/../../common/include/emulight/os.h:51: 	register const char* r3 asm("r3") = msg;
	mov	$LC_1,r3	;#, r3
;# /home/daniel/git/mcp/programs/ispf/../../common/include/emulight/os.h:52: 	asm volatile ("emt 10" : "=r"(r0) : "r"(r0), "r"(r1), "r"(r2), "r"(r3) : "memory");
;# 52 "/home/daniel/git/mcp/programs/ispf/../../common/include/emulight/os.h" 1
	emt 10	
;# 0 "" 2
;# /home/daniel/git/mcp/programs/ispf/../../common/include/emulight/os.h:53: }
	add	$04,sp	;#,
L_2:
;# /home/daniel/git/mcp/programs/ispf/src/main.c:23: 		ISPFProcess();
	jsr	pc,_ISPFProcess	;# ISPFProcess
;# /home/daniel/git/mcp/programs/ispf/../../common/include/emulight/os.h:31: 	asm volatile ("emt 7");
;# 31 "/home/daniel/git/mcp/programs/ispf/../../common/include/emulight/os.h" 1
	emt 7
;# 0 "" 2
	br	L_2		;#
