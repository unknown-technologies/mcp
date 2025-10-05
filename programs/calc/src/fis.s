	.even
	.globl	_FAdd
_FAdd:	mov	2(sp),	r0	# a
	mov	4(sp),	r1	# b
	mov	2(r0),	-(sp)
	mov	(r0),	-(sp)
	mov	2(r1),	-(sp)
	mov	(r1),	-(sp)

	fadd	sp

	mov	(sp)+,	(r0)
	mov	(sp)+,	2(r0)
	rts	pc

	.even
	.globl	_FSub
_FSub:	mov	2(sp),	r0	# a
	mov	4(sp),	r1	# b
	mov	2(r0),	-(sp)
	mov	(r0),	-(sp)
	mov	2(r1),	-(sp)
	mov	(r1),	-(sp)

	fsub	sp

	mov	(sp)+,	(r0)
	mov	(sp)+,	2(r0)
	rts	pc

	.even
	.globl	_FMul
_FMul:	mov	2(sp),	r0	# a
	mov	4(sp),	r1	# b
	mov	2(r0),	-(sp)
	mov	(r0),	-(sp)
	mov	2(r1),	-(sp)
	mov	(r1),	-(sp)

	fmul	sp

	mov	(sp)+,	(r0)
	mov	(sp)+,	2(r0)
	rts	pc

	.even
	.globl	_FDiv
_FDiv:	mov	2(sp),	r0	# a
	mov	4(sp),	r1	# b
	mov	2(r0),	-(sp)
	mov	(r0),	-(sp)
	mov	2(r1),	-(sp)
	mov	(r1),	-(sp)

	fdiv	sp

	mov	(sp)+,	(r0)
	mov	(sp)+,	2(r0)
	rts	pc
