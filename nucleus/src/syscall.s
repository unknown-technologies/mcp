	.text
	.even
	.globl	_OSSyscallInit

_OSSyscallInit:
	mov	r0,	-(sp)
	mov	r1,	-(sp)
	mov	r2,	-(sp)
	mov	r3,	-(sp)
	mov	$EMT_table, r3	# EMT table
	mov	$10,	r1	# first EMT ID
	mov	$10,	r2	# count
.L0:	mov	(r3)+,	r0
	emt	5
	inc	r1
	sob	r2,	.L0
	mov	(sp)+,	r3
	mov	(sp)+,	r2
	mov	(sp)+,	r1
	mov	(sp)+,	r0
	rts	pc

	.globl	_OSLog
	.globl	_OSLoadProgram
	.globl	_OSUnloadProgram
	.globl	_OSSpawn
	.globl	_OSJoin
	.globl	_OSTerminate
	.globl	___DSOpen
	.globl	___DSRead
	.globl	___DSSeek
	.globl	___DSClose

EMT_OSLog:
	mov	r3,	-(sp)
	mov	r2,	-(sp)
	mov	r1,	-(sp)
	mov	r0,	-(sp)
	jsr	pc,	_OSLog
	add	$8,	sp
	rti

EMT_OSLoadProgram:
	mov	r1,	-(sp)
	mov	r0,	-(sp)
	jsr	pc,	_OSLoadProgram
	add	$4,	sp
	rti

EMT_OSUnloadProgram:
	mov	r0,	-(sp)
	jsr	pc,	_OSUnloadProgram
	clr	(sp)+
	rti

EMT_OSSpawn:
	mov	r3,	-(sp)
	mov	r2,	-(sp)
	mov	r1,	-(sp)
	mov	r0,	-(sp)
	jsr	pc,	_OSSpawn
	add	$8,	sp
	rti

EMT_OSJoin:
	mov	r0,	-(sp)
	jsr	pc,	_OSJoin
	clr	(sp)+
	rti

EMT_OSTerminate:
	mov	r0,	-(sp)
	jsr	pc,	_OSTerminate
	clr	(sp)+
	rti

EMT_DSOpen:
	mov	r0,	-(sp)
	jsr	pc,	___DSOpen
	clr	(sp)+
	rti

EMT_DSRead:
	mov	r2,	-(sp)
	mov	r1,	-(sp)
	mov	r0,	-(sp)
	jsr	pc,	___DSRead
	add	$6,	sp
	rti

EMT_DSSeek:
	mov	r1,	-(sp)
	mov	r0,	-(sp)
	jsr	pc,	___DSSeek
	add	$4,	sp
	rti

EMT_DSClose:
	mov	r0,	-(sp)
	jsr	pc,	___DSClose
	clr	(sp)+
	rti

	.data
	.even
EMT_table:
	.word	EMT_OSLog			# EMT 10: OSLog
	.word	EMT_OSLoadProgram		# EMT 11: OSLoadProgram
	.word	EMT_OSUnloadProgram		# EMT 12: OSUnloadProgram
	.word	EMT_OSSpawn			# EMT 13: OSSpawn
	.word	EMT_OSJoin			# EMT 14: OSJoin
	.word	EMT_OSTerminate			# EMT 15: OSTerminate
	.word	EMT_DSOpen			# EMT 16: DSOpen
	.word	EMT_DSRead			# EMT 17: DSRead
	.word	EMT_DSSeek			# EMT 18: DSSeek
	.word	EMT_DSClose			# EMT 19: DSClose
