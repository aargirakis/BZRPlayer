	*****************************************************
	****      Quartet ST replayer for EaglePlayer    ****
	****         all adaptions by Wanted Team,	 ****
	****      DeliTracker (?) compatible version	 ****
	*****************************************************

* note: seems this impl is still garbage - see "Speedball 2": the pitch 
* of the played samples is too fast, i.e. the player DOES NOT correctly
* adjust for songs that were designed for 50Hz rather than 200Hz
	
	incdir	"dh2:include/"
	include 'misc/Eagleplayer.i'

	SECTION	Player,CODE

	PLAYERHEADER Tags

	dc.b	'$VER: Quartet ST player module V1.2 (29 Aug 2009)',0
	even
Tags
	dc.l	DTP_PlayerVersion,3
	dc.l	EP_PlayerVersion,9
	dc.l	DTP_PlayerName,PlayerName
	dc.l	DTP_RequestDTVersion,DELIVERSION
	dc.l	DTP_Creator,Creator
	dc.l	DTP_Check2,Check2
	dc.l	DTP_Interrupt,Interrupt
	dc.l	DTP_InitPlayer,InitPlayer
	dc.l	DTP_EndPlayer,EndPlayer
	dc.l	DTP_InitSound,InitSound
	dc.l	DTP_EndSound,EndSound
	dc.l	EP_Get_ModuleInfo,GetInfos
	dc.l	DTP_ExtLoad,ExtLoad
	dc.l	DTP_Volume,SetVolume
	dc.l	DTP_Balance,SetBalance
	dc.l	EP_Voices,SetVoices
	dc.l	EP_StructInit,StructInit
	dc.l	EP_GetPositionNr,GetPosition
	dc.l	EP_SampleInit,SampleInit
	dc.l	EP_Flags,EPB_Volume!EPB_Balance!EPB_ModuleInfo!EPB_Voices!EPB_SampleInfo!EPB_Songend!EPB_Analyzer!EPB_Packable!EPB_Restart!EPB_LoadFast
	dc.l	TAG_DONE

PlayerName
	dc.b	'Quartet ST',0
Creator
	dc.b	'(c) 1989-90 by Rob Povey & Steve',10
	dc.b	'Wetherill, adapted by Wanted Team',0
Prefix
	dc.b	'QTS.',0
SampleName
	dc.b	'SMP.set',0
SMP
	dc.b	'SMP.',0
	even
ModulePtr
	dc.l	0
EagleBase
	dc.l	0
Speed
	dc.w	0
SamplesPtr
	dc.l	0
Timer
	dc.w	0
Bit2Flag
	dc.b	0
AmigaFlag
	dc.b	0
Base
	dc.l	0
Songend
	dc.l	'WTWT'
RightVolume
	dc.w	64
LeftVolume
	dc.w	64
Voice1
	dc.w	-1
Voice2
	dc.w	-1
Voice3
	dc.w	-1
Voice4
	dc.w	-1
StructAdr
	ds.b	UPS_SizeOF

***************************************************************************
******************************* EP_SampleInit *****************************
***************************************************************************

SampleInit
	moveq	#EPR_NotEnoughMem,D7
	lea	EPG_SampleInfoStructure(A5),A3
	move.l	SamplesPtr(PC),D0
	beq.b	return
	move.l	D0,A2

	lea	lbW0255C0(PC),A1
	moveq	#19,D5
Normal
	jsr	ENPP_AllocSampleStruct(A5)
	move.l	D0,(A3)
	beq.b	return
	move.l	D0,A3

	move.l	(A1)+,D1
	beq.b	NoSamp
	move.l	D1,A2
	move.l	A2,EPS_Adr(A3)			; sample address
	moveq	#0,D1
	move.w	-4(A2),D1
	move.l	D1,EPS_Length(A3)		; sample length
	move.l	#64,EPS_Volume(A3)
	tst.b	Bit2Flag
	beq.b	NoName
	lea	-124(A2),A2
	move.l	A2,EPS_SampleName(A3)		; sample name
	move.w	#8,EPS_MaxNameLen(A3)
NoName
	move.w	#USITY_RAW,EPS_Type(A3)
	move.w	#USIB_Playable!USIB_Saveable!USIB_8BIT,EPS_Flags(A3)
NoSamp
	dbf	D5,Normal

	moveq	#0,D7
return
	move.l	D7,D0
	rts

***************************************************************************
******************************* DTP_ExtLoad *******************************
***************************************************************************

ExtLoad
	move.l	dtg_PathArrayPtr(A5),A0
	clr.b	(A0)
	move.l	dtg_CopyDir(A5),A0
	jsr	(A0)
	bsr.s	CopyName2
	move.l	dtg_LoadFile(A5),A0
	jsr	(A0)
	tst.l	D0
	beq.b	ExtLoadOK
	move.l	dtg_PathArrayPtr(A5),A0
	clr.b	(A0)
	move.l	dtg_CopyDir(A5),A0
	jsr	(A0)
	bsr.b	CopyName
	move.l	dtg_LoadFile(A5),A0
	jmp	(A0)

ExtLoadOK
	rts

CopyName
	movea.l	dtg_PathArrayPtr(A5),A0
loop1
	tst.b	(A0)+
	bne.s	loop1
	subq.l	#1,A0
	lea	SampleName(PC),A3
smp2
	move.b	(A3)+,(A0)+
	bne.s	smp2
	rts

CopyName2
	move.l	dtg_PathArrayPtr(A5),A0
loop
	tst.b	(A0)+
	bne.s	loop
	subq.l	#1,A0
	move.l	A0,A3
	move.l	dtg_FileArrayPtr(A5),A1
smp
	move.b	(A1)+,(A0)+
	bne.s	smp

	cmpi.b	#'Q',(A3)
	beq.b	Q_OK
	cmpi.b	#'q',(A3)
	bne.s	ExtError
Q_OK
	cmpi.b	#'T',1(A3)
	beq.b	T_OK
	cmpi.b	#'t',1(A3)
	bne.s	ExtError
T_OK
	cmpi.b	#'S',2(A3)
	beq.b	S_OK
	cmpi.b	#'s',2(A3)
	bne.s	ExtError
S_OK
	cmpi.b	#'.',3(A3)
	bne.s	ExtError

	move.b	#'S',(A3)+
	move.b	#'M',(A3)+
	move.b	#'P',(A3)

	bra.b	ExtOK
ExtError
	clr.b	-2(A0)
ExtOK
	clr.b	-1(A0)
	rts

***************************************************************************
********************************* EP_GetPosNr *****************************
***************************************************************************

GetPosition
	move.l	Base(PC),A0
	move.l	$26(A0),D1
	sub.l	$22(A0),D1
	divu.w	#192,D1
	moveq	#0,D0
	move.w	D1,D0
	rts

***************************************************************************
************************ DTP_Volume DTP_Balance ***************************
***************************************************************************

SetVolume
SetBalance
	move.w	dtg_SndLBal(A5),D0
	mulu.w	dtg_SndVol(A5),D0
	lsr.w	#6,D0

	move.w	D0,LeftVolume

	move.w	dtg_SndRBal(A5),D0
	mulu.w	dtg_SndVol(A5),D0
	lsr.w	#6,D0

	move.w	D0,RightVolume
	moveq	#0,D0
	rts

*------------------------------- Set Two -------------------------------*

SetTwo
	move.l	A0,-(A7)
	lea	StructAdr+UPS_Voice1Adr(PC),A0
	cmp.l	#$DFF0A0,$2E(A4)
	beq.s	.SetVoice
	lea	StructAdr+UPS_Voice2Adr(PC),A0
	cmp.l	#$DFF0B0,$2E(A4)
	beq.s	.SetVoice
	lea	StructAdr+UPS_Voice3Adr(PC),A0
	cmp.l	#$DFF0C0,$2E(A4)
	beq.s	.SetVoice
	lea	StructAdr+UPS_Voice4Adr(PC),A0
.SetVoice
	move.l	A1,(A0)
	move.w	14(A4),D0
	lsr.w	#1,D0
	move.w	D0,UPS_Voice1Len(A0)
	move.l	(A7)+,A0
	rts

*------------------------------- Set Per -------------------------------*

SetPer
	move.l	A0,-(A7)
	lea	StructAdr+UPS_Voice1Per(PC),A0
	cmp.l	#$DFF0A0,$2E(A4)
	beq.s	.SetVoice
	lea	StructAdr+UPS_Voice2Per(PC),A0
	cmp.l	#$DFF0B0,$2E(A4)
	beq.s	.SetVoice
	lea	StructAdr+UPS_Voice3Per(pc),A0
	cmp.l	#$DFF0C0,$2E(A4)
	beq.s	.SetVoice
	lea	StructAdr+UPS_Voice4Per(pc),A0
.SetVoice
	move.l	$16(A4),D0
	bsr.w	lbC00DE66
	move.w	D0,(A0)
	move.l	(A7)+,A0
	rts

***************************************************************************
****************************** EP_Voices  *********************************
***************************************************************************

SetVoices
	lea	Voice1(PC),A0
	lea	StructAdr(PC),A1
	move.w	#$FFFF,D1
	move.w	D1,(A0)+			Voice1=0 setzen
	btst	#0,D0
	bne.s	.NoVoice1
	clr.w	-2(A0)
	clr.w	$DFF0A8
	clr.w	UPS_Voice1Vol(A1)
.NoVoice1
	move.w	D1,(A0)+			Voice2=0 setzen
	btst	#1,D0
	bne.s	.NoVoice2
	clr.w	-2(A0)
	clr.w	$DFF0B8
	clr.w	UPS_Voice2Vol(A1)
.NoVoice2
	move.w	D1,(A0)+			Voice3=0 setzen
	btst	#2,D0
	bne.s	.NoVoice3
	clr.w	-2(A0)
	clr.w	$DFF0C8
	clr.w	UPS_Voice3Vol(A1)
.NoVoice3
	move.w	D1,(A0)+			Voice4=0 setzen
	btst	#3,D0
	bne.s	.NoVoice4
	clr.w	-2(A0)
	clr.w	$DFF0D8
	clr.w	UPS_Voice4Vol(A1)
.NoVoice4
	move.w	D0,UPS_DMACon(A1)
	moveq	#0,D0
	rts

***************************************************************************
******************************* EP_StructInit *****************************
***************************************************************************

StructInit
	lea	StructAdr(PC),A0
	rts

***************************************************************************
******************************* DTP_Check2 ********************************
***************************************************************************

Check2
	move.l	dtg_ChkData(A5),A0
	move.w	(A0),D1
	beq.b	Fault
	cmp.w	#$10,D1
	bhi.b	Fault
	cmp.b	#4,7(A0)
	bne.b	Fault
	cmp.b	#4,6(A0)
	bhi.b	Fault
	tst.l	8(A0)
	bne.b	Fault
	cmp.w	#'WT',12(A0)
	beq.b	Skippy
NoSpec
	tst.l	12(A0)
	bne.b	Fault
Skippy
	cmp.l	#$4C,24(A0)
	bhi.b	Fault
	move.l	24(A0),D1
	and.w	#3,D1
	bne.b	Fault
	cmp.w	#$0056,16(A0)
	beq.b	Oki
Fault
	moveq	#-1,D0
	rts
Oki
	moveq	#0,D0
	rts

***************************************************************************
***************************** EP_Get_ModuleInfo ***************************
***************************************************************************

GetInfos
	lea	InfoBuffer(PC),A0
	rts

LoadSize	=	4
Length		=	12
SamplesSize	=	20
SongSize	=	28
Samples		=	36
CalcSize	=	44

InfoBuffer
	dc.l	MI_LoadSize,0		;4
	dc.l	MI_Length,0		;12
	dc.l	MI_SamplesSize,0	;20
	dc.l	MI_Songsize,0		;28
	dc.l	MI_Samples,0		;36
	dc.l	MI_Calcsize,0		;44
	dc.l	MI_MaxSamples,20
	dc.l	MI_Prefix,Prefix
	dc.l	0

***************************************************************************
***************************** DTP_Intterrupt ******************************
***************************************************************************

Interrupt	
	movem.l	D0-D7/A0-A6,-(SP)

	lea	StructAdr(PC),A0
	st	UPS_Enabled(A0)
	clr.w	UPS_Voice1Per(A0)
	clr.w	UPS_Voice2Per(A0)
	clr.w	UPS_Voice3Per(A0)
	clr.w	UPS_Voice4Per(A0)
	move.w	#UPSB_Adr!UPSB_Len!UPSB_Per!UPSB_Vol,UPS_Flags(A0)

	bsr.w	Play

	lea	StructAdr(PC),A0
	clr.w	UPS_Enabled(A0)

	movem.l	(SP)+,D0-D7/A0-A6
	rts

SongEndTest
	movem.l	A1/A5,-(A7)
	lea	Songend(PC),A1
	cmp.l	#$DFF0A0,$2E(A4)
	bne.b	test1
	clr.b	(A1)
	bra.b	test
test1
	cmp.l	#$DFF0B0,$2E(A4)
	bne.b	test2
	clr.b	1(A1)
	bra.b	test
test2
	cmp.l	#$DFF0C0,$2E(A4)
	bne.b	test3
	clr.b	2(A1)
	bra.b	test
test3
	cmp.l	#$DFF0D0,$2E(A4)
	bne.b	test
	clr.b	3(A1)
test
	tst.l	(A1)
	bne.b	SkipEnd
	move.l	#'WTWT',(A1)
	move.l	EagleBase(PC),A5
	move.l	dtg_SongEnd(A5),A1
	jsr	(A1)
SkipEnd
	movem.l	(A7)+,A1/A5
	rts

***************************************************************************
***************************** DTP_InitPlayer ******************************
***************************************************************************

InitPlayer
	moveq	#0,D0
	movea.l	dtg_GetListData(A5),A0
	jsr	(A0)

	lea	ModulePtr(PC),A6
	move.l	A0,(A6)+			; module buffer
	move.l	A5,(A6)+			; EagleBase
	lea	InfoBuffer(PC),A4
	move.l	D0,LoadSize(A4)
	lea	16(A0),A1
	subq.l	#8,D0
	subq.l	#8,D0

	moveq	#12,D2
	moveq	#3,D1
FindLast
	add.l	D2,A1
	sub.l	D2,D0
	bmi.w	Short
	cmp.w	#$0046,(A1)
	bne.b	FindLast
	dbf	D1,FindLast
	sub.l	D2,D0
	bmi.w	Short
	add.l	D2,A1
	sub.l	A0,A1
	move.l	A1,SongSize(A4)
	move.l	A1,CalcSize(A4)
	move.w	14(A0),(A6)+			; Speed	

	moveq	#1,D0
	movea.l	dtg_GetListData(A5),A0
	jsr	(A0)

	move.l	A0,(A6)+			; SamplesPtr
	clr.w	(A6)+				; Timer
	add.l	D0,LoadSize(A4)
	moveq	#0,D4
	move.b	1(A0),D4
	subq.w	#1,D4
	move.l	D4,Samples(A4)
	clr.w	(A6)				; Bit2Flag+AmigaFlag
	cmp.l	#'2BIT',222(A0)
	bne.b	NoBit
	st	(A6)
NoBit
	cmp.l	#83230,D0
	blt.b	NoAmiga
	cmp.l	#'HfR0',760(A0)			; Projectyle Amiga samples
	bne.b	NoAmiga
	cmp.l	#'MP7E',13588(A0)
	bne.b	NoAmiga
	st	1(A6)
NoAmiga
	move.l	ModulePtr(PC),A1
	cmp.w	#'WT',12(A1)
	bne.b	NoOoh
	st	1(A6)
NoOoh
	add.l	D0,A0				; end of file!!!
	bsr.w	InitSamples
	tst.l	D4
	bne.b	Corrupt
	sub.l	SamplesPtr(PC),D5
	lea	InfoBuffer(PC),A4
	move.l	D5,SamplesSize(A4)
	add.l	D5,CalcSize(A4)

	movea.l	dtg_AudioAlloc(A5),A0
	jmp	(A0)

Short
	moveq	#EPR_ModuleTooShort,D0
	rts

Corrupt
	moveq	#EPR_CorruptModule,D0
	rts

***************************************************************************
***************************** DTP_EndPlayer *******************************
***************************************************************************

EndPlayer
	movea.l	dtg_AudioFree(A5),A0
	jmp	(A0)

***************************************************************************
***************************** DTP_InitSound *******************************
***************************************************************************

InitSound
	lea	StructAdr(PC),A0
	lea	UPS_SizeOF(A0),A1
ClearUPS
	clr.w	(A0)+
	cmp.l	A0,A1
	bne.b	ClearUPS

	move.w	Timer(PC),D0
	bne.b	TimerOK
	move.w	dtg_Timer(A5),D0
	move.w	Speed(PC),D1
	beq.b	NoSpeed
	mulu.w	#50,D0				; * default 50 Hz
	divu.w	D1,D0				; new speed value
	bra.b	SetTimer
NoSpeed
	addq.w	#2,D0
	lsr.w	#2,D0				; 200 Hz
SetTimer
	lea	Timer(PC),A0
	move.w	D0,(A0)
TimerOK
	move.w	D0,dtg_Timer(A5)

	lea	Songend(PC),A0
	move.l	#'WTWT',(A0)

	move.l	ModulePtr(PC),lbW02CE56
	bra.w	InitSong

***************************************************************************
***************************** DTP_EndSound ********************************
***************************************************************************

EndSound
	lea	$DFF000,A0
	move.w	#15,$96(A0)
	moveq	#0,D0
	move.w	D0,$A8(A0)
	move.w	D0,$B8(A0)
	move.w	D0,$C8(A0)
	move.w	D0,$D8(A0)
	rts

***************************************************************************
***************************** Quartet ST player ***************************
***************************************************************************

; Player from "Projectyle" (c) 1990 by Eldritch The Cat/Electronic Arts

;	MOVEM.L	D0/A0,-(SP)
;	LEA	$DFF000,A0
;	MOVE.W	#15,$96(A0)
;	MOVEQ	#0,D0
;	MOVE.W	D0,$A8(A0)
;	MOVE.W	D0,$B8(A0)
;	MOVE.W	D0,$C8(A0)
;	MOVE.W	D0,$D8(A0)
;	MOVEM.L	(SP)+,D0/A0
;	RTS

lbC00D94A	LEA	lbW025610(pc),A4
	MOVEA.L	$22(A4),A3
	BSR.S	lbC00D97C
	LEA	lbW025642(pc),A4
	MOVEA.L	$22(A4),A3
	BSR.S	lbC00D97C
	LEA	lbW025674(pc),A4
	MOVEA.L	$22(A4),A3
	BSR.S	lbC00D97C
	LEA	lbW0256A6(pc),A4
	MOVEA.L	$22(A4),A3
	BSR.S	lbC00D97C
	RTS

lbC00D97C	MOVEA.L	A3,A0
	MOVEQ	#0,D0
	BRA.S	lbC00D98E

lbC00D982	CMPI.W	#$50,(A0)
	BNE.S	lbC00D98A
	ADDQ.W	#1,D0
lbC00D98A	bsr	lbC00D9B0
lbC00D98E	CMPI.W	#$46,(A0)
	BNE.S	lbC00D982
	TST.W	D0
	BEQ.S	lbC00D99E
	MOVE.L	A3,$26(A4)
	RTS

lbC00D99E	MOVE.L	#lbW0218EA,$22(A4)
	MOVE.L	#lbW0218EA,$26(A4)
	RTS

lbC00D9B0	CMPI.W	#$46,(A0)
	BEQ.S	lbC00D9BA
	ADDA.W	#12,A0
lbC00D9BA	RTS

lbC00D9BC	LEA	lbW025610(pc),A0
	LEA	lbL025930(pc),A1
	LEA	$DFF0A0,A2
	MOVEQ	#1,D0
	MOVE.W	#$80,D1
	MOVEQ	#3,D7
lbC00D9D6	CLR.B	0(A0)
	MOVE.W	D0,2(A0)
	MOVE.W	D1,4(A0)
	MOVE.L	#lbW02190C,10(A0)
	MOVE.L	#lbW02190C,$10(A0)
	MOVE.W	#1,8(A0)
	MOVE.W	#2,14(A0)
	MOVE.W	#2,$14(A0)
	CLR.L	$16(A0)
	CLR.L	$1A(A0)
	CLR.L	$1E(A0)
	MOVE.W	#$52,6(A0)
	MOVE.L	A1,$2A(A0)

	move.l	A1,$32(A0)			; stack base

	MOVE.L	A2,$2E(A0)
	ADD.W	D0,D0
	ADD.W	D1,D1
	LEA	$32+4(A0),A0			; extended
	LEA	$258(A1),A1
	LEA	$10(A2),A2
	DBRA	D7,lbC00D9D6
	RTS

Play
;	BTST	#0,lbB021936
;	beq	lbC00DB42
;	BTST	#1,lbB021936
;	bne	lbC00DB42
;	MOVEM.L	D0-D6/A0-A4,-(SP)
;	CMPI.W	#2,lbW0219A8			; volume flag
;	BEQ.S	lbC00DAD0
;	MOVE.W	lbW0255BE,D0
;	CMPI.W	#3,lbW0219A8
;	BEQ.S	lbC00DAA2
;	CMPI.W	#5,lbW0219A8
;	BEQ.S	lbC00DAD0
;	SUBQ.W	#1,D0
;	MOVE.W	D0,lbW0255BE

	move.w	LeftVolume(PC),D0
	and.w	Voice1(PC),D0
	lea	StructAdr+UPS_Voice1Vol(PC),A0
	move.w	D0,(A0)

	MOVE.W	D0,$DFF0A8			; volume

	move.w	RightVolume(PC),D0
	and.w	Voice2(PC),D0
	lea	StructAdr+UPS_Voice2Vol(PC),A0
	move.w	D0,(A0)

	MOVE.W	D0,$DFF0B8

	move.w	RightVolume(PC),D0
	and.w	Voice3(PC),D0
	lea	StructAdr+UPS_Voice3Vol(PC),A0
	move.w	D0,(A0)

	MOVE.W	D0,$DFF0C8

	move.w	LeftVolume(PC),D0
	and.w	Voice4(PC),D0
	lea	StructAdr+UPS_Voice4Vol(PC),A0
	move.w	D0,(A0)

	MOVE.W	D0,$DFF0D8
;	TST.W	D0
;	BNE.S	lbC00DAD0
;	MOVE.W	#5,lbW0219A8
;	BRA.S	lbC00DAD0

;lbC00DAA2	ADDQ.W	#1,D0
;	MOVE.W	D0,lbW0255BE
;	MOVE.W	D0,$DFF0A8			; volume
;	MOVE.W	D0,$DFF0B8
;	MOVE.W	D0,$DFF0C8
;	MOVE.W	D0,$DFF0D8
;	CMPI.W	#$40,D0
;	BNE.S	lbC00DAD0
;	MOVE.W	#2,lbW0219A8
lbC00DAD0	LEA	$DFF000,A0
	LEA	lbW025610(pc),A4
	bsr	lbC00DC88
	LEA	lbW025610(pc),A4
	bsr	lbC00DB44
	LEA	lbW025642(pc),A4
	bsr	lbC00DB44
	LEA	lbW025674(pc),A4
	bsr	lbC00DB44
	LEA	lbW0256A6(pc),A4
	bsr	lbC00DB44
;	MOVE.W	lbW021912(pc),D0
;lbC00DB0E	MOVE.W	D0,-(SP)
	LEA	lbW025610(pc),A4
	bsr	lbC00DB86
	LEA	lbW025642(pc),A4
	bsr	lbC00DB86
	LEA	lbW025674(pc),A4
	bsr	lbC00DB86
	LEA	lbW0256A6(pc),A4
	bsr	lbC00DB86
;	MOVE.W	(SP)+,D0
;	DBRA	D0,lbC00DB0E
;	MOVEM.L	(SP)+,D0-D6/A0-A4
lbC00DB42	RTS

lbC00DB44	CMPI.W	#$53,6(A4)
	BNE.S	lbC00DB84
	MOVE.L	$1E(A4),D0
	ADD.L	$16(A4),D0
	MOVE.L	D0,$16(A4)

	bsr.w	SetPer

	TST.L	$1E(A4)
	BMI.S	lbC00DB72
	CMP.L	$1A(A4),D0
	BLT.S	lbC00DB84
	MOVE.W	#$50,6(A4)
	MOVE.L	$1A(A4),$16(A4)

	bsr.w	SetPer

	RTS

lbC00DB72	CMP.L	$1A(A4),D0
	BGT.S	lbC00DB84
	MOVE.W	#$50,6(A4)
	MOVE.L	$1A(A4),$16(A4)

	bsr.w	SetPer

lbC00DB84	RTS

lbC00DB86	SUBQ.W	#1,8(A4)
	bne	lbC00DBD4
	MOVEA.L	$26(A4),A0
	MOVEA.L	$2A(A4),A2
lbC00DB96	MOVE.W	(A0)+,D0
	MOVE.W	D0,6(A4)
	CMP.W	#$50,D0
	BEQ.S	lbC00DBD6			; OK
	CMP.W	#$52,D0
	BEQ.S	lbC00DBF0			; OK
	CMP.W	#$46,D0
	BEQ.S	lbC00DC06			; diff, now OK
	CMP.W	#$56,D0
	BEQ.S	lbC00DC0C			; OK
	CMP.W	#$6C,D0
	beq	lbC00DC44			; OK
	CMP.W	#$4C,D0
	beq	lbC00DC52			; diff, now OK
	CMP.W	#$53,D0
	beq	lbC00DC74			; OK
lbC00DBCC	MOVE.L	A2,$2A(A4)
	MOVE.L	A0,$26(A4)
lbC00DBD4	RTS

lbC00DBD6	MOVE.W	2(A4),$DFF096		; DMA off
	MOVE.W	(A0)+,8(A4)
	MOVE.L	(A0)+,$16(A4)

	bsr.w	SetPer

	ADDQ.L	#4,A0
	BSET	#0,0(A4)
	BRA.S	lbC00DBCC

lbC00DBF0	MOVE.W	2(A4),$DFF096		; DMA off
	MOVE.W	(A0)+,8(A4)
	ADDQ.L	#8,A0
	BSET	#0,0(A4)
	BRA.S	lbC00DBCC

lbC00DC06	MOVEA.L	$22(A4),A0

	move.l	$32(A4),$2A(A4)			; return stack base
	bsr.w	SongEndTest

	BRA.w	lbC00DB96

lbC00DC0C	ADDQ.W	#6,A0
	MOVEA.L	(A0)+,A1
	MOVE.L	A1,10(A4)
	MOVE.W	-4(A1),14(A4)

	move.b	AmigaFlag(PC),D0
	bne.b	Ami1
	
;	lsr.w	#1,14(A4)
	
	move.l	D0,-(A7)		; quick hack to replace above - not handled by vasm
	moveq   #0,d0
	move.w	14(A4),d0
	lsr.w	#1,d0
	move.w	d0,14(A4)	
	move.l	(A7)+,D0
	
Ami1
	bsr.w	SetTwo

	MOVE.W	-8(A1),D0
	BPL.S	lbC00DC32
	MOVE.W	#2,$14(A4)
	MOVE.L	#lbW02190C,$10(A4)
	bra	lbC00DB96

lbC00DC32
	move.b	AmigaFlag(PC),D7
	bne.b	Ami2
	lsr.w	#1,D0
Ami2
	MOVE.W	D0,$14(A4)
	ADDA.W	14(A4),A1
	SUBA.W	D0,A1
	MOVE.L	A1,$10(A4)
	bra	lbC00DB96

lbC00DC44	ADDA.L	#10,A0
	MOVE.L	A0,-(A2)
	CLR.W	-(A2)
	bra	lbC00DB96

lbC00DC52	ADDQ.L	#6,A0
	MOVE.W	(A0)+,D2
	ADDQ.L	#2,A0
	MOVE.W	(A2)+,D1
	TST.W	D1

	bmi.b	Update

	BNE.S	lbC00DC62
Back
	MOVE.W	D2,D1
	ADDQ.W	#1,D1
lbC00DC62	SUBQ.W	#1,D1
	BEQ.S	lbC00DC6E
	MOVEA.L	(A2),A0
	MOVE.W	D1,-(A2)
	bra	lbC00DB96

Update
	move.l	$32(A4),-(A2)
	clr.w	-(A2)
	bra.b	Back

lbC00DC6E	ADDQ.L	#4,A2
	bra	lbC00DB96

lbC00DC74	MOVE.W	(A0)+,8(A4)
	MOVE.L	(A0)+,$1A(A4)
;	MOVE.L	(A0)+,D0
;	ASL.L	#2,D0
;	MOVE.L	D0,$1E(A4)

	move.l	(A0)+,$1E(A4)

	bra	lbC00DBCC

lbC00DC88	MOVEQ	#3,D2
lbC00DC8A	MOVEA.L	$2E(A4),A1
	BCLR	#0,0(A4)
	BEQ.S	lbC00DCD8
	CMPI.W	#$52,6(A4)
	BNE.S	lbC00DCAE
	MOVE.W	#1,4(A1)			; length
	MOVE.L	#lbW02190C,0(A1)		; address
	BRA.S	lbC00DD00

lbC00DCAE	MOVE.L	10(A4),0(A1)		; address
	MOVE.W	14(A4),D0
	LSR.W	#1,D0
	MOVE.W	D0,4(A1)			; length
	BSET	#1,0(A4)
	MOVE.W	4(A4),$9C(A0)			; INTREQ
	MOVE.W	2(A4),D0
	BSET	#15,D0
	MOVE.W	D0,$96(A0)			; DMA on
	BRA.S	lbC00DD00

lbC00DCD8	MOVE.W	$1E(A0),D0		; INTREQR
	AND.W	4(A4),D0
	BEQ.S	lbC00DD00
	MOVE.W	4(A4),$9C(A0)			; INTREQ
	BCLR	#1,0(A4)
	BEQ.S	lbC00DD00
	MOVE.L	$10(A4),0(A1)			; repeat address
	MOVE.W	$14(A4),D0
	LSR.W	#1,D0
	MOVE.W	D0,4(A1)			; repeat length
lbC00DD00	MOVE.L	$16(A4),D0
	bsr	lbC00DE66
;	SWAP	D0
	MOVE.W	D0,6(A1)			; period
	LEA	$32+4(A4),A4			; extended
	DBRA	D2,lbC00DC8A
	RTS

InitSong
	bsr	lbC00D9BC
	bsr	lbC00DDB8
	bsr	lbC00D94A
;	CLR.W	lbW0255BE
;	MOVE.W	#3,lbW0219A8
;	BSET	#0,lbB021936
	RTS

;lbC00DD3C	CMPI.W	#$46,(A0)
;	BEQ.S	lbC00DD70
;	CMPI.W	#$56,(A0)
;	bne	lbC00DD64
;	MOVE.L	8(A0),D0
;	CLR.L	D1
;	LEA	lbW0255C0,A1
;lbC00DD56	CMP.L	0(A1,D1.L),D0
;	BEQ.S	lbC00DD60
;	ADDQ.L	#4,D1
;	BRA.S	lbC00DD56

;lbC00DD60	MOVE.L	D1,8(A0)
;lbC00DD64	CMPI.W	#$46,(A0)
;	BEQ.S	lbC00DD6E
;	ADDA.W	#12,A0
;lbC00DD6E	BRA.S	lbC00DD3C

;lbC00DD70	RTS

lbC00DD72	CMPI.W	#$46,(A0)
	BEQ.S	lbC00DDB6
	CMPI.W	#$56,(A0)
	bne	lbC00DDAA
	MOVE.L	8(A0),D0
	LEA	lbW0255C0(pc),A1
	MOVE.L	0(A1,D0.L),8(A0)		; install sample ptr
;	MOVE.L	#lbL031E56,D0
;	ADDI.L	#8,D0
;	CMP.L	8(A0),D0
;	bne	lbC00DDAA
;	MOVE.L	0(A1),8(A0)

	bne.b	lbC00DDAA
	move.l	#lbW02190C,8(A0)		; set empty sample ptr

lbC00DDAA	CMPI.W	#$46,(A0)
	BEQ.S	lbC00DDB4
	ADDA.W	#12,A0
lbC00DDB4	BRA.S	lbC00DD72

lbC00DDB6	RTS

lbC00DDB8
;	LEA	lbW02CE56,A0

	move.l	lbW02CE56(PC),A0
	cmp.l	#$4C,24(A0)
	bgt.w	InitDone

	ADDA.W	#$10,A0
	MOVE.L	A0,lbW025632
	BSR.w	lbC00DE0A
	MOVE.L	A0,lbW025664

	move.l	A0,D1
	sub.l	lbW025632(PC),D1
	move.l	D1,D0
	lea	lbW025610(PC),A1

	BSR.w	lbC00DE0A
	MOVE.L	A0,lbW025696

	move.l	A0,D1
	sub.l	lbW025664(PC),D1
	cmp.l	D1,D0
	bge.b	NoHi1
	move.l	D1,D0
	lea	lbW025642(PC),A1
NoHi1

	BSR.S	lbC00DE0A
	MOVE.L	A0,lbW0256C8

	move.l	A0,D1
	sub.l	lbW025696(PC),D1
	cmp.l	D1,D0
	bge.b	NoHi2
	move.l	D1,D0
	lea	lbW025674(PC),A1
NoHi2
	move.l	ModulePtr(PC),D1
	lea	InfoBuffer(PC),A2
	add.l	SongSize(A2),D1
	sub.l	lbW0256C8(PC),D1
	cmp.l	D1,D0
	bge.b	NoHi3
	move.l	D1,D0
	lea	lbW0256A6(PC),A1
NoHi3
	divu.w	#192,D0
	addq.w	#1,D0
	move.w	D0,Length+2(A2)
	lea	Base(PC),A0
	move.l	A1,(A0)

	MOVEA.L	lbW025632(pc),A0
	bsr	lbC00DD72
	MOVEA.L	lbW025664(pc),A0
	bsr	lbC00DD72
	MOVEA.L	lbW025696(pc),A0
	bsr	lbC00DD72
	MOVEA.L	lbW0256C8(pc),A0
	bsr	lbC00DD72
InitDone
	RTS

lbC00DE0A	CMPI.W	#$46,(A0)
	BEQ.S	lbC00DE1C
;	CMPI.W	#$46,(A0)
;	BEQ.S	lbC00DE1A
	ADDA.W	#12,A0
lbC00DE1A	BRA.S	lbC00DE0A

lbC00DE1C	ADDA.W	#12,A0
	RTS

InitSamples
;	LEA	lbL031E56,A5
;	ADDA.W	#$8E,A5

	move.l	SamplesPtr(PC),A2
	lea	$8E(A2),A3

	LEA	lbW0255C0(pc),A4
	MOVEQ	#$13,D0
lbC00DE34
;	MOVE.L	(A5)+,D1
;	ADDI.L	#lbL031E56,D1
;	ADDI.L	#8,D1

	move.l	(A3)+,D1
	beq.b	NoSamp1
	bmi.w	Corek
	btst	#0,D1
	bmi.w	Corek
	lea	(A2,D1.L),A1
	cmp.l	A1,A0
	blt.b	OutOfRange
	tst.b	Bit2Flag
	beq.b	SampOK
	move.l	-120(A1),D7
	and.l	#$00FFFFFF,D7
	cmp.l	#$00424954,D7
	beq.b	SampOK
OutOfRange
	moveq	#0,D1
	bra.b	NoSamp1
SampOK
	add.l	A2,D1
	addq.l	#8,D1
NoSamp1
	MOVE.L	D1,(A4)+
	DBRA	D0,lbC00DE34
;	LEA	lbW0255C0,A0

	lea	lbW0255C0(PC),A3
	moveq	#0,D5

	MOVEQ	#$13,D7
lbC00DE50
;	MOVEA.L	(A0)+,A1

	move.l	(A3)+,D1
	beq.b	NoConv
	move.l	D1,A1
	cmp.w	#'WT',-2(A1)
	beq.b	NoConv

	MOVE.W	-4(A1),D6

	moveq	#0,D3
	bclr	#0,D6				; even length
	beq.b	Even
	moveq	#1,D3
Even
	lea	(A1,D6.W),A2
	cmp.l	A2,A0
	blt.b	NoConv				; Out Of Range
	move.w	#'WT',-2(A1)			; sample init done flag
	subq.l	#1,D4

	BRA.S	lbC00DE5C

lbC00DE58	SUBI.B	#$80,(A1)+
lbC00DE5C	DBRA	D6,lbC00DE58

	tst.b	1(A6)
	bne.b	NoDown
	move.l	D1,A1
	move.l	A1,A4
DownSampling
	move.b	(A1)+,D6
	ext.w	D6
	move.b	(A1)+,D1
	ext.w	D1
	add.w	D1,D6
	lsr.w	#1,D6
	move.b	D6,(A4)+
	cmp.l	A1,A2
	bne.b	DownSampling
NoDown
	add.l	D3,A1
	cmp.l	A1,D5
	bge.b	NoConv
	move.l	A1,D5
NoConv
	DBRA	D7,lbC00DE50
	RTS

Corek
	moveq	#1,D4
	rts

lbC00DE66
;	TST.L	D0
;	BEQ.S	lbC00DE96

	lsr.l	#3,D0
	beq.b	lbC00DE96			; for safety

;	MOVEM.L	D1/D2,-(SP)
;	LSR.L	#3,D0
	MOVE.L	#$369E99+1,D1
	DIVU.W	D0,D1

	move.w	D1,D0

;	MOVE.W	D1,D2
;	SWAP	D2
;	CLR.W	D1
;	DIVU.W	D0,D1
;	MOVE.W	D1,D2
;	MOVE.L	D2,D0
;	MOVEM.L	(SP)+,D1/D2
;	CMP.L	lbL02190E,D0
;	BGE.S	lbC00DE96
;	MOVE.L	D0,lbL02190E
lbC00DE96

	addq.w	#1,D0

	RTS

lbW0218EA	dc.w	$52
	dc.w	$2710
	dc.w	0
	dc.w	0
	dc.w	0
	dc.w	0

	dc.w	$46
	dc.w	0
	dc.w	0
	dc.w	0
	dc.w	0
	dc.w	0

;	dc.w	0
;	dc.w	0
;	dc.w	0
;	dc.w	0
;	dc.w	0
;lbW02190C	dc.w	0
;lbL02190E	dc.l	$7C0000			; minimum period
;lbW021912	dc.w	3			; 50 Hz counter
;lbB021936	dc.w	0
;lbW0219A8	dc.w	0			; volume flag
;lbW0255BE	dc.w	0


;	Section	Stack,Code_BSS

lbW0255C0
	ds.b	20*4

lbW025610
	ds.b	34
lbW025632
	ds.b	16+4

lbW025642
	ds.b	34
lbW025664
	ds.b	16+4

lbW025674
	ds.b	34
lbW025696
	ds.b	16+4

lbW0256A6
	ds.b	34
lbW0256C8
	ds.b	16+4

lbW02CE56
	ds.b	4

	ds.b	600
lbL025930
	ds.b	600
	ds.b	600
	ds.b	600

	Section	Buffy,Data_C

	dc.w	-1
	dc.w	-1
	dc.w	4
	dc.w	0
lbW02190C
	ds.b	4

;lbL031E56
