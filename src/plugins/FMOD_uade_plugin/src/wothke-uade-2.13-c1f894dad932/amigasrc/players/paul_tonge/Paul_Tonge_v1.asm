	*****************************************************
	****    Paul Tonge replayer for EaglePlayer	 ****
	****        all adaptions by Wanted Team,	 ****
	****      DeliTracker incompatible version	 ****
	*****************************************************

	incdir	"dh2:include/"
	include "misc/EaglePlayer.i"
	include "hardware/intbits.i"
	include "exec/exec_lib.i"
	include	"dos/dos_lib.i"

	SECTION	Player,CODE

	PLAYERHEADER Tags

	dc.b	'$VER: Paul Tonge player module V1.0 (18 Dec 2011)',0
	even
Tags
	dc.l	DTP_PlayerVersion,1
	dc.l	EP_PlayerVersion,9
	dc.l	DTP_RequestDTVersion,'WT'
	dc.l	DTP_PlayerName,PlayerName
	dc.l	DTP_Creator,Creator
	dc.l	DTP_Check2,Check2
	dc.l	DTP_ExtLoad,ExtLoad
	dc.l	DTP_Interrupt,Interrupt
	dc.l	DTP_InitPlayer,InitPlayer
	dc.l	DTP_EndPlayer,EndPlayer
	dc.l	DTP_InitSound,InitSound
	dc.l	DTP_EndSound,EndSound
	dc.l	EP_Get_ModuleInfo,GetInfos
	dc.l	EP_SampleInit,SampleInit
	dc.l	EP_ModuleChange,ModuleChange
	dc.l	DTP_Volume,SetVolume
	dc.l	DTP_Balance,SetBalance
	dc.l	EP_Voices,SetVoices
	dc.l	EP_StructInit,StructInit
	dc.l	EP_GetPositionNr,GetPosition
	dc.l	EP_Flags,EPB_Volume!EPB_Balance!EPB_ModuleInfo!EPB_Voices!EPB_SampleInfo!EPB_Songend!EPB_Analyzer
	dc.l	TAG_DONE
PlayerName
	dc.b	'Paul Tonge',0
Creator
	dc.b	'(c) 1993 by Paul Tonge,',10
	dc.b	'adapted by Wanted Team',0
Prefix
	dc.b	'PAT.',0
SampleName
	dc.b	'SMP.set',0
ReplayName
	dc.b	'WantedTeam.bin',0
	even
ModulePtr
	dc.l	0
SamplesPtr
	dc.l	0
ReplayPtr
	dc.l	0
EagleBase
	dc.l	0
Change
	dc.w	0
Songend
	dc.l	'WTWT'
SongEndTemp
	dc.l	0
CurrentPos
	dc.w	0
Voice1
	dc.w	-1
Voice2
	dc.w	-1
Voice3
	dc.w	-1
Voice4
	dc.w	-1
RightVolume
	dc.w	64
LeftVolume
	dc.w	64
StructAdr
	ds.b	UPS_SizeOF

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
	lea	SampleName(PC),A3
	bsr.b	CopyName
	move.l	dtg_LoadFile(A5),A0
	jsr	(A0)
	tst.l	D0
	beq.b	ExtLoadOK
	rts
ExtLoadOK
	move.l	dtg_PathArrayPtr(A5),A0
	clr.b	(A0)
	move.l	dtg_CopyDir(A5),A0
	jsr	(A0)
	lea	ReplayName(PC),A3
	bsr.b	CopyName
	move.l	dtg_LoadFile(A5),A0
	jsr	(A0)
	rts

CopyName
	movea.l	dtg_PathArrayPtr(A5),A0
loop1
	tst.b	(A0)+
	bne.s	loop1
	subq.l	#1,A0
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

	cmpi.b	#'P',(A3)
	beq.b	P_OK
	cmpi.b	#'p',(A3)
	bne.s	ExtError
P_OK
	cmpi.b	#'A',1(A3)
	beq.b	A_OK
	cmpi.b	#'a',1(A3)
	bne.s	ExtError
A_OK
	cmpi.b	#'T',2(A3)
	beq.b	T_OK
	cmpi.b	#'t',2(A3)
	bne.s	ExtError
T_OK
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
	moveq	#0,D0
	move.w	CurrentPos(PC),D0
	rts

***************************************************************************
**************************** EP_ModuleChange ******************************
***************************************************************************

ModuleChange
	move.w	Change(PC),D0
	bne.s	NoChange
	move.l	IntAddress(PC),EPG_ARG1(A5)
	lea	PatchTable(PC),A1
	move.l	A1,EPG_ARG3(A5)
	move.l	#2600,D1
	move.l	D1,EPG_ARG2(A5)
	moveq	#-2,D0
	move.l	D0,EPG_ARG5(A5)		
	moveq	#1,D0
	move.l	D0,EPG_ARG4(A5)			;Search-Modus
	moveq	#5,D0
	move.l	D0,EPG_ARGN(A5)
	move.l	EPG_ModuleChange(A5),A0
	jsr	(A0)
NoChange
	move.w	#1,Change
	moveq	#0,D0
	rts

***************************************************************************
*************************** DTP_Volume DTP_Balance ************************
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

ChangeVolume
	move.l	D1,-(A7)
	move.l	A6,D1
	cmp.w	#$F0A0,D1
	beq.s	Left1
	cmp.w	#$F0B0,D1
	beq.s	Right1
	cmp.w	#$F0C0,D1
	beq.s	Right2
	cmp.w	#$F0D0,D1
	bne.s	Exit2
Left2
	mulu.w	LeftVolume(PC),D0
	and.w	Voice4(PC),D0
	bra.s	Ex
Left1
	mulu.w	LeftVolume(PC),D0
	and.w	Voice1(PC),D0
	bra.s	Ex

Right1
	mulu.w	RightVolume(PC),D0
	and.w	Voice2(PC),D0
	bra.s	Ex
Right2
	mulu.w	RightVolume(PC),D0
	and.w	Voice3(PC),D0
Ex
	lsr.w	#6,D0
	move.w	D0,8(A6)
Exit2
	move.l	(A7)+,D1
	rts

*-------------------------------- Set Vol -------------------------------*

SetVol
	move.l	A0,-(SP)
	lea	StructAdr+UPS_Voice1Vol(PC),A0
	cmp.l	#$DFF0A0,A6
	beq.b	.SetVoice
	lea	StructAdr+UPS_Voice2Vol(PC),A0
	cmp.l	#$DFF0B0,A6
	beq.b	.SetVoice
	lea	StructAdr+UPS_Voice3Vol(PC),A0
	cmp.l	#$DFF0C0,A6
	beq.b	.SetVoice
	lea	StructAdr+UPS_Voice4Vol(PC),A0
.SetVoice
	move.w	D0,(A0)
	move.l	(SP)+,A0
	rts

*-------------------------------- Set Two -------------------------------*

SetTwo
	move.l	A2,-(SP)
	lea	StructAdr+UPS_Voice1Adr(PC),A2
	cmp.l	#$DFF0A0,A3
	beq.b	.SetVoice
	lea	StructAdr+UPS_Voice2Adr(PC),A2
	cmp.l	#$DFF0B0,A3
	beq.b	.SetVoice
	lea	StructAdr+UPS_Voice3Adr(PC),A2
	cmp.l	#$DFF0C0,A3
	beq.b	.SetVoice
	lea	StructAdr+UPS_Voice4Adr(PC),A2
.SetVoice
	move.l	$46(A0),(A2)
	move.w	$40(A0),UPS_Voice1Len(A2)
	move.l	(SP)+,A2
	rts

*-------------------------------- Set Per -------------------------------*

SetPer
	move.l	A1,-(SP)
	lea	StructAdr+UPS_Voice1Vol(PC),A1
	cmp.l	#$DFF0A0,A6
	beq.b	.SetVoice
	lea	StructAdr+UPS_Voice2Vol(PC),A1
	cmp.l	#$DFF0B0,A6
	beq.b	.SetVoice
	lea	StructAdr+UPS_Voice3Vol(PC),A1
	cmp.l	#$DFF0C0,A6
	beq.b	.SetVoice
	lea	StructAdr+UPS_Voice4Vol(PC),A1
.SetVoice
	move.w	(A0),(A1)
	move.l	(SP)+,A1
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
******************************* EP_SampleInit *****************************
***************************************************************************

SampleInit
	moveq	#EPR_NotEnoughMem,D7
	lea	EPG_SampleInfoStructure(A5),A3
	move.l	SamplesPtr(PC),D0
	beq.b	return
	move.l	D0,A2
	move.l	A2,D2

	move.l	InfoBuffer+Samples(PC),D5
	subq.l	#1,D5
hop
	jsr	ENPP_AllocSampleStruct(A5)
	move.l	D0,(A3)
	beq.b	return
	move.l	D0,A3

	move.l	(A2)+,D1
	add.l	D2,D1
	moveq	#0,D0
	move.w	(A2)+,D0
	add.l	D0,D0
	move.l	D1,EPS_Adr(A3)			; sample address
	move.l	D0,EPS_Length(A3)		; sample length
	move.l	#64,EPS_Volume(A3)
	move.w	#USITY_RAW,EPS_Type(A3)
	move.w	#USIB_Playable!USIB_Saveable!USIB_8BIT,EPS_Flags(A3)
	addq.l	#6,A2
	dbf	D5,hop

	moveq	#0,D7
return
	move.l	D7,D0
	rts

***************************************************************************
******************************* DTP_Check2 ********************************
***************************************************************************

Check2
	movea.l	dtg_ChkData(A5),A0
	moveq	#-1,D0
	move.l	A0,A1
	cmp.w	#$000C,(A0)+
	bne.b	fail
	moveq	#2,D2
next
	move.w	(A0)+,D1
	bmi.b	fail
	beq.b	skip
	btst	#0,D1
	bne.b	fail
	move.w	(A1,D1.W),D1
	ble.b	fail
	cmp.b	#$80,-1(A1,D1.W)
	beq.b	skip
	cmp.b	#$8F,-1(A1,D1.W)
	bne.b	fail
skip
	dbf	D2,next
found
	moveq	#0,D0
fail
	rts

***************************************************************************
***************************** EP_Get_ModuleInfo ***************************
***************************************************************************

GetInfos
	lea	InfoBuffer(PC),A0
	rts

LoadSize	=	4
Samples		=	12
Voices		=	20
Length		=	28

InfoBuffer
	dc.l	MI_LoadSize,0		;4
	dc.l	MI_Samples,0		;12
	dc.l	MI_Voices,0		;20
	dc.l	MI_Length,0		;28
	dc.l	MI_MaxVoices,4
	dc.l	MI_AuthorName,PlayerName
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
	move.l	ReplayPtr(PC),A0
	jsr	(A0)				; play module

	lea	StructAdr(PC),A0
	clr.w	UPS_Enabled(A0)

	movem.l	(SP)+,D0-D7/A0-A6
	rts

SongEnd
	movem.l	A1/A5,-(A7)
	move.l	EagleBase(PC),A5
	move.l	dtg_SongEnd(A5),A1
	jsr	(A1)
	movem.l	(A7)+,A1/A5
	rts

SetAudioVector
	movem.l	D0/A1/A6,-(A7)
	movea.l	4.W,A6
	lea	StructInt(PC),A1
	moveq	#INTB_AUD0,D0
	jsr	SetIntVector(A6)		; XXX SetIntVector was _LVOSetIntVector
	move.l	D0,Channel0
	lea	StructInt(PC),A1
	moveq	#INTB_AUD1,D0
	jsr	SetIntVector(A6)
	move.l	D0,Channel1
	lea	StructInt(PC),A1
	moveq	#INTB_AUD2,D0
	jsr	SetIntVector(A6)
	move.l	D0,Channel2
	lea	StructInt(PC),A1
	moveq	#INTB_AUD3,D0
	jsr	SetIntVector(A6)
	move.l	D0,Channel3
	movem.l	(A7)+,D0/A1/A6
	rts

ClearAudioVector
	movea.l	4.W,A6
	movea.l	Channel0(PC),A1
	moveq	#INTB_AUD0,D0
	jsr	SetIntVector(A6)
	movea.l	Channel1(PC),A1
	moveq	#INTB_AUD1,D0
	jsr	SetIntVector(A6)
	movea.l	Channel2(PC),A1
	moveq	#INTB_AUD2,D0
	jsr	SetIntVector(A6)
	movea.l	Channel3(PC),A1
	moveq	#INTB_AUD3,D0
	jmp	SetIntVector(A6)

Channel0
	dc.l	0
Channel1
	dc.l	0
Channel2
	dc.l	0
Channel3
	dc.l	0
StructInt
	dc.l	0
	dc.l	0
	dc.w	$205
	dc.l	IntName
	dc.l	0
IntAddress
	dc.l	0
IntName
	dc.b	'Paul Tonge Audio Interrupt',0,0
	even

***************************************************************************
***************************** DTP_InitPlayer ******************************
***************************************************************************

InitPlayer
	moveq	#0,D0
	movea.l	dtg_GetListData(A5),A0
	jsr	(A0)
	lea	ModulePtr(PC),A6
	move.l	A0,(A6)+			; module ptr
	lea	InfoBuffer(PC),A4
	move.l	D0,LoadSize(A4)
	lea	SongEndTemp(PC),A1
	clr.l	(A1)
	moveq	#0,D0
	moveq	#3,D1
CheckV
	tst.w	(A0)+
	beq.b	NoV
	addq.l	#1,D0
	st	(A1)
NoV
	addq.l	#1,A1
	dbf	D1,CheckV
	move.l	D0,Voices(A4)
	subq.l	#6,A0
	move.w	(A0)+,D0
	bne.b	Oki
	move.w	(A0)+,D0
	bne.b	Oki
	move.w	(A0)+,D0
	bne.b	Oki
	move.w	4(A0),D0
Oki
	moveq	#12,D1
	sub.l	D1,D0
	lsr.l	#1,D0
	subq.l	#1,D0
	bne.b	NoZero
	moveq	#1,D0
NoZero
	move.l	D0,Length(A4)

	moveq	#1,D0
	movea.l	dtg_GetListData(A5),A0
	jsr	(A0)
	move.l	A0,(A6)+			; sample ptr
	add.l	D0,LoadSize(A4)

	moveq	#0,D0
NextS
	tst.l	(A0)
	beq.b	LastS
	addq.l	#1,D0
	lea	12(A0),A0
	bra.b	NextS
LastS
	move.l	D0,Samples(A4)

	moveq	#2,D0
	movea.l	dtg_GetListData(A5),A0
	jsr	(A0)
	move.l	A0,(A6)+			; replay ptr
	move.l	A0,A2
	move.l	A5,(A6)+			; EagleBase
	add.l	D0,LoadSize(A4)

Find1
	cmp.w	#$48E7,(A0)+
	bne.b	Find1
	subq.l	#2,A0
	lea	IntAddress(PC),A1
	move.l	A0,(A1)
FindRTE
	cmp.w	#$4E73,(A0)+
	bne.b	FindRTE
	subq.l	#2,A0
	addq.w	#2,(A0)+			; RTS now
Find2
	cmp.l	#$220B101A,(A0)
	beq.b	PtrFound1
	addq.l	#2,A0
	bra.b	Find2
PtrFound1
	move.w	-2(A0),D1
	lea	4(A2,D1.W),A1
	move.l	SamplesPtr(PC),(A1)
Find3
	cmp.l	#$26497001,(A0)
	beq.b	PtrFound2
	addq.l	#2,A0
	bra.b	Find3
PtrFound2
	move.w	-2(A0),D1
	lea	4(A2,D1.W),A1
	move.l	ModulePtr(PC),(A1)

	clr.w	(A6)				; Change

	bsr.w	ModuleChange

	movea.l	EPG_ModuleChange(A5),A0
	jmp	(A0)

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
	move.l	ReplayPtr(PC),A0
	clr.l	4(A0)
	move.b	#1,4(A0)
	lea	Songend(PC),A0
	move.l	4(A0),(A0)
	clr.w	8(A0)
	rts

***************************************************************************
***************************** DTP_EndSound ********************************
***************************************************************************

EndSound
	bsr.w	ClearAudioVector
	lea	$DFF000,A0
	move.w	#15,$96(A0)
	moveq	#0,D0
	move.w	D0,$A8(A0)
	move.w	D0,$B8(A0)
	move.w	D0,$C8(A0)
	move.w	D0,$D8(A0)
	rts

	*----------------- PatchTable for Paul Tonge -------------------*

PatchTable
	dc.w	Code0-PatchTable,(Code0End-Code0)/2-1,Patch0-PatchTable
	dc.w	Code1-PatchTable,(Code1End-Code1)/2-1,Patch1-PatchTable
	dc.w	Code2-PatchTable,(Code2End-Code2)/2-1,Patch2-PatchTable
	dc.w	Code3-PatchTable,(Code3End-Code3)/2-1,Patch3-PatchTable
	dc.w	Code4-PatchTable,(Code4End-Code4)/2-1,Patch4-PatchTable
	dc.w	Code5-PatchTable,(Code5End-Code5)/2-1,Patch5-PatchTable
	dc.w	0

; Audio Interrupt patch for Paul Tonge modules

Code0
	MOVE.L	A0,$70.W
	MOVE.W	#$FF,$DFF09E
Code0End
Patch0
	bsr.w	SetAudioVector
	move.w	#$FF,$DFF09E
	rts

; Address/length patch for Paul Tonge modules

Code1
	MOVE.W	$40(A0),4(A3)
Code1End
Patch1
	move.w	$40(A0),4(A3)
	bsr.w	SetTwo
	rts

; Period patch for Paul Tonge modules

Code2
	MOVE.W	0(A0),6(A6)
Code2End
Patch2
	move.w	(A0),6(A6)
	bsr.w	SetPer
	rts

; Volume patch for Paul Tonge modules

Code3
	MOVE.W	0(A2,D0.W),8(A6)
Code3End
Patch3
	move.w	(A2,D0.W),D0
	bsr.w	ChangeVolume
	bsr.w	SetVol
	move.w	D0,8(A6)
	rts

; Restart song patch for Paul Tonge modules

Code4
	MOVE.W	#15,$DFF096
Code4End
Patch4
	move.w	#15,$DFF096
	move.b	#1,(A5)
	clr.w	CurrentPos
	bsr.w	SongEnd
	rts

; SongEnd patch for Paul Tonge modules

Code5
	MOVEA.L	6(A0),A1
	MOVE.W	(A1)+,D0
Code5End
Patch5
	move.l	6(A0),A1
	cmp.l	#$DFF0A0,$4A(A0)
	bne.b	NoPos
	addq.w	#1,CurrentPos
NoPos
	tst.w	(A1)
	bne.b	NoEnd
SongEndTest
	movem.l	A1/A5,-(A7)
	lea	Songend(PC),A1
	cmp.l	#$DFF0A0,$4A(A0)
	bne.b	test1
	clr.b	(A1)
	bra.b	test
test1
	cmp.l	#$DFF0B0,$4A(A0)
	bne.b	test2
	clr.b	1(A1)
	bra.b	test
test2
	cmp.l	#$DFF0C0,$4A(A0)
	bne.b	test3
	clr.b	2(A1)
	bra.b	test
test3
	cmp.l	#$DFF0D0,$4A(A0)
	bne.b	test
	clr.b	3(A1)
test
	tst.l	(A1)+
	bne.b	SkipEnd
	move.l	(A1),-(A1)
	clr.w	8(A1)					; CurrentPos
	move.l	EagleBase(PC),A5
	move.l	dtg_SongEnd(A5),A1
	jsr	(A1)
SkipEnd
	movem.l	(A7)+,A1/A5
NoEnd
	move.w	(A1)+,D0
	rts
