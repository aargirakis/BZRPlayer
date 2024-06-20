
****************************************************************************
	SECTION	PlayAY_ZXAYAMADrs000000,CODE
ProgStart:
lbC000000:	moveq	#0,d0
		rts

		dc.l	lbC000000
		dc.b	'AMAD'
		dc.b	2
		dc.b	0
		dc.b	1
		dc.b	11
lbL000010:	dc.l	0
lbL000014:	dc.l	0
lbL000018:	dc.l	0
		dc.w	0
		dc.w	0
		dc.w	InitSound-*
		dc.w	0
		dc.w	Interrupt-*
		dc.w	0
		dc.b	0
		dc.b	0
		dc.b	'Amadeus 1.0',0
		dc.b	'(C) 1987 Frantisek Fuka - Fuxoft',0
		dc.b	'(C) 1992-1994 Patrik Rak - Raxoft',0,0

*-----------------------------------------------------------------------------*
InitSound:	lea	14(a0),a1
		moveq	#0,d0
		move.w	(a0)+,d0
		suba.l	d0,a1
		move.l	a1,lbL0000D6
		move.b	(a0)+,lbB0005C6		; BSS
		moveq	#0,d0
		move.b	(a0)+,d0
		mulu.w	(a0)+,d0
		add.w	(a0)+,d0
		lea	lbL0005C2,a1			; BSS
		move.w	d0,(a1)+
		move.w	(a0)+,(a1)
		move.l	lbL000014(pc),a1
		move.l	(a0)+,(a1)
		move.l	a0,a1
		bra.s	lbC0000DA
	
*-----------------------------------------------------------------------------*
Interrupt:	bsr.l	lbC000136
		moveq	#0,d0
		lea	lbL0005C2,a0		; BSS
		tst.w	(a0)
		beq.s	lbC0000C2
		subq.w	#1,(a0)+
		bne.s	lbC0000C2
		move.w	(a0),d0
lbC0000C2:	rts

lbC0000C4:	moveq	#0,d7
		move.b	(a1)+,d6
		move.b	(a1)+,d7
		lsl.w	#8,d7
		move.b	d6,d7
		add.l	lbL0000D6,d7
		rts

lbL0000D6:	dc.l	0

lbC0000DA:	lea	lbL0003E8,a6		; BSS
		bsr.s	lbC0000C4
		move.l	d7,(a6)
		bsr.s	lbC0000C4
		move.l	d7,$1E(a6)
		bsr.s	lbC0000C4
		move.l	d7,$3C(a6)
		move.w	#$108,d0
		move.w	d0,4(a6)
		move.w	d0,$22(a6)
		move.w	d0,$40(a6)
		lea	$DA(a6),a1
		move.l	a1,-12(a6)
		lea	$80(a1),a1
		move.l	a1,-8(a6)
		lea	$80(a1),a1
		move.l	a1,-4(a6)
		clr.w	$16(a6)
		clr.w	$34(a6)
		clr.w	$52(a6)
		clr.l	$18(a6)
		clr.l	$36(a6)
		clr.l	$54(a6)
		sf	-14(a6)
		rts
	
lbC000136:	lea	lbL0003E8,a6		; BSS
		lea	(a6),a5
		move.l	-12(a6),a4
		moveq	#1,d0
		bsr.s	lbC000198
		move.l	a4,-12(a6)
		lea	$1E(a6),a5
		move.l	-8(a6),a4
		moveq	#2,d0
		bsr.s	lbC000198
		move.l	a4,-8(a6)
		lea	$3C(a6),a5
		move.l	-4(a6),a4
		moveq	#3,d0
		bsr.s	lbC000198
		move.l	a4,-4(a6)
		move.b	$41(a6),d0
		add.b	d0,d0
		or.b	$23(a6),d0
		add.b	d0,d0
		or.b	5(a6),d0
		moveq	#7,d5
		bsr.s	lbC000192
		moveq	#6,d5
		lea	-$1C(a6),a1
		move.l	lbL000010,a3
lbC00018A:	move.w	(a1)+,(a3)+
		dbra	d5,lbC00018A
		rts

lbC000192:	move.b	d0,-$1C(a6,d5.w)
		rts

lbC000198:	move.b	d0,-13(a6)
		subq.b	#1,4(a5)
		beq.l	lbC0002A0
lbC0001A4:	subq.b	#1,10(a5)
		bne.s	lbC0001E0
		move.l	6(a5),a1
lbC0001AE:	move.b	(a1)+,d0
		cmp.b	#$80,d0
		bne.s	lbC0001BE
		bsr.l	lbC0000C4
		move.l	d7,a1
		bra.s	lbC0001AE

lbC0001BE:	cmp.b	#$1E,d0
		bcs.s	lbC0001D4
		subi.b	#$32,d0
		move.b	d0,11(a5)
		move.b	#1,10(a5)
		bra.s	lbC0001DC

lbC0001D4:	move.b	d0,11(a5)
		move.b	(a1)+,10(a5)
lbC0001DC:	move.l	a1,6(a5)
lbC0001E0:	move.w	12(a5),d0
		beq.s	lbC000260
		btst	#2,$16(a5)
		bne.s	lbC000260
		move.l	$12(a5),d0
		beq.s	lbC000260
		move.l	d0,a1
lbC0001F6:	moveq	#0,d0
		move.b	(a1)+,d0
		move.l	a1,$12(a5)
		cmp.b	#$80,d0
		bne.s	lbC00020C
		bsr.l	lbC0000C4
		move.l	d7,a1
		bra.s	lbC0001F6

lbC00020C:	cmp.b	#$82,d0
		bne.s	lbC00021A
		bset	#3,$16(a5)
		bra.s	lbC0001F6

lbC00021A:	cmp.b	#$83,d0
		bne.s	lbC000228
		bclr	#3,$16(a5)
		bra.s	lbC0001F6

lbC000228:	cmp.b	#$84,d0
		bne.s	lbC000236
		eor.b	#9,5(a5)
		bra.s	lbC0001F6

lbC000236:	btst	#3,$16(a5)
		beq.s	lbC000254
		add.b	$1C(a5),d0
		move.b	d0,$1C(a5)
		add.b	d0,d0
		move.l	lbL000018(pc),a3
		move.w	0(a3,d0.w),12(a5)
		bra.s	lbC000260

lbC000254:	ext.w	d0
		add.w	d0,d0
		add.w	d0,d0
		add.w	d0,d0
		add.w	d0,12(a5)
lbC000260:	move.b	-14(a6),d0
		and.b	$1DE(a6),d0
		moveq	#6,d5
		bsr.l	lbC000192
		bclr	#2,$16(a5)
		moveq	#7,d5
		add.b	-13(a6),d5
		move.w	12(a5),d0
		beq.s	lbC000284
		move.b	11(a5),d0
lbC000284:	bsr.l	lbC000192
		subq.b	#8,d5
		add.b	d5,d5
		move.b	12(a5),d0
		bsr.l	lbC000192
		addq.b	#1,d5
		move.b	13(a5),d0
		bsr.l	lbC000192
		rts

lbC0002A0:	move.l	(a5),a1
lbC0002A2:	moveq	#0,d0
		move.b	(a1)+,d0
		move.l	a1,(a5)
		btst	#7,d0
		bne.s	lbC000310
		moveq	#0,d5
		tst.b	d0
		beq.s	lbC0002CE
		add.b	$17(a5),d0
		move.b	d0,$1C(a5)
		bclr	#3,$16(a5)
		add.b	d0,d0
		move.l	lbL000018,a3
		move.w	0(a3,d0.w),d5
lbC0002CE:	move.b	(a1)+,4(a5)
		move.l	a1,(a5)
		move.w	d5,12(a5)
		move.l	$18(a5),$12(a5)
		bset	#2,$16(a5)
		btst	#1,$16(a5)
		bne.l	lbC0001A4
		btst	#0,$16(a5)
		beq.s	lbC0002FC
		bset	#1,$16(a5)
lbC0002FC:	move.l	14(a5),a3
		move.b	(a3)+,11(a5)
		move.b	(a3)+,10(a5)
		move.l	a3,6(a5)
		bra.l	lbC000260

lbC000310:	lea	lbC0002A2(pc),a0
		add.b	d0,d0
		lea	lbW00031E(pc,d0.w),a3
		add.w	(a3),a3
		jmp	(a3)

lbW00031E:	dc.w	lbC000340-*
		dc.w	lbC000348-*
		dc.w	lbC000352-*
		dc.w	lbC00035A-*
		dc.w	lbC000368-*
		dc.w	lbC00036E-*
		dc.w	lbC000388-*
		dc.w	lbC000374-*
		dc.w	lbC00037E-*
		dc.w	lbC000384-*
		dc.w	lbC000392-*
		dc.w	lbC00039A-*
		dc.w	lbC0003A8-*
		dc.w	lbC0003AC-*
		dc.w	lbC0003B4-*
		dc.w	lbC0003BC-*
		dc.w	lbC0003C4-*

lbC000340:	bsr.l	lbC0000C4
		move.l	d7,a1
		jmp	(a0)

lbC000348:	bsr.l	lbC0000C4
		move.l	a1,-(a4)
		move.l	d7,a1
		jmp	(a0)

lbC000352:	move.b	(a1)+,-(a4)
		subq.l	#1,a4
		move.l	a1,-(a4)
		jmp	(a0)

lbC00035A:	subq.b	#1,5(a4)
		beq.s	lbC000364
		move.l	(a4),a1
		jmp	(a0)

lbC000364:	addq.l	#6,a4
		jmp	(a0)

lbC000368:	move.b	(a1)+,-14(a6)
		jmp	(a0)

lbC00036E:	move.b	(a1)+,5(a5)
		jmp	(a0)

lbC000374:	bsr.l	lbC0000C4
		move.l	d7,14(a5)
		jmp	(a0)

lbC00037E:	move.b	(a1)+,$17(a5)
		jmp	(a0)

lbC000384:	move.l	(a4)+,a1
		jmp	(a0)

lbC000388:	bsr.l	lbC0000C4
		move.l	d7,$18(a5)
		jmp	(a0)

lbC000392:	bset	#0,$16(a5)
		bra.s	lbC0003A0

lbC00039A:	bclr	#0,$16(a5)
lbC0003A0:	bclr	#1,$16(a5)
		jmp	(a0)

lbC0003A8:	addq.l	#2,a1
		jmp	(a0)

lbC0003AC:	move.b	(a1)+,d0
		add.b	d0,-14(a6)
		jmp	(a0)

lbC0003B4:	move.b	(a1)+,d0
		add.b	d0,$17(a5)
		jmp	(a0)

lbC0003BC:	move.b	$17(a5),d0
		move.w	d0,-(a4)
		jmp	(a0)

lbC0003C4:	move.w	(a4)+,d0
		move.b	d0,$17(a5)
		jmp	(a0)


	SECTION	PlayAY_ZXAYAMADrs0003CC,BSS
		ds.l	7
lbL0003E8:	ds.l	$76
		ds.w	1
lbL0005C2:	ds.l	1
lbB0005C6:	ds.b	2
