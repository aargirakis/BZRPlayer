	*****************************************************
	****         Archon II - Custom Module,	         ****
	****        all adaptions by Wanted Team	 ****
	*****************************************************

	incdir	"dh2:include/"
	include 'misc/eagleplayer2.01.i'

	SECTION Player,Code_C

Ar_Start
	PLAYERHEADER PlayerTagArray

	dc.b	"$VER: Archon II - Custom Module,",10
	dc.b	'adapted by Don Adan/Wanted Team',0
	even

PlayerTagArray
	dc.l	DTP_CustomPlayer,1
	dc.l	DTP_PlayerName,PlayerName
	dc.l	DTP_Interrupt,Interrupt
	dc.l	DTP_InitPlayer,InitPlayer
	dc.l	DTP_EndPlayer,EndPlayer
	dc.l	DTP_InitSound,InitSound
	dc.l	DTP_EndSound,EndSound
	dc.l	DTP_SubSongRange,SubSongRange
	dc.l	EP_Get_ModuleInfo,Get_ModuleInfo
	dc.l	TAG_DONE
PlayerName
	dc.b	"CustomPlay",0
Text
	dc.b	10
	dc.b	"                 ARCHON II",10
	dc.b	"                   ADEPT",10
	dc.b	10
	dc.b	"     an Electronic Arts production of",10
	dc.b	"            a Free Fall game",10
	dc.b	10
	dc.b	"            by Anne Westfall",10
	dc.b	"             & Jon Freeman",10
	dc.b	"            & Paul Reiche III",10
	dc.b	10
	dc.b	"           adapted & enhanced",10
	dc.b	"            for the Amiga by",10
	dc.b	"              Jon Freeman",10
	dc.b	"             Anne Westfall",10

	dc.b	"     (C) 1984-86 FREE FALL ASSOCIATES",0
	even

EagleBase
	dc.l	0
Timer
	dc.w	0

***************************************************************************
***************************** DTP_SubSongRange ****************************
***************************************************************************

SubSongRange
	moveq	#0,D0
	moveq	#1,D1
	rts

***************************************************************************
***************************** EP_Get_ModuleInfo ***************************
***************************************************************************

Get_ModuleInfo
	lea	InfoBuffer(PC),A0
	rts

InfoBuffer
	dc.l	MI_SpecialInfo,Text
	dc.l	MI_LoadSize,Ar_End-Ar_Start
	dc.l	0

***************************************************************************
***************************** DTP_InitPlayer ******************************
***************************************************************************

InitPlayer
	move.l	A5,EagleBase
	move.l	4.W,lbL00006C

	bsr.w	Init1
	bsr.w	Init2

	moveq	#0,D0
	rts

***************************************************************************
***************************** DTP_EndPlayer *******************************
***************************************************************************

EndPlayer
	bsr.w	EndPlay
	bsr.w	Ending
	moveq	#0,D0
	rts

***************************************************************************
***************************** DTP_Intterrupt ******************************
***************************************************************************

Interrupt
	movem.l	D1-A6,-(SP)

	bsr.w	Play

	movem.l	(SP)+,D1-A6
	moveq	#0,D0
	rts

SongEnd
	movem.l	A1/A5,-(A7)
	move.l	EagleBase(PC),A5
	move.l	dtg_SongEnd(A5),A1
	jsr	(A1)
	movem.l	(A7)+,A1/A5
	rts

***************************************************************************
***************************** DTP_InitSound *******************************
***************************************************************************

InitSound
	move.w	Timer(PC),D0
	bne.b	Done
	move.w	dtg_Timer(A5),D0
	mulu.w	#5,D0
	divu.w	#6,D0			; 60Hz
	lea	Timer(PC),A0
	move.w	D0,(A0)
Done	move.w	D0,dtg_Timer(A5)
	move.w	dtg_SndNum(A5),D0
	bne.b	Second
	bsr.w	lbC014DD8
	bra.b	Skip
Second
	bsr.w	lbC014F98
Skip
	bra.w	StartMusic

***************************************************************************
***************************** DTP_EndSound ********************************
***************************************************************************

EndSound
	rts

***************************************************************************
***************************************************************************
***************************************************************************

; Music from "Archon II" (c) 1986 by Electronic Arts

lbC014DD8	MOVEQ	#$20,D0
	MOVE.L	D0,-(SP)
	PEA	lbL01574E
	JSR	lbC022940
	ADDQ.L	#8,SP
	MOVE.L	#lbW0155F8,lbL01574E
	MOVEQ	#-1,D0
	MOVE.W	D0,lbW015752
	MOVE.L	#lbW01563A,lbL015756
	MOVEQ	#-2,D0
	MOVE.W	D0,lbW01575A
	MOVE.L	#lbW01568E,lbL01575E
	MOVE.W	D0,lbW015762
	MOVE.L	#lbW0156E2,lbL015766
	MOVE.W	#$FFFF,lbW01576A
	MOVEQ	#1,D0
	MOVE.W	D0,lbW0155F8
	LEA	lbL015450,A0
	MOVE.L	A0,lbL0155FA
	MOVE.W	D0,lbW0155FE
	MOVE.L	#lbL015490,lbL015600
	MOVE.W	D0,lbW015604
	MOVE.L	#lbB0154C7,lbL015606
	MOVE.W	D0,lbW01560A
	MOVE.L	A0,lbL01560C
	MOVE.W	D0,lbW015610
	MOVE.L	#lbL0155C0,lbL015612
	MOVEQ	#0,D0
	MOVE.W	D0,lbW015616
	MOVEQ	#1,D0
	MOVE.W	D0,lbW01563A
	LEA	lbL015462,A0
	MOVE.L	A0,lbL01563C
	MOVE.W	#4,lbW015640
	MOVE.L	#lbL0154F6,lbL015642
	MOVE.W	#2,lbW015646
	MOVE.L	#lbL015508,lbL015648
	MOVE.W	D0,lbW01564C
	MOVE.L	#lbL01552A,lbL01564E
	MOVE.W	D0,lbW015652
	MOVE.L	A0,lbL015654
	MOVE.W	D0,lbW015658
	MOVE.L	#lbB0155CB,lbL01565A
	MOVEQ	#0,D0
	MOVE.W	D0,lbL01565E
	MOVEQ	#1,D0
	MOVE.W	D0,lbW01568E
	LEA	lbL015472,A0
	MOVE.L	A0,lbL015690
	MOVE.W	D0,lbW015694
	MOVE.L	#lbL01555A,lbL015696
	MOVE.W	D0,lbW01569A
	MOVE.L	#lbB015591,lbL01569C
	MOVE.W	D0,lbW0156A0
	MOVE.L	A0,lbL0156A2
	MOVE.W	D0,lbW0156A6
	MOVE.L	#lbL0155D6,lbL0156A8
	MOVEQ	#0,D0
	MOVE.W	D0,lbL0156AC
	MOVEQ	#1,D0
	MOVE.W	D0,lbW0156E2
	LEA	lbL01547E,A0
	MOVE.L	A0,lbL0156E4
	MOVE.W	#11,lbW0156E8
	LEA	lbB01544D,A0
	MOVE.L	A0,lbL0156EA
	MOVE.W	D0,lbW0156EE
	MOVE.L	#lbL01547E,lbL0156F0
	MOVE.W	D0,lbW0156F4
	MOVE.L	A0,lbL0156F6
	CLR.W	lbL0156FA
	RTS

lbC014F98	MOVEQ	#$20,D0
	MOVE.L	D0,-(SP)
	PEA	lbL01574E
	JSR	lbC022940
	ADDQ.L	#8,SP
	MOVE.L	#lbW015706,lbL01574E
	MOVEQ	#-1,D0
	MOVE.W	D0,lbW015752
	MOVE.L	#lbW015718,lbL015756
	MOVEQ	#-2,D0
	MOVE.W	D0,lbW01575A
	MOVE.L	#lbW01572A,lbL01575E
	MOVE.W	D0,lbW015762
	MOVE.L	#lbW01573C,lbL015766
	MOVE.W	#$FFFF,lbW01576A
	MOVEQ	#1,D0
	MOVE.W	D0,lbW015706
	MOVE.L	#lbL015450,lbL015708
	MOVE.W	D0,lbW01570C
	MOVE.L	#lbL0155C0,lbL01570E
	MOVEQ	#0,D0
	MOVE.W	D0,lbW015712
	MOVEQ	#1,D0
	MOVE.W	D0,lbW015718
	MOVE.L	#lbL015462,lbL01571A
	MOVE.W	D0,lbW01571E
	MOVE.L	#lbB0155CB,lbL015720
	MOVEQ	#0,D0
	MOVE.W	D0,lbW015724
	MOVEQ	#1,D0
	MOVE.W	D0,lbW01572A
	MOVE.L	#lbL015472,lbL01572C
	MOVE.W	D0,lbW015730
	MOVE.L	#lbL0155D6,lbL015732
	MOVEQ	#0,D0
	MOVE.W	D0,lbW015736
	MOVEQ	#1,D0
	MOVE.W	D0,lbW01573C
	MOVE.L	#lbL01547E,lbL01573E
	MOVE.W	D0,lbW015742
	MOVE.L	#lbB01544D,lbL015744
	CLR.W	lbW015748
	RTS

lbC015098	LINK.W	A6,#-$1A
	MOVEM.L	D2-D4/A2,-(SP)
	CLR.B	-1(A6)
lbC0150A4	MOVE.B	-1(A6),D0
	CMPI.B	#4,D0
	BCC.L	lbC0151F0
	MOVEQ	#0,D1
	MOVE.B	-1(A6),D1
	MOVE.L	D1,$10(SP)
	ASL.L	#2,D1
	MOVEA.L	D1,A0
	ADDA.L	#lbL01576E,A0
	MOVEQ	#0,D1
	MOVE.B	-1(A6),D1
	MOVEQ	#6,D0
	JSR	lbC0241E8
	ASL.L	#2,D0
	MOVEA.L	D0,A1
	ADDA.L	#lbL01577E,A1
	MOVE.L	A1,D0
	MOVE.L	D0,(A0)
	MOVE.L	$10(SP),D0
	ASL.L	#2,D0
	MOVEA.L	D0,A0
	ADDA.L	#lbL01576E,A0
	MOVEA.L	(A0),A1
	MOVE.L	D1,$18(SP)
	ADDQ.L	#1,D1
	MOVE.L	#$1F0000,D0
	MOVE.L	D1,$20(SP)
	JSR	lbC024158
	MOVE.L	D0,(A1)
	MOVE.L	$10(SP),D0
	ASL.L	#2,D0
	MOVEA.L	D0,A0
	ADDA.L	#lbL01576E,A0
	MOVEA.L	(A0),A1
	MOVE.L	#$100000,D0
	MOVE.L	$20(SP),D1
	JSR	lbC024158
	ADDI.L	#$2F0000,D0
	MOVE.L	D0,4(A1)
	MOVE.L	$10(SP),D0
	ASL.L	#2,D0
	MOVEA.L	D0,A0
	MOVE.L	A0,$1C(SP)
	ADDA.L	#lbL01576E,A0
	MOVEA.L	(A0),A1
	ADDQ.L	#8,A1
	MOVE.L	$10(SP),D1
	ASL.L	#2,D1
	MOVEA.L	D1,A0
	MOVE.L	A0,$24(SP)
	ADDA.L	#lbL01576E,A0
	MOVEA.L	(A0),A2
	MOVE.L	#$80000,D2
	SUB.L	4(A2),D2
	MOVE.L	$10(SP),D3
	MOVEQ	#4,D4
	ASL.L	D3,D4
	ADDI.L	#$10,D4
	MOVE.L	D2,D0
	MOVE.L	D4,D1
	JSR	lbC024198
	MOVE.L	D0,(A1)
	ASL.L	#2,D3
	MOVEA.L	D3,A0
	ADDA.L	#lbL01576E,A0
	MOVEA.L	(A0),A1
	ADDQ.L	#8,A1
	MOVE.L	#$80000,4(A1)
	MOVEA.L	$24(SP),A0
	ADDA.L	#lbL01576E,A0
	MOVEA.L	(A0),A1
	ADDA.W	#$10,A1
	MOVEQ	#0,D0
	MOVE.L	D0,(A1)
	MOVEA.L	$24(SP),A0
	ADDA.L	#lbL01576E,A0
	MOVEA.L	(A0),A1
	ADDA.W	#$10,A1
	MOVE.L	D0,4(A1)
	MOVE.L	$10(SP),D0
	ASL.L	#2,D0
	MOVEA.L	D0,A0
	ADDA.L	#lbL0157DE,A0
	MOVEA.L	$1C(SP),A1
	ADDA.L	#lbL01576E,A1
	MOVE.L	(A1),-(SP)
	MOVE.L	(A0),-(SP)
	MOVE.L	$20(SP),-(SP)
	JSR	lbC0232CE
	LEA	12(SP),SP
	ADDQ.B	#1,-1(A6)
	BRA.L	lbC0150A4

lbC0151F0	MOVEM.L	(SP)+,D2-D4/A2
	UNLK	A6
	RTS

;lbC0151F8	LINK.W	A6,#-4
;	MOVEM.L	D6/D7,-(SP)
;	MOVEQ	#4,D6
;lbC015202	CMP.W	14(A6),D6
;	BGE.S	lbC015244
;	MOVE.L	D6,D0
;	EXT.L	D0
;	MOVEA.L	8(A6),A0
;	ADDA.L	D0,A0
;	MOVE.B	(A0),D7
;	ADDQ.L	#1,A0
;	MOVEA.L	8(A6),A1
;	ADDA.L	D0,A1
;	SUBQ.L	#1,D0
;	MOVE.L	D0,-(SP)
;	MOVE.L	A1,-(SP)
;	MOVE.L	A0,-(SP)
;	JSR	lbC022980
;	LEA	12(SP),SP
;	MOVE.L	D6,D0
;	EXT.L	D0
;	ASL.L	#1,D0
;	SUBQ.L	#1,D0
;	MOVEA.L	8(A6),A0
;	ADDA.L	D0,A0
;	MOVE.B	D7,(A0)
;	MOVE.L	D6,D0
;	ASL.W	#1,D6
;	BRA.S	lbC015202

;lbC015244	MOVEM.L	(SP)+,D6/D7
;	UNLK	A6
;	RTS

Init2
	LINK.W	A6,#-12
	MOVEQ	#0,D0
	MOVE.L	D0,-4(A6)
	MOVE.L	D0,-8(A6)
lbC01525A	MOVE.L	-4(A6),D0
	ASL.L	#2,D0
	MOVEA.L	D0,A0
	ADDA.L	#lbL0157DE,A0
	MOVE.L	-8(A6),D1
	ASL.L	#1,D1
	MOVEA.L	lbL0155E6,A1
	ADDA.L	D1,A1
	MOVE.L	A1,(A0)
	MOVEA.L	D0,A0
	ADDA.L	#lbL01543A,A0
	MOVE.L	(A0),D0
	MOVEQ	#2,D1
	JSR	lbC024198
	ADD.L	D0,-8(A6)
	MOVE.L	-4(A6),D0
	ADDQ.L	#1,D0
	MOVE.L	D0,-4(A6)
	CMPI.L	#4,D0
	BLT.S	lbC01525A
;	TST.W	10(A6)
;	BNE.S	lbC0152AC
;	BSR.L	lbC014DD8
;	BRA.S	lbC0152B0

;lbC0152AC	BSR.L	lbC014F98
lbC0152B0	JSR	lbC022FC8
	BSR.L	lbC015098
	MOVE.L	#$4B00,-(SP)
	JSR	lbC022F86
	ADDQ.L	#4,SP
	JSR	lbC0232C4
	MOVEQ	#2,D1
	MOVEA.L	D0,A0
	MOVE.L	D1,$30(A0)
	MOVE.L	#$F4,-(SP)
	MOVE.L	D0,-12(A6)
	JSR	lbC0232B4
	ADDQ.L	#4,SP

	unlk	A6
	rts

StartMusic
	CLR.L	-(SP)
	MOVEQ	#1,D0
	MOVE.L	D0,-(SP)
	MOVE.L	lbL0155E2,-(SP)
	JSR	lbC0231DC
	LEA	12(SP),SP
;	UNLK	A6
	RTS

;	JSR	lbC023F48
;	MOVE.L	#$80,-(SP)
;	MOVE.L	lbL0157E6,-(SP)
;	BSR.L	lbC0151F8
;	ADDQ.L	#8,SP
;	RTS

EndPlay
	JSR	lbC022F6A
	JSR	lbC023188
	RTS

;	LINK.W	A6,#-8
;	MOVEM.L	A2,-(SP)
;	MOVEQ	#0,D0
;	MOVE.L	D0,-4(A6)
;	MOVE.L	D0,-8(A6)
;lbC01533C	MOVE.L	-8(A6),D0
;	ASL.L	#2,D0
;	MOVEA.L	D0,A0
;	ADDA.L	#lbL01543A,A0
;	MOVE.L	(A0),D0
;	ADD.L	D0,-4(A6)
;	MOVE.L	-8(A6),D0
;	ADDQ.L	#1,D0
;	MOVE.L	D0,-8(A6)
;	CMPI.L	#4,D0
;	BLT.S	lbC01533C
;	MOVEQ	#2,D0
;	MOVE.L	D0,-(SP)
;	MOVE.L	-4(A6),-(SP)
;	JSR	$C00200
;	ADDQ.L	#8,SP
;	MOVE.L	D0,lbL0155E6
;	MOVEQ	#0,D0
;	MOVE.L	D0,-8(A6)
;	MOVE.L	D0,-4(A6)
;lbC015382	MOVE.L	-8(A6),D0
;	ASL.L	#2,D0
;	MOVEA.L	D0,A0
;	ADDA.L	#lbL01542A,A0
;	MOVE.L	-4(A6),D1
;	ASL.L	#1,D1
;	MOVEA.L	lbL0155E6,A1
;	ADDA.L	D1,A1
;	MOVEA.L	D0,A2
;	ADDA.L	#lbL01543A,A2
;	CLR.L	-(SP)
;	MOVE.L	(A2),-(SP)
;	MOVE.L	A1,-(SP)
;	MOVE.L	(A0),-(SP)
;	JSR	lbC01FFE0
;	LEA	$10(SP),SP
;	MOVE.L	-8(A6),D0
;	ASL.L	#2,D0
;	MOVEA.L	D0,A0
;	ADDA.L	#lbL01543A,A0
;	MOVE.L	(A0),D0
;	MOVEQ	#2,D1
;	JSR	lbC024198
;	ADD.L	D0,-4(A6)
;	MOVE.L	-8(A6),D0
;	ADDQ.L	#1,D0
;	MOVE.L	D0,-8(A6)
;	CMPI.L	#4,D0
;	BLT.S	lbC015382
;	MOVEM.L	(SP)+,A2
;	UNLK	A6
;	RTS

;	dc.w	0
;	dc.w	0
;	dc.w	$1FC
;	dc.w	0
;	dc.w	$AAA9
GUIT8AOCT.MSG	dc.b	'GUIT-8A.OCT',0
FUZZ4XXOCT.MSG	dc.b	'FUZZ-4XX.OCT',0
WVIBEXOCT.MSG	dc.b	'WVIBEX.OCT',0
SPACE14OCT.MSG	dc.b	'SPACE1-4.OCT',0,0
lbL01542A	dc.l	GUIT8AOCT.MSG
	dc.l	FUZZ4XXOCT.MSG
	dc.l	WVIBEXOCT.MSG
	dc.l	SPACE14OCT.MSG
lbL01543A	dc.l	$400
	dc.l	$200
	dc.l	$200
	dc.l	$200
	dc.w	$8C7F
	dc.b	$80
lbB01544D	dc.b	$C1
	dc.b	$7F
	dc.b	$80
lbL015450	dc.l	$C52A2B2C
	dc.l	$2D2E2F30
	dc.l	$31323333
	dc.l	$35363738
	dc.w	$3980
lbL015462	dc.l	$C37FC52B
	dc.l	$2C2D2E2F
	dc.l	$30313233
	dc.l	$34353680
lbL015472	dc.l	$C27FC52B
	dc.l	$2C2D2E23
	dc.l	$22212080
lbL01547E	dc.l	$C51E1F20
	dc.l	$211F2021
	dc.l	$231F2425
	dc.l	$262F2E2D
	dc.w	$2C80
lbL015490	dc.l	$823AC43A
	dc.l	$C4373C84
	dc.l	$3BC83AC3
	dc.l	$7F823AC4
	dc.l	$3AC4373C
	dc.l	$843BC83A
	dc.l	$C537393A
	dc.l	$3C823DC4
	dc.l	$3DC4433A
	dc.l	$843EC83E
	dc.l	$C37FC83D
	dc.l	$C53A3C3D
	dc.l	$3FC83EC4
	dc.w	$7F3E
	dc.b	$80
lbB0154C7	dc.b	$C5
	dc.l	$4545C443
	dc.l	$3EC54343
	dc.l	$C4413CC5
	dc.l	$4141C43F
	dc.l	$C43AC53F
	dc.l	$3FC43E3C
	dc.l	$358436C3
	dc.l	$36C43AC5
	dc.l	$37393A3C
	dc.l	$C43DC53A
	dc.l	$3C3D3FC3
	dc.w	$3E80
lbL0154F6	dc.l	$C51F1F26
	dc.l	$262B2B26
	dc.l	$261F1F26
	dc.l	$262B2B26
	dc.w	$2680
lbL015508	dc.l	$C51B1B22
	dc.l	$22272722
	dc.l	$221B1B22
	dc.l	$22272722
	dc.l	$221A1A21
	dc.l	$21262621
	dc.l	$211A1A21
	dc.l	$21262621
	dc.w	$2180
lbL01552A	dc.l	$C51F1F26
	dc.l	$262B2B26
	dc.l	$261D1D24
	dc.l	$24292924
	dc.l	$241B1B22
	dc.l	$22272722
	dc.l	$221A1A21
	dc.l	$21262621
	dc.l	$2118181F
	dc.l	$1F24241B
	dc.l	$1B222227
	dc.l	$27C32680
lbL01555A	dc.l	$822EC42E
	dc.l	$C42B3084
	dc.l	$2FC82EC3
	dc.l	$7F822EC4
	dc.l	$2EC42B30
	dc.l	$842FC82E
	dc.l	$C52B2D2E
	dc.l	$308231C4
	dc.l	$31C4372E
	dc.l	$8432C832
	dc.l	$C37FC831
	dc.l	$C52E3031
	dc.l	$33C832C4
	dc.w	$7F32
	dc.b	$80
lbB015591	dc.b	$C5
	dc.l	$3939C437
	dc.l	$32C53737
	dc.l	$C43530C5
	dc.l	$3535C433
	dc.l	$C42EC533
	dc.l	$33C43230
	dc.l	$29842AC3
	dc.l	$2AC42EC5
	dc.l	$2B2D2E30
	dc.l	$C431C52E
	dc.l	$303133C3
	dc.w	$3280
lbL0155C0	dc.l	$C33AC537
	dc.l	$373737C3
	dc.w	$377F
	dc.b	$80
lbB0155CB	dc.b	$C3
	dc.l	$37C52E2E
	dc.l	$2E2EC32E
	dc.w	$7F80
lbL0155D6	dc.l	$C31FC51F
	dc.l	$1F1F1FC3
	dc.l	$1F7F8000
lbL0155E2	dc.l	lbL01574E
lbL0155E6	dc.l	lbL02AE50
;	dc.l	0
;	dc.l	0
;	dc.l	$2000000
;	dc.w	$AB29
lbW0155F8	dc.w	0
lbL0155FA	dc.l	0
lbW0155FE	dc.w	0
lbL015600	dc.l	0
lbW015604	dc.w	0
lbL015606	dc.l	0
lbW01560A	dc.w	0
lbL01560C	dc.l	0
lbW015610	dc.w	0
lbL015612	dc.l	0
lbW015616	dc.w	0
	dc.w	0
	dc.w	0
	dc.w	0
	dc.w	0
	dc.w	0
	dc.w	0
	dc.w	0
	dc.w	0
	dc.w	0
	dc.w	0
	dc.w	0
	dc.w	0
	dc.w	0
	dc.w	0
	dc.w	0
	dc.w	0
	dc.w	0
lbW01563A	dc.w	0
lbL01563C	dc.l	0
lbW015640	dc.w	0
lbL015642	dc.l	0
lbW015646	dc.w	0
lbL015648	dc.l	0
lbW01564C	dc.w	0
lbL01564E	dc.l	0
lbW015652	dc.w	0
lbL015654	dc.l	0
lbW015658	dc.w	0
lbL01565A	dc.l	0
lbL01565E	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
lbW01568E	dc.w	0
lbL015690	dc.l	0
lbW015694	dc.w	0
lbL015696	dc.l	0
lbW01569A	dc.w	0
lbL01569C	dc.l	0
lbW0156A0	dc.w	0
lbL0156A2	dc.l	0
lbW0156A6	dc.w	0
lbL0156A8	dc.l	0
lbL0156AC	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.w	0
lbW0156E2	dc.w	0
lbL0156E4	dc.l	0
lbW0156E8	dc.w	0
lbL0156EA	dc.l	0
lbW0156EE	dc.w	0
lbL0156F0	dc.l	0
lbW0156F4	dc.w	0
lbL0156F6	dc.l	0
lbL0156FA	dc.l	0
	dc.l	0
	dc.l	0
lbW015706	dc.w	0
lbL015708	dc.l	0
lbW01570C	dc.w	0
lbL01570E	dc.l	0
lbW015712	dc.w	0
	dc.w	0
	dc.w	0
lbW015718	dc.w	0
lbL01571A	dc.l	0
lbW01571E	dc.w	0
lbL015720	dc.l	0
lbW015724	dc.w	0
	dc.w	0
	dc.w	0
lbW01572A	dc.w	0
lbL01572C	dc.l	0
lbW015730	dc.w	0
lbL015732	dc.l	0
lbW015736	dc.w	0
	dc.w	0
	dc.w	0
lbW01573C	dc.w	0
lbL01573E	dc.l	0
lbW015742	dc.w	0
lbL015744	dc.l	0
lbW015748	dc.w	0
	dc.w	0
	dc.w	0
lbL01574E	dc.l	0
lbW015752	dc.w	0
	dc.w	0
lbL015756	dc.l	0
lbW01575A	dc.w	0
	dc.w	0
lbL01575E	dc.l	0
lbW015762	dc.w	0
	dc.w	0
lbL015766	dc.l	0
lbW01576A	dc.w	0
	dc.w	0
lbL01576E	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
lbL01577E	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
lbL0157DE	dc.l	0
	dc.l	0
lbL0157E6	dc.l	0
	dc.l	0
	dc.l	0



lbC020E98	LINK.W	A6,#0
	MOVEQ	#0,D0
	MOVEA.L	8(A6),A0
	MOVE.B	D0,14(A0)
	MOVE.B	D0,9(A0)
	CLR.L	10(A0)
	MOVE.B	#4,8(A0)
	MOVEQ	#-1,D0
	MOVE.L	D0,-(SP)
	JSR	lbC023B50
	ADDQ.L	#4,SP
	MOVEA.L	8(A6),A0
	MOVE.B	D0,15(A0)
	CLR.L	-(SP)
	JSR	lbC023AF8
	ADDQ.L	#4,SP
	MOVEA.L	8(A6),A0
	MOVE.L	D0,$10(A0)
	ADDA.W	#$14,A0
	MOVE.L	A0,-(SP)
	JSR	lbC023CA0
	ADDQ.L	#4,SP
	UNLK	A6
	RTS

lbC020EEC	LINK.W	A6,#0
	MOVEQ	#0,D0
	MOVEA.L	8(A6),A0
	MOVE.B	15(A0),D0
	MOVE.L	D0,-(SP)
	JSR	lbC023B64
	ADDQ.L	#4,SP
	UNLK	A6
	RTS

lbC020F08	LINK.W	A6,#-14
	MOVEQ	#$44,D0
	MOVE.L	D0,-(SP)
	PEA	lbL022200
	JSR	lbC022940
	ADDQ.L	#8,SP
	MOVEQ	#0,D0
	MOVE.L	D0,lbL022226
	MOVE.L	D0,-(SP)
	PEA	lbL022200
	MOVE.L	D0,-(SP)
	PEA	audiodevice.MSG
	JSR	lbC023C0C
	LEA	$10(SP),SP
	MOVE.L	lbL021E7C,lbL02220E
	MOVE.L	#lbL021E80,-10(A6)
	MOVEQ	#14,D0
	MOVE.L	D0,-(SP)
	PEA	lbL022334
	JSR	lbC022940
	ADDQ.L	#8,SP
	PEA	lbL022334
	JSR	lbC023CA0
	ADDQ.L	#4,SP
	CLR.L	-4(A6)
lbC020F76	MOVE.L	-4(A6),D0
	CMPI.L	#4,D0
	BCC.L	lbC0210A0
	ASL.L	#2,D0
	MOVEA.L	D0,A0
	MOVE.L	A0,0(SP)
	ADDA.L	#lbL022120,A0
	MOVE.L	-4(A6),D0
	MOVEQ	#$24,D1
	JSR	lbC024220
	MOVEA.L	D0,A1
	ADDA.L	#lbL022000,A1
	MOVE.L	A1,(A0)
	MOVEA.L	0(SP),A0
	ADDA.L	#lbL022120,A0
	MOVEA.L	(A0),A1
	MOVE.L	#lbW022140,$16(A1)
	MOVEA.L	0(SP),A0
	ADDA.L	#lbL022120,A0
	MOVE.L	(A0),-(SP)
	PEA	lbL022334
	JSR	lbC023AB0
	ADDQ.L	#8,SP
	MOVE.L	-4(A6),D0
	ASL.L	#2,D0
	MOVEA.L	D0,A0
	MOVE.L	A0,0(SP)
	ADDA.L	#lbL022130,A0
	MOVE.L	-4(A6),D0
	MOVEQ	#$24,D1
	JSR	lbC024220
	MOVEA.L	D0,A1
	ADDA.L	#lbL022090,A1
	MOVE.L	A1,(A0)
	MOVEA.L	0(SP),A0
	ADDA.L	#lbL022130,A0
	MOVEA.L	(A0),A1
	MOVE.L	#lbW022140,$16(A1)
	MOVEA.L	0(SP),A0
	ADDA.L	#lbL022130,A0
	MOVE.L	(A0),-(SP)
	PEA	lbL022334
	JSR	lbC023AB0
	ADDQ.L	#8,SP
	MOVE.L	-4(A6),D0
	ASL.L	#2,D0
	MOVEA.L	D0,A0
	ADDA.L	#lbL022244,A0
	MOVEA.L	-10(A6),A1
	MOVE.L	A1,(A0)
	MOVEQ	#$44,D0
	MOVE.L	D0,-(SP)
	MOVE.L	A1,-(SP)
	PEA	lbL022200
	JSR	lbC022980
	LEA	12(SP),SP
	MOVE.L	-4(A6),D0
	MOVEQ	#1,D1
	ASL.L	D0,D1
	MOVE.B	D1,-5(A6)
	MOVEA.L	-10(A6),A0
	MOVE.W	#$20,$1C(A0)
	MOVE.B	#$7F,9(A0)
	MOVE.B	#$41,$1E(A0)
	LEA	-5(A6),A1
	MOVE.L	A1,$22(A0)
	MOVEQ	#1,D0
	MOVE.L	D0,$26(A0)
	MOVE.L	A0,-(SP)
	JSR	lbC023C78
	ADDQ.L	#4,SP
	ADDI.L	#$44,-10(A6)
	ADDQ.L	#1,-4(A6)
	BRA.L	lbC020F76

lbC0210A0	UNLK	A6
	RTS

lbC0210A4	LINK.W	A6,#-2
	MOVEM.L	D7,-(SP)
	MOVE.B	11(A6),D7
	MOVE.L	D7,D0
	EXT.W	D0
	EXT.L	D0
	ASL.L	#1,D0
	MOVEA.L	D0,A0
	ADDA.L	#lbL021FD4,A0
	MOVE.W	8(A6),D0
	MOVE.W	(A0),D1
	CMP.W	D1,D0
	BNE.S	lbC0210DA
	MOVE.B	10(A6),D0
	CMPI.B	#$53,D0
	BNE.S	lbC0210DA
	CMPI.B	#4,D7
	BLT.S	lbC0210DC
lbC0210DA	MOVEQ	#-1,D7
lbC0210DC	MOVE.L	D7,D0
	MOVEM.L	(SP)+,D7
	UNLK	A6
	RTS

lbC0210E6	LINK.W	A6,#-$52
	MOVEM.L	D7,-(SP)
	MOVE.B	11(A6),D0
	EXT.W	D0
	EXT.L	D0
	MOVEA.L	D0,A0
	ADDA.L	#lbL021FF8,A0
	CLR.B	(A0)
	MOVEA.L	D0,A0
	ADDA.L	#lbL021FF4,A0
	TST.B	(A0)
	BEQ.S	lbC021114
	MOVEM.L	(SP)+,D7
	UNLK	A6
	RTS

lbC021114	MOVE.L	#lbL021E80,-$4E(A6)
	CLR.L	-6(A6)
	MOVE.B	11(A6),D0
	EXT.W	D0
	EXT.L	D0
	ASL.L	#1,D0
	MOVEA.L	D0,A0
	ADDA.L	#lbL021FEC,A0
	MOVE.W	(A0),D7
lbC021134	MOVE.L	D7,D0
	BTST	#0,D0
	BEQ.S	lbC021178
	MOVEQ	#$44,D0
	MOVE.L	D0,-(SP)
	PEA	-$4A(A6)
	MOVE.L	-$4E(A6),-(SP)
	JSR	lbC022980
	LEA	12(SP),SP
	MOVE.W	#8,-$2E(A6)
	MOVE.B	#1,-$2C(A6)
	PEA	-$4A(A6)
	JSR	lbC023C78
	ADDQ.L	#4,SP
	MOVEA.L	-6(A6),A0
	ADDA.L	#lbL021FBC,A0
	MOVE.B	#$FF,(A0)
lbC021178	ADDI.L	#$44,-$4E(A6)
	ADDQ.L	#1,-6(A6)
	MOVE.L	D7,D0
	ASR.W	#1,D7
	BNE.S	lbC021134
	MOVE.B	11(A6),D0
	EXT.W	D0
	EXT.L	D0
	MOVE.L	D0,4(SP)
	ASL.L	#1,D0
	MOVEA.L	D0,A0
	ADDA.L	#lbL021FEC,A0
	CLR.W	(A0)
	MOVEA.L	4(SP),A0
	ADDA.L	#lbL021FF4,A0
	MOVE.B	#1,(A0)
	MOVE.L	4(SP),D0
	ASL.L	#1,D0
	MOVEA.L	D0,A0
	ADDA.L	#lbL021FD4,A0
	MOVE.W	(A0),D0
	ADDQ.W	#1,D0
	MOVE.W	D0,(A0)
	MOVEM.L	(SP)+,D7
	UNLK	A6
	RTS

lbC0211CC	LINK.W	A6,#-6
	MOVEM.L	D6/D7,-(SP)
	MOVE.L	8(A6),D7
	MOVEQ	#0,D6
lbC0211DA	MOVE.L	D7,D0
	BTST	#0,D0
	BEQ.S	lbC0211EE
	MOVEA.L	D6,A0
	ADDA.L	#lbL021FBC,A0
	MOVE.B	15(A6),(A0)
lbC0211EE	ADDQ.L	#1,D6
	MOVE.L	D7,D0
	ASR.W	#1,D7
	BNE.S	lbC0211DA
	MOVEM.L	(SP)+,D6/D7
	UNLK	A6
	RTS

lbC0211FE	LINK.W	A6,#-12
	MOVEM.L	D2/D5-D7,-(SP)
lbC021206	MOVEQ	#0,D5
lbC021208	MOVE.W	#1,-12(A6)
	MOVEQ	#0,D6
	MOVE.L	D5,D0
	MOVE.L	D5,D1
	ASL.L	#1,D1
	MOVEA.L	$10(A6),A0
	ADDA.L	D1,A0
	MOVE.W	(A0),D7
lbC02121E	MOVE.L	D7,D0
	BTST	#0,D0
	BEQ.S	lbC02123C
	MOVEA.L	D6,A0
	ADDA.L	#lbL021FBC,A0
	MOVE.B	(A0),D0
	CMPI.B	#$FF,D0
	BEQ.S	lbC02123C
	CLR.W	-12(A6)
	BRA.S	lbC021244

lbC02123C	ADDQ.L	#1,D6
	MOVE.L	D7,D0
	ASR.W	#1,D7
	BNE.S	lbC02121E
lbC021244	TST.W	-12(A6)
	BEQ.S	lbC021270
	MOVE.L	D5,D0
	MOVE.L	D5,D1
	ASL.L	#1,D1
	MOVEA.L	$10(A6),A0
	ADDA.L	D1,A0
	MOVE.W	(A0),D1
	EXT.L	D1
	MOVEQ	#4,D2
	MOVE.L	D2,-(SP)
	MOVE.L	D1,-(SP)
	BSR.L	lbC0211CC
	ADDQ.L	#8,SP
	MOVE.L	D5,D0
	MOVEM.L	(SP)+,D2/D5-D7
	UNLK	A6
	RTS

lbC021270	ADDQ.L	#1,D5
	MOVE.L	D5,D0
	MOVE.B	11(A6),D1
	EXT.W	D1
	EXT.L	D1
	CMP.L	D1,D0
	BCS.S	lbC021208
	MOVEQ	#0,D5
lbC021282	MOVE.L	D5,D0
	MOVE.L	D5,D1
	ASL.L	#1,D1
	MOVEA.L	$10(A6),A0
	ADDA.L	D1,A0
	MOVE.W	(A0),D7
	MOVEQ	#0,D6
lbC021292	CMPI.L	#4,D6
	BCC.S	lbC0212E4
	MOVE.L	D6,D0
	MOVE.L	D6,D1
	ASL.L	#1,D1
	MOVEA.L	D1,A0
	ADDA.L	#lbL021FEC,A0
	MOVE.W	(A0),D1
	CMP.W	D7,D1
	BNE.S	lbC0212E0
	MOVEA.L	D0,A0
	ADDA.L	#lbL021FFC,A0
	MOVE.B	(A0),D1
	CMP.B	15(A6),D1
	BGE.S	lbC0212E0
	MOVE.L	D0,-(SP)
	BSR.L	lbC0210E6
	ADDQ.L	#4,SP
	MOVE.L	D7,D0
	EXT.L	D0
	MOVEQ	#4,D1
	MOVE.L	D1,-(SP)
	MOVE.L	D0,-(SP)
	BSR.L	lbC0211CC
	ADDQ.L	#8,SP
	MOVE.L	D5,D0
	MOVEM.L	(SP)+,D2/D5-D7
	UNLK	A6
	RTS

lbC0212E0	ADDQ.L	#1,D6
	BRA.S	lbC021292

lbC0212E4	ADDQ.L	#1,D5
	MOVE.L	D5,D0
	MOVE.B	11(A6),D1
	EXT.W	D1
	EXT.L	D1
	CMP.L	D1,D0
	BCS.S	lbC021282
	MOVEQ	#0,D5
lbC0212F6	MOVE.L	D5,D0
	MOVE.L	D5,D1
	ASL.L	#1,D1
	MOVEA.L	$10(A6),A0
	ADDA.L	D1,A0
	MOVE.W	(A0),D7
	MOVEQ	#0,D6
lbC021306	CMPI.L	#4,D6
	BCC.S	lbC021344
	MOVE.L	D6,D0
	MOVE.L	D6,D1
	ASL.L	#1,D1
	MOVEA.L	D1,A0
	ADDA.L	#lbL021FEC,A0
	MOVE.W	(A0),D1
	AND.W	D7,D1
	TST.W	D1
	BEQ.S	lbC021340
	MOVEA.L	D0,A0
	ADDA.L	#lbL021FFC,A0
	MOVE.B	(A0),D1
	CMP.B	15(A6),D1
	BGE.S	lbC021340
	MOVE.L	D0,-(SP)
	BSR.L	lbC0210E6
	ADDQ.L	#4,SP
	BRA.L	lbC021206

lbC021340	ADDQ.L	#1,D6
	BRA.S	lbC021306

lbC021344	ADDQ.L	#1,D5
	MOVE.L	D5,D0
	MOVE.B	11(A6),D1
	EXT.W	D1
	EXT.L	D1
	CMP.L	D1,D0
	BCS.S	lbC0212F6
	MOVEQ	#-1,D0
	MOVEM.L	(SP)+,D2/D5-D7
	UNLK	A6
	RTS

lbC02135E	LINK.W	A6,#-12
	MOVE.W	14(A6),D0
	EXT.L	D0
	CMPI.L	#$FFFFFFFE,D0
	BEQ.S	lbC021388
	CMPI.L	#$FFFFFFFF,D0
	BNE.S	lbC021398
	MOVEQ	#2,D0
	MOVE.L	D0,-4(A6)
	MOVE.L	#lbW021FC4,-12(A6)
	BRA.S	lbC0213A6

lbC021388	MOVEQ	#2,D0
	MOVE.L	D0,-4(A6)
	MOVE.L	#lbW021FC0,-12(A6)
	BRA.S	lbC0213A6

lbC021398	MOVEQ	#4,D0
	MOVE.L	D0,-4(A6)
	MOVE.L	#lbW021FC8,-12(A6)
lbC0213A6	MOVE.W	10(A6),D0
	EXT.L	D0
	MOVE.L	-12(A6),-(SP)
	MOVE.L	D0,-(SP)
	MOVE.L	-4(A6),-(SP)
	BSR.L	lbC0211FE
	LEA	12(SP),SP
	EXT.L	D0
	MOVE.L	D0,-8(A6)
	ADDQ.L	#1,D0
	BNE.S	lbC0213CE
	MOVEQ	#0,D0
	UNLK	A6
	RTS

lbC0213CE	MOVE.L	-8(A6),D0
	ASL.L	#1,D0
	MOVEA.L	-12(A6),A0
	ADDA.L	D0,A0
	MOVE.W	(A0),D0
	UNLK	A6
	RTS

lbC0213E0	LINK.W	A6,#-10
	MOVEM.L	D7,-(SP)
	MOVEQ	#0,D7
lbC0213EA	MOVE.L	D7,D0
	EXT.W	D0
	EXT.L	D0
	MOVEA.L	D0,A0
	MOVE.L	A0,4(SP)
	ADDA.L	#lbL021FF4,A0
	TST.B	(A0)
	BEQ.S	lbC021458
	MOVEA.L	D0,A0
	ADDA.L	#lbL021FF4,A0
	CLR.B	(A0)
	ASL.L	#1,D0
	MOVEA.L	D0,A0
	ADDA.L	#lbL021FD4,A0
	MOVE.W	(A0),D0
	ADDQ.W	#1,D0
	MOVE.W	D0,(A0)
	MOVE.B	#$53,-4(A6)
	MOVE.L	4(SP),D0
	ASL.L	#1,D0
	MOVEA.L	D0,A0
	ADDA.L	#lbL021FD4,A0
	MOVE.L	4(SP),D0
	ASL.L	#1,D0
	MOVEA.L	D0,A1
	ADDA.L	#lbL021FD4,A1
	MOVE.W	(A1),D0
	ANDI.W	#$7FFF,D0
	MOVE.W	D0,(A0)
	MOVE.W	D0,-6(A6)
	MOVE.B	D7,-3(A6)
	MOVE.L	-6(A6),D0
	MOVEM.L	(SP)+,D7
	UNLK	A6
	RTS

lbC021458	ADDQ.B	#1,D7
	CMPI.B	#4,D7
	BLT.S	lbC0213EA
	MOVEQ	#-1,D0
	MOVEM.L	(SP)+,D7
	UNLK	A6
	RTS

lbC02146A	LINK.W	A6,#-14
	MOVEM.L	D4-D7,-(SP)
	MOVE.B	11(A6),D0
	EXT.W	D0
	EXT.L	D0
	MOVE.L	D0,$10(SP)
	ASL.L	#1,D0
	MOVEA.L	D0,A0
	ADDA.L	#lbL02214C,A0
	TST.W	(A0)
	BEQ.S	lbC021500
	MOVE.L	$10(SP),D0
	ASL.L	#1,D0
	MOVEA.L	D0,A0
	ADDA.L	#lbL021FDC,A0
	MOVE.W	(A0),D5
	MOVE.L	$10(SP),D0
	ASL.L	#1,D0
	MOVEA.L	D0,A0
	ADDA.L	#lbL021FEC,A0
	MOVE.W	(A0),D7
	MOVEQ	#0,D6
lbC0214AE	MOVE.L	D7,D0
	BTST	#0,D0
	BEQ.S	lbC0214F8
	MOVE.L	D5,D0
	MULU.W	#$CD,D0
	LSR.W	#8,D0
	MOVE.L	D0,D4
	CMPI.L	#1,D6
	BEQ.S	lbC0214D0
	CMPI.L	#2,D6
	BNE.S	lbC0214E2
lbC0214D0	MOVE.L	D6,D0
	MOVE.L	D6,D1
	ASL.L	#1,D1
	MOVEA.L	D1,A0
	ADDA.L	#lbL021FA0,A0
	MOVE.W	D4,(A0)
	BRA.S	lbC0214F8

lbC0214E2	MOVE.L	D6,D0
	MOVE.L	D6,D1
	ASL.L	#1,D1
	MOVEA.L	D1,A0
	ADDA.L	#lbL021FA0,A0
	MOVE.W	#$100,D1
	SUB.W	D4,D1
	MOVE.W	D1,(A0)
lbC0214F8	ADDQ.L	#1,D6
	MOVE.L	D7,D0
	ASR.W	#1,D7
	BNE.S	lbC0214AE
lbC021500	MOVEM.L	(SP)+,D4-D7
	UNLK	A6
	RTS

lbC021508	LINK.W	A6,#-6
	MOVEM.L	D7,-(SP)
	MOVEA.L	8(A6),A0
	MOVE.L	$14(A0),-(SP)
	BSR.L	lbC0210A4
	ADDQ.L	#4,SP
	MOVE.L	D0,D7
	TST.B	D7
	BPL.S	lbC02152C
	MOVEM.L	(SP)+,D7
	UNLK	A6
	RTS

lbC02152C	MOVE.L	D7,D0
	EXT.W	D0
	EXT.L	D0
	MOVE.L	D0,4(SP)
	ASL.L	#1,D0
	MOVEA.L	D0,A0
	ADDA.L	#lbL021FDC,A0
	MOVEA.L	8(A6),A1
	MOVE.W	$18(A1),(A0)
	MOVE.L	4(SP),-(SP)
	BSR.L	lbC02146A
	ADDQ.L	#4,SP
	MOVEM.L	(SP)+,D7
	UNLK	A6
	RTS

lbC02155A	LINK.W	A6,#0
	TST.L	12(A6)
	BNE.S	lbC02156C
	MOVE.L	#lbW022140,12(A6)
lbC02156C	MOVEA.L	8(A6),A0
	MOVE.L	12(A6),$16(A0)
	MOVEA.L	12(A6),A0
	MOVEA.L	8(A6),A1
	MOVE.L	(A0),14(A1)
	UNLK	A6
	RTS

lbC021586	LINK.W	A6,#-14
	MOVEM.L	D2/D5-D7,-(SP)
	MOVEQ	#-1,D0
	MOVEA.L	8(A6),A0
	MOVE.L	D0,$14(A0)
	BSR.L	lbC0213E0
	MOVEA.L	8(A6),A0
	MOVE.L	D0,$14(A0)
	ADDQ.L	#1,D0
	BNE.S	lbC0215B0
	MOVEM.L	(SP)+,D2/D5-D7
	UNLK	A6
	RTS

lbC0215B0	MOVEA.L	8(A6),A0
	MOVE.L	$14(A0),-(SP)
	BSR.L	lbC0210A4
	ADDQ.L	#4,SP
	MOVE.L	D0,D5
	EXT.W	D5
	EXT.L	D5
	MOVEA.L	D5,A0
	ADDA.L	#lbL021FFC,A0
	MOVEA.L	8(A6),A1
	MOVE.B	$1B(A1),(A0)
	MOVE.L	D5,D0
	MOVE.L	D5,D1
	ASL.L	#1,D1
	MOVEA.L	D1,A0
	ADDA.L	#lbL021FDC,A0
	MOVE.W	$18(A1),(A0)
	MOVEA.L	D1,A0
	ADDA.L	#lbL021FE4,A0
	MOVE.W	$1E(A1),(A0)
	MOVEA.L	D1,A0
	ADDA.L	#lbL021FEC,A0
	MOVE.W	$1C(A1),D1
	MOVE.W	D1,(A0)
	MOVE.L	D1,D7
	MOVE.L	D7,D1
	EXT.L	D1
	MOVE.L	D0,-(SP)
	MOVE.L	D1,-(SP)
	BSR.L	lbC0211CC
	ADDQ.L	#8,SP
	MOVE.L	D5,D0
	MOVE.L	D5,D1
	ASL.L	#1,D1
	MOVEA.L	D1,A0
	ADDA.L	#lbL02214C,A0
	MOVEA.L	8(A6),A1
	MOVE.W	$18(A1),D1
	MOVE.L	A0,$10(SP)
	TST.W	D1
	BMI.S	lbC021638
	CMPI.W	#$140,D1
	BGE.S	lbC021638
	MOVEQ	#1,D0
	BRA.S	lbC02163A

lbC021638	MOVEQ	#0,D0
lbC02163A	MOVEA.L	$10(SP),A0
	MOVE.W	D0,(A0)
	TST.W	D0
	BEQ.S	lbC02164C
	MOVE.L	D5,-(SP)
	BSR.L	lbC02146A
	ADDQ.L	#4,SP
lbC02164C	MOVEQ	#0,D6
lbC02164E	MOVE.L	D7,D0
	BTST	#0,D0
	BEQ.L	lbC0216FA
	MOVE.L	D6,D0
	MOVE.L	D6,D1
	ASL.L	#2,D1
	MOVEA.L	D1,A0
	ADDA.L	#lbL022120,A0
	MOVEA.L	8(A6),A1
	MOVE.L	$24(A1),-(SP)
	MOVE.L	(A0),-(SP)
	BSR.L	lbC02155A
	ADDQ.L	#8,SP
	MOVE.L	D6,D0
	MOVE.L	D6,D1
	ASL.L	#2,D1
	MOVEA.L	D1,A0
	ADDA.L	#lbL022130,A0
	MOVEA.L	8(A6),A1
	MOVE.L	$28(A1),-(SP)
	MOVE.L	(A0),-(SP)
	BSR.L	lbC02155A
	ADDQ.L	#8,SP
	MOVE.L	D5,D0
	MOVE.L	D5,D1
	ASL.L	#1,D1
	MOVEA.L	D1,A0
	ADDA.L	#lbL02214C,A0
	TST.W	(A0)
	BNE.S	lbC0216B8
	MOVE.L	D6,D1
	MOVE.L	D6,D2
	ASL.L	#1,D2
	MOVEA.L	D2,A0
	ADDA.L	#lbL021FA0,A0
	MOVE.W	#$100,(A0)
lbC0216B8	MOVEA.L	D6,A0
	ADDA.L	#lbL021FB8,A0
	MOVEQ	#0,D0
	MOVE.B	D0,(A0)
	MOVEA.L	D6,A0
	ADDA.L	#lbL021FD0,A0
	MOVE.B	D0,(A0)
	MOVE.L	D6,-(SP)
	JSR	lbC0227D6
	ADDQ.L	#4,SP
	MOVEA.L	8(A6),A1
	MOVEA.L	$20(A1),A0
	MOVE.L	D0,$10(SP)
	MOVEA.L	$10(SP),A1
	MOVEQ	#13,D0
lbC0216EA	MOVE.B	(A0)+,(A1)+
	DBRA	D0,lbC0216EA
	MOVE.L	D6,-(SP)
	JSR	lbC0227F6
	ADDQ.L	#4,SP
lbC0216FA	ADDQ.L	#1,D6
	MOVE.L	D7,D0
	ASR.W	#1,D7
	BNE.L	lbC02164E
	MOVEA.L	D5,A0
	ADDA.L	#lbL021FF8,A0
	MOVE.B	#1,(A0)
	MOVEM.L	(SP)+,D2/D5-D7
	UNLK	A6
	RTS

lbC021718	LINK.W	A6,#-2
	MOVEA.L	8(A6),A0
	MOVE.L	$14(A0),-(SP)
	BSR.L	lbC0210A4
	ADDQ.L	#4,SP
	MOVE.B	D0,-1(A6)
	TST.B	D0
	BPL.S	lbC021736
	UNLK	A6
	RTS

lbC021736	MOVE.B	-1(A6),D0
	EXT.W	D0
	EXT.L	D0
	MOVE.L	D0,-(SP)
	BSR.L	lbC0210E6
	ADDQ.L	#4,SP
	UNLK	A6
	RTS

lbC02174A	LINK.W	A6,#-$4E
	MOVEM.L	D7/A5,-(SP)
	MOVEA.L	8(A6),A0
	MOVE.L	$14(A0),-(SP)
	BSR.L	lbC0210A4
	ADDQ.L	#4,SP
	EXT.W	D0
	EXT.L	D0
	MOVE.L	D0,-10(A6)
	TST.L	D0
	BPL.S	lbC021774
	MOVEM.L	(SP)+,D7/A5
	UNLK	A6
	RTS

lbC021774	LEA	lbL021E80,A5
	MOVE.L	-10(A6),D0
	ASL.L	#1,D0
	MOVEA.L	D0,A0
	ADDA.L	#lbL021FEC,A0
	MOVE.W	(A0),D7
lbC02178A	MOVE.L	D7,D0
	BTST	#0,D0
	BEQ.S	lbC0217CA
	MOVEQ	#$44,D0
	MOVE.L	D0,-(SP)
	PEA	-$4E(A6)
	MOVE.L	A5,-(SP)
	JSR	lbC022980
	LEA	12(SP),SP
	MOVEQ	#$14,D0
	MOVE.L	D0,-(SP)
	PEA	-$4E(A6)
	JSR	lbC022940
	ADDQ.L	#8,SP
	MOVEA.L	8(A6),A1
	MOVEA.L	$30(A1),A0
	MOVE.L	$2C(A1),-(SP)
	PEA	-$4E(A6)
	JSR	(A0)
	ADDQ.L	#8,SP
lbC0217CA	ADDA.W	#$44,A5
	MOVE.L	D7,D0
	ASR.W	#1,D7
	BNE.S	lbC02178A
	MOVEM.L	(SP)+,D7/A5
	UNLK	A6
	RTS

lbC0217DC	LINK.W	A6,#-4
	MOVE.W	#1,lbW0221D0
	CLR.L	-4(A6)
lbC0217EC	MOVE.L	-4(A6),D0
	CMPI.L	#4,D0
	BGE.S	lbC021812
	MOVEA.L	D0,A0
	ADDA.L	#lbL021FF8,A0
	TST.B	(A0)
	BEQ.S	lbC02180C
	MOVE.L	D0,-(SP)
	BSR.L	lbC0210E6
	ADDQ.L	#4,SP
lbC02180C	ADDQ.L	#1,-4(A6)
	BRA.S	lbC0217EC

lbC021812	UNLK	A6
	RTS

lbC021816	LINK.W	A6,#0
	MOVEA.L	8(A6),A0
	MOVE.B	$1A(A0),D0
	EXT.W	D0
	EXT.L	D0
	SUBI.L	#1,D0
	BLT.S	lbC0218AA
	CMPI.L	#6,D0
	BGE.S	lbC0218AA
	ASL.L	#1,D0
	JMP	lbC02183C(PC,D0.L)

lbC02183C	BRA.S	lbC021848

	BRA.S	lbC021854

	BRA.S	lbC021860

	BRA.S	lbC02186C

	BRA.S	lbC021878

	BRA.S	lbC021884

lbC021848	MOVE.L	8(A6),-(SP)
	BSR.L	lbC021586
	ADDQ.L	#4,SP
	BRA.S	lbC0218AA

lbC021854	MOVE.L	8(A6),-(SP)
	BSR.L	lbC021718
	ADDQ.L	#4,SP
	BRA.S	lbC0218AA

lbC021860	MOVE.L	8(A6),-(SP)
	BSR.L	lbC021508
	ADDQ.L	#4,SP
	BRA.S	lbC0218AA

lbC02186C	MOVE.L	8(A6),-(SP)
	BSR.L	lbC02174A
	ADDQ.L	#4,SP
	BRA.S	lbC0218AA

lbC021878	MOVE.L	8(A6),-(SP)
	BSR.L	lbC0217DC
	ADDQ.L	#4,SP
	BRA.S	lbC0218AA

lbC021884	MOVEA.L	8(A6),A0
	MOVE.B	$1B(A0),D0
	EXT.W	D0
	EXT.L	D0
	MOVE.W	$18(A0),D1
	EXT.L	D1
	MOVE.L	D1,-(SP)
	MOVE.L	D0,-(SP)
	BSR.L	lbC02135E
	ADDQ.L	#8,SP
	MOVEA.L	8(A6),A0
	MOVE.W	D0,$1C(A0)
	NOP
lbC0218AA	MOVEA.L	8(A6),A0
	TST.L	14(A0)
	BEQ.S	lbC0218C0
	MOVE.L	A0,-(SP)
	JSR	lbC023BCC
	ADDQ.L	#4,SP
	BRA.S	lbC0218D0

lbC0218C0	MOVEQ	#$34,D0
	MOVE.L	D0,-(SP)
	MOVE.L	8(A6),-(SP)
	JSR	lbC023A98
	ADDQ.L	#8,SP
lbC0218D0	UNLK	A6
	RTS

lbC0218D4	LINK.W	A6,#-$2C
	MOVEM.L	D7/A4/A5,-(SP)
	PEA	-$2C(A6)
	BSR.L	lbC020E98
	ADDQ.L	#4,SP
	LEA	-$2C(A6),A0
	MOVE.L	A0,lbL021E7C
	BSR.L	lbC020F08
	MOVEQ	#$28,D0
	MOVE.L	D0,-(SP)
	PEA	lbL0225E2
	JSR	lbC022940
	ADDQ.L	#8,SP
	CLR.L	-(SP)
	PEA	lbL0225E2
	MOVEQ	#1,D0
	MOVE.L	D0,-(SP)
	PEA	timerdevice.MSG
	JSR	lbC023C0C
	LEA	$10(SP),SP
	MOVE.L	lbL021E7C,lbL0225F0
	CLR.L	lbL022602
	MOVE.L	#$411A,lbL022606
	MOVE.W	#9,lbW0225FE
	PEA	lbL0225E2
	JSR	lbC023C78
	ADDQ.L	#4,SP
lbC021952	MOVE.L	lbL021E7C,-(SP)
	JSR	lbC023BE0
	ADDQ.L	#4,SP
	MOVE.L	lbL021E7C,-(SP)
	JSR	lbC023BB8
	ADDQ.L	#4,SP
	MOVEA.L	D0,A5
	LEA	lbL0225E2,A0
	CMPA.L	A0,A5
	BNE.S	lbC0219BA
	CLR.L	lbL022602
	MOVE.L	#$411A,lbL022606
	MOVE.W	#9,lbW0225FE
	TST.W	lbW0221D0
	BEQ.S	lbC0219A4
	MOVE.W	#1,lbW0221D2
	BRA.S	lbC021952

lbC0219A4	PEA	lbL0225E2
	JSR	lbC023C78
	ADDQ.L	#4,SP
	JSR	lbC022650
	BRA.S	lbC021952

lbC0219BA	LEA	lbL021E80,A4
	MOVEQ	#0,D7
lbC0219C2	MOVE.L	A4,D0
	MOVEA.L	A4,A0
	CMPA.L	A0,A5
	BNE.S	lbC0219F6
	MOVE.L	D7,D1
	EXT.W	D1
	EXT.L	D1
	MOVEA.L	D1,A0
	ADDA.L	#lbL021FBC,A0
	MOVE.B	(A0),D1
	BTST	#2,D1
	BNE.L	lbC021952
	MOVE.L	D7,D0
	EXT.W	D0
	EXT.L	D0
	MOVE.L	D0,-(SP)
	JSR	lbC0227F6
	ADDQ.L	#4,SP
	BRA.L	lbC021952

lbC0219F6	ADDA.W	#$44,A4
	ADDQ.B	#1,D7
	CMPI.B	#4,D7
	BLT.S	lbC0219C2
	MOVE.L	A5,D0
	MOVEA.L	A5,A0
	MOVE.L	A0,-(SP)
	BSR.L	lbC021816
	ADDQ.L	#4,SP
	BRA.L	lbC021952

Init1
	LINK.W	A6,#-6
	MOVE.W	#1,lbW0221D4
	MOVEQ	#0,D0
	MOVE.L	#$2A0,-(SP)
	PEA	lbL022342
	MOVE.W	D0,lbW0221D0
	MOVE.W	D0,lbW0221D2
	JSR	lbC022940
	ADDQ.L	#8,SP
	MOVEQ	#$5C,D0
	MOVE.L	D0,-(SP)
	PEA	lbL02215C
	JSR	lbC022940
	ADDQ.L	#8,SP
	MOVE.L	#$E0,-(SP)
	PEA	lbL022254
	JSR	lbC022940
	ADDQ.L	#8,SP
	MOVEQ	#8,D0
	MOVE.L	D0,-(SP)
	PEA	lbL021FEC
	JSR	lbC022940
	ADDQ.L	#8,SP
	MOVEQ	#4,D0
	MOVE.L	D0,-(SP)
	PEA	lbL021FF8
	JSR	lbC022940
	ADDQ.L	#8,SP
	CLR.B	-1(A6)
lbC021A8E	MOVE.B	-1(A6),D0
	CMPI.B	#4,D0
	BGE.S	lbC021AD0
	EXT.W	D0
	EXT.L	D0
	MOVE.L	D0,0(SP)
	ASL.L	#1,D0
	MOVEA.L	D0,A0
	ADDA.L	#lbL021FD4,A0
	MOVE.W	#1,(A0)
	MOVEA.L	0(SP),A0
	ADDA.L	#lbL021FF4,A0
	MOVE.B	#1,(A0)
	MOVEA.L	0(SP),A0
	ADDA.L	#lbL021FBC,A0
	MOVE.B	#$FF,(A0)
	ADDQ.B	#1,-1(A6)
	BRA.S	lbC021A8E

lbC021AD0	MOVE.L	#lbL022342,lbL022196
	LEA	lbW0225E0,A0
	MOVE.L	A0,lbL02219A
	MOVE.L	A0,lbL022192
	MOVE.B	#1,lbB022164
	MOVE.L	lbL021E78,D0
	MOVE.B	D0,lbB022165
	MOVE.L	#SfxTask.MSG,lbL022166
	LEA	lbL02215C,A0
	CLR.L	-(SP)
	PEA	lbC0218D4(PC)
	MOVE.L	A0,-(SP)
	MOVE.L	A0,lbL022158
	JSR	lbC023AC8
	LEA	12(SP),SP
	UNLK	A6
	RTS

Ending
	LINK.W	A6,#-$5A
	MOVE.W	#1,lbW0221D0
	LEA	-$34(A6),A0
	PEA	-$5A(A6)
	MOVE.L	A0,-$38(A6)
	BSR.L	lbC020E98
	ADDQ.L	#4,SP
	MOVEQ	#$34,D0
	MOVE.L	D0,-(SP)
	MOVE.L	-$38(A6),-(SP)
	JSR	lbC022940
	ADDQ.L	#8,SP
	LEA	-$5A(A6),A0
	MOVEA.L	-$38(A6),A1
	MOVE.L	A0,14(A1)
	MOVEQ	#-1,D0
	MOVE.L	D0,$14(A1)
	MOVE.B	#5,$1A(A1)
	MOVE.L	A1,-(SP)
	MOVE.L	lbL021E7C,-(SP)
	JSR	lbC023BA0
	ADDQ.L	#8,SP
	PEA	-$5A(A6)
	JSR	lbC023BE0
	ADDQ.L	#4,SP
	MOVEQ	#15,D0
	MOVE.L	D0,lbL022218
	PEA	lbL022200
	JSR	lbC023C2C
	ADDQ.L	#4,SP
lbC021BA4	TST.W	lbW0221D2
	BEQ.S	lbC021BA4
	PEA	lbL0225E2
	JSR	lbC023C2C
	ADDQ.L	#4,SP
	MOVEA.L	-$38(A6),A0
	CLR.B	$1A(A0)
	MOVE.L	A0,-(SP)
	MOVE.L	lbL021E7C,-(SP)
	JSR	lbC023BA0
	ADDQ.L	#8,SP
	PEA	-$5A(A6)
	JSR	lbC023BE0
	ADDQ.L	#4,SP
	PEA	-$5A(A6)
	JSR	lbC023BB8
	ADDQ.L	#4,SP
	PEA	-$5A(A6)
	MOVE.L	D0,-$38(A6)
	BSR.L	lbC020EEC
	ADDQ.L	#4,SP
	MOVE.L	lbL021E7C,-(SP)
	BSR.L	lbC020EEC
	ADDQ.L	#4,SP
	PEA	lbL02215C
	JSR	lbC023AE4
	ADDQ.L	#4,SP
	UNLK	A6
	RTS

lbC021C16	LINK.W	A6,#-$5E
	TST.W	lbW0221D4
	BNE.S	lbC021C28
	MOVEQ	#-1,D0
	UNLK	A6
	RTS

lbC021C28	LEA	-$38(A6),A0
	PEA	-$5E(A6)
	MOVE.L	A0,-$3C(A6)
	BSR.L	lbC020E98
	ADDQ.L	#4,SP
	MOVEQ	#$14,D0
	MOVE.L	D0,-(SP)
	MOVE.L	-$3C(A6),-(SP)
	JSR	lbC022940
	ADDQ.L	#8,SP
	LEA	-$5E(A6),A0
	MOVEA.L	-$3C(A6),A1
	MOVE.L	A0,14(A1)
	MOVEQ	#-1,D0
	MOVE.L	D0,-4(A6)
	MOVE.B	#6,$1A(A1)
	MOVE.B	$1B(A6),$1B(A1)
	MOVE.W	$1E(A6),$18(A1)
	MOVE.L	A1,-(SP)
	MOVE.L	lbL021E7C,-(SP)
	JSR	lbC023BA0
	ADDQ.L	#8,SP
	PEA	-$5E(A6)
	JSR	lbC023BE0
	ADDQ.L	#4,SP
	PEA	-$5E(A6)
	JSR	lbC023BB8
	ADDQ.L	#4,SP
	MOVEA.L	D0,A0
	MOVE.W	$1C(A0),D1
	MOVE.L	D0,-$3C(A6)
	TST.W	D1
	BEQ.S	lbC021CFA
	MOVEA.L	-$3C(A6),A0
	MOVE.L	8(A6),$20(A0)
	MOVE.L	12(A6),$24(A0)
	MOVE.L	$10(A6),$28(A0)
	MOVE.W	$16(A6),$1E(A0)
	MOVE.B	#1,$1A(A0)
	MOVE.L	A0,-(SP)
	MOVE.L	lbL021E7C,-(SP)
	JSR	lbC023BA0
	ADDQ.L	#8,SP
	PEA	-$5E(A6)
	JSR	lbC023BE0
	ADDQ.L	#4,SP
	PEA	-$5E(A6)
	JSR	lbC023BB8
	ADDQ.L	#4,SP
	MOVEA.L	D0,A0
	MOVE.L	$14(A0),-4(A6)
	MOVE.L	D0,-$3C(A6)
lbC021CFA	PEA	-$5E(A6)
	BSR.L	lbC020EEC
	ADDQ.L	#4,SP
	MOVE.L	-4(A6),D0
	UNLK	A6
	RTS

;	LINK.W	A6,#-4
;	MOVE.L	#$10001,-(SP)
;	MOVEQ	#$34,D0
;	MOVE.L	D0,-(SP)
;	JSR	lbC023A80
;	ADDQ.L	#8,SP
;	MOVEA.L	D0,A0
;	MOVE.L	8(A6),$14(A0)
;	MOVE.W	14(A6),$18(A0)
;	MOVE.B	#3,$1A(A0)
;	MOVE.L	D0,-(SP)
;	MOVE.L	lbL021E7C,-(SP)
;	MOVE.L	D0,-4(A6)
;	JSR	lbC023BA0
;	ADDQ.L	#8,SP
;	UNLK	A6
;	RTS

;	LINK.W	A6,#-4
;	MOVE.L	#$10001,-(SP)
;	MOVEQ	#$34,D0
;	MOVE.L	D0,-(SP)
;	JSR	lbC023A80
;	ADDQ.L	#8,SP
;	MOVEA.L	D0,A0
;	MOVE.L	8(A6),$14(A0)
;	MOVE.L	12(A6),$30(A0)
;	MOVE.L	$10(A6),$2C(A0)
;	MOVE.B	#4,$1A(A0)
;	MOVE.L	D0,-(SP)
;	MOVE.L	lbL021E7C,-(SP)
;	MOVE.L	D0,-4(A6)
;	JSR	lbC023BA0
;	ADDQ.L	#8,SP
;	UNLK	A6
;	RTS

lbC021D96	LINK.W	A6,#-4
	MOVE.L	8(A6),-(SP)
	BSR.L	lbC0210A4
	ADDQ.L	#4,SP
	TST.B	D0
	BPL.S	lbC021DAC
	UNLK	A6
	RTS

lbC021DAC	MOVE.L	#$10001,-(SP)
	MOVEQ	#$34,D0
	MOVE.L	D0,-(SP)
	JSR	lbC023A80
	ADDQ.L	#8,SP
	MOVEA.L	D0,A0
	MOVE.L	8(A6),$14(A0)
	MOVE.B	#2,$1A(A0)
	MOVE.L	D0,-(SP)
	MOVE.L	lbL021E7C,-(SP)
	MOVE.L	D0,-4(A6)
	JSR	lbC023BA0
	ADDQ.L	#8,SP
	UNLK	A6
	RTS

;	LINK.W	A6,#-6
;	MOVEM.L	D7,-(SP)
;	CLR.W	-6(A6)
;	MOVE.L	8(A6),-(SP)
;	BSR.L	lbC0210A4
;	ADDQ.L	#4,SP
;	MOVE.B	D0,-3(A6)
;	TST.B	D0
;	BPL.S	lbC021E0C
;	MOVEQ	#0,D0
;	MOVEM.L	(SP)+,D7
;	UNLK	A6
;	RTS

;lbC021E0C	MOVE.B	-3(A6),D0
;	EXT.W	D0
;	EXT.L	D0
;	ASL.L	#1,D0
;	MOVEA.L	D0,A0
;	ADDA.L	#lbL021FEC,A0
;	MOVE.W	(A0),D7
;lbC021E20	MOVE.L	D7,D0
;	BTST	#0,D0
;	BEQ.S	lbC021E44
;	MOVE.W	-6(A6),D0
;	EXT.L	D0
;	MOVEA.L	D0,A0
;	ADDA.L	#lbL021FB8,A0
;	TST.B	(A0)
;	BNE.S	lbC021E44
;	MOVEQ	#1,D0
;	MOVEM.L	(SP)+,D7
;	UNLK	A6
;	RTS

;lbC021E44	ADDQ.W	#1,-6(A6)
;	MOVE.L	D7,D0
;	ASR.W	#1,D7
;	BNE.S	lbC021E20
;	MOVEQ	#0,D0
;	MOVEM.L	(SP)+,D7
;	UNLK	A6
;	RTS

;	CLR.W	lbW0221D4
;	RTS

;	MOVE.W	#1,lbW0221D4
;	RTS

;	dc.l	0
;	dc.l	0
;	dc.l	$3880000
;	dc.w	$DDAB
lbL021E78	dc.l	7
lbL021E7C	dc.l	0
lbL021E80	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
lbL021F90	dc.l	0
	dc.l	0
lbL021F98	dc.l	0
	dc.l	0
lbL021FA0	dc.l	0
	dc.l	0
lbL021FA8	dc.l	0
	dc.l	0
lbL021FB0	dc.l	0
	dc.l	0
lbL021FB8	dc.l	0
lbL021FBC	dc.l	0
lbW021FC0	dc.w	2
	dc.w	4
lbW021FC4	dc.w	1
	dc.w	8
lbW021FC8	dc.w	3
	dc.w	5
	dc.w	10
	dc.w	12
lbL021FD0	dc.l	0
lbL021FD4	dc.l	0
	dc.l	0
lbL021FDC	dc.l	0
	dc.l	0
lbL021FE4	dc.l	0
	dc.l	0
lbL021FEC	dc.l	0
	dc.l	0
lbL021FF4	dc.l	0
lbL021FF8	dc.l	0
lbL021FFC	dc.l	0
lbL022000	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
lbL022090	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
lbL022120	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
lbL022130	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
lbW022140	dc.w	1
	dc.w	0
	dc.w	1
	dc.w	0
	dc.w	0
	dc.w	0
lbL02214C	dc.l	0
	dc.l	0
	dc.l	0
lbL022158	dc.l	0
lbL02215C	dc.l	0
	dc.l	0
lbB022164	dc.b	0
lbB022165	dc.b	0
lbL022166	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
lbL022192	dc.l	0
lbL022196	dc.l	0
lbL02219A	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.w	0
lbL0221B8	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	lbL0221B8
lbW0221D0	dc.w	0
lbW0221D2	dc.w	0
lbW0221D4	dc.w	1
audiodevice.MSG	dc.b	'audio.device',0
timerdevice.MSG	dc.b	'timer.device',0
SfxTask.MSG	dc.b	'SfxTask',0
;	dc.w	0
;	dc.l	$4140000
;	dc.w	$DEB1
lbL022200	dc.l	0
	dc.l	0
	dc.l	0
	dc.w	0
lbL02220E	dc.l	0
	dc.l	0
	dc.w	0
lbL022218	dc.l	0
	dc.l	0
	dc.l	0
	dc.w	0
lbL022226	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.w	0
lbL022244	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
lbL022254	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
lbL022334	dc.l	0
	dc.l	0
	dc.l	0
	dc.w	0
lbL022342	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.w	0
lbW0225E0	dc.w	0
lbL0225E2	dc.l	0
	dc.l	0
	dc.l	0
	dc.w	0
lbL0225F0	dc.l	0
	dc.l	0
	dc.l	0
	dc.w	0
lbW0225FE	dc.w	0
	dc.w	0
lbL022602	dc.l	0
lbL022606	dc.l	0
;	dc.l	0
;	dc.l	0
;	dc.w	$328
;	dc.w	0
;	dc.w	$DF7B

lbC022618	MOVE.L	4(SP),D0
	ASL.L	#1,D0
	MOVE.L	D0,D1
	ASL.L	#1,D0
	MOVEA.L	D0,A0
	ADDA.L	#lbL022130,A0
	MOVEA.L	(A0),A1
	MOVEA.L	#lbL021F90,A0
	ADDA.L	D1,A0
	CMPI.L	#$10000,14(A1)
	BNE.S	lbC022644
	MOVEQ	#0,D0
	MOVE.W	(A0),D0
	RTS

lbC022644	MOVE.L	14(A1),D0
	ASR.L	#8,D0
	MULU.W	(A0),D0
	LSR.L	#8,D0
	RTS

lbC022650	LINK.W	A6,#-$56
	MOVEM.L	D2/D4-D7/A5,-(SP)
	MOVEQ	#0,D7
	PEA	lbL022334
	JSR	lbC0238D0
	ADDQ.L	#4,SP
lbC022668	MOVEA.L	#lbL021FF8,A0
	ADDA.L	D7,A0
	TST.B	(A0)
	BEQ.L	lbC02274E
	MOVE.L	D7,D0
	ASL.L	#1,D0
	MOVEA.L	#lbL021FE4,A0
	ADDA.L	D0,A0
	TST.W	(A0)
	BNE.S	lbC022694
	MOVE.L	D7,-(SP)
	JSR	lbC0210E6
	ADDQ.L	#4,SP
	BRA.L	lbC02274E

lbC022694	MOVEA.L	#lbL021FEC,A0
	ADDA.L	D0,A0
	MOVE.W	(A0),D6
	MOVEQ	#0,D5
	MOVEA.L	#lbL021E80,A5
lbC0226A6	BTST	#0,D6
	BEQ.L	lbC022734
	MOVE.L	D5,-(SP)
	BSR.L	lbC022618
	MOVE.L	D0,D4
	MOVE.L	D5,-(SP)
	BSR.L	lbC022760
	ADDQ.L	#8,SP
	MOVE.L	D5,D2
	ASL.L	#1,D2
	MOVEA.L	#lbL021FA8,A0
	ADDA.L	D2,A0
	MOVE.W	D0,-14(A6)
	CMP.W	(A0),D0
	BNE.S	lbC0226DE
	MOVEA.L	#lbL021FB0,A0
	ADDA.L	D2,A0
	CMP.W	(A0),D4
	BEQ.S	lbC022734
lbC0226DE	MOVEQ	#$2C,D0
	MOVE.L	D0,-(SP)
	PEA	-$52(A6)
	MOVE.L	A5,-(SP)
	JSR	lbC022980
	LEA	12(SP),SP
	MOVE.L	D4,D1
	CMPI.W	#$72,D4
	BCC.S	lbC0226FC
	MOVEQ	#$48,D1
lbC0226FC	MOVE.W	D1,-$28(A6)
	MOVEA.L	#lbL021FB0,A0
	ADDA.L	D2,A0
	MOVE.W	D1,(A0)
	MOVEA.L	#lbL021FA8,A0
	ADDA.L	D2,A0
	MOVE.W	-14(A6),D1
	MOVE.W	D1,-$26(A6)
	MOVE.W	D1,(A0)
	MOVE.W	#12,-$36(A6)
	MOVE.B	#1,-$34(A6)
	PEA	-$52(A6)
	JSR	lbC023C78
	ADDQ.L	#4,SP
lbC022734	ADDA.W	#$44,A5
	ADDQ.W	#1,D5
	ASR.W	#1,D6
	BNE.L	lbC0226A6
	MOVE.L	D7,D1
	ASL.L	#1,D1
	MOVEA.L	#lbL021FE4,A0
	ADDA.L	D1,A0
	SUBQ.W	#1,(A0)
lbC02274E	ADDQ.W	#1,D7
	CMPI.W	#4,D7
	BLT.L	lbC022668
	MOVEM.L	(SP)+,D2/D4-D7/A5
	UNLK	A6
	RTS

lbC022760	MOVE.L	D2,-(SP)
	MOVE.W	10(SP),D0
	EXT.L	D0
	ASL.L	#1,D0
	MOVE.L	D0,D2
	ASL.L	#1,D0
	MOVEA.L	D0,A0
	ADDA.L	#lbL022120,A0
	MOVEA.L	(A0),A1
	MOVE.L	14(A1),D0
	LSR.L	#8,D0
	MOVEA.L	#lbL021FA0,A0
	ADDA.L	D2,A0
	MOVE.W	(A0),D1
	CMPI.L	#$100,D0
	BNE.S	lbC0227AE
	MOVEA.L	#lbL021F98,A0
	ADDA.L	D2,A0
	CMPI.W	#$100,D1
	BNE.S	lbC0227A6
	MOVEQ	#0,D0
	MOVE.W	(A0),D0
	MOVE.L	(SP)+,D2
	RTS

lbC0227A6	MOVE.W	(A0),D0
	MULU.W	D1,D0
	LSR.L	#8,D0
	BRA.S	lbC0227D2

lbC0227AE	MOVE.L	D0,10(SP)
	MOVEA.L	#lbL021F98,A0
	ADDA.L	D2,A0
	CMPI.W	#$100,D1
	BNE.S	lbC0227C6
	MOVEQ	#0,D0
	MOVE.W	(A0),D0
	BRA.S	lbC0227CC

lbC0227C6	MOVE.W	(A0),D0
	MULU.W	D1,D0
	LSR.L	#8,D0
lbC0227CC	MULU.W	12(SP),D0
	LSR.L	#8,D0
lbC0227D2	MOVE.L	(SP)+,D2
	RTS

lbC0227D6	MOVE.W	6(SP),D0
	MOVEA.W	D0,A0
	ASL.W	#2,D0
	ADDA.L	#lbL021FD0,A0
	MOVE.B	(A0),D1
	EXT.W	D1
	ADD.W	D1,D0
	MULS.W	#14,D0
	ADDI.L	#lbL022254,D0
	RTS

lbC0227F6	LINK.W	A6,#-$12
	MOVEM.L	D2/D7/A3-A5,-(SP)
	MOVE.W	10(A6),D7
	EXT.L	D7
lbC022804	MOVEA.L	D7,A0
	ADDA.L	#lbL021FB8,A0
	TST.B	(A0)
	BNE.L	lbC02292E
	MOVE.L	D7,-(SP)
	BSR.S	lbC0227D6
	ADDQ.L	#4,SP
	MOVEA.L	D0,A4
	MOVE.W	4(A4),D0
	TST.W	D0
	BMI.S	lbC022828
	SUBQ.W	#1,D0
	MOVE.W	D0,4(A4)
lbC022828	ADDQ.W	#1,D0
	BNE.S	lbC02285E
	TST.L	(A4)
	BEQ.S	lbC02283E
	MOVEA.L	(A4),A0
	MOVEA.L	A4,A1
	MOVEQ	#6,D0
lbC022836	MOVE.W	(A0)+,(A1)+
	DBRA	D0,lbC022836
	BRA.S	lbC022804

lbC02283E	MOVEA.L	D7,A0
	ADDA.L	#lbL021FD0,A0
	TST.B	(A0)
	BEQ.S	lbC02284E
	SUBQ.B	#1,(A0)
	BRA.S	lbC022804

lbC02284E	MOVEA.L	D7,A0
	ADDA.L	#lbL021FB8,A0
	MOVE.B	#1,(A0)
	BRA.L	lbC02292E

lbC02285E	TST.L	6(A4)
	BEQ.S	lbC02288E
	MOVEA.L	D7,A0
	ADDA.L	#lbL021FD0,A0
	ADDQ.B	#1,(A0)
	MOVE.L	D7,-(SP)
	BSR.L	lbC0227D6
	ADDQ.L	#4,SP
	MOVEA.L	10(A4),A0
	MOVE.L	D0,$14(SP)
	MOVEA.L	$14(SP),A1
	MOVEQ	#6,D0
lbC022884	MOVE.W	(A0)+,(A1)+
	DBRA	D0,lbC022884
	BRA.L	lbC022804

lbC02288E	MOVEA.L	10(A4),A5
	MOVE.L	D7,D1
	ASL.L	#1,D1
	MOVEA.L	D1,A0
	ADDA.L	#lbL021F98,A0
	MOVE.W	6(A5),(A0)
	MOVEA.L	D1,A0
	ADDA.L	#lbL021F90,A0
	MOVE.W	4(A5),(A0)
	MOVE.L	D7,D0
	ASL.L	#2,D0
	MOVEA.L	D0,A0
	ADDA.L	#lbL022244,A0
	MOVEA.L	(A0),A3
	MOVE.L	8(A5),$22(A3)
	MOVE.L	(A5),D0
	ASL.L	#1,D0
	MOVE.L	D0,$26(A3)
	MOVE.L	D7,-(SP)
	BSR.L	lbC022618
	ADDQ.L	#4,SP
	MOVE.L	D7,D1
	ASL.L	#1,D1
	MOVEA.L	D1,A0
	ADDA.L	#lbL021FB0,A0
	MOVE.W	D0,(A0)
	MOVE.W	D0,$2A(A3)
	MOVE.L	D1,D2
	ADDI.L	#lbL021FA8,D2
	MOVE.L	D7,-(SP)
	BSR.L	lbC022760
	ADDQ.L	#4,SP
	MOVEA.L	D2,A0
	MOVE.W	D0,(A0)
	MOVE.W	D0,$2C(A3)
	MOVE.W	4(A4),D0
	ADDQ.W	#2,D0
	BNE.S	lbC02290A
	CLR.W	$2E(A3)
	BRA.S	lbC022914

lbC02290A	MOVE.W	4(A4),D0
	ADDQ.W	#1,D0
	MOVE.W	D0,$2E(A3)
lbC022914	CLR.W	4(A4)
	MOVE.W	#3,$1C(A3)
	MOVE.B	#$10,$1E(A3)
	MOVE.L	A3,-(SP)
	JSR	lbC023C78
	ADDQ.L	#4,SP
lbC02292E	MOVEM.L	(SP)+,D2/D7/A3-A5
	UNLK	A6
	RTS

;	dc.w	0
;	dc.w	0
;	dc.w	$94
;	dc.w	0
;	dc.w	$DFA1

lbC022940	MOVEA.L	4(SP),A0
	MOVE.L	8(SP),D0
	CLR.L	D1
	BTST	#0,D0
	BNE.S	lbC022976
	BTST	#0,7(SP)
	BNE.S	lbC022976
	BTST	#1,D0
	BEQ.S	lbC02296A
	ASR.L	#1,D0
	SUBQ.L	#1,D0
lbC022962	MOVE.W	D1,(A0)+
	DBRA	D0,lbC022962
	RTS

lbC02296A	ASR.L	#2,D0
	SUBQ.L	#1,D0
lbC02296E	MOVE.L	D1,(A0)+
	DBRA	D0,lbC02296E
	RTS

lbC022976	SUBQ.L	#1,D0
lbC022978	MOVE.B	D1,(A0)+
	DBRA	D0,lbC022978
	RTS

lbC022980	MOVEA.L	4(SP),A0
	MOVEA.L	8(SP),A1
	MOVE.L	12(SP),D0
	BTST	#0,D0
	BNE.S	lbC0229C0
	BTST	#0,7(SP)
	BNE.S	lbC0229C0
	BTST	#0,11(SP)
	BNE.S	lbC0229C0
	BTST	#1,D0
	BEQ.S	lbC0229B4
	ASR.L	#1,D0
	SUBQ.L	#1,D0
lbC0229AC	MOVE.W	(A0)+,(A1)+
	DBRA	D0,lbC0229AC
	RTS

lbC0229B4	ASR.L	#2,D0
	SUBQ.L	#1,D0
lbC0229B8	MOVE.L	(A0)+,(A1)+
	DBRA	D0,lbC0229B8
	RTS

lbC0229C0	SUBQ.L	#1,D0
lbC0229C2	MOVE.B	(A0)+,(A1)+
	DBRA	D0,lbC0229C2
	RTS

;	dc.w	0
;	dc.w	0
;	dc.w	0
;	dc.w	0
;	dc.w	$A5C
;	dc.w	0
;	dc.w	$E239

lbC0229D8	LINK.W	A6,#-2
	MOVE.W	#1,-2(A6)
lbC0229E2	MOVE.W	-2(A6),D0
	CMPI.W	#8,D0
	BGT.S	lbC022A08
	EXT.L	D0
	MOVEQ	#8,D1
	SUB.L	D0,D1
	ASL.L	#2,D1
	MOVEA.L	D1,A0
	ADDA.L	#lbL02345C,A0
	MOVEQ	#1,D1
	ASL.L	D0,D1
	MOVE.L	D1,(A0)
	ADDQ.W	#1,-2(A6)
	BRA.S	lbC0229E2

lbC022A08	UNLK	A6
	RTS

lbC022A0C	LINK.W	A6,#-2
	MOVEM.L	D7,-(SP)
	MOVE.W	10(A6),D7
	MOVE.L	D7,D0
	MOVE.L	D7,D1
	EXT.L	D1
	ASL.L	#2,D1
	MOVEA.L	D1,A0
	ADDA.L	#lbL0234C0,A0
	CMPI.L	#$FFFFFFFF,(A0)
	BNE.S	lbC022A38
	MOVEM.L	(SP)+,D7
	UNLK	A6
	RTS

lbC022A38	MOVE.L	D7,D0
	MOVE.L	D7,D1
	EXT.L	D1
	ASL.L	#2,D1
	MOVEA.L	D1,A0
	ADDA.L	#lbL0234C0,A0
	MOVE.L	(A0),-(SP)
	JSR	lbC021D96
	ADDQ.L	#4,SP
	MOVE.L	D7,D0
	MOVE.L	D7,D1
	EXT.L	D1
	ASL.L	#2,D1
	MOVEA.L	D1,A0
	ADDA.L	#lbL0234C0,A0
	MOVEQ	#-1,D1
	MOVE.L	D1,(A0)
	MOVEM.L	(SP)+,D7
	UNLK	A6
	RTS

lbC022A6E	LINK.W	A6,#-$1C
	MOVEM.L	D2/D3/D5-D7/A2-A5,-(SP)
	MOVE.B	15(A6),D5
	EXT.W	D5
	MOVEQ	#-1,D7
lbC022A7E	ADDQ.W	#1,D7
	SUBI.W	#12,D5
	BGE.S	lbC022A7E
	MOVE.L	D7,D0
	ADDI.W	#12,D5
	MOVE.W	10(A6),D7
	MOVE.L	D7,D1
	MOVE.L	D7,D2
	EXT.L	D2
	ASL.L	#4,D2
	MOVEA.L	D2,A0
	ADDA.L	#lbL02347C,A0
	MOVEA.L	A0,A5
	MOVEA.L	(A5),A0
	MOVEA.L	10(A0),A4
	MOVE.W	D0,-$14(A6)
	EXT.L	D0
	MOVEQ	#0,D2
	MOVE.B	12(A5),D2
	CMP.L	D2,D0
	BLS.S	lbC022AC0
	ANDI.W	#$FF,D2
	MOVE.W	D2,-$14(A6)
lbC022AC0	MOVE.W	-$14(A6),D0
	EXT.L	D0
	MOVEQ	#0,D1
	MOVE.B	13(A5),D1
	CMP.L	D1,D0
	BCC.S	lbC022AD8
	ANDI.W	#$FF,D1
	MOVE.W	D1,-$14(A6)
lbC022AD8	MOVE.W	-$14(A6),D0
	EXT.L	D0
	ASL.L	#2,D0
	MOVEA.L	D0,A0
	ADDA.L	#lbL02345C,A0
	MOVE.L	(A0),D6
	MOVE.W	8(A5),D0
	MULU.W	D6,D0
	MOVE.L	D0,(A4)
	ASL.L	#1,D0
	MOVEA.L	4(A5),A0
	ADDA.L	D0,A0
	MOVE.L	A0,8(A4)
	MOVE.L	D5,D0
	MOVE.L	D5,D1
	EXT.L	D1
	ASL.L	#1,D1
	MOVEA.L	D1,A1
	ADDA.L	#lbL023444,A1
	MOVE.W	(A1),4(A4)
	MOVE.L	D7,D1
	EXT.L	D1
	MOVEA.L	D1,A0
	ADDA.L	#lbL0234BC,A0
	MOVE.B	(A0),D2
	ANDI.W	#$FF,D2
	MOVE.W	D2,6(A4)
	MOVEA.L	(A5),A0
	TST.L	(A0)
	BEQ.S	lbC022B70
	MOVEA.L	(A0),A1
	MOVEA.L	10(A1),A3
	MOVE.W	10(A5),D2
	MULU.W	D6,D2
	MOVE.L	D2,(A3)
	MOVE.L	(A4),D3
	SUB.L	D2,D3
	ASL.L	#1,D3
	MOVEA.L	8(A4),A0
	ADDA.L	D3,A0
	MOVE.L	A0,8(A3)
	MOVE.L	D5,D2
	EXT.L	D2
	ASL.L	#1,D2
	MOVEA.L	D2,A0
	ADDA.L	#lbL023444,A0
	MOVE.W	(A0),4(A3)
	MOVEA.L	D1,A0
	ADDA.L	#lbL0234BC,A0
	MOVE.B	(A0),D1
	ANDI.W	#$FF,D1
	MOVE.W	D1,6(A3)
lbC022B70	TST.B	15(A5)
	BNE.S	lbC022B80
	MOVE.B	14(A5),D0
	CMP.B	15(A6),D0
	BEQ.S	lbC022BEE
lbC022B80	MOVE.L	D7,D0
	EXT.L	D0
	MOVE.L	D0,-(SP)
	BSR.L	lbC022A0C
	ADDQ.L	#4,SP
	MOVE.L	D7,D0
	MOVE.L	D7,D1
	EXT.L	D1
	ASL.L	#2,D1
	MOVEA.L	D1,A0
	ADDA.L	#lbL0234C0,A0
	MOVE.L	D7,D1
	EXT.L	D1
	ASL.L	#2,D1
	MOVEA.L	D1,A1
	ADDA.L	#lbL0238A2,A1
	MOVE.L	D7,D1
	EXT.L	D1
	ASL.L	#2,D1
	MOVEA.L	D1,A2
	ADDA.L	#lbL0238B2,A2
	MOVE.L	A0,$28(SP)
	MOVEA.L	$10(A6),A0
	MOVE.B	6(A0),D1
	EXT.W	D1
	EXT.L	D1
	MOVE.W	4(A0),D2
	EXT.L	D2
	MOVE.L	D2,-(SP)
	MOVE.L	D1,-(SP)
	MOVE.L	#$FFFF,-(SP)
	MOVE.L	(A2),-(SP)
	MOVE.L	(A1),-(SP)
	MOVE.L	(A5),-(SP)
	JSR	lbC021C16
	LEA	$18(SP),SP
	MOVEA.L	$28(SP),A0
	MOVE.L	D0,(A0)
lbC022BEE	MOVE.B	15(A6),14(A5)
	MOVE.L	D7,D0
	EXT.L	D0
	MOVEA.L	D0,A0
	ADDA.L	#lbL0234D0,A0
	MOVE.B	(A0),D0
	ANDI.B	#$40,D0
	MOVE.B	D0,15(A5)
	MOVEM.L	(SP)+,D2/D3/D5-D7/A2-A5
	UNLK	A6
	RTS

lbC022C12	LINK.W	A6,#-2
	CLR.W	-2(A6)
lbC022C1A	MOVE.W	-2(A6),D0
	CMPI.W	#4,D0
	BGE.S	lbC022C34
	EXT.L	D0
	MOVE.L	D0,-(SP)
	BSR.L	lbC022A0C
	ADDQ.L	#4,SP
	ADDQ.W	#1,-2(A6)
	BRA.S	lbC022C1A

lbC022C34	UNLK	A6
	RTS

lbC022C38	LINK.W	A6,#0
	MOVEM.L	A2,-(SP)
	MOVE.W	10(A6),D0
	EXT.L	D0
	ASL.L	#1,D0
	MOVEA.L	D0,A0
	ADDA.L	#lbL0234EC,A0
	MOVEQ	#0,D0
	MOVE.W	D0,(A0)
	MOVE.W	10(A6),D0
	EXT.L	D0
	ASL.L	#1,D0
	MOVEA.L	D0,A0
	ADDA.L	#lbL0234F4,A0
	MOVE.W	(A0),D0
	SUBQ.W	#1,D0
	MOVE.W	D0,(A0)
	TST.W	D0
	BLE.S	lbC022C76
	MOVEM.L	(SP)+,A2
	UNLK	A6
	RTS

lbC022C76	MOVE.W	10(A6),D0
	EXT.L	D0
	ASL.L	#1,D0
	MOVEA.L	D0,A0
	ADDA.L	#lbL0234E4,A0
	MOVE.W	(A0),D1
	ADDQ.W	#1,D1
	MOVE.W	D1,(A0)
	MOVE.W	10(A6),D1
	EXT.L	D1
	ASL.L	#1,D1
	MOVEA.L	D1,A0
	ADDA.L	#lbL0234F4,A0
	MOVEA.L	D0,A1
	ADDA.L	#lbL0234E4,A1
	MOVE.W	(A1),D0
	MULS.W	#6,D0
	MOVEA.L	12(A6),A2
	MOVEA.L	(A2),A1
	ADDA.L	D0,A1
	MOVE.W	(A1),(A0)
	MOVE.W	10(A6),D0
	EXT.L	D0
	ASL.L	#1,D0
	MOVEA.L	D0,A0
	ADDA.L	#lbL0234F4,A0
	MOVE.W	(A0),D0
	TST.W	D0
	BNE.S	lbC022D3C
	MOVE.W	10(A6),D0
	EXT.L	D0
	ASL.L	#1,D0
	MOVEA.L	D0,A0
	ADDA.L	#lbL0234E4,A0
	MOVEQ	#0,D0
	MOVE.W	D0,(A0)
	MOVE.W	10(A6),D0
	EXT.L	D0
	ASL.L	#1,D0
	MOVEA.L	D0,A0
	ADDA.L	#lbL0234F4,A0
	MOVEA.L	(A2),A1
	MOVE.W	(A1),(A0)
	TST.W	10(A6)
	BNE.S	lbC022D3C
	SUBQ.W	#1,lbW023500
	TST.W	lbW023500
	BGT.S	lbC022D3C
	MOVE.B	#1,lbB023440
	BSR.L	lbC022C12
	MOVE.W	#1,lbW02343E

	bsr.w	SongEnd

	MOVE.B	lbB023506,D0
	EXT.W	D0
	EXT.L	D0
	TST.L	D0
	BLE.S	lbC022D3C
	MOVEQ	#1,D1
	ASL.L	D0,D1
	MOVE.L	D1,-(SP)
	MOVE.L	lbL023502,-(SP)
	JSR	lbC023B38
	ADDQ.L	#8,SP
lbC022D3C	MOVEM.L	(SP)+,A2
	UNLK	A6
	RTS

lbC022D44	LINK.W	A6,#-$14
	MOVEM.L	D2/D6/D7/A2,-(SP)
	MOVE.W	10(A6),D7
	EXT.L	D7
lbC022D52	TST.W	lbW02343E
	BNE.L	lbC022E2A
	MOVE.L	D7,D0
	MOVE.L	D7,D1
	ASL.L	#1,D1
	MOVEA.L	D1,A0
	ADDA.L	#lbL0234E4,A0
	MOVE.W	(A0),D2
	MULS.W	#6,D2
	MOVEA.L	12(A6),A1
	MOVEA.L	(A1),A0
	ADDA.L	D2,A0
	MOVEA.L	D1,A2
	ADDA.L	#lbL0234EC,A2
	MOVE.W	(A2),D1
	MOVE.W	D1,$1C(SP)
	ADDQ.W	#1,D1
	MOVE.W	D1,(A2)
	MOVE.W	$1C(SP),D1
	EXT.L	D1
	MOVEA.L	2(A0),A2
	ADDA.L	D1,A2
	MOVE.B	(A2),D6
	MOVE.L	D6,D1
	BTST	#7,D1
	BEQ.S	lbC022DC2
	MOVE.L	D6,D1
	ANDI.B	#15,D1
	TST.B	D1
	BNE.S	lbC022DB6
	MOVE.L	A1,-(SP)
	MOVE.L	D0,-(SP)
	BSR.L	lbC022C38
	ADDQ.L	#8,SP
	BRA.S	lbC022D52

lbC022DB6	MOVEA.L	D7,A0
	ADDA.L	#lbL0234D0,A0
	MOVE.B	D6,(A0)
	BRA.S	lbC022D52

lbC022DC2	MOVE.L	D7,D0
	MOVE.L	D7,D1
	ASL.L	#2,D1
	MOVEA.L	D1,A0
	ADDA.L	#lbL0234D4,A0
	MOVEA.L	D0,A1
	ADDA.L	#lbL0234D0,A1
	MOVE.B	(A1),D1
	EXT.W	D1
	EXT.L	D1
	ANDI.L	#15,D1
	ASL.L	#2,D1
	MOVEA.L	D1,A1
	ADDA.L	#lbW023508,A1
	MOVE.L	(A1),D1
	ADD.L	D1,(A0)
	CMPI.B	#$7F,D6
	BEQ.S	lbC022E2A
	MOVE.B	lbB023507,D0
	ADD.B	D6,D0
	MOVE.L	D0,D6
	BLT.S	lbC022E22
	CMPI.B	#$60,D6
	BGE.S	lbC022E22
	MOVE.L	D6,D0
	EXT.W	D0
	EXT.L	D0
	MOVE.L	12(A6),-(SP)
	MOVE.L	D0,-(SP)
	MOVE.L	D7,-(SP)
	BSR.L	lbC022A6E
	LEA	12(SP),SP
	NOP
lbC022E22	MOVEM.L	(SP)+,D2/D6/D7/A2
	UNLK	A6
	RTS

lbC022E2A	MOVE.L	D7,-(SP)
	BSR.L	lbC022A0C
	ADDQ.L	#4,SP
	MOVEM.L	(SP)+,D2/D6/D7/A2
	UNLK	A6
	RTS

Play
lbC022E3A	ADDQ.L	#1,lbL023438
	MOVE.W	lbW023442,D0
	EXT.L	D0
	MOVEQ	#1,D1
	ASL.L	D0,D1
	MOVE.L	D1,-(SP)
	MOVE.L	lbL0235CE,-(SP)
	JSR	lbC023B38
	ADDQ.L	#8,SP
	MOVEQ	#0,D0
	RTS

lbC022E60	LINK.W	A6,#-14
	MOVEM.L	D6/D7/A4/A5,-(SP)
	MOVE.L	lbL023558,D6
	LEA	lbL0234D4,A5
	MOVEA.L	lbL0234FC,A4
	MOVEQ	#0,D7
	SUBQ.L	#8,A4
lbC022E7E	ADDQ.L	#8,A4
	TST.L	(A4)
	BEQ.S	lbC022EBE
	TST.L	(A5)
	BGT.S	lbC022E98
	MOVE.L	D7,D0
	EXT.L	D0
	MOVE.L	A4,-(SP)
	MOVE.L	D0,-(SP)
	BSR.L	lbC022D44
	ADDQ.L	#8,SP
	BRA.S	lbC022EBE

lbC022E98	MOVE.L	(A5),D0
	CMP.L	D6,D0
	BGT.S	lbC022EBE
	MOVE.L	D7,D0
	EXT.L	D0
	MOVEA.L	D0,A0
	ADDA.L	#lbL0234D0,A0
	MOVE.B	(A0),D0
	BTST	#6,D0
	BEQ.S	lbC022EBE
	MOVE.L	D7,D0
	EXT.L	D0
	MOVE.L	D0,-(SP)
	BSR.L	lbC022A0C
	ADDQ.L	#4,SP
lbC022EBE	MOVEA.L	A5,A0
	ADDQ.L	#4,A5
	SUB.L	D6,(A0)
	ADDQ.W	#1,D7
	CMPI.W	#4,D7
	BLT.S	lbC022E7E
	MOVEM.L	(SP)+,D6/D7/A4/A5
	UNLK	A6
	RTS

lbC022ED4	LINK.W	A6,#-8
	MOVEQ	#-1,D0
	MOVE.L	D0,-(SP)
	JSR	lbC023B50
	ADDQ.L	#4,SP
	MOVE.L	lbL023438,-8(A6)
	MOVE.W	D0,lbW023442
lbC022EF2	MOVE.W	lbW023442,D0
	EXT.L	D0
	MOVEQ	#1,D1
	ASL.L	D0,D1
	MOVE.L	D1,-(SP)
	JSR	lbC023B24
	ADDQ.L	#4,SP
	MOVE.B	#1,lbB023441
	TST.B	lbB023440
	BNE.S	lbC022F62
	MOVE.L	-8(A6),D0
	MOVE.L	lbL023438,D1
	MOVE.L	D1,-8(A6)
	SUB.L	D0,D1
	MOVE.L	D1,-4(A6)
	CMPI.L	#5,D1
	BLE.S	lbC022F3C
	MOVEQ	#5,D0
	MOVE.L	D0,-4(A6)
	BRA.S	lbC022F52

lbC022F3C	MOVE.W	lbW023548,D0
	EXT.L	D0
	MOVE.L	-4(A6),D1
	CMP.L	D0,D1
	BLE.S	lbC022F52
	MOVE.W	D1,lbW023548
lbC022F52	TST.L	-4(A6)
	BEQ.S	lbC022F62
	BSR.L	lbC022E60
	SUBQ.L	#1,-4(A6)
	BRA.S	lbC022F52

lbC022F62	CLR.B	lbB023441
	BRA.S	lbC022EF2

lbC022F6A	MOVE.B	#1,lbB023440
lbC022F72	TST.B	lbB023441
	BNE.S	lbC022F72
	RTS

lbC022F7C	BSR.S	lbC022F6A
	CLR.B	lbB023440
	RTS

lbC022F86	LINK.W	A6,#-4
	MOVE.B	lbB023440,D0
	EXT.W	D0
	EXT.L	D0
	MOVE.L	D0,-4(A6)
	BSR.S	lbC022F6A
	MOVEQ	#9,D0
	MOVE.W	10(A6),D1
	ANDI.L	#$FFFF,D1
	ASL.L	D0,D1
	MOVE.L	D1,D0
	MOVE.L	#$96,D1
	JSR	lbC024158
	MOVE.L	D0,lbL023558
	TST.L	-4(A6)
	BNE.S	lbC022FC4
	BSR.S	lbC022F7C
lbC022FC4	UNLK	A6
	RTS

lbC022FC8	LINK.W	A6,#-10
	MOVEQ	#1,D0
	MOVE.B	D0,lbB023440
	EXT.W	D0
	MOVE.W	D0,lbW02343E
	CLR.B	lbB023441
	MOVEQ	#$38,D0
	MOVE.L	D0,-(SP)
	PEA	lbL0237D2
	JSR	lbC022940
	ADDQ.L	#8,SP
	MOVEQ	#$38,D0
	MOVE.L	D0,-(SP)
	PEA	lbL02383A
	JSR	lbC022940
	ADDQ.L	#8,SP
	MOVE.L	#$200,-(SP)
	PEA	lbL0235D2
	JSR	lbC022940
	ADDQ.L	#8,SP
	MOVEQ	#$5C,D0
	MOVE.L	D0,-(SP)
	PEA	lbL023572
	JSR	lbC022940
	ADDQ.L	#8,SP
;	MOVEQ	#$16,D0
;	MOVE.L	D0,-(SP)
;	PEA	lbL02355C
;	JSR	lbC022940
;	ADDQ.L	#8,SP
	MOVE.L	#lbL0235D2,lbL0235AC
	LEA	lbW0237D0,A0
	MOVE.L	A0,lbL0235A8
	MOVE.L	A0,lbL0235B0
	MOVE.B	#1,lbB02357A
	MOVE.W	lbW02343C,D0
	MOVE.B	D0,lbB02357B
	MOVE.L	#lbL023572,lbL0235CE
	MOVE.L	#lbL0237D2,-6(A6)
	CLR.W	-2(A6)
lbC023084	MOVE.W	-2(A6),D0
	CMPI.W	#4,D0
	BGE.S	lbC0230EA
	EXT.L	D0
	MOVE.L	D0,0(SP)
	ASL.L	#4,D0
	MOVEA.L	D0,A0
	ADDA.L	#lbL02347C,A0
	MOVEA.L	-6(A6),A1
	MOVE.L	A1,(A0)
	MOVE.L	0(SP),D0
	MOVEQ	#12,D1
	JSR	lbC024220
	MOVEA.L	D0,A0
	ADDA.L	#lbL02380A,A0
	MOVE.L	A0,10(A1)
	MOVE.L	0(SP),D0
	ASL.L	#2,D0
	MOVEA.L	D0,A0
	ADDA.L	#lbL0234C0,A0
	MOVEQ	#-1,D0
	MOVE.L	D0,(A0)
	MOVEA.L	0(SP),A0
	ADDA.L	#lbL0234D0,A0
	MOVE.B	#$C1,(A0)
	ADDI.L	#14,-6(A6)
	ADDQ.W	#1,-2(A6)
	BRA.S	lbC023084

lbC0230EA	MOVE.L	#lbL02383A,-6(A6)
	CLR.W	-2(A6)
lbC0230F6	MOVE.W	-2(A6),D0
	CMPI.W	#4,D0
	BGE.S	lbC023128
	MOVEA.L	-6(A6),A0
	MOVE.W	#$FFFE,4(A0)
	MULS.W	#12,D0
	MOVEA.L	D0,A1
	ADDA.L	#lbL023872,A1
	MOVE.L	A1,10(A0)
	ADDI.L	#14,-6(A6)
	ADDQ.W	#1,-2(A6)
	BRA.S	lbC0230F6

lbC023128	BSR.L	lbC0229D8
	MOVE.L	#$4B00,-(SP)
	BSR.L	lbC022F86
	ADDQ.L	#4,SP
	CLR.L	-(SP)
	PEA	lbC022ED4(PC)
	PEA	lbL023572
	JSR	lbC023AC8
	LEA	12(SP),SP
;	MOVEQ	#$16,D0
;	MOVE.L	D0,-(SP)
;	PEA	lbL02355C
;	JSR	lbC022940
;	ADDQ.L	#8,SP
;	LEA	lbC022E3A(PC),A0
;	MOVE.L	A0,lbL02356E
;	MOVE.B	#2,lbB023564
;	PEA	lbL02355C
;	MOVEQ	#5,D0
;	MOVE.L	D0,-(SP)
;	JSR	lbC023A50
;	ADDQ.L	#8,SP
	UNLK	A6
	RTS

lbC023188	LINK.W	A6,#-4
	BSR.L	lbC022F6A
	BSR.L	lbC022C12
;	MOVE.W	lbW02343E,D0
;	TST.W	D0
;	BNE.S	lbC0231B8
;	MOVE.L	lbL023438,-4(A6)
;lbC0231A6	MOVE.L	lbL023438,D0
;	SUB.L	-4(A6),D0
;	CMPI.L	#$40,D0
;	BLT.S	lbC0231A6
;lbC0231B8	PEA	lbL02355C
;	MOVEQ	#5,D0
;	MOVE.L	D0,-(SP)
;	JSR	lbC023A68
;	ADDQ.L	#8,SP
	PEA	lbL023572
	JSR	lbC023AE4
	ADDQ.L	#4,SP
	UNLK	A6
	RTS

lbC0231DC	LINK.W	A6,#-2
	MOVEM.L	A2,-(SP)
	BSR.L	lbC022F6A
	BSR.L	lbC0229D8
	MOVE.L	8(A6),lbL0234FC
	MOVE.W	14(A6),lbW023500
	MOVE.B	$13(A6),lbB023506
	CLR.L	-(SP)
	JSR	lbC023AF8
	ADDQ.L	#4,SP
	MOVEQ	#$10,D1
	MOVE.L	D1,-(SP)
	PEA	lbL0234D4
	MOVE.L	D0,lbL023502
	JSR	lbC022940
	ADDQ.L	#8,SP
	MOVEQ	#8,D0
	MOVE.L	D0,-(SP)
	PEA	lbL0234EC
	JSR	lbC022940
	ADDQ.L	#8,SP
	MOVEQ	#8,D0
	MOVE.L	D0,-(SP)
	PEA	lbL0234E4
	JSR	lbC022940
	ADDQ.L	#8,SP
	CLR.W	-2(A6)
lbC02324E	MOVE.W	-2(A6),D0
	CMPI.W	#4,D0
	BGE.S	lbC0232A2
	EXT.L	D0
	ASL.L	#1,D0
	MOVEA.L	D0,A0
	ADDA.L	#lbL0234F4,A0
	MOVE.W	-2(A6),D0
	EXT.L	D0
	ASL.L	#3,D0
	MOVEA.L	lbL0234FC,A1
	ADDA.L	D0,A1
	MOVEA.L	(A1),A2
	MOVE.W	(A2),(A0)
	MOVE.W	-2(A6),D0
	EXT.L	D0
	ASL.L	#4,D0
	MOVEA.L	D0,A0
	ADDA.L	#lbL02347C,A0
	MOVE.B	#$80,14(A0)
	MOVEA.L	D0,A0
	ADDA.L	#lbL02347C,A0
	MOVE.B	#1,15(A0)
	ADDQ.W	#1,-2(A6)
	BRA.S	lbC02324E

lbC0232A2	CLR.W	lbW02343E
	BSR.L	lbC022F7C
	MOVEM.L	(SP)+,A2
	UNLK	A6
	RTS

lbC0232B4	LINK.W	A6,#0
	MOVE.B	11(A6),lbB023507
	UNLK	A6
	RTS

lbC0232C4	LEA	lbW023508,A0
	MOVE.L	A0,D0
	RTS

lbC0232CE	LINK.W	A6,#-8
	MOVEM.L	A5,-(SP)
	MOVEQ	#0,D0
	MOVE.B	11(A6),D0
	MOVE.L	D0,4(SP)
	ASL.L	#4,D0
	MOVEA.L	D0,A0
	ADDA.L	#lbL02347C,A0
	MOVEA.L	A0,A5
	MOVE.L	4(SP),D0
	ASL.L	#2,D0
	MOVEA.L	D0,A0
	ADDA.L	#lbL0238A2,A0
	MOVE.L	$10(A6),(A0)
	MOVE.L	4(SP),D0
	ASL.L	#2,D0
	MOVEA.L	D0,A0
	ADDA.L	#lbL0238B2,A0
	SUBA.L	A1,A1
	MOVE.L	A1,(A0)
	MOVE.L	12(A6),4(A5)
	MOVEA.L	(A5),A0
	MOVE.L	A1,(A0)
	CLR.B	13(A5)
	CLR.W	10(A5)
	MOVE.W	#1,8(A5)
	MOVEA.L	(A5),A0
	MOVE.W	#$FFFE,4(A0)
	MOVE.B	#7,12(A5)
	MOVEM.L	(SP)+,A5
	UNLK	A6
	RTS

;	LINK.W	A6,#-12
;	MOVEM.L	A5,-(SP)
;	MOVEQ	#0,D0
;	MOVE.B	11(A6),D0
;	MOVE.L	D0,4(SP)
;	ASL.L	#4,D0
;	MOVEA.L	D0,A0
;	ADDA.L	#lbL02347C,A0
;	MOVEA.L	A0,A5
;	MOVE.L	4(SP),D0
;	ASL.L	#2,D0
;	MOVEA.L	D0,A0
;	ADDA.L	#lbL0238A2,A0
;	MOVE.L	$14(A6),(A0)
;	MOVE.L	4(SP),D0
;	ASL.L	#2,D0
;	MOVEA.L	D0,A0
;	ADDA.L	#lbL0238B2,A0
;	MOVE.L	$18(A6),(A0)
;	MOVEA.L	$10(A6),A0
;	MOVE.B	6(A0),D0
;	MOVEQ	#9,D1
;	SUB.B	D0,D1
;	MOVE.B	D1,12(A5)
;	MOVE.B	7(A0),D0
;	MOVEQ	#9,D1
;	SUB.B	D0,D1
;	MOVE.B	D1,13(A5)
;	MOVE.W	2(A0),D0
;	MOVE.W	D0,8(A5)
;	MOVEA.L	(A5),A0
;	MOVE.W	#1,4(A0)
;	MOVEA.L	$10(A6),A1
;	MOVE.W	2(A1),D0
;	MOVE.W	4(A1),D1
;	SUB.W	D1,D0
;	MOVE.W	D0,10(A5)
;	MOVE.L	A0,8(SP)
;	BEQ.S	lbC0233DA
;	MOVE.L	4(SP),D0
;	MOVEQ	#14,D1
;	JSR	lbC024220
;	MOVEA.L	D0,A1
;	ADDA.L	#lbL02383A,A1
;	BRA.S	lbC0233DC

;lbC0233DA	SUBA.L	A1,A1
;lbC0233DC	MOVEA.L	8(SP),A0
;	MOVE.L	A1,(A0)
;	MOVEQ	#0,D0
;	MOVEA.L	$10(A6),A0
;	MOVE.B	6(A0),D0
;	SUBQ.L	#1,D0
;	MOVEQ	#1,D1
;	ASL.L	D0,D1
;	MOVEQ	#0,D0
;	MOVE.W	2(A0),D0
;	JSR	lbC0241E8
;	ASL.L	#1,D0
;	MOVEA.L	12(A6),A0
;	SUBA.L	D0,A0
;	MOVE.L	A0,4(A5)
;	MOVEM.L	(SP)+,A5
;	UNLK	A6
;	RTS

;	LINK.W	A6,#0
;	MOVEQ	#0,D0
;	MOVE.B	11(A6),D0
;	MOVEA.L	D0,A0
;	ADDA.L	#lbL0234BC,A0
;	MOVE.B	15(A6),(A0)
;	UNLK	A6
;	RTS

;	dc.l	0
;	dc.l	$11C
;	dc.l	$E281
lbL023438	dc.l	0
lbW02343C	dc.w	6
lbW02343E	dc.w	1
lbB023440	dc.b	1
lbB023441	dc.b	0
lbW023442	dc.w	0
lbL023444	dc.l	$140012E
	dc.l	$11D010D
	dc.l	$FE00F0
	dc.l	$E200D6
	dc.l	$CA00BE
	dc.l	$B400AA
lbL02345C	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
lbL02347C	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
lbL0234BC	dc.l	$1010101
lbL0234C0	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
lbL0234D0	dc.l	0
lbL0234D4	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
lbL0234E4	dc.l	0
	dc.l	0
lbL0234EC	dc.l	0
	dc.l	0
lbL0234F4	dc.l	0
	dc.l	0
lbL0234FC	dc.l	0
lbW023500	dc.w	0
lbL023502	dc.l	0
lbB023506	dc.b	0
lbB023507	dc.b	0
lbW023508	dc.w	1
	dc.w	0
	dc.w	$60
	dc.w	0
	dc.w	$30
	dc.w	0
	dc.w	$18
	dc.w	0
	dc.w	12
	dc.w	0
	dc.w	6
	dc.w	0
	dc.w	3
	dc.w	0
	dc.w	2
	dc.w	0
	dc.w	$48
	dc.w	0
	dc.w	$24
	dc.w	0
	dc.w	$12
	dc.w	0
	dc.w	9
	dc.w	0
	dc.w	$20
	dc.w	0
	dc.w	$10
	dc.w	0
	dc.w	8
	dc.w	0
	dc.w	4
	dc.w	0
lbW023548	dc.w	0
;	dc.w	0
;	dc.w	0
;	dc.w	0
;	dc.w	0
;	dc.w	$374
;	dc.w	0
;	dc.w	$E35F
lbL023558	dc.l	0
;lbL02355C	dc.l	0
;	dc.l	0
;lbB023564	dc.b	0
;	dc.b	0
;	dc.b	0
;	dc.b	0
;	dc.b	0
;	dc.b	0
;	dc.b	0
;	dc.b	0
;	dc.b	0
;	dc.b	0
;lbL02356E	dc.l	0
lbL023572	dc.l	0
	dc.l	0
lbB02357A	dc.b	0
lbB02357B	dc.b	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
lbL0235A8	dc.l	0
lbL0235AC	dc.l	0
lbL0235B0	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.w	0
lbL0235CE	dc.l	0
lbL0235D2	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.w	0
lbW0237D0	dc.w	0
lbL0237D2	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
lbL02380A	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
lbL02383A	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
lbL023872	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
lbL0238A2	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
lbL0238B2	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
;	dc.l	$C40000
;	dc.w	$E391

lbC0238D0	MOVEA.L	4(SP),A1
	MOVEM.L	A5/A6,-(SP)
	JSR	lbC0238E4
	MOVEM.L	(SP)+,A5/A6
	RTS

lbC0238E4	MOVEA.L	0(A1),A0
	BRA.S	lbC0238F0

lbC0238EA	RTS

lbC0238EC	MOVEA.L	0(A0),A0
lbC0238F0	TST.L	(A0)
	BEQ.S	lbC0238EA
	MOVEA.L	$16(A0),A1
	MOVE.L	0(A1),D0
	BEQ.S	lbC0238EC
	BGE.S	lbC02390E
	ADD.L	14(A0),D0
	BVS.S	lbC02392C
	CMP.L	4(A1),D0
	BGT.S	lbC02391A
	BRA.S	lbC02393A

lbC02390E	ADD.L	14(A0),D0
	BVS.S	lbC02394C
	CMP.L	4(A1),D0
	BGE.S	lbC02395A
lbC02391A	MOVE.L	D0,14(A0)
lbC02391E	MOVE.L	$12(A0),D1
	BEQ.S	lbC0238EC
	SWAP	D0
	MOVEA.L	D1,A1
	MOVE.W	D0,(A1)
	BRA.S	lbC0238EC

lbC02392C	MOVE.L	#$80000000,D0
	BRA.S	lbC02393A

lbC023934	CMP.L	4(A1),D0
	BGT.S	lbC02396E
lbC02393A	MOVEQ	#8,D1
	ADD.L	D1,$16(A0)
	MOVEA.L	$16(A0),A1
	MOVE.L	0(A1),D1
	BLT.S	lbC023934
	BRA.S	lbC02396A

lbC02394C	MOVE.L	#$7FFFFFFF,D0
	BRA.S	lbC02395A

lbC023954	CMP.L	4(A1),D0
	BLT.S	lbC02396E
lbC02395A	MOVEQ	#8,D1
	ADD.L	D1,$16(A0)
	MOVEA.L	$16(A0),A1
	MOVE.L	0(A1),D1
	BGT.S	lbC023954
lbC02396A	MOVE.L	-4(A1),D0
lbC02396E	MOVE.L	D0,14(A0)
	TST.L	D1
	BNE.S	lbC02391E
	MOVE.L	$1A(A0),D1
	BEQ.S	lbC023980
	MOVE.L	D1,$16(A0)
lbC023980	MOVE.L	$1E(A0),D1
	BEQ.S	lbC02391E
	BRA.L	lbC02391E


;lbC023A50	MOVE.L	A6,-(SP)
;	MOVEM.L	8(SP),D0/A1
;	MOVEA.L	lbL00006C,A6
;	JSR	-$A8(A6)
;	MOVEA.L	(SP)+,A6
;	RTS

;lbC023A68	MOVE.L	A6,-(SP)
;	MOVEM.L	8(SP),D0/A1
;	MOVEA.L	lbL00006C,A6
;	JSR	-$AE(A6)
;	MOVEA.L	(SP)+,A6
;	RTS

lbC023A80	MOVE.L	A6,-(SP)
	MOVEM.L	8(SP),D0/D1
	MOVEA.L	lbL00006C,A6
	JSR	-$C6(A6)
	MOVEA.L	(SP)+,A6
	RTS

lbC023A98	MOVE.L	A6,-(SP)
	MOVEA.L	8(SP),A1
	MOVE.L	12(SP),D0
	MOVEA.L	lbL00006C,A6
	JSR	-$D2(A6)
	MOVEA.L	(SP)+,A6
	RTS

lbC023AB0	MOVE.L	A6,-(SP)
	MOVEM.L	8(SP),A0/A1
	MOVEA.L	lbL00006C,A6
	JSR	-$F0(A6)
	MOVEA.L	(SP)+,A6
	RTS

lbC023AC8	MOVEM.L	A2/A3/A6,-(SP)
	MOVEM.L	$10(SP),A1-A3
	MOVEA.L	lbL00006C,A6
	JSR	-$11A(A6)
	MOVEM.L	(SP)+,A2/A3/A6
	RTS

lbC023AE4	MOVE.L	A6,-(SP)
	MOVEA.L	8(SP),A1
	MOVEA.L	lbL00006C,A6
	JSR	-$120(A6)
	MOVEA.L	(SP)+,A6
	RTS

lbC023AF8	MOVE.L	A6,-(SP)
	MOVEA.L	8(SP),A1
	MOVEA.L	lbL00006C,A6
	JSR	-$126(A6)
	MOVEA.L	(SP)+,A6
	RTS

lbC023B24	MOVE.L	A6,-(SP)
	MOVE.L	8(SP),D0
	MOVEA.L	lbL00006C,A6
	JSR	-$13E(A6)
	MOVEA.L	(SP)+,A6
	RTS

lbC023B38	MOVE.L	A6,-(SP)
	MOVEA.L	8(SP),A1
	MOVE.L	12(SP),D0
	MOVEA.L	lbL00006C,A6
	JSR	-$144(A6)
	MOVEA.L	(SP)+,A6
	RTS

lbC023B50	MOVE.L	A6,-(SP)
	MOVE.L	8(SP),D0
	MOVEA.L	lbL00006C,A6
	JSR	-$14A(A6)
	MOVEA.L	(SP)+,A6
	RTS

lbC023B64	MOVE.L	A6,-(SP)
	MOVE.L	8(SP),D0
	MOVEA.L	lbL00006C,A6
	JSR	-$150(A6)
	MOVEA.L	(SP)+,A6
	RTS

lbC023BA0	MOVE.L	A6,-(SP)
	MOVEM.L	8(SP),A0/A1
	MOVEA.L	lbL00006C,A6
	JSR	-$16E(A6)
	MOVEA.L	(SP)+,A6
	RTS

lbC023BB8	MOVE.L	A6,-(SP)
	MOVEA.L	8(SP),A0
	MOVEA.L	lbL00006C,A6
	JSR	-$174(A6)
	MOVEA.L	(SP)+,A6
	RTS

lbC023BCC	MOVE.L	A6,-(SP)
	MOVEA.L	8(SP),A1
	MOVEA.L	lbL00006C,A6
	JSR	-$17A(A6)
	MOVEA.L	(SP)+,A6
	RTS

lbC023BE0	MOVE.L	A6,-(SP)
	MOVEA.L	8(SP),A0
	MOVEA.L	lbL00006C,A6
	JSR	-$180(A6)
	MOVEA.L	(SP)+,A6
	RTS

lbC023C0C	MOVE.L	A6,-(SP)
	MOVEA.L	8(SP),A0
	MOVEM.L	12(SP),D0/A1
	MOVE.L	$14(SP),D1
	MOVEA.L	lbL00006C,A6
	JSR	-$1BC(A6)
	MOVEA.L	(SP)+,A6
	RTS


lbC023C2C	MOVE.L	A6,-(SP)
	MOVEA.L	8(SP),A1
	MOVEA.L	lbL00006C,A6
	JSR	-$1C2(A6)
	MOVEA.L	(SP)+,A6
	RTS


lbC023C78	MOVEA.L	4(SP),A1
	MOVE.L	A6,-(SP)
	MOVEA.L	$14(A1),A6
	JSR	-$1E(A6)
	MOVEA.L	(SP)+,A6
	RTS

lbC023CA0	MOVEA.L	4(SP),A0
	MOVE.L	A0,(A0)
	ADDQ.L	#4,(A0)
	CLR.L	4(A0)
	MOVE.L	A0,8(A0)
	RTS


lbC024158	MOVE.L	D2,-(SP)
	MOVE.L	D3,-(SP)
	TST.L	D1
	BEQ.S	lbC024182
	TST.L	D0
	BEQ.S	lbC024180
	CLR.L	D2
	MOVEQ	#$1F,D3
lbC024168	ASL.L	#1,D0
	ROXL.L	#1,D2
	CMP.L	D1,D2
	BCS.S	lbC024178
	SUB.L	D1,D2
	ADD.L	#1,D0
lbC024178	DBRA	D3,lbC024168
	MOVE.L	D2,D1
	BRA.S	lbC024184

lbC024180	CLR.L	D1
lbC024182	CLR.L	D0
lbC024184	MOVE.L	(SP)+,D3
	MOVE.L	(SP)+,D2
	RTS

lbC024198	MOVEM.L	D2-D5,-(SP)
	MOVE.L	D1,D5
	BEQ.S	lbC0241D2
	BPL.S	lbC0241A4
	NEG.L	D1
lbC0241A4	MOVE.L	D0,D4
	BEQ.S	lbC0241D0
	BPL.S	lbC0241AC
	NEG.L	D0
lbC0241AC	CLR.L	D2
	MOVEQ	#$1F,D3
lbC0241B0	ASL.L	#1,D0
	ROXL.L	#1,D2
	CMP.L	D1,D2
	BCS.S	lbC0241BC
	SUB.L	D1,D2
	ADDQ.L	#1,D0
lbC0241BC	DBRA	D3,lbC0241B0
	MOVE.L	D2,D1
	EOR.L	D4,D5
	BPL.S	lbC0241C8
	NEG.L	D0
lbC0241C8	EOR.L	D1,D4
	BPL.S	lbC0241D4
	NEG.L	D1
	BRA.S	lbC0241D4

lbC0241D0	CLR.L	D1
lbC0241D2	CLR.L	D0
lbC0241D4	MOVEM.L	(SP)+,D2-D5
	RTS

lbC0241E8	MOVEM.L	D1-D3,-(SP)
	MOVE.L	D0,D2
	BEQ.S	lbC024210
	TST.L	D1
	BNE.S	lbC0241F8
	CLR.L	D0
	BRA.S	lbC024210

lbC0241F8	MOVE.L	D0,D3
	MULU.W	D1,D3
	SWAP	D2
	MULU.W	D1,D2
	SWAP	D2
	CLR.W	D2
	ADD.L	D2,D3
	SWAP	D1
	MULU.W	D1,D0
	SWAP	D0
	CLR.W	D0
	ADD.L	D3,D0
lbC024210	MOVEM.L	(SP)+,D1-D3
	RTS

lbC024220	MOVEM.L	D1-D4,-(SP)
	MOVE.L	D0,D4
	EOR.L	D1,D4
	TST.L	D0
	BEQ.S	lbC02425C
	BPL.S	lbC024230
	NEG.L	D0
lbC024230	MOVE.L	D0,D2
	TST.L	D1
	BNE.S	lbC02423A
	CLR.L	D0
	BRA.S	lbC02425C

lbC02423A	BPL.S	lbC02423E
	NEG.L	D1
lbC02423E	MOVE.L	D0,D3
	MULU.W	D1,D3
	SWAP	D2
	MULU.W	D1,D2
	SWAP	D2
	CLR.W	D2
	ADD.L	D2,D3
	SWAP	D1
	MULU.W	D1,D0
	SWAP	D0
	CLR.W	D0
	ADD.L	D3,D0
	TST.L	D4
	BPL.S	lbC02425C
	NEG.L	D0
lbC02425C	MOVEM.L	(SP)+,D1-D4
	RTS

lbL00006C
	dc.l	0

lbL02AE50
	incbin	ram:Samples
Ar_End
