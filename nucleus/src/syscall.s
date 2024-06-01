	.text
	.even
	.globl	_OSSyscallInit
	.globl	___emttbl

_OSSyscallInit:
	mov	r0,	-(sp)
	mov	r1,	-(sp)
	mov	r2,	-(sp)
	mov	$EMT_table, r0	# EMT handlers
	mov	___emttbl,r1	# EMT table
	add	$24,	r1	# first EMT ID
	mov	$17,	r2	# count
1:	mov	(r0)+,	(r1)+
	sob	r2,	1b
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
	.globl	_OSGetDate
	.globl	_OSSetDate
	.globl	___DSOpen
	.globl	___DSGetInfo
	.globl	___DSRead
	.globl	___DSWrite
	.globl	___DSSeek
	.globl	___DSClose
	.globl	_FSList
	.globl	_VOLGetLabel
	.globl	_OSGetLog

EMT_OSGetDate:
	mov	r0,	-(sp)
	jsr	pc,	_OSGetDate
	tst	(sp)+
	rti

EMT_OSSetDate:
	mov	r0,	-(sp)
	jsr	pc,	_OSSetDate
	tst	(sp)+
	rti

EMT_OSLog:
	mov	r3,	-(sp)
	mov	r2,	-(sp)
	mov	r1,	-(sp)
	mov	r0,	-(sp)
	jsr	pc,	_OSLog
	add	$8,	sp
	rti

EMT_OSLoadProgram:
	mov	r0,	-(sp)
	jsr	pc,	_OSLoadProgram
	tst	(sp)+
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
	tst	(sp)+
	rti

EMT_OSTerminate:
	mov	r0,	-(sp)
	jsr	pc,	_OSTerminate
	tst	(sp)+
	rti

EMT_DSOpen:
	mov	r0,	-(sp)
	jsr	pc,	___DSOpen
	tst	(sp)+
	rti

EMT_DSGetInfo:
	mov	r1,	-(sp)
	mov	r0,	-(sp)
	jsr	pc,	___DSGetInfo
	add	$4,	sp
	rti

EMT_DSRead:
	mov	r2,	-(sp)
	mov	r1,	-(sp)
	mov	r0,	-(sp)
	jsr	pc,	___DSRead
	add	$6,	sp
	rti

EMT_DSWrite:
	mov	r2,	-(sp)
	mov	r1,	-(sp)
	mov	r0,	-(sp)
	jsr	pc,	___DSWrite
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
	tst	(sp)+
	rti

EMT_FSList:
	mov	r2,	-(sp)
	mov	r1,	-(sp)
	mov	r0,	-(sp)
	jsr	pc,	_FSList
	add	$6,	sp
	rti

EMT_VOLGetLabel:
	mov	r1,	-(sp)
	mov	r0,	-(sp)
	jsr	pc,	_VOLGetLabel
	add	$4,	sp
	rti

EMT_OSGetLog:
	mov	r1,	-(sp)
	mov	r0,	-(sp)
	jsr	pc,	_OSGetLog
	add	$4,	sp
	rti

	.data
	.even
EMT_table:
	.word	EMT_OSGetDate			# EMT 12: OSGetDate
	.word	EMT_OSSetDate			# EMT 13: OSSetDate
	.word	EMT_OSLog			# EMT 14: OSLog
	.word	EMT_OSLoadProgram		# EMT 15: OSLoadProgram
	.word	EMT_OSUnloadProgram		# EMT 16: OSUnloadProgram
	.word	EMT_OSSpawn			# EMT 17: OSSpawn
	.word	EMT_OSJoin			# EMT 18: OSJoin
	.word	EMT_OSTerminate			# EMT 19: OSTerminate
	.word	EMT_DSOpen			# EMT 20: DSOpen
	.word	EMT_DSGetInfo			# EMT 21: DSGetInfo
	.word	EMT_DSRead			# EMT 22: DSRead
	.word	EMT_DSWrite			# EMT 23: DSWrite
	.word	EMT_DSSeek			# EMT 24: DSSeek
	.word	EMT_DSClose			# EMT 25: DSClose
	.word	EMT_FSList			# EMT 26: FSList
	.word	EMT_VOLGetLabel			# EMT 27: VOLGetLabel
	.word	EMT_OSGetLog			# EMT 28: OSGetLog
