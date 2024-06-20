*-----------------------------------------------------------------------------*
*				PlayAY					      *
*-----------------------------------------------------------------------------*
Test		= 0
MSG		= 1
SongNr		= 0
Deli		= 1

	if	Test
TT		lea	data,a0
		bsr	Check2
		bne.s	.Error
		bsr	Initplayer
		bne.s	.Error
		bsr	InitSound
.wait:		cmp.b	#$ff,$dff006
		bne.s	.wait
		bsr	interrupt
		btst	#6,$bfe001
		bne.s	.wait
		bsr	EndSound
.Error:		move.w	#15,$dff096
		bsr	EndPlayer
		illegal
	endc

			incdir	include:
			include	exec/exec_lib.i
			include	dos/dos_lib.i
			include	misc/eagleplayer.i
			include	"libraries/reqtools_lib.i"
			include	"libraries/reqtools.i"

		Playerheader	Tags


		rsreset
AY_Heat		rs.b	4
AC_APTRToHeat	rs.l	1
AC_Name		rs.l	1			;4 Zeichen
AC_Unknown1	rs.l	1
AC_Merk1	rs.l	1
AC_Merk2	rs.l	1
AC_Merk3	rs.l	1
AYC_InitPlayer	rs.w	1
AYC_EndPlayer	rs.w	1
AYC_InitSound	rs.w	1
AYC_EndSound	rs.w	1
AYC_Interrupt	rs.w	1
AYC_NextPattern	rs.w	1
AYC_PrevPattern	rs.w	1
AYC_FormatName	rs.b	0
AYC_SizeOF	rs.b	0			;42

		rsreset
EagleBase	rs.l	1
MsgPort		rs.l	1
ModuleName	rs.l	1
FormatName	rs.l	1
Chk_Data	rs.l	1
Chk_Player	rs.l	1
		rs.w	1
		rs.w	1
MaxVolume	rs.w	1
Wert		rs.l	1
		rs.w	1
		rs.l	9
PufferEnd	rs.b	0			=72

		dc.b	'$VER: AY emulator 2.1 (20 Dec 96)',0
		even

Tags:		dc.l	DTP_PlayerVersion,2<<16!1
		dc.l	DTP_PlayerName,PlayAY.MSG
		dc.l	DTP_Creator,Creatorname
		dc.l	DTP_Process,Prozessstart
		dc.l	DTP_StackSize,$1000
		dc.l	DTP_MsgPort,BSSMerkPuffer+MsgPort
		ifeq	Deli
		dc.l	EP_EagleBase,BSSMerkPuffer+EagleBase
		dc.l	EP_Check5,Check
		dc.l	EP_Flags,EPB_SongEnd!EPB_NextPatt!EPB_PrevPatt
		else
		dc.l	DTP_DeliBase,BSSMerkPuffer+EagleBase
		dc.l	DTP_Flags,6
		dc.l	DTP_Check2,Check
		dc.l	DTP_RequestDTVersion,$11
		endc
		dc.l	DTP_Config,Config
		dc.l	DTP_NewSubSongRange,SubSongRange
		dc.l	DTP_InitPlayer,InitPlayer
		dc.l	DTP_EndPlayer,EndPlayer
		dc.l	DTP_InitSound,InitSound
		dc.l	DTP_EndSound,EndSound
		dc.l	DTP_Interrupt,Interrupt
		
		
; XXX this is NOT in the binary!!! but UADE tries to call this..		
		dc.l	DTP_Volume,Volume
NextPatternTag:	dc.l	Tag_Ignore,NextPattern
PrevPatternTag:	dc.l	Tag_Ignore,PrevPattern
		dc.l	DTP_ModuleName,BSSMerkPuffer+ModuleName
		dc.l	DTP_FormatName,BSSMerkPuffer+FormatName
		dc.l	0


*-----------------------------------------------------------------------------*
Check:		move.l	DTG_ChkData(a5),a0
Check2:		if	Test
		lea	BSSMerkPuffer(pc),a4
		lea	EaglePuffer(pc),a5
		move.l	a0,DTG_ChkData(a5)
		lea	MY_Dosname(pc),a1
		move.l	4,a6
		moveq	#37,d0
		jsr	_LVOOpenLibrary(a6)
		move.l	d0,DTG_Dosbase(a5)
		bsr	Config
		bsr	Volume
		move.l	DTG_ChkData(a5),a0
		endc		

		move.l	a0,a3
		lea	10(a0),a2
		cmpi.l	#"ZXAY",(a0)+
		bne.l	Error
		move.l	(a0)+,d6		;Kennung fuer Player
		move.w	(a0),d5			;Kennung fuer Player
		move.l	PlayerListAdr(pc),d2
		tst.w	(a2)
		beq.s	.ListIsEmpty
		add.w	(a2),a2
		bra.s	.lbC00010E

	*--- Es wird die Liste nach schon geladenen Playern durchsucht ---*
.ListIsEmpty:	move.l	d2,a2
		move.l	(a2),d2
		beq.s	LoadAPlayer
.lbC00010E:	lea	8(a2),a0
		bsr.s	CheckPlayer
		bne.s	.ListIsEmpty
lbC000116:	lea	Formatname(a4),a0
		lea	$2A(a2),a1
		move.l	a1,(a0)+			;Formatname
		move.l	a3,(a0)+			;Module
		move.l	a2,(a0)				;Start des Players
		move.l	#$10203,Wert(a4)
		moveq	#0,d0
		rts

CheckPlayer:	cmp.l	(a0)+,d6		; e.g. "AMAD"
		bne.s	.Return
		cmp.b	(a0)+,d5
		bhi.s	.Return
		cmp.b	(a0)+,d5
		bcs.s	.Return
		cmpi.b	#1,(a0)
.Return:	rts

*-----------------------------------------------------------------------------*
LoadAPlayer:	lea	PlayAY_Playerpfad(pc),a0
		move.l	d6,PlayerKuerzel-PlayAY_PlayerPfad(a0)
		move.l	a0,d1
		move.l	DTG_DOSBase(a5),a6
		jsr	_LVOLoadSeg(a6)
		add.l	d0,d0
		beq.s	Error
		add.l	d0,d0
		addq.l	#4,d0
			; d0 now points to data of the CODE segment
		move.l	d0,a0
		move.l	d0,a2
		cmpi.l	#$70004E75,(a0)+		; signatur des dummy programs "return -1"
		bne.s	UnloadSeg
		cmpa.l	(a0)+,a2			; check back-pointer
		bne.s	UnloadSeg
		bsr.s	CheckPlayer
		bne.s	UnloadSeg
		move.l	a2,a1
		lea	PlayerListAdr(pc),a0
		move.l	4,a6
		jsr	_LVOAddHead(a6)
		bra.s	lbC000116

UnloadSeg:	move.l	a2,d1
		subq.l	#4,d1
		lsr.l	#2,d1
		jsr	_LVOUnLoadSeg(a6)
Error:		moveq	#-1,d0
Return:		rts

*-----------------------------------------------------------------------------*
Prozessstart:	if	MSG
		lea	ProzessStart.MSG,a2
		bsr	DisplayMSG
		endc

		lea	BSSMerkPuffer(pc),a4
		move.l	EagleBase(a4),a5
		move.l	4,a6
		
; XXXX where is the bloody code that sends the messages? 		
		
Wait:		moveq	#-1,d0
		jsr	_LVOWait(a6)
		move.l	d0,-(sp)
HoleMsg:	move.l	MsgPort(a4),a0
		jsr	_LVOGetMsg(a6)
		tst.l	d0
		beq.s	NoMSg
		movem.l	d0/a4-a6,-(sp)
		move.l	d0,a0
		move.l	DTMN_Function(a0),a0
		jsr	(a0)			;Springe in Tag rein
		movem.l	(sp)+,a1/a4-a6
		move.l	d0,DTMN_Result(a1)
		jsr	_LVOReplyMsg(a6)
		bra.s	HoleMsg

NoMSg:		move.l	(sp)+,d0
		btst	#12,d0			;Break-Signal ?
		beq.s	Wait
		move.l	DTG_Dosbase(a5),a6
		move.l	PlayerListAdr(pc),d7
EjectPlayers:	move.l	d7,a2
		move.l	(a2),d7
		beq.s	Return
		bsr.s	UnloadSeg
		bra.s	EjectPlayers

PlayerListAdr:	dc.l	PlayerList
PlayerList:	dc.l	0
		dc.l	PlayerListAdr

*-----------------------------------------------------------------------------*
Config:		if	MSG
		lea	Config.MSG,a2
		bsr	DisplayMSG

		sub.l	a1,a1
		move.l	4,a6
		jsr	_LVOFindTask(a6)
		move.l	d0,a2
		bsr	DisplayMSG
		endc

		lea	lbL000528,a0
		move.l	#$64649C9C,(a0)+
		move.w	#$3FFF,d2
		moveq	#0,d3
.MachWas:	move.l	d3,d0
		move.l	d0,d1
		asl.l	#3,d1
		sub.l	d0,d1
		asl.l	#3,d1
		add.l	d0,d1
		add.l	d1,d1
		add.l	d0,d1
		asl.l	#4,d1
		sub.l	d0,d1
		add.l	d1,d1
		sub.l	d0,d1
		addi.l	#$E90,d0
		lsl.w	#4,d0
		add.l	d0,d1
		bclr	#$1F,d1
		subq.l	#1,d1
		move.l	d1,d3
		lsr.w	#8,d1
		move.b	d1,(a0)+
		dbra	d2,.MachWas
		moveq	#0,d0
		rts

*-----------------------------------------------------------------------------*
Volume:		*lea	BSSMerkPuffer(pc),a4
		ifeq	Test
		move.w	DTG_SndVol(a5),MaxVolume(a4)
		else
		move.w	#64,MaxVolume(a4)
		endc
		rts

*-----------------------------------------------------------------------------*
InitPlayer:	ifeq	Test
		ifeq	Deli
		jsr	ENPP_AllocAudio(a5)
		tst.l	d0
		bne.s	AllocAudioFail
		endc
		endc

		move.l	Chk_Data(a4),a0

		lea	SubSongRange(pc),a1
		move.b	$11(a0),1(a1)
		move.b	$10(a0),5(a1)
		lea	Chk_Player(a4),a0
		move.l	(a0)+,a1
		move.l	a1,(a0)				;Nodenkopf setzen

		lea	15(a1),a1
		moveq	#0,d0
		move.b	(a1)+,d0
		add.w	d0,d0
		lea	$3A(a4),a0
		move.l	a0,(a1)+
		lea	Wert(a4),a0
		move.l	a0,(a1)+
		lea	lbW0003F2(pc),a0
		add.w	d0,a0
		move.l	a0,(a1)
		moveq	#AYC_InitPlayer,d0		;$1c
		bsr.s	Sprung
		tst.l	d0
		bne.s	lbC000290

		*-- NextPattern setzen --*
		moveq	#AYC_NextPattern,d1
		move.l	$18(a4),a2		;Playerstart (incl Liste) !!!
		add.l	d1,a2
		tst.w	(a2)
		beq.s	.NoNext
		move.l	#DTP_NextPatt,NextPatternTag
.NoNext:	moveq	#AYC_PrevPattern,d1
		move.l	$18(a4),a2		;Playerstart (incl Liste) !!!
		add.l	d1,a2
		tst.w	(a2)
		beq.w	.NoPrev
.NoPrev:	move.l	#DTP_PrevPatt,PrevPatternTag
		moveq	#0,d0
		rts

*-----------------------------------------------------------------------------*
EndPlayer:	moveq	#$1E,d0
		bsr.s	Sprung
lbC000290:	ifeq	Test
		ifeq	Deli
		jsr	ENPP_FreeAudio(a5)
		endc
		endc
AllocAudioFail:	moveq	#-1,d0
		move.l	#Tag_Ignore,NextPatternTag
		move.l	#Tag_Ignore,PrevPatternTag
		rts

lbL00029A:	dc.l	$DFF0A0
		dc.l	$DFF0B0
		dc.l	$DFF0C0
		dc.l	$DFF0D0

*-----------------------------------------------------------------------------*
NextPattern:	moveq	#AYC_NextPattern,d0		;$26
		bra.s	Sprung

*-----------------------------------------------------------------------------*
PrevPattern:	moveq	#AYC_PrevPattern,d0		;$28
Sprung:		
		move.l	$18(a4),a2		;Playerstart (incl Liste) !!!
		add.l	d0,a2		; e.g. zeigt auf "initSound" offset des add-ons
		move.w	(a2),d0			; lädt den offset
		beq.s	.Return
		add.l	d0,a2			; addiert offset und zeigt damit auf die "initSound" routine
		movem.l	a4-a6,-(sp)
		jsr	(a2)
		movem.l	(sp)+,a4-a6
.Return:	rts

*-----------------------------------------------------------------------------*
InitSound:	moveq	#0,d0
;		ifeq	Test
;		move.w	DTG_SndNum(a5),d0
;		else
		moveq	#SongNr,d0
;		endc
		
		move.w	d0,d1
		move.l	$10(a4),a0
		lea	$12(a0),a0
		add.w	(a0),a0
		add.w	d0,d0
		add.w	d0,d0
		add.w	d0,a0
		move.l	a0,a1
		add.w	(a0)+,a1
		move.l	a1,8(a4)
		add.w	(a0),a0
		
		
; an offset +$20 des add-on players befindet sich ein word-offset zur InitSound routine 
; des add-ons..	
		moveq	#AYC_InitSound,d0			;$20
		bsr.s	Sprung
		lea	$22(a4),a1
		lea	$1E(a4),a0
		moveq	#3,d0
.Schleif:	moveq	#0,d1
		move.b	(a0)+,d1
		move.w	d1,d2
		add.w	d2,d2
		add.w	d2,d2
		move.l	lbL00029A(pc,d2.w),(a1)+
		move.w	d1,(a1)+
		dbra	d0,.Schleif
		rts

*-----------------------------------------------------------------------------*
Interrupt:	movem.l	d2-d7/a2-a6,-(sp)
		lea	BSSMerkPuffer(pc),a4
		moveq	#AYC_Interrupt,d0			;$24
		bsr.s	Sprung
		tst.l	d0
		beq.s	lbC000326
		move.l	EagleBase(a4),a5
		ifeq	Test
		ifeq	Deli
		jsr	ENPP_SongEnd(a5)
		endc
		endc
lbC000326:	bsr.s	lbC000338
		movem.l	(sp)+,d2-d7/a2-a6
		rts

*-----------------------------------------------------------------------------*
EndSound:	moveq	#AYC_EndSound,d0		;$22
		bsr.l	Sprung
		st	$41(a4)
lbC000338:	lea	$41(a4),a4
		move.w	#$8200,d3
		moveq	#0,d4
		move.b	(a4),d0
		lea	-$25(a4),a0
		lea	-9(a4),a1
		lea	(a4),a2
		moveq	#2,d5
lbC000350:	addq.l	#6,a0
		addq.l	#2,a1
		addq.l	#1,a2
		bsr.s	lbC0003B8
		dbra	d5,lbC000350
		move.w	d4,d1
		beq.s	lbC000386
		move.l	-13(a4),a3
		move.l	#lbL00052C,(a3)
		move.w	#$2000,4(a3)
		bsr.s	lbC000398
		move.b	-1(a4),d1
		asl.w	#5,d1
		addi.w	#$7C,d1
		move.w	d1,6(a3)
		move.w	-9(a4),d1
		bset	d1,d3
lbC000386:	lea	$DFF096,a0
		move.w	d3,(a0)
		not.w	d3
		andi.w	#15,d3
		move.w	d3,(a0)
		rts

lbC000398:	move.b	lbL0003A8(pc,d1.w),d1
		mulu.w	-$25(a4),d1
		lsr.l	#6,d1
		move.w	d1,8(a3)
		rts

lbL0003A8:	dc.l	$10202
		dc.l	$303090F
		dc.w	$151B
		dc.w	$2127
		dc.w	$2D33
		dc.w	$393F

lbC0003B8:	move.b	(a2),d1
		andi.w	#15,d1
		beq.s	lbC0003EE
		btst	#3,d0
		bne.s	lbC0003CC
		cmp.w	d4,d1
		bcs.s	lbC0003CC
		move.w	d1,d4
lbC0003CC:	btst	#0,d0
		bne.s	lbC0003EE
		move.l	(a0),a3
		move.w	(a1),6(a3)
		beq.s	lbC0003EE
		bsr.s	lbC000398
		move.l	#lbL000528,(a3)
		move.w	#2,4(a3)
		move.w	4(a0),d1
		bset	d1,d3
lbC0003EE:	lsr.w	#1,d0
		rts

lbW0003F2:	dc.w	$FE3B
		dc.w	$EFF6
		dc.w	$E27E
		dc.w	$D5C8
		dc.w	$C9C8
		dc.w	$BE75
		dc.w	$B3C4
		dc.w	$A9AD
		dc.w	$A027
		dc.w	$972A
		dc.w	$8EAE
		dc.w	$86AC
		dc.w	$7F1D
		dc.w	$77FB
		dc.w	$713F
		dc.w	$6AE4
		dc.w	$64E4
		dc.w	$5F3A
		dc.w	$59E2
		dc.w	$54D7
		dc.w	$5014
		dc.w	$4B95
		dc.w	$4757
		dc.w	$4356
		dc.w	$3F8F
		dc.w	$3BFD
		dc.w	$38A0
		dc.w	$3572
		dc.w	$3272
		dc.w	$2F9D
		dc.w	$2CF1
		dc.w	$2A6B
		dc.w	$280A
		dc.w	$25CB
		dc.w	$23AC
		dc.w	$21AB
		dc.w	$1FC7
		dc.w	$1DFF
		dc.w	$1C50
		dc.w	$1AB9
		dc.w	$1939
		dc.w	$17CF
		dc.w	$1679
		dc.w	$1536
		dc.w	$1405
		dc.w	$12E5
		dc.w	$11D6
		dc.w	$10D6
		dc.w	$FE4
		dc.w	$EFF
		dc.w	$E28
		dc.w	$D5C
		dc.w	$C9D
		dc.w	$BE7
		dc.w	$B3C
		dc.w	$A9B
		dc.w	$A02
		dc.w	$973
		dc.w	$8EB
		dc.w	$86B
		dc.w	$7F2
		dc.w	$780
		dc.w	$714
		dc.w	$6AE
		dc.w	$64E
		dc.w	$5F4
		dc.w	$59E
		dc.w	$54D
		dc.w	$501
		dc.w	$4B9
		dc.w	$475
		dc.w	$435
		dc.w	$3F9
		dc.w	$3C0
		dc.w	$38A
		dc.w	$357
		dc.w	$327
		dc.w	$2FA
		dc.w	$2CF
		dc.w	$2A7
		dc.w	$281
		dc.w	$25D
		dc.w	$23B
		dc.w	$21B
		dc.w	$1FC
		dc.w	$1E0
		dc.w	$1C5
		dc.w	$1AC
		dc.w	$194
		dc.w	$17D
		dc.w	$168
		dc.w	$153
		dc.w	$140
		dc.w	$12E
		dc.w	$11D
		dc.w	$10D
		dc.w	$FE
		dc.w	$F0
		dc.w	$E2
		dc.w	$D6
		dc.w	$CA
		dc.w	$BE
		dc.w	$B4
		dc.w	$AA
		dc.w	$A0
		dc.w	$97
		dc.w	$8F
		dc.w	$87
		dc.w	$7F

	if	MSG


*-----------------------------------------------------------------------*
*---------------------------- Subroutines ------------------------------*
*-----------------------------------------------------------------------*
*-----------------------------------------------------------------------*
DisplayMsg:	movem.l	d1-a6,-(sp)

		lea	MyReqToolsName(pc),a1
		move.l	4,a6
		moveq	#37,d0
		jsr	_LVOOpenLibrary(a6)
		move.l	d0,a6
		tst.l	d0
		beq.s	.NoReq

		*---- ReqTools-Request ----*
		* Output: d0=Nummer des Gadgets
		*	  0 = ganz rechts
		*	  1 = von links nach rechts
		lea	TextRequestTags(pc),a0	;Tags für Textrequest
		move.l	a2,a1			;Text
		lea	Gadgettexte(pc),a2		;Gadgetlist
		sub.l	a4,a4			;APTR to Arglist
		sub.l	a3,a3			;Reqtoolsinfo
		jsr	_LVOrtEZRequestA(a6)

		move.l	a6,a1
		move.l	4,a6
		jsr	_LVOCloseLibrary(a6)

.NoReq:		movem.l	(sp)+,d1-a6
		moveq	#0,d0
		rts


TextRequestTags:
		dc.l	_RT_Underscore,"_"
		dc.l	RTEZ_ReqTitle,MessageTitle
		dc.l	0


Gadgettexte:	dc.b	"_Ok",0
MessageTitle:	dc.b	"Testplayer-Message",0
MyReqToolsName:	dc.b	"reqtools.library",0
Prozessstart.MSG:dc.b	"Prozessstart",0
Config.MSG:	dc.b	"Config",0
		even
TextStruct:
	dc.l	0					; ^Text
	dc.l	0					; ^ParamList
	dc.l	0					; ^Window
	dc.l	0					; ^MiddleText
	dc.l	0					; ^PositiveText
	dc.l	OKTxt					; ^NegativeText
	dc.l	TitleTxt				; ^Title
	dc.w	$ffff					; KeyMask
	dc.w	0					; textcol
	dc.w	0					; detailcol
	dc.w	0					; blockcol
	dc.w	0					; versionnumber
	dc.w	0					; Timeout
	dc.l	0					; abortmask
	dc.l	0					; reserved

OKTxt:		dc.b	'  OK  ',0
TitleTxt:	dc.b	'TestPlayer-Information',0
	endc

*-----------------------------------------------------------------------------*
SubSongRange:	dc.w	0,0,0

BSSMerkPuffer:	ds.b	PufferEnd
;EagleBase:	ds.l	1
;MsgPortAdr:	ds.l	1
;ModuleName:	ds.l	1
;FormatName:	ds.l	15

		if	Test
EaglePuffer	ds.b	EPG_Sizeof
		endc

PlayAY_Playerpfad:
		ifeq	Test
		dc.b	'EP:Eagleplayers/AYPlayers/ZXAY'
PlayerKuerzel:	dc.b	0,0,0,0,0
		else
		dc.b	'EP:Eagleplayers/AYPlayers/ZXAY'
PlayerKuerzel:	dc.b	0,0,0,0,0
My_Dosname:	dc.b	"dos.library",0
		endc		
PlayAY.MSG:	dc.b	'PlayAY',0
Creatorname:	dc.b	'AY-3-8912 Emulator interface (C) 1992-1995 Raxoft',10
		dc.b	"Eagleplayeradaption by DEFECT",0,0

	SECTION	PlayAYrs000528,BSS_C
lbL000528:	ds.l	1
lbL00052C:	ds.l	$1000


	if	Test
	Section	Module,code_c
data:		incdir	"modules:playsid/AY/AMAD/"
		*incbin	amad.aztec                    
		incbin	amad.explatoms                
		*incbin	amad.falcon                   
		*incbin	amad.popcorn                  

		*incdir	"Modules:Playsid/AY/emul"
		*incbin	emul.180                      
		*incbin	emul.BionicCommando           
		*incbin	emul.Blasteroids              
		*incbin	emul.ButcherHill              
		*incbin	emul.CombatSchool             
		*incbin	emul.DeathStalker             
		*incbin	emul.DragonNinja              
		*incbin	emul.dragonslair2             
		*incbin	emul.FastFood                 
		*incbin	emul.Flintstones              
		*incbin	emul.FoxFightsBack            
		*incbin	emul.Gauntlet2                
		*incbin	emul.gliderrider              
		*incbin	emul.Gryzor                   
		*incbin	emul.Hotshot                  
		*incbin	emul.LinekerSuperSkills       
		*incbin	emul.OperationWolf            
		*incbin	emul.Outrun                   
		*incbin	emul.Pacmania                 
		*incbin	emul.PeterPackRat             
		*incbin	emul.Platoon                  
		*incbin	emul.ProSkateboardSim         
		*incbin	emul.Rambo3                   
		*incbin	emul.Renegade3                
		*incbin	emul.Robocop                  
		*incbin	emul.RunTheGauntlet           
		*incbin	emul.Saboteur2                
		*incbin	emul.SkateCrazy               
		*incbin	emul.Spellbound               
		*incbin	emul.StarPaws                 
		*incbin	emul.TechnoCop                
		*incbin	emul.Terramex                 
		*incbin	emul.TFF4-Demo                
		*incbin	emul.Thundercats              
		*incbin	emul.TigerRoad                
		*incbin	emul.Typhoon                  
		*incbin	emul.venom                    
		*incbin	emul.Vindicator               
		*incbin	emul.Vixen                    
		*incbin	emul.WECLeMans                
		*incbin	emul.Xenon                    
		*incbin	emul.Zub                      
	endc

