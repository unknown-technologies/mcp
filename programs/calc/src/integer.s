	.even
	.globl _IMul10
_IMul10:
	mov	r2,	-(sp)
	mov	r3,	-(sp)
	mov	6(sp),	r2
	mov	(r2),	r0
	mov	2(r2),	r1
	ashc	$1,	r0
	mov	r0,	r2
	mov	r1,	r3
	ashc	$2,	r0
	add	r3,	r1
	adc	r0
	add	r2,	r0
	mov	6(sp),	r2
	mov	r0,	(r2)
	mov	r1,	2(r2)
	mov	(sp)+,	r3
	mov	(sp)+,	r2
	rts	pc

	.even
	.globl	_IAdd
_IAdd:	mov	2(sp),	r0
	add	4(sp),	2(r0)
	adc	(r0)
	rts	pc
