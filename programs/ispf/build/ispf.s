;# GNU C99 (GCC) version 13.2.1 20230801 (pdp11-aout)
;#	compiled by GNU C version 13.2.1 20230801, GMP version 6.3.0, MPFR version 4.2.1, MPC version 1.3.1, isl version isl-0.26-GMP

;# GGC heuristics: --param ggc-min-expand=100 --param ggc-min-heapsize=131072
;# options passed: -m40 -msoft-float -mno-split -Os -std=c99 -fwrapv -fwrapv-pointer -fzero-initialized-in-bss -fPIC -ffreestanding
	.text
	.data
LC_0:
	.byte 0124,0150,0151,0163,040,0151,0163,040,0163,0157,0146,0164,0167,0141,0162,0145,040,0166,0145,0162,0163,0151,0157,0156,040,0162,070,0176,061,0142,0142,066,070,071,065,0
LC_1:
	.byte 0123,0157,0146,0164,0167,0141,0162,0145,040,0126,0145,0162,0163,0151,0157,0156,0
LC_2:
	.byte 0106,0165,0156,0143,0164,0151,0157,0156,040,0156,0157,0164,040,0151,0155,0160,0154,0145,0155,0145,0156,0164,0145,0144,0
	.text
	.even
	.globl	_ISPFMenuHandler
_ISPFMenuHandler:
	mov	r2,-(sp)	;#,
	mov	04(sp),r2	;# spf, spf
	mov	06(sp),r0	;# action, action
;# /home/daniel/git/mcp/programs/ispf/src/ispf.c:56: 	switch(action) {
	cmp	r0,$05	;# action,
	beq	L_2		;#
	cmp	r0,$07	;# action,
	beq	L_3		;#
	cmp	r0,$04	;# action,
	bne	L_4		;#
;# /home/daniel/git/mcp/programs/ispf/src/ispf.c:58: 			if(spf->current_panel > 0) {
	mov	0122(r2),r0	;# spf_5(D)->current_panel, _12
;# /home/daniel/git/mcp/programs/ispf/src/ispf.c:58: 			if(spf->current_panel > 0) {
	ble	L_1		;#
;# /home/daniel/git/mcp/programs/ispf/../../common/include/emulight/ispf/panel.h:83: 		return &spf->panels[spf->current_panel];
	mov	$012,r1	;#,
	mul	r0,r1	;# _12,
	mov	r1,r0	;#, tmp35
	add	$02,r0	;#, tmp36
;# /home/daniel/git/mcp/programs/ispf/src/ispf.c:59: 				SPFDestroyPanel(SPFCurrentPanel(spf));
	add	r0,r2	;# tmp36, spf
	mov	r2,-(sp)	;# spf,
	jsr	pc,_SPFDestroyPanel	;# SPFDestroyPanel
	add	$02,sp	;#,
L_1:
;# /home/daniel/git/mcp/programs/ispf/src/ispf.c:76: }
	mov	(sp)+,r2	;#,
	rts	pc	
L_7:
;# /home/daniel/git/mcp/programs/ispf/../../common/include/emulight/ispf/panel.h:83: 		return &spf->panels[spf->current_panel];
	mov	$012,r1	;#,
	mul	r0,r1	;# _1,
	mov	r1,r0	;#, tmp39
	add	$02,r0	;#, tmp40
;# /home/daniel/git/mcp/programs/ispf/src/ispf.c:64: 				SPFDestroyPanel(SPFCurrentPanel(spf));
	add	r2,r0	;# spf, tmp40
	mov	r0,-(sp)	;# tmp40,
	jsr	pc,_SPFDestroyPanel	;# SPFDestroyPanel
	add	$02,sp	;#,
L_2:
;# /home/daniel/git/mcp/programs/ispf/src/ispf.c:63: 			while(spf->current_panel > 0) {
	mov	0122(r2),r0	;# spf_5(D)->current_panel, _1
;# /home/daniel/git/mcp/programs/ispf/src/ispf.c:63: 			while(spf->current_panel > 0) {
	bgt	L_7		;#
	br	L_1		;#
L_3:
;# /home/daniel/git/mcp/programs/ispf/src/ispf.c:68: 			SPFShowMessage(spf, "Software Version",
	mov	$LC_0,-(sp)	;#,
	mov	$LC_1,-(sp)	;#,
	mov	r2,-(sp)	;# spf,
	jsr	pc,_SPFShowMessage	;# SPFShowMessage
;# /home/daniel/git/mcp/programs/ispf/src/ispf.c:71: 			break;
	add	$06,sp	;#,
	br	L_1		;#
L_4:
;# /home/daniel/git/mcp/programs/ispf/src/ispf.c:73: 			SPFShowError(spf, "Function not implemented");
	mov	$LC_2,-(sp)	;#,
	mov	r2,-(sp)	;# spf,
	jsr	pc,_SPFShowError	;# SPFShowError
;# /home/daniel/git/mcp/programs/ispf/src/ispf.c:74: 			break;
	add	$04,sp	;#,
;# /home/daniel/git/mcp/programs/ispf/src/ispf.c:76: }
	br	L_1		;#
	.even
	.globl	_ISPFInit
_ISPFInit:
;# /home/daniel/git/mcp/programs/ispf/src/ispf.c:81: 	SPFInit(&ispf);
	mov	$_ispf,-(sp)	;#,
	jsr	pc,_SPFInit	;# SPFInit
;# /home/daniel/git/mcp/programs/ispf/src/ispf.c:82: 	SPFSetMenuBar(&ispf, menus, ISPFMenuHandler);
	mov	$_ISPFMenuHandler,-(sp)	;#,
	mov	$_menus,-(sp)	;#,
	mov	$_ispf,-(sp)	;#,
	jsr	pc,_SPFSetMenuBar	;# SPFSetMenuBar
;# /home/daniel/git/mcp/programs/ispf/src/ispf.c:83: }
	add	$010,sp	;#,
	rts	pc	
	.even
	.globl	_ISPFProcess
_ISPFProcess:
;# /home/daniel/git/mcp/programs/ispf/src/ispf.c:87: 	SPFProcess(&ispf);
	mov	$_ispf,-(sp)	;#,
	jsr	pc,_SPFProcess	;# SPFProcess
;# /home/daniel/git/mcp/programs/ispf/src/ispf.c:88: }
	add	$02,sp	;#,
	rts	pc	
	.data
	.even
_ispf:	.=.+ 0232
	.data
LC_3:
	.byte 0123,0131,0123,0124,0105,0115,0
LC_4:
	.byte 0120,0101,0116,0105,0114,0
LC_5:
	.byte 0110,0105,0114,0120,0
	.data
	.even
_menus:
;# name:
	.word	LC_3
;# items:
	.word	_system_menu
;# name:
	.word	LC_4
;# items:
	.word	_panel_menu
;# name:
	.word	LC_5
;# items:
	.word	_help_menu
;# name:
	.word	0
;# items:
	.word	0
	.data
LC_6:
	.byte 0117,0156,0154,0151,0156,0145,040,0110,0145,0154,0160,0
LC_7:
	.byte 055,0
LC_8:
	.byte 0101,0142,0157,0165,0164,056,056,056,0
	.data
	.even
_help_menu:
;# name:
	.word	LC_6
;# action:
	.word	010
	.=.+ 02
;# name:
	.word	LC_7
;# action:
	.word	0
	.=.+ 02
;# name:
	.word	LC_1
;# action:
	.word	07
	.=.+ 02
;# name:
	.word	LC_8
;# action:
	.word	06
	.=.+ 02
;# name:
	.word	0
;# action:
	.word	0
	.=.+ 02
	.data
LC_9:
	.byte 0120,0141,0156,0145,0154,040,0114,0151,0163,0164,0
LC_10:
	.byte 0103,0154,0157,0163,0145,040,0120,0141,0156,0145,0154,0
LC_11:
	.byte 0103,0154,0157,0163,0145,040,0101,0154,0154,040,0120,0141,0156,0145,0154,0163,0
	.data
	.even
_panel_menu:
;# name:
	.word	LC_9
;# action:
	.word	03
	.=.+ 02
;# name:
	.word	LC_10
;# action:
	.word	04
;# key:
	.word	027
;# name:
	.word	LC_11
;# action:
	.word	05
	.=.+ 02
;# name:
	.word	0
;# action:
	.word	0
	.=.+ 02
	.data
LC_12:
	.byte 0123,0171,0163,0164,0145,0155,040,0111,0156,0146,0157,0162,0155,0141,0164,0151,0157,0156,0
LC_13:
	.byte 0106,0165,0154,0154,0163,0143,0162,0145,0145,0156,040,0103,0154,0157,0143,0153,0
LC_14:
	.byte 0103,0157,0156,0146,0151,0147,0165,0162,0145,040,0103,0154,0157,0143,0153,0
	.data
	.even
_system_menu:
;# name:
	.word	LC_12
;# action:
	.word	0
	.=.+ 02
;# name:
	.word	LC_13
;# action:
	.word	01
	.=.+ 02
;# name:
	.word	LC_7
;# action:
	.word	0
	.=.+ 02
;# name:
	.word	LC_14
;# action:
	.word	02
	.=.+ 02
;# name:
	.word	0
;# action:
	.word	0
	.=.+ 02
