	*****************************************************
	****       MaxTrax replayer for EaglePlayer, 	 ****
	****	     all adaptions by Wanted Team	 ****
	****      DeliTracker compatible (?) version	 ****
	*****************************************************

	incdir	"dh2:include/"
	include 'misc/eagleplayer2.01.i'

	SECTION	Player_Code,CODE

	PLAYERHEADER Tags

	dc.b	'$VER: MaxTrax player module V1.1 (20 Sep 2007)',0
	even
Tags
	dc.l	DTP_PlayerVersion,2
	dc.l	EP_PlayerVersion,9
	dc.l	DTP_RequestDTVersion,DELIVERSION
	dc.l	DTP_PlayerName,PlayerName
	dc.l	DTP_Creator,Creator
	dc.l	DTP_Check2,Check2
	dc.l	DTP_ExtLoad,ExtLoad
	dc.l	DTP_SubSongRange,SubSongRange
	dc.l	DTP_InitPlayer,InitPlayer
	dc.l	DTP_EndPlayer,EndPlayer
	dc.l	DTP_InitSound,InitSound
	dc.l	DTP_EndSound,EndSound
	dc.l	DTP_Interrupt,Interrupt
	dc.l	DTP_Volume,SetVolume
	dc.l	DTP_Balance,SetBalance
	dc.l	EP_Voices,SetVoices
	dc.l	EP_Get_ModuleInfo,GetInfos
	dc.l	EP_StructInit,StructInit
	dc.l	EP_GetPositionNr,GetPosition
	dc.l	EP_SampleInit,SampleInit
	dc.l	EP_Flags,EPB_Volume!EPB_Balance!EPB_ModuleInfo!EPB_Voices!EPB_SampleInfo!EPB_Songend!EPB_Analyzer!EPB_NextSong!EPB_PrevSong!EPB_Packable!EPB_Restart
	dc.l	0

PlayerName
	dc.b	'MaxTrax',0
Creator
	dc.b	"(c) 1991-93 by David 'Talin' Joiner",10
	dc.b	'& Joe Pearce, adapted by Wanted Team',0
Prefix
	dc.b	'MXTX.',0
Smpl
	dc.b	'SMPL.',0
SampleName
	dc.b	'SMPL.set',0
MainName
	dc.b	'MXTX.main',0
	even
EagleBase
	dc.l	0
TwoFiles
	dc.w	0
FileEnd
	dc.l	0
RightVolume
	dc.w	64
LeftVolume
	dc.w	64
StructAdr
	ds.b	UPS_SizeOF

***************************************************************************
***************************** DTP_EndSound ********************************
***************************************************************************

EndSound
	bra.w	StopSong

***************************************************************************
***************************** DTP_Interrupt *******************************
***************************************************************************

Interrupt
	movem.l	D1-D7/A0-A6,-(A7)

	lea	StructAdr(PC),A0
	st	UPS_Enabled(A0)
	clr.w	UPS_Voice1Per(A0)
	clr.w	UPS_Voice2Per(A0)
	clr.w	UPS_Voice3Per(A0)
	clr.w	UPS_Voice4Per(A0)
	move.w	#UPSB_Adr!UPSB_Len!UPSB_Per!UPSB_Vol,UPS_Flags(A0)

	bsr.w	MusicServer

	lea	StructAdr(PC),A0
	clr.w	UPS_Enabled(A0)

	movem.l	(A7)+,D1-D7/A0-A6
	moveq	#0,D0
	rts

SongEnd
	movem.l	A1/A5,-(A7)
	move.l	EagleBase(PC),A5
	move.l	dtg_SongEnd(A5),A1
	jsr	(A1)
	movem.l	(A7)+,A1/A5
	rts

DMAWait
	movem.l	D0/D1,-(SP)
	moveq	#8,D0
.dma1	move.b	$DFF006,D1
.dma2	cmp.b	$DFF006,D1
	beq.b	.dma2
	dbeq	D0,.dma1
	movem.l	(SP)+,D0/D1
	rts

***************************************************************************
******************************* EP_SampleInit *****************************
***************************************************************************

SampleInit
	moveq	#EPR_NotEnoughMem,D7
	lea	EPG_SampleInfoStructure(A5),A3
	lea	SamplesTable,A2

	move.l	InfoBuffer+Samples(PC),D5
	subq.l	#1,D5
hop
	jsr	ENPP_AllocSampleStruct(A5)
	move.l	D0,(A3)
	beq.b	return
	move.l	D0,A3

	move.l	8(A2),D0
	add.l	12(A2),D0
	move.l	D0,EPS_Length(A3)		; sample length
	move.l	4(A2),EPS_Adr(A3)		; sample address
	move.l	#64,EPS_Volume(A3)
	move.w	#USITY_RAW,EPS_Type(A3)
	move.w	#USIB_Playable!USIB_Saveable!USIB_8BIT,EPS_Flags(A3)
	lea	16(A2),A2
	dbf	D5,hop

	moveq	#0,D7
return
	move.l	D7,D0
	rts

***************************************************************************
********************************* EP_GetPosNr *****************************
***************************************************************************

GetPosition
	moveq	#0,D0
	move.l	_globaldata+glob_Current,D1
	beq.b	Zero
	move.l	_globaldata+glob_CurrentScore,A0
	sub.l	(A0),D1
	divu.w	#24*4,D1
	move.w	D1,D0
Zero
	rts

***************************************************************************
******************************* EP_StructInit *****************************
***************************************************************************

StructInit
	lea	StructAdr(PC),A0
	rts

***************************************************************************
***************************** EP_Get_ModuleInfo ***************************
***************************************************************************

GetInfos
	lea	InfoBuffer(PC),A0
	rts

SubSongs	=	4
LoadSize	=	12
Songsize	=	20
Length		=	28
Samples		=	36
SamplesSize	=	44
Calcsize	=	52

InfoBuffer
	dc.l	MI_SubSongs,0		;4
	dc.l	MI_LoadSize,0		;12
	dc.l	MI_Songsize,0		;20
	dc.l	MI_Length,0		;28
	dc.l	MI_Samples,0		;36
	dc.l	MI_SamplesSize,0	;44
	dc.l	MI_Calcsize,0		;52
	dc.l	MI_MaxSubSongs,256
	dc.l	MI_MaxSamples,64
	dc.l	MI_Prefix,Prefix
	dc.l	0

***************************************************************************
******************************* DTP_Check2 ********************************
***************************************************************************

Check2
	movea.l	dtg_ChkData(A5),A0
	moveq	#-1,D0

	move.l	dtg_ChkSize(A5),D1
	lea	(A0,D1.L),A1
	cmp.l	#'MXTX',(A0)+
	bne.b	Fault
	tst.w	(A0)+
	beq.b	Fault
	bmi.b	Fault
	tst.w	(A0)+
	bpl.b	NoMicro				; no microtonal?
	lea	256(A0),A0
NoMicro
	move.w	(A0)+,D1			; songs number
	beq.b	Fault
	cmp.w	#256,D1
	bhi.b	Fault
	subq.w	#1,D1
SongCheck
	move.l	(A0)+,D2
	beq.b	Fault
	bmi.b	Fault
	mulu.w	#6,D2
	add.l	D2,A0
	cmp.l	A0,A1
	ble.b	Fault
	dbf	D1,SongCheck
	cmp.w	#64,(A0)
	bhi.b	Fault
	lea	TwoFiles(PC),A1
	move.w	(A0),(A1)
	moveq	#0,D0
Fault
	rts

***************************************************************************
******************************* DTP_ExtLoad *******************************
***************************************************************************

ExtLoad
	moveq	#0,D0
	move.w	TwoFiles(PC),D1
	bne.b	ExtLoadOK
	movea.l	dtg_PathArrayPtr(A5),A0
	clr.b	(A0)
	movea.l	dtg_CopyDir(A5),A0
	jsr	(A0)
	bsr.s	CopyName
	movea.l	dtg_LoadFile(A5),A0
	jsr	(A0)
	tst.l	D0
	beq.b	ExtLoadOK
	move.l	dtg_PathArrayPtr(A5),A0
	clr.b	(A0)
	move.l	dtg_CopyDir(A5),A0
	jsr	(A0)
	lea	SampleName(PC),A3
	bsr.b	CopyName2
	move.l	dtg_LoadFile(A5),A0
	jsr	(A0)
	tst.l	D0
	beq.b	ExtLoadOK
	move.l	dtg_PathArrayPtr(A5),A0
	clr.b	(A0)
	move.l	dtg_CopyDir(A5),A0
	jsr	(A0)
	lea	MainName(PC),A3
	bsr.b	CopyName2
	move.l	dtg_LoadFile(A5),A0
	jsr	(A0)
ExtLoadOK
	rts

CopyName2
	movea.l	dtg_PathArrayPtr(A5),A0
loop1
	tst.b	(A0)+
	bne.s	loop1
	subq.l	#1,A0
smp2
	move.b	(A3)+,(A0)+
	bne.s	smp2
	rts

CopyName
	move.l	dtg_PathArrayPtr(A5),A0
loop	tst.b	(A0)+
	bne.s	loop
	subq.l	#1,A0
	move.l	dtg_FileArrayPtr(A5),A1
	move.l	A0,A3
copy	move.b	(A1)+,(A0)+
	bne.s	copy
	lea	Smpl(PC),A1
	move.b	(A1)+,(A3)+
	move.b	(A1)+,(A3)+
	move.b	(A1)+,(A3)+
	move.b	(A1)+,(A3)+
	rts

***************************************************************************
***************************** DTP_SubSongRange ****************************
***************************************************************************

SubSongRange
	moveq	#0,D0
	move.l	InfoBuffer+SubSongs(PC),D1
	subq.l	#1,D1
	rts

***************************************************************************
***************************** DTP_InitPlayer ******************************
***************************************************************************

InitPlayer
	move.l	4.W,_SysBase
	lea	Buffy,A0
	lea	BuffyEnd,A1
ClearBuffy
	clr.w	(A0)+
	cmp.l	A0,A1
	bne.b	ClearBuffy

	bsr.w	InitMusic

	moveq	#0,D0
	movea.l	dtg_GetListData(A5),A0
	jsr	(A0)

	move.l	A5,EagleBase

	lea	InfoBuffer(PC),A4	; A4 reserved for InfoBuffer
	move.l	D0,LoadSize(A4)
	lea	(A0,D0.L),A1
	move.l	A1,FileEnd
	move.w	8(A0),D0
	tst.w	6(A0)
	bpl.b	NoMicky			; no microtonal ?
	move.w	8+256(A0),D0
NoMicky
	move.w	D0,SubSongs+2(A4)

	bsr.w	LoadPerf			; install score

	lea	InfoBuffer(PC),A4	; A4 reserved for InfoBuffer
	move.l	A2,D0
	addq.l	#2,D0
	move.l	D0,D7
	sub.l	A0,D0
	move.l	D0,Songsize(A4)
	move.l	D0,Calcsize(A4)

	tst.w	(A2)
	bne.b	OneFile

	move.l	EagleBase(PC),A5
	moveq	#1,D0
	movea.l	dtg_GetListData(A5),A0
	jsr	(A0)

	add.l	D0,LoadSize(A4)
	move.l	A0,D7
	lea	(A0,D0.L),A1
	move.l	A1,FileEnd

	move.l	A0,A2
	cmp.l	#'MXTX',(A2)+
	bne.b	Error
	tst.w	(A2)+
	beq.b	Error
	bmi.b	Error
	tst.w	(A2)+
	bpl.b	NoMick				; no microtonal?
	lea	256(A2),A2
NoMick
	move.w	(A2)+,D1
	beq.b	OneFile
	cmp.w	#256,D1
	bhi.b	Error
	subq.w	#1,D1
SongSkip
	move.l	(A2)+,D2
	beq.b	Error
	bmi.b	Error
	mulu.w	#6,D2
	add.l	D2,A2
	dbf	D1,SongSkip
	cmp.w	#64,(A2)
	bhi.b	Error
OneFile
	move.w	(A2),Samples+2(A4)
	move.l	FileEnd(PC),D3

	bsr.w	INSTALLSAMPLES			; install samples

	lea	InfoBuffer(PC),A4	; A4 reserved for InfoBuffer
	move.l	A2,D0
	sub.l	D7,D0
	move.l	D0,SamplesSize(A4)
	add.l	D0,Calcsize(A4)

	bra.w	OpenMusic			; audio device

Error
	moveq	#EPR_ErrorInFile,D0
	rts

***************************************************************************
***************************** DTP_EndPlayer *******************************
***************************************************************************

EndPlayer
	bsr.w	CloseMusic
	moveq	#0,D0
	rts

***************************************************************************
***************************** DTP_InitSound *******************************
***************************************************************************

InitSound
	lea	StructAdr(PC),A0
	move.l	A0,A2
	lea	UPS_SizeOF(A0),A1
ClearUPS
	clr.w	(A0)+
	cmp.l	A0,A1
	bne.b	ClearUPS
	move.b	#15,UPS_DMACon+1(A2)		; for Delitracker

	lea	right_round(PC),A0
	clr.w	(A0)

	move.l	#PAL_CLOCKS,D0
	cmp.w	#$3770,dtg_Timer(A5)
	blt.b	PALTimer
	move.l	#NTSC_CLOCKS,D0
PALTimer
	lea	_globaldata,A0
	move.l	D0,glob_ColorClocks(A0)

	move.w	dtg_SndNum(A5),D0
	bsr.w	SelectScore
	lea	InfoBuffer(PC),A1
	move.l	_globaldata+glob_CurrentScore,A0
	move.l	4(A0),D1
	lsr.l	#4,D1
	addq.l	#1,D1
	move.l	D1,Length(A1)
	moveq	#0,D0
	cmp.w	#1,D1
	beq.w	PlaySong
	bra.w	LoopSong

***************************************************************************
************************* DTP_Volume, DTP_Balance *************************
***************************************************************************
; Copy Volume and Balance Data to internal buffer

SetVolume
SetBalance
	move.w	dtg_SndLBal(A5),D0
	mulu.w	dtg_SndVol(A5),D0
	lsr.w	#6,D0				; durch 64
	move.w	D0,LeftVolume

	move.w	dtg_SndRBal(A5),D0
	mulu.w	dtg_SndVol(A5),D0
	lsr.w	#6,D0				; durch 64
	move.w	D0,RightVolume			; Right Volume
	rts

; taken from MSXS player by perfect Gaelan G.

ChangeVolume
* D4.W - Volume
* D0.W - Channel number
	BTST	D0,#%0110
	BNE.B	.RIGHT
	MULU.W	LeftVolume(PC),D4
	BRA.B	.DONE
.RIGHT	MULU.W	RightVolume(PC),D4
.DONE	LSR.W	#6,D4
	RTS

*------------------------------- Set Both -------------------------------*

SetBoth
	move.l	A0,-(A7)
	lea	StructAdr+UPS_Voice1Vol(PC),A0
	tst.w	D0
	beq.s	.SetVoice
	lea	StructAdr+UPS_Voice2Vol(PC),A0
	cmp.w	#1,D0
	beq.s	.SetVoice
	lea	StructAdr+UPS_Voice3Vol(PC),A0
	cmp.w	#2,D0
	beq.s	.SetVoice
	lea	StructAdr+UPS_Voice4Vol(PC),A0
.SetVoice
	move.w	D4,(A0)
	move.w	D1,-2(A0)
	move.l	(A7)+,A0
	rts

*------------------------------- Set Two -------------------------------*

SetTwo
	movem.l	D1/A1,-(A7)
	lea	StructAdr+UPS_Voice1Adr(PC),A1
	tst.w	D0
	beq.s	.SetVoice
	lea	StructAdr+UPS_Voice2Adr(PC),A1
	cmp.w	#1,D0
	beq.s	.SetVoice
	lea	StructAdr+UPS_Voice3Adr(PC),A1
	cmp.w	#2,D0
	beq.s	.SetVoice
	lea	StructAdr+UPS_Voice4Adr(PC),A1
.SetVoice
	move.l	samp_Waveform(A0),(A1)
	move.l	samp_AttackSize(A0),D1
	lsr.l	#1,D1
	move.w	D1,UPS_Voice1Len(A1)
	movem.l	(A7)+,D1/A1
	rts

***************************************************************************
**************************** EP_Voices ************************************
***************************************************************************

SetVoices
	lea	StructAdr(PC),A1
	move.w	D0,UPS_DMACon(A1)	;Stimme an = Bit gesetzt
					;Bit 0 = Kanal 1 usw.
	moveq	#0,D0
	rts

***************************************************************************
****************************** MaxTrax player *****************************
***************************************************************************

*==========================================================================*
*   MaxTrax Music Player - audio device handler 			   *
*   Copyright 1991 Talin (David Joiner) & Joe Pearce                       *
*                                                                          *
*   This library is free software; you can redistribute it and/or          *
*   modify it under the terms of the GNU Lesser General Public             *
*   License as published by the Free Software Foundation; either           *
*   version 2.1 of the License, or (at your option) any later version.     *
*                                                                          *
*   This library is distributed in the hope that it will be useful,        *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
*    Library General Public License for more details.                      *
*                                                                          *
*   You should have received a copy of the GNU Lesser General Public       *
*   License along with this library; if not, write to the Free             *
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA               *
*	02111-1307 USA.                                                    *
*                                                                          *
*   Contact information:                                                   *
*   The Wyrmkeep Entertainment Co.                                         *
*   Attn: Joe Pearce                                                       *
*   P. O. Box 1585                                                         *
*   Costa Mesa, CA 92628-1585                                              *
*   www.wyrmkeep.com                                                       *
*==========================================================================*

*	To do: Use volume of envelope to break ties when picking voices.

	INCLUDE 'exec/exec_lib.i'
;	INCLUDE 'dos/dos_lib.i'

;			include 'exec/types.i'
;			include 'exec/execbase.i'
;			include 'exec/ports.i'
;			include 'exec/lists.i'
;			include 'exec/memory.i'
			include 'devices/audio.i'
;			include 'graphics/gfxbase.i'
;			include 'hardware/intbits.i'
			include 'hardware/custom.i'
;			include 'dos/dosextens.i'

;			include 'driver.i'

*==========================================================================*
*   MaxTrax Music Player - audio device handler include file               *
*   Copyright 1991 Talin (David Joiner) & Joe Pearce                       *
*                                                                          *
*   This library is free software; you can redistribute it and/or          *
*   modify it under the terms of the GNU Lesser General Public             *
*   License as published by the Free Software Foundation; either           *
*   version 2.1 of the License, or (at your option) any later version.     *
*                                                                          *
*   This library is distributed in the hope that it will be useful,        *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
*    Library General Public License for more details.                      *
*                                                                          *
*   You should have received a copy of the GNU Lesser General Public       *
*   License along with this library; if not, write to the Free             *
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA               *
*	02111-1307 USA.                                                        *
*                                                                          *
*   Contact information:                                                   *
*   The Wyrmkeep Entertainment Co.                                         *
*   Attn: Joe Pearce                                                       *
*   P. O. Box 1585                                                         *
*   Costa Mesa, CA 92628-1585                                              *
*   www.wyrmkeep.com                                                       *
*==========================================================================*

* various features enabled in the driver

HAS_MODULATION		equ		0
;HAS_MICROTONAL		equ		0
;HAS_ALLMIDI			equ		0
HAS_FULLCHANVOL		equ		0
;IN_MUSICX			equ		0
;FASTSOUND			equ		0

* various limits on this version of the driver (all power of two)

NUM_SAMPLES			equ		64
NUM_PATCHES			equ		64
NUM_VOICES			equ		4
NUM_CHANNELS			equ		16
NUM_SCORES			equ		8*32
NUM_REPEATS			equ		4

NUM_PATCHES_MASK	equ		(64-1)

PERIOD_THRESH		equ		288					; sample period threshold
PERIOD_ENTRIES		equ		((288*3)+11)		; size of period table

NTSC_CLOCKS			equ		3579546				; Color clock values
PAL_CLOCKS			equ		3546895

K_VALUE				equ		$09fd77
PREF_PERIOD			equ		$08fd77
PERIOD_LIMIT		equ		$06f73d

MEV_ERR_NONE		equ		0					; just was it says			
MEV_ERR_NO_MEMORY	equ		1					; ran out of memory		
MEV_ERR_AUDIODEV	equ		2					; couldn't open audio dev	

		STRUCTURE	StopEvent,0
			BYTE	sev_Command
			BYTE	sev_Data
			LONG	sev_StopTime
			LABEL	sev_sizeof

		STRUCTURE	CookedEvent,0
			BYTE	cev_Command
			BYTE	cev_Data
			WORD	cev_StartTime
			WORD	cev_StopTime
			LABEL	cev_sizeof

		STRUCTURE	EnvelopeData,0
			WORD	env_Duration				; duration in milliseconds	
			WORD	env_Volume					; volume of envelope		
			LABEL	env_sizeof

		STRUCTURE	SampleData,0
			APTR	samp_NextSample
			APTR	samp_Waveform
			LONG	samp_AttackSize
			LONG	samp_SustainSize
			LABEL	samp_sizeof

		STRUCTURE	PatchData,0
			APTR	patch_Sample				; Amiga sample data		
			APTR	patch_Attack				; array of env. segments	
			APTR	patch_Release				; array of env. segments	
			WORD	patch_AttackCount			; number of attack env.	
			WORD	patch_ReleaseCount			; number of release env.	
			WORD	patch_Volume				; sample volume 			
			WORD	patch_Tune					; sample tuning			
			BYTE	patch_Number				; self-identifing			
			BYTE	patch_pad
			LABEL	patch_sizeof

		STRUCTURE	VoiceData,0
			APTR	voice_Channel
			APTR	voice_Patch					; patch playing on channel	
			APTR	voice_Envelope				; in patch data 			
			LONG	voice_UniqueID				; sampled sound playing	
			LONG	voice_LastTicks				; last total tick value	
			LONG	voice_TicksLeft				; left in envelope	segment	
			LONG	voice_PortaTicks			; portamento timing		
			LONG	voice_IncrVolume			; incremental volume		
			LONG	voice_PeriodOffset
;		ifne FASTSOUND
;			APTR	voice_CurFastIOB			; current fast iob playing
;			APTR	voice_NextFastIOB			; next fast iob to play
;			APTR	voice_FastBuffer			; pointer to buffer area
;		endc
			WORD	voice_EnvelopeLeft			; segments left in env.	
			WORD	voice_NoteVolume			; note attack volume		
			WORD	voice_BaseVolume			; base volume of segment	
			WORD	voice_LastPeriod			; last calculated period	
			BYTE	voice_BaseNote				; note we are playing		
			BYTE	voice_EndNote				; portamento note			
			BYTE	voice_Number				; self-identifying			
			BYTE	voice_Link					; stereo sampled sound		
			BYTE	voice_Priority				; priority of this voice	
			BYTE	voice_Status				; envelope status			
			BYTE	voice_Flags
			BYTE	voice_LastVolume
			LABEL	voice_sizeof

voice_FastSizeLeft	equ		voice_PeriodOffset
voice_FastSample	equ		voice_Envelope
voice_FastVolume	equ		voice_BaseVolume
voice_FastPeriod	equ		voice_LastPeriod
voice_FastNextBuf	equ		voice_EndNote

VOICE_STOLEN		equ		(1<<0)				; voice was stolen			
VOICE_PORTAMENTO	equ		(1<<1)				; portamento playing		
VOICE_DAMPER		equ		(1<<2)				; note still on via damper 
VOICE_BLOCKED		equ		(1<<3)				; sample player using this	
VOICE_RECALC		equ		(1<<4)

VOICEB_STOLEN		equ		0
VOICEB_PORTAMENTO	equ		1
VOICEB_DAMPER		equ		2
VOICEB_BLOCKED		equ		3
VOICEB_RECALC		equ		4

ENV_FREE			equ		0			; voice not in use					
ENV_HALT			equ		1			; don't just kill the sample		
ENV_DECAY			equ		2			; in release envelope section		
ENV_RELEASE			equ		3			; enter release envelope section	
ENV_SUSTAIN			equ		4			; finished attack envelopes		
ENV_ATTACK			equ		5			; in attack envelope section		
ENV_START			equ		6			; hasn't had envelope processing	

		STRUCTURE	ChannelData,0
			APTR	chan_Patch					; patch on this channel	
			WORD	chan_RPN					; registered parameter #	
	ifne HAS_MODULATION
			WORD	chan_Modulation				; modulation level			
			WORD	chan_ModulationTime			; modulation time (msec)	
	endc
;	ifne HAS_MICROTONAL
;			WORD	chan_Microtonal				; -1 or next utonal adjust 
;	endc
			WORD	chan_PortamentoTime			; portamento time (msec)	
			WORD	chan_PitchBend				; current pitch bend value	
			WORD	chan_RealBend				; real pitch bend value	
			BYTE	chan_PitchBendRange			; pitch bend sensitivity	
			BYTE	chan_Volume					; volume					
			BYTE	chan_VoicesActive			; count of voices active	
			BYTE	chan_Number					; self identifing			
			BYTE	chan_Flags					; pan, etc.
			BYTE	chan_LastNote				; last note released on channel
			BYTE	chan_Program				; used by Music-X
			BYTE	chan_pad
			LABEL	chan_sizeof

CHAN_PAN			equ		(1<<0)				; 0 = left, 1 = right		
CHAN_MONO			equ		(1<<1)				; MONO mode, not POLY		
CHAN_PORTAMENTO		equ		(1<<2)				; portamento active		
CHAN_DAMPER			equ		(1<<3)				; "damper pedal" down		
CHAN_MICROTONAL		equ		(1<<4)				; use microtonal table
CHAN_MODTYPE		equ		(1<<5)				; period or volume modulation
CHAN_ALTERED		equ		(1<<7)				; channel params altered	

CHANB_PAN			equ		0
CHANB_MONO			equ		1
CHANB_PORTAMENTO	equ		2
CHANB_DAMPER		equ		3
CHANB_MICROTONAL	equ		4
CHANB_MODTYPE		equ		5
CHANB_ALTERED		equ		7

NO_BEND				equ		(64<<7)				; centered bend wheel val	
MAX_BEND_RANGE		equ		24					; 24 semitones				

		STRUCTURE	GlobalData,0
			LONG	glob_UniqueID				; for sampled sounds		
			LONG	glob_ColorClocks			; system color clock value	
			LONG	glob_Ticks					; current tick value		
			LONG	glob_TickUnit				; tick unit				
			LONG	glob_FrameUnit				; in fixed-point ms		
			LONG	glob_TempoTime				; time of tempo change		
			LONG	glob_TempoTicks				; tempo change counter		
	ifne HAS_MODULATION
			LONG	glob_SineValue				; for modulation affects	
	endc
			APTR	glob_SyncTask				; video sync values		
			LONG	glob_SyncSig
			LONG	glob_CurrentTime			; current event time		
			APTR	glob_CurrentScore
			APTR	glob_Current				; current event			
			STRUCT	glob_RepeatPoint,4*NUM_REPEATS	; repeat points		
			STRUCT	glob_NoteOff,sev_sizeof*NUM_VOICES	; stop event bufs
			WORD	glob_Frequency				; driver frequency			
			WORD	glob_TotalScores			; total number of scores	
			WORD	glob_Volume					; programmatic volume		
			WORD	glob_Filter					; global start filter		
			WORD	glob_Tempo					; global start tempo		
			WORD	glob_CurrentTempo			; current tempo			
			WORD	glob_StartTempo				; start tempo if continuous
			WORD	glob_DeltaTempo				; delta tempo if continuous
			STRUCT	glob_RepeatCount,NUM_REPEATS	; times to repeat
			BYTE	glob_RepeatTotal			; total repeats currently	
			BYTE	glob_VoicesActive			; number of voice used		
			BYTE	glob_LastVoice				; last voice affected		
			BYTE	glob_Flags
			BYTE	glob_SaveFilter
;	ifne HAS_ALLMIDI
;			BYTE	glob_RunningStatus			; running status (if used)	
;	else
			BYTE	glob_pad
;	endc
			LABEL	glob_sizeof

MUSIC_PLAYING		equ		(1<<0)				; events being handled		
MUSIC_ADDED_NOTE	equ		(1<<1)				; NOTE ON did add a note	
MUSIC_SILENT		equ		(1<<2)				; no voices playing		
MUSIC_VELOCITY		equ		(1<<3)				; handle attack velocity	
MUSIC_LOOP			equ		(1<<4)				; loop the song			
MUSIC_PLAYNOTE		equ		(1<<5)				; active PlayNote

MUSICB_PLAYING		equ		0
MUSICB_ADDED_NOTE	equ		1
MUSICB_SILENT		equ		2
MUSICB_VELOCITY		equ		3
MUSICB_LOOP			equ		4
MUSICB_PLAYNOTE		equ		5

MUSIC_PRI_SCORE		equ		0
MUSIC_PRI_NOTE		equ		1
MUSIC_PRI_SOUND		equ		2

		STRUCTURE	MaxTraxInfo,0
			WORD	mxtx_TotalScores			; total number of scores
			BYTE	mxtx_Volume					; programmatic volume
			BYTE	mxtx_SyncValue				; last sync value
			BYTE	mxtx_Flags					; communication flags
			BYTE	mxtx_Changed				; set to 1 for change
			APTR	mxtx_OpenFunc
			APTR	mxtx_ReadFunc
			APTR	mxtx_CloseFunc
			STRUCT	mxtx_Priority,16			; current priorities
			LABEL	mxtx_sizeof

* communication flags are PLAYING, SILENT, VELOCITY, LOOP

* Values between 0 and 127 are NOTE events. { NOTE #, VOL | CHAN, STOP }
COMMAND_TEMPO		equ		$80			; { COMMAND, TEMPO, N/A }			
COMMAND_SPECIAL		equ		$a0			; { COMMAND, CHAN, SPEC # | VAL }	
COMMAND_CONTROL		equ		$b0			; { COMMAND, CHAN, CTRL # | VAL }	
COMMAND_PROGRAM		equ		$c0			; { COMMAND, CHANNEL, PROG # }		
COMMAND_BEND		equ		$e0			; { COMMAND, CHANNEL, BEND VALUE }	
COMMAND_SYSEX		equ		$f0			; { COMMAND, TYPE, SIZE }
COMMAND_REALTIME	equ		$f8			; { COMMAND, REALTIME, N/A }		
COMMAND_END			equ		$ff			; { COMMAND, N/A, N/A }			

SPECIAL_MARK		equ		$00			; song marker (no data)		
SPECIAL_SYNC		equ		$01			; send sync signal	(data)		
SPECIAL_BEGINREP	equ		$02			; begin repeat section (times) 
SPECIAL_ENDREP		equ		$03			; end repeat section (no data)	

SYSEX_MICROTONAL	equ		0

		STRUCTURE	ScoreData,0
			APTR	score_Data
			LONG	score_NumEvents
			LABEL	score_sizeof

PERF_ALL			equ		0
PERF_SCORE			equ		1
PERF_SAMPLES		equ		2
PERF_PARTSAMPLES	equ		3

		STRUCTURE	SoundBlock,0
			APTR	sblk_Data
			LONG	sblk_Length
			WORD	sblk_Volume
			WORD	sblk_Period
			WORD	sblk_Pan
			LABEL	sblk_sizeof

		STRUCTURE	NoteBlock,0
			WORD	nblk_Note
			WORD	nblk_Patch
			WORD	nblk_Duration
			WORD	nblk_Volume
			WORD	nblk_Pan
			LABEL	nblk_sizeof

EXTRA_NONE			equ		0
EXTRA_NOTE			equ		1
EXTRA_PLAYSOUND		equ		2
EXTRA_STOPSOUND		equ		3
EXTRA_CHECKSOUND	equ		4
EXTRA_ADVANCE		equ		5
EXTRA_CHECKNOTE		equ		6

SOUND_LEFT_SIDE		equ		$01
SOUND_RIGHT_SIDE	equ		$02
SOUND_STEREO		equ		$03
SOUND_LOOP			equ		$04
MUST_HAVE_SIDE		equ		$08

SOUNDB_LEFT_SIDE	equ		0
SOUNDB_RIGHT_SIDE	equ		1
SOUNDB_LOOP			equ		2
MUSTB_HAVE_SIDE		equ		3

		STRUCTURE	DiskSample,0
			WORD	dsamp_Number
			WORD	dsamp_Tune
			WORD	dsamp_Volume
			WORD	dsamp_Octaves
			LONG	dsamp_AttackLength
			LONG	dsamp_SustainLength
			WORD	dsamp_AttackCount
			WORD	dsamp_ReleaseCount
			LABEL	dsamp_sizeof

DSAMPF_FILTER		equ		(1<<0)
DSAMPF_VELOCITY		equ		(1<<1)
DSAMPF_MICROTONAL	equ		(1<<15)

MAXTRAX_VBLANKFREQ	equ		(64+1)


;SEE_EXTERNAL		equ		1		; REMOVED
;DEBUG			equ		0		; REMOVED
;C_ENTRY		equ		0		; REMOVED
;DO_DMACHECK		equ		0		; removed

JSRLIB		macro
;			xref	_LVO\1
			jsr		_LVO\1(a6)
			endm

JSRDEV		macro
;			xref	_do_audio
;			jsr		_do_audio
			jsr		DEV_\1(a6)
			endm

;DEFX		macro
;		ifne SEE_EXTERNAL
;			xdef	\1
;		endc
;			endm

;DMACHECK	macro
;		ifne DO_DMACHECK
;			bsr		dma_check
;		endc
;			endm

;KDEBUG		macro
;			pea		\1
;			bsr		knum
;			addq.w	#4,sp
;			endm

;COLOR0		macro
;			move.w	#\1,$00dff180
;			endm

		; multiplies word values, result is a word

SCALE		macro

		ifeq	(\1&(\1-1))

		ifeq	\1-2
			add.w	\2,\2
		endc

		ifeq	\1-4
			add.w	\2,\2
			add.w	\2,\2
		endc

		ifeq	\1-8
			lsl.w	#3,\2
		endc

		ifeq	\1-16
			lsl.w	#4,\2
		endc

		ifeq	\1-32
			lsl.w	#5,\2
		endc

		else
			mulu.w	#\1,\2
		endc

			endm

*==========================================================================*
*
*	external references
*
*==========================================================================*

;			xref	_SysBase
;			xref	_GfxBase
;			xref	_DOSBase


_SysBase
	dc.l	0
;_GfxBase
;	dc.l	0
;_DOSBase
;	dc.l	0

*==========================================================================*
*
*	global data area
*
*==========================================================================*
	
;			section	__MERGED,DATA

;			xdef	_maxtrax
;	ifne	1
;			DEFX	_globaldata
;			DEFX	_patch
;			DEFX	_channel
;			DEFX	_voice
;	else
;			xdef	_globaldata
;			xdef	_patch
;			xdef	_channel
;			xdef	_voice
;	endc
;			DEFX	_xchannel
;			DEFX	_extra_op
;			DEFX	_extra_data

;_patch		ds.b	NUM_PATCHES*patch_sizeof
;_voice		ds.b	NUM_VOICES*voice_sizeof
;_channel	ds.b	NUM_CHANNELS*chan_sizeof
;_xchannel	ds.b	chan_sizeof
;_maxtrax	ds.b	mxtx_sizeof
;_globaldata	ds.b	glob_sizeof
;_extra_op	dc.w	0
;_extra_data	dc.l	0

;_scoreptr	dc.l	0
;_scoremax	dc.w	0

;		ifne HAS_MICROTONAL
;			DEFX	_microtonal
;_microtonal	ds.w	128
;		endc

;vblank_name	dc.b	'MaxTrax_VBlank',0
;			ds.w	0
;music_name	dc.b	'MT_Music',0
;			ds.w	0
;extra_name	dc.b	'MT_Extra',0
;			ds.w	0

;			DEFX	_vblank_server
;			DEFX	_music_server
;			DEFX	_extra_server
;_vblank_server
;			dc.l	0,0
;			dc.b	5			; bug LN_TYPE as first
;			dc.b	NT_INTERRUPT		; bug LN_PRI as second
;			dc.l	vblank_name
;			dc.l	0
;			dc.l	MusicVBlank
;_music_server
;			dc.l	0,0
;			dc.b	-32			; bug LN_TYPE as first
;			dc.b	NT_INTERRUPT		; bug LN_PRI as second
;			dc.l	music_name
;			dc.l	0
;			dc.l	MusicServer
;_extra_server
;			dc.l	0,0
;			dc.b	-32			; bug LN_TYPE as first
;			dc.b	NT_INTERRUPT		; bug LN_PRI as second
;			dc.l	extra_name
;			dc.l	0
;			dc.l	ExtraServer

;			dc.w	0

;			DEFX	_audio_play
;			DEFX	_audio_ctrl
;			DEFX	_audio_stop
;			DEFX	_audio_env
;			DEFX	_play_port
;			DEFX	_temp_port
;			DEFX	_AudioDevice
;_audio_play
;			dc.l	0
;_audio_ctrl
;			dc.l	0
;_audio_stop
;			dc.l	0
;_audio_env
;			dc.l	0
;_play_port
;			dc.l	0
;_temp_port
;			dc.l	0
;_AudioDevice
;			dc.l	0

;			DEFX	_asample
;_asample
;			ds.l	NUM_SAMPLES

* 30 - serial number for LGPL release

			dc.b	'>'
			dc.b	0,0,3,0
			dc.b	'<'

;			section code,code

* the following assembly langauge file is shared between MaxTrax & Music-X

;			include 'shared.asm'

*==========================================================================*
*   MaxTrax Music Player - audio device handler (shared module)            *
*   Copyright 1991 Talin (David Joiner) & Joe Pearce                       *
*                                                                          *
*   This library is free software; you can redistribute it and/or          *
*   modify it under the terms of the GNU Lesser General Public             *
*   License as published by the Free Software Foundation; either           *
*   version 2.1 of the License, or (at your option) any later version.     *
*                                                                          *
*   This library is distributed in the hope that it will be useful,        *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
*    Library General Public License for more details.                      *
*                                                                          *
*   You should have received a copy of the GNU Lesser General Public       *
*   License along with this library; if not, write to the Free             *
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA               *
*	02111-1307 USA.                                                    *
*                                                                          *
*   Contact information:                                                   *
*   The Wyrmkeep Entertainment Co.                                         *
*   Attn: Joe Pearce                                                       *
*   P. O. Box 1585                                                         *
*   Costa Mesa, CA 92628-1585                                              *
*   www.wyrmkeep.com                                                       *
*==========================================================================*

*==========================================================================*
*
*	CalcVolume - calculate current volume for a voice
*
*	UWORD CalcVolume(struct VoiceData *v)
*
*==========================================================================*

;		ifne C_ENTRY
;			DEFX	_CalcVolume
;_CalcVolume
;			move.l	4(sp),a0						; voice
;		endc
CalcVolume
;		ifne IN_MUSICX
;			moveq	#64,d0
;		else
			lea		_globaldata,a1					; ptr to global data
			move.w	glob_Volume(a1),d0				; get global volume
;		endc
			move.w	voice_NoteVolume(a0),d1			; get attack volume
			cmp.w	#128,d1							; 128 is max volume
			bpl.s	.1								; if vol >= 128, skip
			mulu.w	d1,d0							; multiply into volume
			lsr.w	#7,d0							; scale by 128

* experiment: only consider the top byte of envelope vol 

.1			moveq	#0,d1
			move.b	voice_BaseVolume(a0),d1			; get envelope volume
			cmp.w	#128,d1							; $80 is max volume
			bpl.s	.2								; if vol > $80, skip
			mulu.w	d1,d0							; multiply into volume
			lsr.w	#7,d0							; scale by $80

.2
		ifne HAS_FULLCHANVOL
			move.l	voice_Channel(a0),a1			; get channel
			moveq	#0,d1
			move.b	chan_Volume(a1),d1				; get channel volume
			bmi.s	.3								; if vol >= 128, skip
			mulu.w	d1,d0							; multiply into volume
			lsr.w	#7,d0							; scale by 128
		endc

.3			cmp.w	#65,d0							; if vol < 65, done
			bmi.s	.4
			moveq	#64,d0							; else set to 64
.4			move.b	d0,voice_LastVolume(a0)			; save in LastVolume
			rts

*==========================================================================*
*
*	voice picking routine
*
*	struct VoiceData *pick_voice(UWORD pick,WORD pri)
*
*==========================================================================*

LEFT_0			equ		0
RIGHT_0			equ		1
RIGHT_1			equ		2
LEFT_1			equ		3

SIBLING_SIDE	macro
			eor.w	#3,\1
				endm

OTHER_SIDE		macro
			eor.w	#1,\1
				endm

;BEST			macro	
;			cmp.b	\1,\2				; test if dd > ds

;			blt.s	\@MIN1				; yes, ds has right value (unsigned)
;			move.b	\1,\2				; move dd into ds
;\@MIN1
;				endm

right_round	dc.b	0								; bounce control
left_round	dc.b	0

;		ifne C_ENTRY
;			DEFX	_pick_voice
;_pick_voice
;			move.w	4(sp),d0						; get initial voice number
;			move.w	6(sp),d1						; get pri
;		endc
pick_voice
			movem.l	a2/d2-d5,-(sp)

;		ifne IN_MUSICX
;			lea		_voice_table,a0					; load addr of voice table
;		else
			lea		_voice,a0						; load addr of voice table
;		endc

			move.w	d1,d5							; save pri in d5

;		ifeq IN_MUSICX
			bclr.l	#MUSTB_HAVE_SIDE,d5				; clear MUST bit
			bclr.l	#MUSTB_HAVE_SIDE,d1				; test and clear MUST bit
			bne.s	pv1								; if was set, skip
;		endc

			move.w	d0,d1
			OTHER_SIDE	d1							; get other side vice num

			move.w	d0,d2							; find 'best' of siblings
			SCALE	voice_sizeof,d2
			move.b	voice_Status(a0,d2.w),d2
			move.w	d0,d3
			SIBLING_SIDE d3
			SCALE	voice_sizeof,d3
			move.b	voice_Status(a0,d3.w),d3
;			BEST	d3,d2


	CMP.B	D3,D2				; test if dd > ds
	BLT.B	.MIN1				; yes, ds has right value (unsigned)
	MOVE.B	D3,D2				; move dd into ds
.MIN1

			move.b	d2,d4							; save vnear -> d4

			move.w	d1,d2							; find 'best' of 'others'
			SCALE	voice_sizeof,d2
			move.b	voice_Status(a0,d2.w),d2
			move.w	d1,d3
			SIBLING_SIDE d3
			SCALE	voice_sizeof,d3
			move.b	voice_Status(a0,d3.w),d3
;			BEST	d3,d2							; vfar -> d2


	CMP.B	D3,D2				; test if dd > ds
	BLT.B	.MIN2				; yes, ds has right value (unsigned)
	MOVE.B	D3,D2				; move dd into ds
.MIN2

			cmp.b	#ENV_RELEASE,d4					; is vnear <= ENV_RELEASE
			ble.s	pv1								; yes, so no change

			cmp.b	#ENV_RELEASE,d2					; is vfar > ENV_RELEASE
			bgt.s	pv1								; yes, so no change

			move.w	d1,d0							; other side better pick

* now we know which side it's going to be on, left or right
* now choose between 0 and 1

pv1			jsr		pick_voice1						; pick from siblings

			move.l	d0,d2							; get pointer to that voice
			SCALE	voice_sizeof,d2
			lea		0(a0,d2.w),a1

;		ifeq IN_MUSICX
			btst.b	#VOICEB_BLOCKED,voice_Flags(a1)	; if blocked, pick another
			bne.s	pv3

			cmp.b	voice_Priority(a1),d5			; compare to voice priority
			bpl.s	pv99							; if v->Pri <= pri, OK
			
pv3			SIBLING_SIDE d0							; prob, so try other sibling
			move.l	d0,d2							; get pointer to that voice
			SCALE	voice_sizeof,d2
			lea		0(a0,d2.w),a1

			btst.b	#VOICEB_BLOCKED,voice_Flags(a1)	; if blocked, pick another
			bne.s	pv4

			cmp.b	voice_Priority(a1),d5			; compare to voice priority
			bpl.s	pv99							; if v->Pri <= pri, OK

* ick, so go to OTHER_SIDE

pv4			OTHER_SIDE d0
			jsr		pick_voice1						; pick from siblings

			move.l	d0,d2							; get pointer to that voice
			SCALE	voice_sizeof,d2
			lea		0(a0,d2.w),a1

			btst.b	#VOICEB_BLOCKED,voice_Flags(a1)	; if blocked, pick another
			bne.s	pv5

			cmp.b	voice_Priority(a1),d5			; compare to voice priority
			bpl.s	pv99							; if v->Pri <= pri, OK
			
pv5			SIBLING_SIDE d0							; prob, so try LAST VOICE
			move.l	d0,d2							; get pointer to that voice
			SCALE	voice_sizeof,d2
			lea		0(a0,d2.w),a1

			btst.b	#VOICEB_BLOCKED,voice_Flags(a1)	; if blocked, pick another
			bne.s	pv6

			cmp.b	voice_Priority(a1),d5			; compare to voice priority
			bpl.s	pv99							; if v->Pri <= pri, OK

* total failure!!

pv6			moveq	#0,d0							; no voice found
			bra.s	pv98

* success...

pv99
;		endc
			and.b	#2,d0							; a2 has round_ptr
			move.b	d0,(a2)							; set-up next bounce val
			move.l	a1,d0							; return voice found

pv98		movem.l	(sp)+,a2/d2-d5					; done!
			rts

* this subroutine chooses between two voices on same side

pick_voice1
			lea		right_round,a2					; assume use right_round
			move.w	d0,d1							; decide which round
			subq.w	#1,d1
			btst.l	#1,d1
			beq.s	.2
			addq.w	#1,a2							; use left_round

.2			move.l	d0,d2
			SCALE	voice_sizeof,d2
			move.b	voice_Status(a0,d2.w),d2
;			move.b	voice_LastVolume(a0,d2.w),d4	; "pick" voice volume

			move.w	d0,d1
			SIBLING_SIDE d1
			move.w	d1,d3
			SCALE	voice_sizeof,d3
			sub.b	voice_Status(a0,d3.w),d2
			bmi.s	.3								; if other higher, use pick
			bne.s	.4								; if pick high, use other

;			cmp.b	voice_LastVolume(a0,d3.w),d4	; compare w/"other" volume
;			bgt		.4						; if pick vol > other vol, use other 

			move.b	d0,d3							; voice #
			and.b	#2,d3							; check on same side
			cmp.b	(a2),d3							; break tie, get round data
			bne.s	.3								; if not same, use pick
.4			move.w	d1,d0							; switch to other

.3			rts

*==========================================================================*
*
*	CalcNote - calculate note # and period for voice
*
*	UWORD CalcNote(struct VoiceData *v)
*
*==========================================================================*

;		ifne C_ENTRY
;			DEFX	_CalcNote
;_CalcNote
;			move.l	4(sp),a0						; get voice pointer
;		endc
CalcNote
			movem.l	a2/a3/d2/d3,-(sp)				; save regs

			move.l	voice_Channel(a0),a2			; get channel pointer
			move.w	chan_RealBend(a2),d2			; get adj bend value
			moveq	#0,d3

			clr.w	voice_LastPeriod(a0)			; set to error value

			btst.b	#VOICEB_PORTAMENTO,voice_Flags(a0)	; is there a portamento
			beq.s	.1

;		ifne HAS_MICROTONAL
;			btst.b	#CHANB_MICROTONAL,chan_Flags(a2)	; microtonal active?
;			beq		.2
;			lea		_microtonal,a1					; addr of microtonal table
;			moveq	#0,d0
;			move.b	voice_EndNote(a0),d0			; note we're going to
;			add.w	d0,d0							; access as word
;			move.w	0(a1,d0.w),d1					; real microtonal value
;			move.b	voice_BaseNote(a0),d0			; note we're going from
;			add.w	d0,d0							; access as word
;			sub.w	0(a1,d0.w),d1					; subract microtonal value
;			ext.l	d1
;			move.l	voice_PortaTicks(a0),d0			; current PortaTicks
;			move.l	a0,-(sp)
;			jsr		mulu							; delta m'tonal * p'ticks
;			move.l	(sp)+,a0
;			tst.l	d0
;			bpl		.22
;			neg.l	d0
;			lsr.l	#8,d0							; divide neg # by 256
;			neg.l	d0
;			bra		.23
;.22			lsr.l	#8,d0							; divide by 256
;.23			divs.w	chan_PortamentoTime(a2),d0		; divide by PortamentoTime
;			move.w	d0,d3							; save extra bend value
;			bra		.1
;.2
;		endc

			move.b	voice_EndNote(a0),d1			; note we're going to
			sub.b	voice_BaseNote(a0),d1			; note we're going from
			ext.w	d1
			ext.l	d1
			move.l	voice_PortaTicks(a0),d0			; current PortaTicks
			move.l	a0,-(sp)
			jsr		mulu							; delta note * PortaTicks
			move.l	(sp)+,a0
			divs.w	chan_PortamentoTime(a2),d0		; divide by PortamentoTime
			move.w	d0,d3							; save extra bend value

.1
		ifne HAS_MODULATION
			tst.w	chan_Modulation(a2)				; modulation?
			beq.s	.80								; no...

			btst.b	#CHANB_MODTYPE,chan_Flags(a2)	; period modulation?
			bne.s	.80								; no...

			move.l	_globaldata+glob_SineValue,d0	; calc table index
			lsr.l	#8,d0							; / 256

			moveq	#0,d1
			move.w	chan_ModulationTime(a2),d1		; ModTime

			move.l	a0,-(sp)
			jsr		modu							; (SineValue >> 8) % ModTime
			move.l	(sp)+,a0

			lsl.l	#8,d0							; * 256
			divu.w	chan_ModulationTime(a2),d0		; / ModTime

			lea		_sine_table,a1
			move.b	0(a1,d0.w),d1
			ext.w	d1
			add.w	d1,d1
			muls.w	chan_Modulation(a2),d1
			divs.w	#127,d1
			add.w	d1,d3
.80
		endc

			ext.l	d3
			ext.l	d2
			add.l	d3,d2							; tone = bend + mod

			moveq	#0,d0
			move.b	voice_BaseNote(a0),d0			; get base note
;		ifne HAS_MICROTONAL
;			btst.b	#CHANB_MICROTONAL,chan_Flags(a2)	; microtonal active?
;			beq		.6

;			lea		_microtonal,a1					; addr of microtonal table
;			add.w	d0,d0							; access as word
;			moveq	#0,d1
;			move.w	0(a1,d0.w),d1					; add microtonal to tone
;			add.l	d1,d2
;			bra		.7
;.6
;		endc

			lsl.w	#8,d0
			add.l	d0,d2							; tone += note << 8

* really should be pre-calculate this

.7			move.l	voice_Patch(a0),a1				; get patch pointer
			move.w	patch_Tune(a1),d0				; add tuning to period
			ext.l	d0
			lsl.l	#8,d0
			divs.w	#24,d0							; (tune * 256) / 24
			ext.l	d0
			add.l	d0,d2							; add to tone

			sub.l	#(45<<8),d2						; based on midi note #45

			add.l	d2,d2							; try to do math w/o .divs#
			add.l	d2,d2
			divs.w	#3,d2
			ext.l	d2
			lsl.l	#4,d2							; ((tone << 2) / 3) << 4

			neg.l	d2
			add.l	#K_VALUE,d2						; logperiod = K - 'tone'

			move.l	patch_Sample(a1),a3				; get sample pointer

			btst.b	#VOICEB_RECALC,voice_Flags(a0)	; a recalc?
			bne.s	.30								; yep, skip ahead

			clr.l	voice_PeriodOffset(a0)			; clear period offset

.13			cmp.l	#PREF_PERIOD,d2					; is <= PREF_PERIOD
			ble.s	.12								; yes, so don't shift

			move.l	samp_NextSample(a3),d1			; is this last sample?
			beq.s	.12								; yes, can't shift any more

			move.l	d1,a3
			add.l	#$10000,voice_PeriodOffset(a0)	; remember logperiod shift
			sub.l	#$10000,d2						; adjust logperiod
			bra		.13

.30			sub.l	voice_PeriodOffset(a0),d2		; directly change period

.12			cmp.l	#PERIOD_LIMIT,d2				; logperiod < PERIOD_LIMIT?
			blt.s	.99								; oops, bad period, error

.14			move.l	a0,a2							; preserve across call
			move.l	d2,d0
			bsr		IntAlg
			move.w	d0,voice_LastPeriod(a2)

.99			move.l	a3,d0							; return sample to play
			movem.l	(sp)+,a2/a3/d2/d3				; restore regs
			rts										; done!

*==========================================================================*
*
*	EnvelopeManager - handles envelopes of playing notes
*
*	void EnvelopeManager(ULONG delta)
*
*	LONG calc_incrvol(LONG delta_volume,UWORD time)
*
*==========================================================================*

;		ifne C_ENTRY
;			DEFX _calc_incrvol
;_calc_incrvol
;			move.l	4(sp),d0						; delta
;			move.w	6(sp),d1						; time
;		endc
calc_incrvol
			movem.l	d2/d3,-(sp)
			tst.w	d1								; is time zero?
			beq.s	.3								; yes, just return delta

			move.l	d0,d2							; save original delta
			bpl.s	.2								; if positive, ok
			neg.l	d0								; else negate

.2			move.w	d0,d3							; save it
			mulu.w	#1000,d0						; 1000 * delta
;		ifne IN_MUSICX
;			mulu.w	#60,d1
;		else
			mulu.w	_globaldata+glob_Frequency,d1	; time * frequency
;		endc
			jsr		divu							; (1000 * delta)/(time * freq)
			cmp.w	d0,d3							; compare incrvol & delta
			bmi.s	.1								; time too short, go special

			tst.l	d2								; was delta negative?
			bpl.s	.3								; no, skip			
			neg.l	d0								; else negate incrvol

.3			movem.l	(sp)+,d2/d3
			rts

.1			move.l	d2,d0							; get saved delta
			bra		.3

DoOneVoice
;			COLOR0	$0fff
;		ifne IN_MUSICX
;			tst.b	voice_Status(a2)
;			beq		.19
;		endc
			move.l	voice_Channel(a2),d0			; any note playing?
			beq		.19								; no, skip
			move.l	d0,a3

			add.l	d2,voice_LastTicks(a2)			; add delta to LastTicks

			cmp.b	#ENV_SUSTAIN,voice_Status(a2)	; in sustain?
			bne.s	.2								; no, skip ahead

			btst.b	#CHANB_ALTERED,chan_Flags(a3)	; channel altered?
			bne		.18								; yes, must do calc's
			btst.b	#VOICEB_PORTAMENTO,voice_Flags(a2)	; portamento on?
			bne		.18								; yes, must do calc's
		ifne HAS_MODULATION
			tst.w	chan_Modulation(a3)				; modulation on?
			bne		.18								; yes, must do calc's
		endc
			bra		.19								; otherwise, skip

.2			cmp.b	#ENV_HALT,voice_Status(a2)		; halt voice?
			bne.s	.3								; no, skip ahead

			move.l	a2,a0
			bsr		KillVoice						; kill voice
			bra		.19								; go to next voice

.3			move.l	voice_Patch(a2),a5				; get patch

			cmp.b	#ENV_START,voice_Status(a2)		; starting a voice?
			bne.s	.4								; no, check another status

			move.l	patch_Attack(a5),a0				; envelope
			move.l	a0,voice_Envelope(a2)			; put attack env in voice
			beq.s	.5								; but there is none...

													; initialize voice
			move.w	patch_AttackCount(a5),voice_EnvelopeLeft(a2)
			moveq	#0,d0
			move.w	env_Duration(a0),d0				; get duration (a0 = env)
			lsl.l	#8,d0							; shift left 8
			move.l	d0,voice_TicksLeft(a2)			; save as TicksLeft
			move.b	#ENV_ATTACK,voice_Status(a2)	; in attack envelope
			move.l	d2,voice_LastTicks(a2)			; reset LastTicks to delta
			moveq	#0,d0
			move.w	env_Volume(a0),d0				; get volume (a0 = env)
			move.w	env_Duration(a0),d1				; get duration (a0 = env)
			bsr		calc_incrvol					; calc volume delta
			move.l	d0,voice_IncrVolume(a2)			; and save in voice
			bra.s	.9								; do envelope management

.5			move.b	#ENV_SUSTAIN,voice_Status(a2)	; in sustain
			move.w	patch_Volume(a5),voice_BaseVolume(a2)	; set base volume
			move.l	d2,voice_LastTicks(a2)			; reset LastTicks to delta
			bra		.18								; but do calc's			

.4			cmp.b	#ENV_RELEASE,voice_Status(a2)	; releasing a voice?
			bne.s	.9								; no, so do env management

			move.l	patch_Release(a5),a0			; envelope
			move.l	a0,voice_Envelope(a2)			; put release env in voice
			bne.s	.6								; and there is one...

			move.b	#ENV_HALT,voice_Status(a2)		; set to halt
			moveq	#0,d4							; set new volume to zero
			bra		.17								; send audio request

.6			move.w	patch_ReleaseCount(a5),voice_EnvelopeLeft(a2)
			moveq	#0,d0
			move.w	env_Duration(a0),d0				; get duration (a0 = env)
			lsl.l	#8,d0							; shift left 8
			move.l	d0,voice_TicksLeft(a2)			; save as TicksLeft
			move.b	#ENV_DECAY,voice_Status(a2)		; in release envelope
			move.l	d2,voice_LastTicks(a2)				; reset LastTicks to delta
			moveq	#0,d0
			move.w	env_Volume(a0),d0				; get volume (a0 = env)
			moveq	#0,d1
			move.w	voice_BaseVolume(a2),d1			; subtract current volume
			sub.l	d1,d0
			move.w	env_Duration(a0),d1				; get duration (a0 = env)
			bsr		calc_incrvol					; calc volume delta
			move.l	d0,voice_IncrVolume(a2)			; and save in voice

.9			cmp.b	#ENV_SUSTAIN,voice_Status(a2)	; need to do env management?
			beq		.18								; none, skip

			cmp.l	voice_TicksLeft(a2),d2			; compare delta & time left
			bmi.s	.10								; time left, just count down

			move.l	voice_Envelope(a2),a0			; copy final volume from env
			move.w	env_Volume(a0),voice_BaseVolume(a2)	; into voice

			subq.w	#1,voice_EnvelopeLeft(a2)		; decrement envelope count
			bne.s	.11								; still > 0, keep status

													; no envelopes left
			cmp.b	#ENV_DECAY,voice_Status(a2)		; was doing release?
			bne.s	.12								; no, was attack

			move.b	#ENV_HALT,voice_Status(a2)		; set to halt
			moveq	#0,d4							; set new volume to zero
			bra		.17								; send audio request

.12			move.b	#ENV_SUSTAIN,voice_Status(a2)	; start sustain
			move.l	d2,voice_LastTicks(a2)			; reset last ticks
			bra.s	.18

.11			addq.w	#4,a0							; next envelope segment
			move.l	a0,voice_Envelope(a2)
			moveq	#0,d0
			move.w	env_Duration(a0),d0				; get duration (a0 = env)
			lsl.l	#8,d0							; shift left 8
			move.l	d0,voice_TicksLeft(a2)			; save as TicksLeft
			moveq	#0,d0
			move.w	env_Volume(a0),d0				; get volume (a0 = env)
			moveq	#0,d1
			move.w	voice_BaseVolume(a2),d1			; subtract current volume
			sub.l	d1,d0
			move.w	env_Duration(a0),d1				; get duration (a0 = env)
			bsr		calc_incrvol					; calc volume delta
			move.l	d0,voice_IncrVolume(a2)			; and save in voice
			bra.s	.18			

.10			moveq	#0,d0							; calc new base volume
			move.w	voice_BaseVolume(a2),d0
			add.l	voice_IncrVolume(a2),d0
			bmi.s	.13
			cmp.l	#$00008000,d0
			bmi.s	.14
			move.w	#$8000,d0
			bra.s	.14
.13			moveq	#0,d0
.14			move.w	d0,voice_BaseVolume(a2)			; save new base volume
			sub.l	d2,voice_TicksLeft(a2)			; subtract delta from time

.18			move.l	a2,a0
			bsr		CalcVolume						; calc new volume
			move.w	d0,d4

			btst.b	#VOICEB_PORTAMENTO,voice_Flags(a2)	; portamento going?
			beq.s	.16								; no, so skip

			add.l	d2,voice_PortaTicks(a2)			; add delta to porta time
			move.l	voice_PortaTicks(a2),d0
			lsr.l	#8,d0
			cmp.w	chan_PortamentoTime(a3),d0		; compare portamento times
			bmi.s	.20								; portamento not over

			bclr.b	#VOICEB_PORTAMENTO,voice_Flags(a2)	; clear portamento flag
			move.b	voice_EndNote(a2),voice_BaseNote(a2)	; set base note
			bra.s	.20

.16			
		ifne HAS_MODULATION
			tst.w	chan_Modulation(a3)				; if modulation, calc note
			bne.s	.20
		endc

			btst.b	#CHANB_ALTERED,chan_Flags(a3)	; channel altered?
			beq.s	.17								; no, don't recalc note

.20			bset.b	#VOICEB_RECALC,voice_Flags(a2)	; its a recalc
			move.l	a2,a0
			bsr		CalcNote

.17			move.l	_audio_env,a1					; get envelope audio block
			move.b	#IOF_QUICK,IO_FLAGS(a1)			; a quick action
			clr.b	IO_ERROR(a1)					; clear error
			moveq	#0,d0
			move.b	voice_Number(a2),d0
			moveq	#1,d1
			lsl.l	d0,d1
			move.l	d1,IO_UNIT(a1)					; unit = 1 << voice#
			move.w	voice_LastPeriod(a2),d1			; test LastPeriod
			beq.s	.21								; if zero, special meaning
			move.w	d1,ioa_Period(a1)

	LEA	StructAdr(PC),A6
	BTST	D0,UPS_DMACon+1(A6)
	BNE.B	.NoVoice
	CLR.W	D4
.NoVoice
	BSR.W	ChangeVolume
	BSR.W	SetBoth

			move.w	d4,ioa_Volume(a1)
			bra.s	.22
.21			move.w	#1000,ioa_Period(a1)
			clr.w	ioa_Volume(a1)

.22			move.l	_AudioDevice,a6
			JSRDEV	BEGINIO

.19
;			COLOR0	$0777
			rts

;		ifne C_ENTRY
;			DEFX	_EnvelopeManager
;_EnvelopeManager
;			move.l	4(sp),d0						; delta
;		endc
EnvelopeManager
			movem.l	d2/d3/d4/a2/a3/a5/a6,-(sp)
			move.l	d0,d2

;		ifne IN_MUSICX
;			lea		_voice_table,a2
;		else
			lea		_voice,a2
;		endc
			moveq	#NUM_VOICES-1,d3
.1			bsr		DoOneVoice
			lea		voice_sizeof(a2),a2
			dbra	d3,.1

;		ifne IN_MUSICX
;			lea		_channel_table,a0
;		else
			lea		_channel,a0
;		endc
			moveq	#NUM_CHANNELS-1,d0
.30			bclr.b	#CHANB_ALTERED,chan_Flags(a0)
			lea		chan_sizeof(a0),a0
			dbra	d0,.30

;		ifeq IN_MUSICX
			bclr.b	#CHANB_ALTERED,_xchannel+chan_Flags
;		endc

		ifne HAS_MODULATION
			lsr.l	#2,d2
			move.l	d2,_globaldata+glob_SineValue
		endc

			movem.l	(sp)+,d2/d3/d4/a2/a3/a5/a6
			rts

;		ifne IN_MUSICX
;			xdef	_EnvOneVoice
;			xdef	EnvOneVoice
;_EnvOneVoice
;			move.l	4(sp),a0						; voice
;			move.l	8(sp),d0						; delta
;EnvOneVoice
;			movem.l	d2/d3/d4/a2/a3/a5/a6,-(sp)

;			move.l	d0,d2
;			move.l	a0,a2

;			bsr		DoOneVoice

;			movem.l	(sp)+,d2/d3/d4/a2/a3/a5/a6
;			rts
;		endc

*==========================================================================*
*
*	KillVoice - immediately terminates sound on one voice
*
*	void KillVoice(struct VoiceData *v)
*	void stop_audio(UWORD num)
*
*==========================================================================*

;		ifne C_ENTRY
;			DEFX	_stop_audio
;_stop_audio
;			move.w	4(sp),d0							; voice number
;		endc
stop_audio
			movem.l	d2/a6,-(sp)
			moveq	#0,d2
			move.w	d0,d2

			move.l	_audio_stop,a1						; stop that voice
			moveq	#1,d1
			lsl.l	d2,d1
			move.l	d1,IO_UNIT(a1)
			move.b	#IOF_QUICK,IO_FLAGS(a1)

			move.l	_AudioDevice,a6						; use BeginIO
			JSRDEV	BEGINIO

			cmp.w	#37,LIB_VERSION(a6)					; if KS v37, skip
			bpl.s	.1

			; the AudControl structure is 16 bytes in size, so...

			move.l	#$00dff000+aud,a0					; use audio flush trick
			lsl.w	#4,d2								; 16 * channel #
			move.l	#$00010000,ac_per(a0,d2.w)			; hit hardware

.1			movem.l	(sp)+,d2/a6
			rts

;		ifne C_ENTRY
;			DEFX	_KillVoice
;_KillVoice
;			move.l	4(sp),a0
;		endc
KillVoice
			move.l	voice_Channel(a0),a1				; get channel

			subq.b	#1,chan_VoicesActive(a1)			; -1 active voice
;		ifeq IN_MUSICX
			lea		_globaldata,a1
			subq.b	#1,glob_VoicesActive(a1)
;		endc

			clr.l	voice_Channel(a0)					; clear voice
			clr.b	voice_Status(a0)					; ENV_FREE
			clr.b	voice_Flags(a0)
			clr.b	voice_Priority(a0)
			clr.l	voice_UniqueID(a0)

			moveq	#0,d0							; let stop_audio finish up
			move.b	voice_Number(a0),d0
			jmp		stop_audio

*==========================================================================*
*
*	ResetChannel - handles RESET ALL CONTROLLERS midi command
*
*	void ResetChannel(struct ChannelData *chan)(a0)
*
*==========================================================================*

;		ifne C_ENTRY
;			DEFX	_ResetChannel
;_ResetChannel
;			move.l	4(sp),a0
;		endc
ResetChannel
		ifne HAS_MODULATION
			clr.w	chan_Modulation(a0)
			move.w	#1000,chan_ModulationTime(a0)			; 1 second
		endc
;		ifne HAS_MICROTONAL
;			move.w	#-1,chan_Microtonal(a0)
;		endc
			move.w	#500,chan_PortamentoTime(a0)			; 1/2 second
			move.w	#NO_BEND,chan_PitchBend(a0)
			clr.w	chan_RealBend(a0)
			move.b	#MAX_BEND_RANGE,chan_PitchBendRange(a0)
			move.b	#128,chan_Volume(a0)
			btst.b	#0,chan_Number(a0)
			bne.s	.1
			bclr.b	#CHANB_PAN,chan_Flags(a0)
			bra.s	.2
.1			bset.b	#CHANB_PAN,chan_Flags(a0)
.2			bclr.b	#CHANB_PORTAMENTO,chan_Flags(a0)
			bclr.b	#CHANB_MICROTONAL,chan_Flags(a0)
			bset.b	#CHANB_ALTERED,chan_Flags(a0)
			rts

*==========================================================================*
*
*	ControlCh - handles CONTROL CHANGE midi command
*
*	void ControlCh(struct ChannelData *chan,UBYTE *midi)
*
*==========================================================================*

;		ifne C_ENTRY
;			DEFX	_ControlCh
;_ControlCh
;			move.l	4(sp),a0
;			move.l	8(sp),a1
;		endc
ControlCh
			move.b	(a1),d0

		ifne HAS_MODULATION
			cmp.b	#1,d0							; modulation level MSB
			bne.s	.1
			move.b	1(a1),d0
			lsl.w	#8,d0
			move.w	d0,chan_Modulation(a0)
			bra		.99

.1			cmp.b	#1+32,d0						; modulation level LSB
			bne.s	.2
			move.b	1(a1),d0
			add.b	d0,d0
			move.b	d0,chan_Modulation+1(a0)
			bra		.99

.2
		endc

			cmp.b	#5,d0							; portamento time MSB
			bne.s	.3
			moveq	#0,d0
			move.b	1(a1),d0
			lsl.w	#7,d0
			move.w	d0,chan_PortamentoTime(a0)
			bra		.99

.3			cmp.b	#5+32,d0						; portamento time LSB
			bne.s	.4
			move.w	chan_PortamentoTime(a0),d0
			and.w	#$3f80,d0
			or.b	1(a1),d0
			move.w	d0,chan_PortamentoTime(a0)
			bra		.99

.4			cmp.b	#6,d0							; data entry MSB
			bne.s	.5
			tst.w	chan_RPN(a0)					; only doing RPN #0
			bne		.99

			moveq	#0,d0
			move.b	1(a1),d0
			cmp.b	#MAX_BEND_RANGE,d0				; can't be more than max
			ble.s	.6
			move.b	#MAX_BEND_RANGE,d0
.6			move.b	d0,chan_PitchBendRange(a0)
			move.w	chan_PitchBend(a0),d1
			sub.w	#NO_BEND,d1						; if no bend, no need to adj
			beq		.99

			mulu.w	d0,d1			; RealBend = (PitchBend - NO_BEND) * Range
			lsl.l	#8,d1			;				* 256
			divu.w	#NO_BEND,d1		;				/ NO_BEND
			move.w	d1,chan_RealBend(a0)
			bset.b	#CHANB_ALTERED,chan_Flags(a0)	; mark as altered
			bra		.99

.5			cmp.b	#7,d0							; Main Volume MSB
			bne.s	.7
			move.b	1(a1),d0
			beq.s	.8
			addq.w	#1,d0
.8			move.b	d0,chan_Volume(a0)
		ifne HAS_FULLCHANVOL
			bset.b	#CHANB_ALTERED,chan_Flags(a0)	; mark as altered
		endc
			bra		.99

.7			cmp.b	#10,d0							; Pan
			bne.s	.9
			move.b	1(a1),d0
			cmp.b	#64,d0							; compare data to 64
			bmi.s	.10								; less than 64, PAN left
			bne.s	.11								; more than 64, PAN right
			btst.b	#0,chan_Number(a0)				; else, base on channel #
			beq.s	.10								; is even, PAN left
.11			bclr.b	#CHANB_PAN,chan_Flags(a0)
			bra		.99
.10			bset.b	#CHANB_PAN,chan_Flags(a0)
			bra		.99

.9
		ifne HAS_MODULATION
			cmp.b	#16,d0							; GPC as Modulation Time MSB
			bne.s	.12
			moveq	#0,d0
			move.b	1(a1),d0
			lsl.w	#7,d0
			move.w	d0,chan_ModulationTime(a0)
			bra		.99

.12			cmp.b	#16+32,d0						; GPC as Modulation Time LSB
			bne.s	.13
			move.w	chan_ModulationTime(a0),d0
			and.w	#$3f80,d0
			or.b	1(a1),d0
			move.w	d0,chan_ModulationTime(a0)
			bra		.99

.13
		endc
;		ifne HAS_MICROTONAL
;			cmp.b	#17,d0							; GPC as Microtonal Set MSB
;			bne		.14
;			moveq	#0,d0
;			move.b	1(a1),d0
;			lsl.w	#8,d0
;			move.w	d0,chan_Microtonal(a0)
;			bra		.99

;.14			cmp.b	#17+32,d0						; GPC as Microtonal Set LSB
;			bne		.15
;			move.b	1(a1),d0
;			add.b	d0,d0
;			move.b	d0,chan_Modulation+1(a0)
;			bra		.99

;.15
;		endc

			cmp.b	#64,d0							; Damper Pedal
			bne.s	.16
			btst.b	#6,1(a1)						; test bit 6 (64)
			beq.s	.17								; do releasing pedal
			bset.b	#CHANB_DAMPER,chan_Flags(a0)
			bra		.99

.17			bclr.b	#CHANB_DAMPER,chan_Flags(a0)	; release dampered voices
;		ifne IN_MUSICX
;			lea		_voice_table,a1
;		else
			lea		_voice,a1
;		endc
			moveq	#NUM_VOICES-1,d0
.18			cmp.l	voice_Channel(a1),a0
			bne.s	.19
			bclr.b	#VOICEB_DAMPER,voice_Flags(a1)
			beq.s	.19
			move.b	#ENV_RELEASE,voice_Status(a1)
.19			lea		voice_sizeof(a1),a1
			dbra	d0,.18
			bra		.99

.16			cmp.b	#65,d0							; Portamento off/on 
			bne.s	.20
			btst.b	#6,1(a1)						; test bit 6 (64)
			beq.s	.21								; do off
			bset.b	#CHANB_PORTAMENTO,chan_Flags(a0)	; set to on
			bra		.99
.21			bclr.b	#CHANB_PORTAMENTO,chan_Flags(a0)	; set to off
			move.b	#-1,chan_LastNote(a2)			; no last note
			bra		.99

.20
;		ifne HAS_MICROTONAL
;			cmp.b	#80,d0							; Microtonal off/on 
;			bne		.22
;			btst.b	#6,1(a1)						; test bit 6 (64)
;			beq		.23								; do off
;			bset.b	#CHANB_MICROTONAL,chan_Flags(a0)	; set to on
;			bra		.99
;.23			bclr.b	#CHANB_MICROTONAL,chan_Flags(a0)	; set to off
;			bra		.99

;.22
;		endc

			cmp.b	#81,d0							; Audio Filter off/on 
			bne.s	.30
			moveq	#0,d0							; default <64, filter off
			cmp.b	#64,1(a1)
			beq.s	.31
			bmi.s	.32

			moveq	#1,d0							; >64, set filter on
			bra.s	.32

.31			move.w	_globaldata+glob_Filter,d0		; =64, set as global flag
.32			bsr		SetAudioFilter
			bra.s	.99

.30			cmp.b	#100,d0							; RPN LSB
			bne.s	.24
			move.b	1(a1),chan_RPN+1(a0)
			bra.s	.99

.24			cmp.b	#101,d0							; RPN MSb
			bne.s	.25
			move.b	1(a1),chan_RPN(a0)
			bra.s	.99

.25			cmp.b	#121,d0							; Reset All Controllers
			bne.s	.26
			bsr		ResetChannel					; a0 has channel already
			bra.s	.99

.26			cmp.b	#123,d0							; All Notes Off
			bne.s	.27
			bsr		AllNotesOff						; a0 has channel already
			bra.s	.99

.27			cmp.b	#126,d0							; MONO mode
			bne.s	.28
			bset.b	#CHANB_MONO,chan_Flags(a0)
			bsr		AllNotesOff						; a0 has channel already
			bra.s	.99

.28			cmp.b	#127,d0							; POLY mode
			bne.s	.29
			bclr.b	#CHANB_MONO,chan_Flags(a0)
			bsr		AllNotesOff						; a0 has channel already
			bra.s	.99

.29			cmp.b	#120,d0							; All Sounds Off
			bne.s	.99
			bsr		AllSoundsOff					; a0 has channel already
;			bra.s	.99

.99			rts

*==========================================================================*
*
*	PitchBend - handles PITCH BEND midi command
*
*	void PitchBend(struct ChannelData *chan,UBYTE *midi)(a0/a1)
*
*==========================================================================*

;		ifne C_ENTRY
;			DEFX	_PitchBend
;_PitchBend
;			move.l	4(sp),a0						; channel
;			move.l	8(sp),a1						; midi stream
;		endc
PitchBend
			moveq	#0,d0
			move.b	1(a1),d0						; (midi[1] << 7) + midi[0]
			lsl.w	#7,d0
			add.b	(a1),d0
			move.w	d0,chan_PitchBend(a0)			; save as pitch bend value

					; (PitchBend - NO_BEND) * PitchBendRange * 256 / NO_BEND;
			sub.w	#NO_BEND,d0
			moveq	#0,d1
			move.b	chan_PitchBendRange(a0),d1
			lsl.w	#8,d1
			muls.w	d1,d0
			divs.w	#NO_BEND,d0
			move.w	d0,chan_RealBend(a0)			; save as real bend value

			bset.b	#CHANB_ALTERED,chan_Flags(a0)	; set altered flag
			rts

*==========================================================================*
*  Math Routines
*==========================================================================*

divs:		;long divide	(primary = primary/secondary)
			move.l	d4,-(sp)
			clr.l	d4			;mark result as positive
			tst.l	d0
			bpl.s	prim_ok
			neg.l	d0
			add.w	#1,d4			;mark as negative
prim_ok:
			tst.l	d1
			bpl.s	sec_ok
			neg.l	d1
			eor.w	#1,d4			;flip sign of result
sec_ok:
			jsr	comdivide
chksign:
			tst.w	d4
			beq.s	posres
			neg.l	d0
posres:
			move.l	(sp)+,d4
;			tst.l	d0
			rts

;modu:		;unsigned long remainder	(primary = primary%secondary)
;			move.l	d1,-(sp)
;			jsr	comdivide
;			move.l	d1,d0
;			move.l	(sp)+,d1
;			tst.l	d0
;			rts

divu:		;unsigned long divide		(primary = primary/secondary)
;			move.l	d1,-(sp)
			jsr	comdivide
;			move.l	(sp)+,d1
;			tst.l	d0
			rts

; divide (dx,ax) by (bx,cx):
;	quotient in (dx,ax)
;	remainder in (bx,cx)
comdivide:
			movem.l	d2/d3,-(sp)
			swap	d1
			tst.w	d1
			bne.s	hardldv
			swap	d1
			move.w	d1,d3		;get divisor
			move.w	d0,d2		;save second part of dividend
			clr.w	d0		;get first part of dividend
			swap	d0
			divu.w	d3,d0		;do first divide
			move.l	d0,d1		;copy first remainder
			swap	d0		;get first quotient
			move.w	d2,d1		;get back second part of dividend
			divu.w	d3,d1		;do second divide
			move.w	d1,d0		;get second quotient
			clr.w	d1		;remainder is small
			swap	d1		;get it
			movem.l	(sp)+,d2/d3
			rts			;and return

hardldv:
			swap	d1
			move.l	d1,d3		;save divisor
			move.l	d0,d1		;copy dividend
			clr.w	d1		;set up remainder
			swap	d1
			swap	d0		;set up quotient
			clr.w	d0
			move.l	#16-1,d2		;do 16 times
.1
			add.l	d0,d0		;shift the quotient
			addx.l	d1,d1		;shift the remainder
			cmp.l	d1,d3		;check if big enough to subtract
			bhi.s	.2
			sub.l	d3,d1		;yes, so subtract it
			add.w	#1,d0		;and add one to quotient
.2
			dbf	d2,.1		;and loop till done
			movem.l	(sp)+,d2/d3
			rts

mulu:		;unsigned long multiply	(primary = primary*secondary)
			movem.l	d2/d3,-(sp)
			move.w	d1,d2
			mulu.w	d0,d2		;d0.l * d1.l
			move.l	d1,d3
			swap	d3
			mulu.w	d0,d3		;d0.l * d1.h
			swap	d3
			clr.w	d3
			add.l	d3,d2
			swap	d0
			mulu.w	d1,d0		;d0.h * d1.l
			swap	d0
			clr.w	d0
			add.l	d2,d0
			movem.l	(sp)+,d2/d3
			rts

*==========================================================================*
*  IntAlg
*      Inputs:  d0 = logarithm in standard format
*      Outputs: d0 = 32-bit integer value
*      Scratch: d1,a0
*      Max Error: 1 part in 30,000
*==========================================================================*

;		ifne C_ENTRY
;			DEFX	_IntAlg
;_IntAlg:
;			move.l	4(sp),d0
;		endc
IntAlg:
			move.l	d2,-(sp)
			move.w	d0,d1					; get mantissa

			lsr.w	#8,d1					; get the "lookup" part.
			add.w	d1,d1					; times 2 for table

			lea		alogtable,a0			; address of antilog table
			add.w	d1,a0					; plus offset

			move.w	(a0)+,d1				; get the digits to use.
			move.w	(a0),d2					; get the next value too
			sub.w	d1,d2					; distance between table entries

; now we need to interpolate.
			lsl.w	#8,d0					; adjust interpolation bits
			add.w	#128,d0					; add rounding factor??
			mulu.w	d0,d2					; times interpolation factor
			swap	d2						; get high word
			add.w	d2,d1					; and adjust result.

; now we have 16 bits in the lower half of d1, and the upper half of d1 is how
; many to shift them over...

			swap	d1						; put it in the top
			bset	#0,d1					; make the lowest bit a 1
			ror.l	#1,d1					; make the leading bit a 1, shift others over

			swap	d0						; get bits to shift.
			cmp.w	#32,d0					; if d0 > 33
			bhs.s	.9						; then fail, we can't alog this.
			eor.b	#31,d0					; reverse direction of shift

			lsr.l	d0,d1					; rotate number
			moveq	#0,d0					; clear d0
			addx.l	d0,d1					; round using carry bit
			move.l	d1,d0					; put in d0
			move.l	(sp)+,d2
			rts								; return

.9			move.l	(sp)+,d2
			moveq	#0,d0					; return 0 (value that can never be logged)
			rts								; return

alogtable:
			dc.w	00000,00178,00356,00535,00714,00893,01073,01254
			dc.w	01435,01617,01799,01981,02164,02348,02532,02716
			dc.w	02902,03087,03273,03460,03647,03834,04022,04211
			dc.w	04400,04590,04780,04971,05162,05353,05546,05738
			dc.w	05932,06125,06320,06514,06710,06906,07102,07299
			dc.w	07496,07694,07893,08092,08292,08492,08693,08894
			dc.w	09096,09298,09501,09704,09908,10113,10318,10524
			dc.w	10730,10937,11144,11352,11560,11769,11979,12189
			dc.w	12400,12611,12823,13036,13249,13462,13676,13891
			dc.w	14106,14322,14539,14756,14974,15192,15411,15630
			dc.w	15850,16071,16292,16514,16737,16960,17183,17408
			dc.w	17633,17858,18084,18311,18538,18766,18995,19224
			dc.w	19454,19684,19915,20147,20379,20612,20846,21080
			dc.w	21315,21550,21786,22023,22260,22498,22737,22977
			dc.w	23216,23457,23698,23940,24183,24426,24670,24915
			dc.w	25160,25406,25652,25900,26148,26396,26645,26895
			dc.w	27146,27397,27649,27902,28155,28409,28664,28919
			dc.w	29175,29432,29690,29948,30207,30466,30727,30988
			dc.w	31249,31512,31775,32039,32303,32568,32834,33101
			dc.w	33369,33637,33906,34175,34446,34717,34988,35261
			dc.w	35534,35808,36083,36359,36635,36912,37190,37468
			dc.w	37747,38028,38308,38590,38872,39155,39439,39724
			dc.w	40009,40295,40582,40870,41158,41448,41738,42029
			dc.w	42320,42613,42906,43200,43495,43790,44087,44384
			dc.w	44682,44981,45280,45581,45882,46184,46487,46791
			dc.w	47095,47401,47707,48014,48322,48631,48940,49251
			dc.w	49562,49874,50187,50500,50815,51131,51447,51764
			dc.w	52082,52401,52721,53041,53363,53685,54008,54333
			dc.w	54658,54983,55310,55638,55966,56296,56626,56957
			dc.w	57289,57622,57956,58291,58627,58964,59301,59640
			dc.w	59979,60319,60661,61003,61346,61690,62035,62381
			dc.w	62727,63075,63424,63774,64124,64476,64828,65182
			dc.w	00000

*==========================================================================*
*	Sine table for modulation effects
*==========================================================================*

		ifne HAS_MODULATION
;			DEFX	_sine_table
_sine_table
			dc.b	0,3,6,9,12,15,18,21,24,27,30,33,36,39,42,45
			dc.b	48,51,54,57,59,62,65,67,70,73,75,78,80,82,85,87
			dc.b	89,91,94,96,98,100,102,103,105,107,108,110,112,113,114,116
			dc.b	117,118,119,120,121,122,123,123,124,125,125,126,126,126,126,127
			dc.b	127,127,126,126,126,126,125,125,124,123,123,122,121,120,119,118
			dc.b	117,116,114,113,112,110,108,107,105,103,102,100,98,96,94,91
			dc.b	89,87,85,82,80,78,75,73,70,67,65,62,59,57,54,51
			dc.b	48,45,42,39,36,33,30,27,24,21,18,15,12,9,6,3
			dc.b	0,-3,-6,-9,-12,-15,-18,-21,-24,-27,-30,-33,-36,-39,-42,-45
			dc.b	-48,-51,-54,-57,-59,-62,-65,-67,-70,-73,-75,-78,-80,-82,-85,-87
			dc.b	-89,-91,-94,-96,-98,-100,-102,-103,-105,-107,-108,-110,-112,-113,-114,-116
			dc.b	-117,-118,-119,-120,-121,-122,-123,-123,-124,-125,-125,-126,-126,-126,-126,-127
			dc.b	-127,-127,-126,-126,-126,-126,-125,-125,-124,-123,-123,-122,-121,-120,-119,-118
			dc.b	-117,-116,-114,-113,-112,-110,-108,-107,-105,-103,-102,-100,-98,-96,-94,-91
			dc.b	-89,-87,-85,-82,-80,-78,-75,-73,-70,-67,-65,-62,-59,-57,-54,-51
   			dc.b	-48,-45,-42,-39,-36,-33,-30,-27,-24,-21,-18,-15,-12,-9,-6,-3
		endc


*==========================================================================*
*
*	InitMusic - initialize the music player, does not open audio device
*
*	WORD InitMusic(void)
*	WORD NewInitMusic(LONG scores)
*	WORD InitMusicTagList(LONG scores,struct TagItem *ti)
*
*==========================================================================*

AUDIO_MEM_SIZE		equ		((3*NUM_VOICES+3)*ioa_SIZEOF+2*MP_SIZE)

;			xdef	_InitMusic
;_InitMusic
;			xdef	InitMusic
InitMusic
			move.l	#NUM_SCORES,d0
;			suba.l	a0,a0
;			bra.s	InitMusicTagList

;			xdef	_NewInitMusic
;_NewInitMusic
;			move.l	4(sp),d0
;			xdef	NewInitMusic
;NewInitMusic
;			suba.l	a0,a0
;			bra.s	InitMusicTagList

;			xdef	_InitMusicTagList
;_InitMusicTagList
;			move.l	4(sp),d0
;			move.l	8(sp),a0
;			xdef	InitMusicTagList
;InitMusicTagList
;			movem.l	a2/a6,-(sp)
;			move.l	a0,a2								; save taglist (if any)

;			tst.l	_globaldata+glob_FrameUnit			; already inited?
;			bne		.99									; tell them OK

			move.w	d0,_scoremax						; maximum scores
;			mulu.w	#score_sizeof,d0					; allocate score buffers
;			add.l	#AUDIO_MEM_SIZE,d0					; and audio blocks

;			move.l	#MEMF_PUBLIC!MEMF_CLEAR,d1
;			move.l	_SysBase,a6
;			JSRLIB	AllocMem
			lea		_audio_play,a0

	MOVE.L	#Buffy,D0

			move.l	d0,(a0)								; audio_play buffers
;			beq		.98									; if zero, error

			move.l	d0,a1

			add.l	#AUDIO_MEM_SIZE,d0
			move.l	d0,_scoreptr

			lea		3*NUM_VOICES*ioa_SIZEOF(a1),a1		; init audio_ctrl ptr
			move.l	a1,4(a0)
			lea		ioa_SIZEOF(a1),a1					; init audio_stop ptr
			move.l	a1,8(a0)
			lea		ioa_SIZEOF(a1),a1					; init audio_env ptr
			move.l	a1,12(a0)
			lea		ioa_SIZEOF(a1),a1					; init play_port ptr
			move.b	#NT_MSGPORT,LN_TYPE(a1)
			move.b	#PA_IGNORE,MP_FLAGS(a1)
			move.l	a1,16(a0)
			lea		MP_SIZE(a1),a1						; init temp_port ptr
			move.b	#NT_MSGPORT,LN_TYPE(a1)
			move.b	#PA_IGNORE,MP_FLAGS(a1)
			move.l	a1,20(a0)

* init various global variables

;			moveq	#0,d1
;			move.b	VBlankFrequency(a6),d1				; get VBlankFreq

	MOVEQ	#50,D1

;			move.l	a2,d0								; tag list?
;			beq.s	.10
;			move.l	a2,a0
;			moveq	#MAXTRAX_VBLANKFREQ,d0
;			bsr		FindTag
;			tst.l	d0
;			beq.s	.10
;			move.l	d0,a0
;			move.w	6(a0),d1							; get user VBlankFreq

.10			lea		_globaldata,a0
			move.w	d1,glob_Frequency(a0)				; save VBlankFreq
			move.l	#(1000<<8),d0						; calc FrameUnit
			move.l	a0,-(sp)
			jsr		divu
			move.l	(sp)+,a0
			move.l	d0,glob_FrameUnit(a0)

			move.l	_scoreptr,glob_CurrentScore(a0)		; init current score

			moveq	#64,d0
			move.w	d0,glob_Volume(a0)					; Volume = 64
			move.b	d0,_maxtrax+mxtx_Volume
			moveq	#0,d0
			move.l	d0,glob_TempoTime(a0)				; TempoTime = 0
			move.l	d0,glob_UniqueID(a0)				; UniqueID = 0
			move.b	d0,glob_Flags(a0)					; Flags = 0

* set ColorClocks

;			cmp.w	#36,LIB_VERSION(a6)					; which version of OS?
;			bpl.s	.1

;			move.l	#NTSC_CLOCKS,glob_ColorClocks(a0)	; KS1.3, check GfxBase
;			move.l	_GfxBase,a1
;			btst.b	#PALn,gb_DisplayFlags+1(a1)
;			beq.s	.2
;			move.l	#PAL_CLOCKS,glob_ColorClocks(a0)
;			beq.s	.2

;.1			move.l	ex_EClockFrequency(a6),d0			; KS2.0, check ExecBase
;			move.l	d0,d1
;			add.l	d1,d1
;			add.l	d1,d1
;			add.l	d1,d0
;			move.l	d0,glob_ColorClocks(a0)

.2			lea		_xchannel,a0
			move.b	#16,chan_Number(a0)
			move.b	#0,chan_Flags(a0)
			move.b	#0,chan_VoicesActive(a0)

			bsr		ResetChannel						; reset xchannel

;			moveq	#INTB_VERTB,d0						; add vblank server
;			lea		_vblank_server,a1
;			move.l	_SysBase,a6
;			JSRLIB	AddIntServer

;			lea		StdOpenFunc,a1
;			move.l	a1,_maxtrax+mxtx_OpenFunc
;			lea		StdReadFunc,a1
;			move.l	a1,_maxtrax+mxtx_ReadFunc
;			lea		StdCloseFunc,a1
;			move.l	a1,_maxtrax+mxtx_CloseFunc

;.99			moveq	#MEV_ERR_NONE,d0					; no problem
;			movem.l	(sp)+,a2/a6
			rts

;.98			jsr		FreeMusic
;			moveq	#MEV_ERR_NO_MEMORY,d0				; problem!
;			movem.l	(sp)+,a2/a6
;			rts

*==========================================================================*
*
*	FreeMusic - close down music player, free all resources
*
*	void FreeMusic(void)
*
*==========================================================================*

;			xdef	_FreeMusic
;_FreeMusic
;			xdef	FreeMusic
;FreeMusic
;			move.l	a6,-(sp)

;			jsr		CloseMusic

;			move.l	#PERF_ALL,d0
;			jsr		UnloadPerf

;			move.l	_SysBase,a6

* remove vblank server

;			tst.l	_globaldata+glob_FrameUnit			; interrupt active?
;			beq.s	.1									; no, skip

;			moveq	#INTB_VERTB,d0						; add vblank server
;			lea		_vblank_server,a1
;			JSRLIB	RemIntServer

;			clr.l	_globaldata+glob_FrameUnit

;.1			move.l	_audio_play,d0						; audio blocks allocated?
;			beq.s	.2									; no, skip
;
;			move.l	d0,a1								; free score buffers
;			moveq	#0,d0								; & audio blocks
;			move.w	_scoremax,d0
;			mulu.w	#score_sizeof,d0
;			add.l	#AUDIO_MEM_SIZE,d0

;			JSRLIB	FreeMem

;			clr.l	_audio_play

;.2			move.l	(sp)+,a6
;			rts

*==========================================================================*
*
*	OpenMusic - allocate audio resources
*
*	WORD OpenMusic(void)
*
*==========================================================================*

audio_name	dc.b	'audio.device',0,0
;			ds.w	0

;			xdef	_OpenMusic
;_OpenMusic
OpenMusic
			movem.l	d2/a2/a3/a6,-(sp)

;			tst.l	_AudioDevice						; if music already open
;			bne		.99									;	done...

			jsr		GetAudioFilter						; save filter state
			move.b	d0,_globaldata+glob_SaveFilter
			bclr.b	#MUSICB_PLAYING,_globaldata+glob_Flags	; turn music off
			bclr.b	#MUSICB_PLAYING,_maxtrax+mxtx_Flags	; turn music off
			clr.b	_globaldata+glob_VoicesActive

* init channels

			moveq	#NUM_CHANNELS-1,d2					; init all channels
			move.l	#_channel+(NUM_CHANNELS-1)*chan_sizeof,a2
			move.l	#_patch+(NUM_CHANNELS-1)*patch_sizeof,a3
.1			move.l	a3,chan_Patch(a2)					; set patch number
			lea		-patch_sizeof(a3),a3
			move.b	d2,chan_Number(a2)					; set channel number
			clr.w	chan_RPN(a2)						; set RPN to 0
			move.l	a2,a0
			bsr		ResetChannel						; reset channel
			lea		-chan_sizeof(a2),a2
			dbra	d2,.1								; loop

* init patches

			moveq	#NUM_PATCHES-1,d2					; init all patches
			move.l	#_patch+(NUM_PATCHES-1)*patch_sizeof,a3
.4			move.b	d2,patch_Number(a3)					; set patch number
			lea		-patch_sizeof(a3),a3
			dbra	d2,.4								; loop

* init voices

			moveq	#NUM_VOICES-1,d2
			move.l	#_voice+(NUM_VOICES-1)*voice_sizeof,a2
.3			clr.l	voice_Channel(a2)					; no channel
			clr.b	voice_Status(a2)					; ENV_FREE
			move.b	d2,voice_Number(a2)					; voice number
			lea		-voice_sizeof(a2),a2
			dbra	d2,.3								; loop

* initialize message ports

			move.l	_play_port,a1
			lea		MP_MSGLIST(a1),a1
			NEWLIST a1
			move.l	_temp_port,a1
			lea		MP_MSGLIST(a1),a1
			NEWLIST a1

* init audio blocks

			move.l	_audio_ctrl,a1						; set-up open device
			move.b	#127,LN_PRI(a1)						; don't allow stealing
			move.l	_play_port,MN_REPLYPORT(a1)			; the reply port
			move.w	#$0f0f,-(sp)						; allocation key is 0f
			move.l	sp,ioa_Data(a1)						; data on stack
			moveq	#1,d0								; only one byte
			move.l	d0,ioa_Length(a1)

			lea		audio_name,a0						; call OpenDevice
			moveq	#0,d0
			moveq	#0,d1
			move.l	_SysBase,a6
			JSRLIB	OpenDevice
			addq.w	#2,sp								; clean-up stack
			
			tst.l	d0									; if not zero, an error
			bne		.98

			move.l	_audio_ctrl,a1
			move.l	IO_DEVICE(a1),d0
			move.l	d0,_AudioDevice						; get AudioDevice ptr

* intitialize audio stopping io block

			move.l	_audio_stop,a2						; set-up open device
			move.l	_play_port,MN_REPLYPORT(a2)			; the reply port
			move.l	d0,IO_DEVICE(a2)					; AudioDevice
			move.w	#CMD_FLUSH,IO_COMMAND(a2)			; CMD_FLUSH
			move.b	#IOF_QUICK,IO_FLAGS(a2)				; IOF_QUICK
			move.w	ioa_AllocKey(a1),ioa_AllocKey(a2)	; copy AllocKey

* init audio blocks and put on play_port

			moveq	#3*NUM_VOICES-1,d2
			move.l	_audio_play,a3
.2			move.l	a2,a0								; copy data to play iob
			move.l	a3,a1
			moveq	#ioa_SIZEOF,d0
			JSRLIB	CopyMem
			move.w	#CMD_WRITE,IO_COMMAND(a3)			; make it CMD_WRITE
			move.l	a3,a1
			JSRLIB	ReplyMsg							; put on port
			lea		ioa_SIZEOF(a3),a3
			dbra	d2,.2								; loop

			move.l	a2,a0								; copy data to env iob
			move.l	_audio_env,a3
			move.l	a3,a1
			moveq	#ioa_SIZEOF,d0
			JSRLIB	CopyMem
			move.w	#ADCMD_PERVOL,IO_COMMAND(a3)		; make it ADCMD_PERVOL

			move.l	_temp_port,d0
			move.l	d0,MN_REPLYPORT(a2)					; reply stop to temp port
			move.l	d0,MN_REPLYPORT(a3)					; reply env to temp port

;			xref	_init_ahandler
;			jsr		_init_ahandler

.99			moveq	#MEV_ERR_NONE,d0
			movem.l	(sp)+,d2/a2/a3/a6
			rts

.98
;			moveq	#MEV_ERR_AUDIODEV,d0

	MOVEQ	#EPR_CantAllocAudio,D0

			movem.l	(sp)+,d2/a2/a3/a6
			rts

*==========================================================================*
*
*	CloseMusic - de-allocate audio resources
*
*	void CloseMusic(void)
*
*==========================================================================*

;			xdef	_CloseMusic
;_CloseMusic
;			xdef	CloseMusic
CloseMusic
			movem.l	d2/a2/a6,-(sp)

			tst.l	_AudioDevice					; music open?
			beq.s	.99								; no, so don't close

			bclr.b	#MUSICB_PLAYING,_globaldata+glob_Flags	; turn music off
			bclr.b	#MUSICB_PLAYING,_maxtrax+mxtx_Flags	; turn music off
			bclr.b	#MUSICB_PLAYNOTE,_maxtrax+mxtx_Flags

* kill all voices with SystemReset & stop_audio

			bsr		SystemReset

			move.w	#NUM_VOICES-1,d2
			lea		_voice,a2
.2			btst.b	#VOICEB_BLOCKED,voice_Flags(a2)
			beq.s	.1
			move.w	d2,d0
			bsr		stop_audio
.1			clr.b	voice_Flags(a2)
			lea		voice_sizeof(a2),a2
			dbra	d2,.2

			moveq	#0,d0								; restore audio filter
			move.b	_globaldata+glob_SaveFilter,d0
			jsr		SetAudioFilter

;			xref	_free_ahandler
;			jsr		_free_ahandler

* close audio device

			move.l	_audio_ctrl,a1
			move.l	_SysBase,a6
			JSRLIB	CloseDevice

			clr.l	_AudioDevice

.99			movem.l	(sp)+,d2/a2/a6
			rts

*==========================================================================*
*
*	NoteOn - handles a NOTE ON midi command
*
*	void NoteOn(struct ChannelData *chan,UBYTE *midi,WORD pri)
*
*==========================================================================*

;		ifne C_ENTRY
;			DEFX	_NoteOn
;_NoteOn
;			move.l	4(sp),a0							; channel
;			move.l	8(sp),a1							; midi stream
;			move.w	12(sp),d0							; priority
;		endc
NoteOn
			movem.l	d2-d5/a2/a3/a5/a6,-(sp)
			move.l	a0,a2
			move.l	a1,a3
			move.w	d0,d2

;		ifne HAS_MICROTONAL
;			move.w	chan_Microtonal(a2),d1				; microtinal setting?
;			bmi		.1									; no, skip
;			moveq	#0,d0
;			move.b	(a3),d0								; note #
;			add.w	d0,d0
;			lea		_microtonal,a5
;			move.w	d1,0(a5,d0.w)						; set microtonal table
;.1
;		endc

			tst.b	1(a1)								; vol = 0 -> NOTE OFF
			beq		.99									; just ignore

			move.l	chan_Patch(a2),a5					; get patch ptr
			tst.l	patch_Sample(a5)					; no sample, exit
			beq		.99

			btst.b	#CHANB_MONO,chan_Flags(a2)			; in mono mode?
			beq.s	.2									; no, skip mono stuff
			tst.b	chan_VoicesActive(a2)				; already voice on?
			beq.s	.2									; no, skip mono stuff

			lea		_voice,a5
			moveq	#NUM_VOICES-1,d0
.3			cmp.l	voice_Channel(a5),a2		; any voice REALLY on channel
			beq.s	.4
			lea		voice_sizeof(a5),a5
			dbra	d0,.3

.4			tst.w	d0									; if none, error
			bmi		.99

			cmp.b	#ENV_SUSTAIN,voice_Status(a5)		; sustain or earlier?
			bmi.s	.5									; no, skip porta code
			btst.b	#CHANB_PORTAMENTO,chan_Flags(a2)	; portamento?
			beq.s	.5									; no, skip porta code

			clr.l	voice_PortaTicks(a5)				; init portamento

			bset.b	#VOICEB_PORTAMENTO,voice_Flags(a5)	; set & test
			beq.s	.24							; not already portamento, skip

			move.b	voice_EndNote(a5),voice_BaseNote(a5) ; leap to old end note
			
.24			move.b	(a3),voice_EndNote(a5)
			move.b	(a3),chan_LastNote(a2)				; remember note

			move.w	#128,d1								; default note velocity
			btst.b	#MUSICB_VELOCITY,_globaldata+glob_Flags	; handling note vel?
			beq.s	.22									; no, just use 128
			moveq	#0,d1
			move.b	1(a3),d1							; get note volume
			addq.b	#1,d1
.22			move.w	d1,voice_NoteVolume(a5)				; set note volume

			bset.b	#MUSICB_ADDED_NOTE,_globaldata+glob_Flags
			move.b	voice_Number(a5),_globaldata+glob_LastVoice
			bra		.99

.2			moveq	#LEFT_0,d0
			btst.b	#CHANB_PAN,chan_Flags(a2)			; pan set?
			beq.s	.6									; no, left voice
			moveq	#RIGHT_0,d0							; yes, right voice
.6			move.w	d2,d1
			bsr		pick_voice							; pick a voice
			tst.l	d0
			beq		.99									; no voice, exit
			move.l	d0,a5

.5			tst.l	voice_Channel(a5)				; if voice in use, kill it
			beq.s	.7

			move.l	a5,a0
			bsr		KillVoice
			move.b	#VOICE_STOLEN,voice_Flags(a5)		; mark as stolen
			bra.s	.8
.7			clr.b	voice_Flags(a5)

.8			move.l	a2,voice_Channel(a5)				; setup voice data
			move.l	chan_Patch(a2),voice_Patch(a5)

			move.b	(a3),voice_BaseNote(a5)

			move.l	a5,a0
			bsr		CalcNote							; calc note period
			move.l	d0,d3								; save sample in d3

			move.b	d2,voice_Priority(a5)				; more voice setup
			move.b	#ENV_START,voice_Status(a5)
			move.w	#128,d1								; default note velocity
			btst.b	#MUSICB_VELOCITY,_globaldata+glob_Flags	; handling note vel?
			beq.s	.23									; no, just use 128
			moveq	#0,d1
			move.b	1(a3),d1
			addq.b	#1,d1

.23
		ifeq HAS_FULLCHANVOL
			moveq	#0,d0
			move.b	chan_Volume(a2),d0				; get channel volume
			bmi.s	.25								; if vol >= 128, skip
			mulu.w	d0,d1							; multiply into volume
			lsr.w	#7,d1							; scale by 128
		endc

.25			move.w	d1,voice_NoteVolume(a5)
			clr.w	voice_BaseVolume(a5)
			clr.l	voice_LastTicks(a5)

			move.w	voice_LastPeriod(a5),d5
			bne.s	.9
			move.w	#1000,d5

.9			move.l	d3,a0
			tst.l	samp_AttackSize(a0)					; any attack wave?
			beq	.10									; no, skip

			move.l	_SysBase,a6							; get an audio block
			move.l	_play_port,a0
			JSRLIB	GetMsg
			tst.l	d0
			beq		.98									; error, undo voice init

			move.l	d0,a1								; set-uo audio request
			move.w	#CMD_WRITE,IO_COMMAND(a1)
			move.b	#ADIOF_PERVOL!IOF_QUICK,IO_FLAGS(a1)
			clr.b	IO_ERROR(a1)
			move.w	#1,ioa_Cycles(a1)
			clr.w	ioa_Volume(a1)
			move.w	d5,ioa_Period(a1)
			moveq	#1,d1
			moveq	#0,d0
			move.b	voice_Number(a5),d0
			lsl.l	d0,d1
			move.l	d1,IO_UNIT(a1)
			move.l	d3,a0
			move.l	samp_Waveform(a0),ioa_Data(a1)
			move.l	samp_AttackSize(a0),ioa_Length(a1)

	BSR.W	SetTwo

			move.l	a1,d4								; save ptr to audio block
			move.l	_AudioDevice,a6
			JSRDEV	BEGINIO

			move.l	d4,a1
			tst.b	IO_ERROR(a1)						; any error?
			beq.s	.51									; no, skip error code

			move.l	_SysBase,a6
			JSRLIB	ReplyMsg							; put message back
			bra		.98									; goto error code

.51
;			DMACHECK

.10
	BSR.W	DMAWait

			move.l	d3,a0
			tst.l	samp_SustainSize(a0)				; any sustain wave?
			beq		.11									; no, skip

			move.l	_SysBase,a6							; get an audio block
			move.l	_play_port,a0
			JSRLIB	GetMsg
			tst.l	d0
			beq		.98									; error, undo voice init

			move.l	d0,a1								; set-uo audio request
			move.w	#CMD_WRITE,IO_COMMAND(a1)
			move.b	#IOF_QUICK,IO_FLAGS(a1)
			clr.b	IO_ERROR(a1)
			clr.w	ioa_Cycles(a1)
			moveq	#1,d1
			moveq	#0,d0
			move.b	voice_Number(a5),d0
			lsl.l	d0,d1
			move.l	d1,IO_UNIT(a1)
			move.l	d3,a0
			move.l	samp_SustainSize(a0),ioa_Length(a1)
			move.l	samp_Waveform(a0),d0
			add.l	samp_AttackSize(a0),d0
			move.l	d0,ioa_Data(a1)
			tst.l	samp_AttackSize(a0)					; was there an attack?
			bne.s	.12									; yes, so ready

			move.b	#ADIOF_PERVOL!IOF_QUICK,IO_FLAGS(a1)	; no, do full set-up
			clr.w	ioa_Volume(a1)
			move.w	d5,ioa_Period(a1)

.12			move.l	a1,d4								; save ptr to audio block
			move.l	_AudioDevice,a6
			JSRDEV	BEGINIO

			move.l	d4,a1
			tst.b	IO_ERROR(a1)						; any error?
			beq.s	.52									; no, skip error code

			move.l	_SysBase,a6
			JSRLIB	ReplyMsg							; put message back

			move.l	d3,a0
			tst.l	samp_AttackSize(a0)					; was there an attack?
			beq.s	.98									; yes, so do error code
;			bra.s	.11

.52
;			DMACHECK

.11
	BSR.W	DMAWait

			addq.b	#1,chan_VoicesActive(a2)			; up voices active
			addq.b	#1,_globaldata+glob_VoicesActive

			cmp.b	#16,chan_Number(a2)					; xchannel?
			beq.s	.13									; yes, skip normal code

			btst.b	#CHANB_MONO,chan_Flags(a2)			; mono mode?
			beq.s	.14									; no, skip porta stuff
			btst.b	#CHANB_PORTAMENTO,chan_Flags(a2)	; portamento on?
			beq.s	.14									; no, skip porta stuff
			move.b	chan_LastNote(a2),d0
			bmi.s	.14									; bit 7 set means no
			cmp.b	voice_BaseNote(a5),d0				; Last Note == BaseNote
			beq.s	.14									; yes, no need for porta

			clr.l	voice_PortaTicks(a5)				; init portamento
			move.b	voice_BaseNote(a5),voice_EndNote(a5)	; EndNote = BaseNote
			move.b	d0,voice_BaseNote(a5)				; BaseNote = LastNote
			bset.b	#VOICEB_PORTAMENTO,voice_Flags(a5)

.14			btst.b	#CHANB_PORTAMENTO,chan_Flags(a2)	; portamento on?
			beq.s	.13									; no, skip
			move.b	(a3),chan_LastNote(a2)				; remember note

.13			bset.b	#MUSICB_ADDED_NOTE,_globaldata+glob_Flags
			move.b	voice_Number(a5),_globaldata+glob_LastVoice

.99			movem.l	(sp)+,d2-d5/a2/a3/a5/a6
			rts

.98			clr.l	voice_Channel(a5)					; un-init voice
			clr.b	voice_Status(a5)
			clr.b	voice_Flags(a5)
			clr.b	voice_Priority(a5)
			clr.l	voice_UniqueID(a5)
			bra		.99

* this is a temporary hack that will become a better, permanent hack later

;		ifne	DO_DMACHECK
;dma_check
;			move.l	a1,d0
;			move.l	_AudioDevice,a1						; use BeginIO
;			cmp.w	#36,LIB_VERSION(a1)					; if KS v35 or less, skip
;			bmi.s	.3

;			move.l	d0,a1

;			move.w	$dff000+vhposr,d0					; wait 1 scanline
;			and.w	#$ff00,d0
;.1			move.w	$dff000+vhposr,d1
;			and.w	#$ff00,d1
;			cmp.w	d0,d1
;			beq.s	.1

;			move.w	#139,d0
;.2			move.w	$dff000+vhposr,d1
;			dbra	d0,.2

;			move.w	IO_UNIT+2(a1),d0					; be sure note on!
;			or.w	#$8000,d0
;			move.w	d0,$dff000+dmacon

;.3			rts
;		endc

*==========================================================================*
*
*	NoteOff - handles a NOTE OFF midi command
*
*	void NoteOff(struct ChannelData *chan,UBYTE *midi)
*
*	NOTE: Only has code for MaxTrax version, not MIDI
*
*==========================================================================*

;		ifne C_ENTRY
;			DEFX	_NoteOff
;_NoteOff
;			move.l	4(sp),a0						; channel
;			move.l	8(sp),a1						; midi stream
;		endc
NoteOff
			move.l	a2,-(sp)

			tst.b	chan_VoicesActive(a0)			; any voices on channel?
			beq.s	.99								; no, exit

			lea		_voice,a2						; calc voice to release
			moveq	#0,d0
			move.b	_globaldata+glob_LastVoice,d0
			SCALE	voice_sizeof,d0
			add.w	d0,a2

			cmp.l	voice_Channel(a2),a0			; if channel wrong, exit
			bne.s	.99

			cmp.b	#ENV_RELEASE,voice_Status(a2)	; if already released, exit
			ble.s	.99

* confirm correct note based on portamento setting

			move.b	(a1),d0							; note to check

			btst.b	#VOICEB_PORTAMENTO,voice_Flags(a2)
			beq.s	.1

			cmp.b	voice_EndNote(a2),d0			; does note match EndNote?
			bne.s	.99								; no, exit
			bra.s	.2								; yes, continue

.1			cmp.b	voice_BaseNote(a2),d0			; does note match BaseNote?
			bne.s	.99								; no, exit

.2			btst.b	#CHANB_DAMPER,chan_Flags(a0)	; 'damper pedal' down?
			bne.s	.3								; yes, do damper code

			move.b	#ENV_RELEASE,voice_Status(a2)	; release note
			bra.s	.99

.3			bset.b	#VOICEB_DAMPER,voice_Flags(a2)	; mark as dampered

.99			move.l	(sp)+,a2
			rts

*==========================================================================*
*
*	AllNotesOff - handles a ALL NOTES OFF midi command
*
*	void AllNotesOff(struct ChannelData *chan)
*
*==========================================================================*

;		ifne C_ENTRY
;			DEFX	_AllNotesOff
;_AllNotesOff
;			move.l	4(sp),a0
;		endc
AllNotesOff
			tst.b	chan_VoicesActive(a0)			; no voices active, exit
			beq.s	.99

			lea		_voice,a1						; go through all voices
			moveq	#NUM_VOICES-1,d0
.1			cmp.l	voice_Channel(a1),a0			; on this channel?
			bne.s	.2								; no, skip

			btst.b	#CHANB_DAMPER,chan_Flags(a0)	; 'damper pedal' down
			beq.s	.3								; no, goto release
			bset.b	#VOICEB_DAMPER,voice_Flags(a1)	; mark voice as dampered
			bra.s	.2

.3			move.b	#ENV_RELEASE,voice_Status(a1)	; set status to RELEASE

.2			lea		voice_sizeof(a1),a1				; next voice
			dbra	d0,.1

.99			rts

*==========================================================================*
*
*	AllSoundsOff - handles a ALL SOUNDS OFF midi command
*
*	void AllSoundsOff(struct ChannelData *chan)
*
*==========================================================================*

;		ifne C_ENTRY
;			DEFX	_AllSoundsOff
;_AllSoundsOff
;			move.l	4(sp),a0
;		endc
AllSoundsOff
			tst.b	chan_VoicesActive(a0)			; no voices active, exit
			beq.s	.99

			movem.l	d2/a2/a3,-(sp)

			move.l	a0,a3							; save channel
			lea		_voice,a2						; go through all voices
			moveq	#NUM_VOICES-1,d2

.1			cmp.l	voice_Channel(a2),a3			; on this channel?
			bne.s	.2								; no, skip

			move.l	a2,a0
			bsr		KillVoice

.2			lea		voice_sizeof(a2),a2				; next voice
			dbra	d2,.1

			movem.l	(sp)+,d2/a2/a3

.99			rts

*==========================================================================*
*
*	SystemReset - handles SYSTEM RESET, terminates sound on all voices
*
*	void SystemReset(void)
*
*==========================================================================*

;			DEFX	_SystemReset
;_SystemReset
SystemReset
			movem.l	d2/a2,-(sp)

* kill all active voices

			lea		_voice,a2
			moveq	#NUM_VOICES-1,d2
.1			tst.l	voice_Channel(a2)
			beq.s	.2
			move.l	a2,a0
			bsr		KillVoice
.2			lea		voice_sizeof(a2),a2
			dbra	d2,.1

* reset all channels

			lea		_channel,a2
			moveq	#NUM_CHANNELS-1,d2
.3			move.l	a2,a0
			bsr		ResetChannel
			bclr.b	#CHANB_MONO,chan_Flags(a2)
			moveq	#-1,d0
			move.b	d0,chan_LastNote(a2)				; no last note
			lea		chan_sizeof(a2),a2
			dbra	d2,.3

;		ifne HAS_MICROTONAL

* reset microtonal values to defaults

;			lea		_microtonal,a2
;			moveq	#128-1,d2
;			moveq	#0,d0
;.4			move.w	d0,(a2)+
;			add.w	#$0100,d0
;			dbra	d2,.4
;		endc

			movem.l	(sp)+,d2/a2
			rts

*==========================================================================*
*
*	ProgramCh - handles PROGRAM CHANGE midi command
*
*	void ProgramCh(struct ChannelData *chan,UBYTE *midi)
*
*==========================================================================*

;		ifne C_ENTRY
;			DEFX	_ProgramCh
;_ProgramCh
;			move.l	4(sp),a0						; channel
;			move.l	8(sp),a1						; midi stream
;		endc
ProgramCh
			moveq	#0,d0
			move.b	(a1),d0							; get patch number
			and.w	#NUM_PATCHES_MASK,d0			; mask to allowable range
			SCALE	patch_sizeof,d0
			lea		_patch,a1						; get base of patches
			add.w	d0,a1							; adjust to right patch
			move.l	a1,chan_Patch(a0)				; save pointer in channel
			rts

*==========================================================================*
*
*	SetTempo - adjust timing to match tempo
*
*	void SetTempo(UWORD tempo)
*
*==========================================================================*

;			xdef	_SetTempo
;_SetTempo
;			move.l	4(sp),d0

;			xdef	SetTempo
SetTempo
			lea		_globaldata,a0
			move.w	d0,glob_CurrentTempo(a0)		; save new tempo

			lsr.w	#4,d0					; (tempo>>4)*(192<<8)/(60*Frequency)
			mulu.w	#(192<<8),d0
			move.w	glob_Frequency(a0),d1
			mulu.w	#60,d1
			move.l	a0,-(sp)
			jsr		divu
			move.l	(sp)+,a0
			move.l	d0,glob_TickUnit(a0)			; -> TickUnit
			rts

*==========================================================================*
*
*	GetTempo - return current tempo setting
*
*	LONG GetTempo(void)
*
*==========================================================================*

;			xdef	_GetTempo
;			xdef	GetTempo
;_GetTempo
;GetTempo
;			lea		_globaldata,a0
;			moveq	#0,d0
;			move.w	glob_CurrentTempo(a0),d0		; get current tempo
;			rts

*==========================================================================*
*
*	SyncSong - set-up or clear song synchronization
*
*	void SyncSong(struct Task *task,ULONG signal)
*
*==========================================================================*

;			xdef	_SyncSong
;_SyncSong
;			move.l	4(sp),a0						; task
;			move.l	8(sp),d0						; signal

;			xdef	SyncSong
;SyncSong
;			lea		_globaldata,a1
;			move.l	a0,glob_SyncTask(a1)			
;			move.l	d0,glob_SyncSig(a1)			
;			rts

*==========================================================================*
*
*	SetAudioFilter - turn on or off audio filter
*
*	void SetAudioFilter(BOOL)
*
*==========================================================================*

;			xdef	_SetAudioFilter
;_SetAudioFilter
;			move.l	4(sp),d0

;			xdef	SetAudioFilter
SetAudioFilter
			move.w	d0,d1
			bsr.s	GetAudioFilter
			tst.w	d1								; test single argument
			bne.s	.1
			bset	#1,$bfe001
			rts
.1			bclr	#1,$bfe001
			rts

*==========================================================================*
*
*	GetAudioFilter - read state of audio filter
*
*	int GetAudioFilter(void)
*
*==========================================================================*

;			xdef	_GetAudioFilter
;_GetAudioFilter
GetAudioFilter
			move.b	$bfe001,d0						; check hardware
			btst	#1,d0
			bne.s	.1								; if bit set, filter is OFF
			moveq	#1,d0							; else it's ON
			rts
.1			moveq	#0,d0							; it's OFF
			rts

*==========================================================================*
*
*	MusicServer - handles score and timing
*
*	void MusicServer(void)
*
*==========================================================================*

; Changes made by --DJ
; Changed all references to _maxtrax and _globaldata to be relative addressing

;			DEFX	_MusicServer
;_MusicServer
MusicServer
;			COLOR0	$0ff0
			movem.l	d2/d3/d4/d5/a2-a6,-(sp)
			subq.w	#2,sp							; space for MIDI stream

			lea		_maxtrax,a5						; preload address of _maxtrax
			lea		_globaldata,a4					; preload address of _globaldata

			tst.b	mxtx_Changed(a5)
			beq.s	.30

			moveq	#0,d0
			move.b	mxtx_Volume(a5),d0				; copy over volume
			move.w	d0,glob_Volume(a4)
			move.b	mxtx_Flags(a5),d0
			and.b	#MUSIC_VELOCITY,d0				; copy over velocity flag
			and.b	#~MUSIC_VELOCITY,glob_Flags(a4)
			or.b	d0,glob_Flags(a4)

			lea		_voice+voice_Channel,a0			; mark all used channels ALTERED
			moveq	#NUM_VOICES-1,d2
.31			move.l	(a0),d0							; d0 <-- channel data pointer
			beq.s	.32								; if no channel data, skip
			move.l	d0,a1							; a1 <-- channel data pointer
			bset.b	#CHANB_ALTERED,chan_Flags(a1)	; set channel as altered
.32			lea		voice_sizeof(a0),a0				; go to next voice
			dbra	d2,.31							; and loop

			clr.b	mxtx_Changed(a5)

.30			move.l	glob_TickUnit(a4),d0			; add TickUnit to Ticks
			add.l	d0,glob_Ticks(a4)

			move.l	glob_Ticks(a4),d4				; shifted clock value
			lsr.l	#8,d4

			moveq	#0,d5
			moveq	#NUM_VOICES-1,d2				; check stop events
			lea		glob_NoteOff(a4),a2				; first NoteOff
			lea		_voice,a3						; first voice

; Q: Why isn't this integrated into the actual Voice table?

; look at the table of note-offs (one for each audio channel)

.1			move.b	d5,glob_LastVoice(a4)			; set last voice
			tst.l	voice_Channel(a3)				; voice playing something?
			beq.s	.2								; no, skip
			move.b	sev_Command(a2),d0				; get note # (error ??)
			bmi.s	.2								; high bit set, no note

			moveq	#0,d1
			move.b	sev_Data(a2),d1					; channel #
			cmp.b	#16,d1							; is it the extra channel?
			beq.s	.3								; yes, do that instead

			move.l	glob_TickUnit(a4),d3			; get TickUnit
			sub.l	d3,sev_StopTime(a2)				; subtract from stoptime
			bgt.s	.2								; still time left, skip

; why is this needed?

			move.b	d0,(sp)							; set-up NoteOff
			clr.b	1(sp)
			lea		_channel,a0						; figure out channel addr
			SCALE	chan_sizeof,d1
			add.w	d1,a0
			move.l	sp,a1
			bsr		NoteOff							; do a NoteOff
			move.b	#$ff,sev_Command(a2)			; set stop event to not used
			bra.s	.2

;.4			sub.l	d3,sev_StopTime(a2)				; subtract TickUnit
;			bra.s	.2

.3			move.l	glob_FrameUnit(a4),d3			; get FrameUnit
			sub.l	d3,sev_StopTime(a2)				; subtract from stoptime
			bgt.s	.2								; still time left, skip

			move.b	d0,(sp)							; set-up NoteOff
			clr.b	1(sp)
			lea		_xchannel,a0					; get xchannel addr
			move.l	sp,a1
			bsr		NoteOff							; do a NoteOff
			move.b	#$ff,sev_Command(a2)			; set stop event to not used

.2			addq.w	#1,d5
			lea		sev_sizeof(a2),a2
			lea		voice_sizeof(a3),a3
			dbra	d2,.1

			btst.b	#MUSICB_PLAYING,glob_Flags(a4)	; music playing?
			beq		.20								; no, skip sequencer code
													; also skipping tempo code

			;COLOR0	$00f0

			move.l	glob_Current(a4),a2				; get current event ptr

*	Here is a real serious kludge to keep from having to much time spent
*	in this interupt...

.5
;			move.w	$00dff000+vhposr,d0				; get beam position
;			bmi		.60								; if >= 128, prematurely end

			moveq	#0,d0
			move.w	cev_StartTime(a2),d0
			add.l	glob_CurrentTime(a4),d0			; calc new CurrrentTime
			cmp.l	d0,d4							; compare event time w/clocks
			bmi		.60								; if greater, stop processing

			move.l	d0,glob_CurrentTime(a4)			; store new CurrentTime
			move.b	cev_Command(a2),d1				; get command
			bmi.s	.7								; not a note, skip ahead

													; reset note added flag
			bclr.b	#MUSICB_ADDED_NOTE,glob_Flags(a4)

			move.b	d1,(sp)							; note #
			move.b	cev_Data(a2),d1					; volume & channel
			moveq	#0,d2
			move.b	d1,d2							; keep copy
			lsr.b	#1,d1
			and.b	#$78,d1							; extract volume
			move.b	d1,1(sp)						; second MIDI value

			and.b	#$0f,d2							; extract channel #
			SCALE	chan_sizeof,d2
			lea		_channel,a0
			add.w	d2,a0							; calc channel addr
			move.l	sp,a1							; MIDI stream
			moveq	#MUSIC_PRI_SCORE,d0				; priority 0 normally

;			COLOR0	$00ff

			bsr		NoteOn

;			COLOR0	$0f0f
													; was a note added?
			btst.b	#MUSICB_ADDED_NOTE,glob_Flags(a4)
			beq		.70								; nope, skip

			moveq	#0,d0
			move.b	glob_LastVoice(a4),d0
			SCALE	sev_sizeof,d0
			lea		glob_NoteOff(a4),a0
			add.w	d0,a0

			move.b	(sp),sev_Command(a0)			; set-up stop event
			move.b	cev_Data(a2),d0
			and.b	#$0f,d0
			move.b	d0,sev_Data(a0)
			moveq	#0,d0
			move.w	cev_StopTime(a2),d0				; get stop time
			add.l	glob_CurrentTime(a4),d0			; add low word CurrentTime
			sub.l	d4,d0							; sub low word Ticks
			lsl.l	#8,d0							; put into TickUnit units
			move.l	d0,sev_StopTime(a0)				; store it!
			bra		.70

.7			cmp.b	#COMMAND_TEMPO,d1					; tempo event?
			bne.s	.8									; no, skip

			move.l	glob_TickUnit(a4),d0
			lsr.l	#8,d0
			cmp.w	cev_StopTime(a2),d0					; is length < TickUnit?
			bmi.s	.9									; no, do continuous

			moveq	#0,d0
			move.b	cev_Data(a2),d0
			lsl.w	#4,d0
			bsr		SetTempo							; set tempo immediately
			clr.l	glob_TempoTime(a4)					; clear tempo time
			bra		.70

.9			move.w	glob_CurrentTempo(a4),d0
			move.w	d0,glob_StartTempo(a4)				; StartTempo = CurrentTempo
			moveq	#0,d1
			move.b	cev_Data(a2),d1
			lsl.w	#4,d1
			sub.w	d0,d1								; (tempo<<4) - StartTempo
			move.w	d1,glob_DeltaTempo(a4)				; store as DeltaTempo

			moveq	#0,d0
			move.w	cev_StopTime(a2),d0
			lsl.l	#8,d0
			move.l	d0,glob_TempoTime(a4)				; length of tempo change
			clr.l	glob_TempoTicks(a4)					; reset tick count
			bra		.70

.8			cmp.b	#COMMAND_END,d1						; end event?
			bne.s	.10									; no, skip to next

	BSR.W	SongEnd

			btst.b	#MUSICB_LOOP,glob_Flags(a4)			; looping score?
			beq.s	.11									; no, cut off
			move.l	glob_CurrentScore(a4),a0			; get start of score
			move.l	score_Data(a0),a2					; make it current event
			clr.l	glob_Ticks(a4)						; reset clocks
			clr.l	glob_CurrentTime(a4)
			bra		.60									; process no more events

.11			bclr.b	#MUSICB_PLAYING,glob_Flags(a4)		; stop music
			bra		.60									; process no more events

.10			cmp.b	#COMMAND_BEND,d1					; pitch bend command?
			bne.s	.12									; no, skip to next

			move.b	cev_StopTime+1(a2),d0				; LSB of bend
			and.b	#$7f,d0
			move.b	d0,(sp)
			move.b	cev_StopTime(a2),d0					; MSB of bend
			and.b	#$7f,d0
			move.b	d0,1(sp)

			move.b	cev_Data(a2),d0						; get channel
			and.w	#$000f,d0
			SCALE	chan_sizeof,d0
			lea		_channel,a0
			add.w	d0,a0
			move.l	sp,a1								; MIDI stream
			bsr		PitchBend
			bra		.70

.12			cmp.b	#COMMAND_CONTROL,d1					; control change command?
			bne.s	.13									; no, skip to next

			move.b	cev_StopTime(a2),(sp)				; control #
			move.b	cev_StopTime+1(a2),1(sp)			; value

			move.b	cev_Data(a2),d0						; get channel
			and.w	#$000f,d0
			SCALE	chan_sizeof,d0
			lea		_channel,a0
			add.w	d0,a0
			move.l	sp,a1								; MIDI stream
			bsr		ControlCh
			bra		.70

.13			cmp.b	#COMMAND_PROGRAM,d1					; program change command?
			bne.s	.14									; no, skip to next

			move.b	cev_StopTime+1(a2),(sp)				; program #

			move.b	cev_Data(a2),d0						; get channel
			and.w	#$000f,d0
			SCALE	chan_sizeof,d0
			lea		_channel,a0
			add.w	d0,a0
			move.l	sp,a1								; MIDI stream
			bsr		ProgramCh
			bra		.70

.14			cmp.b	#COMMAND_SPECIAL,d1					; program change command?
			bne		.70									; no, that's all

			moveq	#0,d1
			move.b	cev_StopTime+1(a2),d1
			move.b	cev_StopTime(a2),d0

			cmp.b	#SPECIAL_SYNC,d0					; sync event?
			bne.s	.15									; no, skip to next

			move.l	glob_SyncTask(a4),d0				; is there a sync task?
			beq		.70

			move.l	d0,a1								; put in right register
			move.l	glob_SyncSig(a4),d0					; get signal
			move.b	d1,mxtx_SyncValue(a5)				; set sync value

			move.l	_SysBase,a6
			JSRLIB	Signal								; signal task
			bra.s	.70

.15			cmp.b	#SPECIAL_BEGINREP,d0				; begin repeat event?
			bne.s	.16									; no, skip to next

			moveq	#0,d0
			move.b	glob_RepeatTotal(a4),d0				; get repeat total
			cmp.b	#NUM_REPEATS,d0						; test against max
			beq.s	.70									; oops, too many

			addq.b	#1,glob_RepeatTotal(a4)				; increment total

			lea		glob_RepeatCount(a4),a0				; value is repeat count
			move.b	d1,0(a0,d0.w)

			lea		glob_RepeatPoint(a4),a0
			add.w	d0,d0
			add.w	d0,d0
			add.w	d0,a0
			lea		cev_sizeof(a2),a1					; get event + 1
			move.l	a1,(a0)								; store in RepeatPoint
			bra.s	.70

.16			cmp.b	#SPECIAL_ENDREP,d0					; begin repeat event?
			bne.s	.70									; no, that's all

			moveq	#0,d0
			move.b	glob_RepeatTotal(a4),d0				; get repeat total
			beq.s	.70									; ain't any?? oh, well

			subq.w	#1,d0								; find right entry
			lea		glob_RepeatCount(a4),a0
			tst.b	0(a0,d0.w)							; test repeat count
			beq.s	.17									; last loop, skip ahead

			subq.b	#1,0(a0,d0.w)						; reduce count
			lea		glob_RepeatPoint(a4),a0				; get repeat point
			add.w	d0,d0
			add.w	d0,d0
			add.w	d0,a0
			move.l	(a0),a2								; set event ptr

			clr.l	glob_Ticks(a4)						; reset clocks
			clr.l	glob_CurrentTime(a4)
			bra.s	.60

.17			move.b	d0,glob_RepeatTotal(a4)				; reduce total
			bra		.70									; continue

.70			lea		cev_sizeof(a2),a2					; go to next event
			bra		.5									; and loop

.60			move.l	a2,glob_Current(a4)					; store last ev checked

			;COLOR0	$0f00

.80			move.l	glob_TempoTime(a4),d0				; cont. tempo on?
			beq.s	.20									; nope...

			move.l	glob_TempoTicks(a4),d1				; adjust TempoTicks
			add.l	glob_TickUnit(a4),d1
			move.l	d1,glob_TempoTicks(a4)

			cmp.l	d0,d1
			bmi.s	.21

			move.w	glob_StartTempo(a4),d0
			add.w	glob_DeltaTempo(a4),d0
			bsr		SetTempo
			clr.l	glob_TempoTime(a4)					; clear tempo time
			bra.s	.20

.21			moveq	#0,d0
			move.w	glob_DeltaTempo(a4),d0
			jsr		mulu								; DeltaTempo * TempoTicks
			move.l	glob_TempoTime(a4),d1
			jsr		divs								; / TempoTime
			add.w	glob_StartTempo(a4),d0
			bsr		SetTempo

.20			move.l	glob_FrameUnit(a4),d0				; do envelopes
;			COLOR0	$0fff
			bsr		EnvelopeManager
;			COLOR0	$0777

* set silent flag correctly and do check_sound calls as needed

			bset.b	#MUSICB_SILENT,glob_Flags(a4)

			lea		_voice,a2
			moveq	#NUM_VOICES-1,d2
.22			tst.l	voice_Channel(a2)
			beq.s	.23
			bclr.b	#MUSICB_SILENT,glob_Flags(a4)
			bra.s	.24

.23			btst.b	#VOICEB_BLOCKED,voice_Flags(a2)		; voice blocked?
			beq.s	.24									; no, so don't check
			move.l	a2,a0
			bsr		check_sound
.24			lea		voice_sizeof(a2),a2
			dbra	d2,.22

			bclr.b	#MUSICB_PLAYNOTE,_maxtrax+mxtx_Flags
			tst.b	_xchannel+chan_VoicesActive
			beq.s	.25
			bset.b	#MUSICB_PLAYNOTE,_maxtrax+mxtx_Flags

.25			move.b	glob_Flags(a4),d0
			and.b	#(MUSIC_PLAYING!MUSIC_SILENT!MUSIC_LOOP),d0
			and.b	#~(MUSIC_PLAYING!MUSIC_SILENT!MUSIC_LOOP),mxtx_Flags(a5)
			or.b	d0,mxtx_Flags(a5)

			addq.w	#2,sp							; cleanup stack
			movem.l	(sp)+,d2/d3/d4/d5/a2-a6

;			COLOR0	$000f

			moveq	#0,d0
			rts

*==========================================================================*
*
*	AdvanceSong - advance a song to next mark
*
*
*	BOOL AdvanceSong(WORD which)
*
*	void advance_song(WORD which)
*
*==========================================================================*

;		ifne C_ENTRY
;			DEFX	_advance_song
;_advance_song
;			move.w	4(sp),d0							; amount to advance
;		endc
advance_song
			move.l	_globaldata+glob_Current,a0			; current location
			move.l	a0,a1								; scan location
			bra.s	.1

.2			cmp.b	#COMMAND_END,cev_Command(a1)		; is it an END event?
			bne.s	.3									; no, skip
			
			move.l	_globaldata+glob_CurrentScore,a1	; else, reset to start
			move.l	score_Data(a1),a0
			move.l	a0,a1
			bra.s	.1									; and goto outer loop

.3			cmp.b	#COMMAND_SPECIAL,cev_Command(a1)	; is it MARK event?
			bne.s	.4									; no, skip
			cmp.b	#SPECIAL_MARK,cev_StopTime(a1)
			bne.s	.4									; no, skip

			lea		cev_sizeof(a1),a1					; advance scan
			move.l	a1,a0								; set start to scan
			bra.s	.1									; and goto outer loop

.4			lea		cev_sizeof(a1),a1					; advance scan
			bra		.2									; continue inner loop

.1			dbra	d0,.2								; loop

			move.l	a0,_globaldata+glob_Current			; location = start
			rts

;			xdef	_AdvanceSong
;_AdvanceSong
;			move.l	4(sp),d0							; amount to advance

;			xdef	AdvanceSong
;AdvanceSong
;			move.w	#EXTRA_ADVANCE,_extra_op			; the op
;			ext.l	d0
;			move.l	d0,_extra_data						; the data

;			move.l	a6,-(sp)							; cause a softint
;			move.l	_SysBase,a6
;			lea		_extra_server,a1
;			JSRLIB	Cause
;			move.l	(sp)+,a6

;.1			tst.w	_extra_op							; busy loop until OS
;			bne		.1									;	notices softint

;			move.l	_extra_data,d0						; get result
;			rts

*==========================================================================*
*
*	PlaySong - start a song playing
*
*	BOOL PlaySong(WORD which)
*
*==========================================================================*

;			xdef	_PlaySong
;_PlaySong
;			move.l	4(sp),d0

;			xdef	PlaySong
PlaySong
			move.w	d2,-(sp)
			move.w	d0,d2

;			jsr		OpenMusic							; open music
;			tst.l	d0
;			bne		.98									; if not 0, error

			bclr.b	#MUSICB_PLAYING,_globaldata+glob_Flags	; turn music off
			bclr.b	#MUSICB_PLAYING,_maxtrax+mxtx_Flags	; turn music off
			bclr.b	#MUSICB_LOOP,_globaldata+glob_Flags	; no looping

			jsr		SystemReset

			move.l	_globaldata+glob_CurrentScore,a0	; reset score pointer
			move.l	score_Data(a0),_globaldata+glob_Current

			move.w	d2,d0								; advance to mark
			jsr		advance_song

			moveq	#-1,d0								; reset note off's
			move.b	d0,_globaldata+glob_NoteOff+sev_Command
			move.b	d0,_globaldata+glob_NoteOff+sev_Command+sev_sizeof
			move.b	d0,_globaldata+glob_NoteOff+sev_Command+2*sev_sizeof
			move.b	d0,_globaldata+glob_NoteOff+sev_Command+3*sev_sizeof

			move.w	_globaldata+glob_Tempo,d0			; set tempo
			lsl.w	#4,d0
			bsr		SetTempo

			move.w	_globaldata+glob_Filter,d0			; set filter
			bsr		SetAudioFilter

			clr.b	_globaldata+glob_RepeatTotal		; clear things
;		ifne HAS_ALLMIDI
;			clr.b	_globaldata+glob_RunningStatus
;		endc
			clr.l	_globaldata+glob_CurrentTime
			clr.l	_globaldata+glob_TempoTime
			clr.l	_globaldata+glob_Ticks

			bset.b	#MUSICB_PLAYING,_globaldata+glob_Flags	; turn music on!
			bset.b	#MUSICB_PLAYING,_maxtrax+mxtx_Flags		; tell user

			move.w	(sp)+,d2
			moveq	#1,d0
			rts

;.98			move.w	(sp)+,d2
;			moveq	#0,d0
;			rts

*==========================================================================*
*
*	LoopSong - start a song playing w/looping
*
* BOOL LoopSong(WORD which)
*
*==========================================================================*

;			xdef	_LoopSong
;_LoopSong
;			move.l	4(sp),d0

;			xdef	LoopSong
LoopSong
			jsr		PlaySong							; condition codes set
			beq.s	.98
			bset.b	#MUSICB_LOOP,_globaldata+glob_Flags	; looping
.98			rts

*==========================================================================*
*
*	StopSong - stop a playing song
*
*	void StopSong(void)
*
*==========================================================================*

;			xdef	_StopSong
;_StopSong
;			xdef	StopSong
StopSong
			movem.l	d2/a2,-(sp)

			bclr.b	#MUSICB_PLAYING,_globaldata+glob_Flags	; turn music off
			bclr.b	#MUSICB_PLAYING,_maxtrax+mxtx_Flags	; turn music off

			lea		_voice,a2								; kill voices
			moveq	#NUM_VOICES-1,d2
.1			tst.l	voice_Channel(a2)
			beq.s	.2
			move.l	a2,a0
			bsr		KillVoice
.2			lea		voice_sizeof(a2),a2
			dbra	d2,.1

			movem.l	(sp)+,d2/a2
			rts

*==========================================================================*
*
*	ContinueSong - continue playing a song
*
*	void ContinueSong(void)
*
*==========================================================================*

;			xdef	_ContinueSong
;_ContinueSong
;			xdef	ContinueSong
;ContinueSong
;			bset.b	#MUSICB_PLAYING,_globaldata+glob_Flags	; turn music on!
;			bset.b	#MUSICB_PLAYING,_maxtrax+mxtx_Flags	; turn music on!
;			rts

*==========================================================================*
*
*	SelectScore - select which score to use
*
*	BOOL SelectScore(WORD which)
*
*==========================================================================*

;			xdef	_SelectScore
;_SelectScore
;			move.l	4(sp),d0

;			xdef	SelectScore
SelectScore
			cmp.w	_globaldata+glob_TotalScores,d0
			bpl.s	.98

			bclr.b	#MUSICB_PLAYING,_globaldata+glob_Flags	; turn music off
			bclr.b	#MUSICB_PLAYING,_maxtrax+mxtx_Flags	; turn music on!
			move.l	_scoreptr,a0
			SCALE	score_sizeof,d0
			add.w	d0,a0
			move.l	a0,_globaldata+glob_CurrentScore

			bsr		SystemReset

			moveq	#1,d0
			rts

.98			moveq	#0,d0
			rts

*==========================================================================*
*
*	PlayNote - play a single note externally to sequencer
*
*	LONG PlayNote(UWORD note,UWORD patch,UWORD duration,UWORD volume,UWORD pan)
*
*==========================================================================*

;			xdef	_PlayNote
;_PlayNote
;			movem.l	d2/d3/d4/a6,-(sp)
;			move.l	4+16(sp),d0
;			move.l	8+16(sp),d1
;			move.l	12+16(sp),d2
;			move.l	16+16(sp),d3
;			move.l	20+16(sp),d4
;			bra.s	pn_merge

;			xdef	PlayNote
;PlayNote
;			movem.l	d2/d3/d4/a6,-(sp)
;pn_merge
;			sub.w	#nblk_sizeof,sp
;			move.w	d0,nblk_Note(sp)
;			move.w	d1,nblk_Patch(sp)
;			move.w	d2,nblk_Duration(sp)
;			move.w	d3,nblk_Volume(sp)
;			move.w	d4,nblk_Pan(sp)

;			move.w	#EXTRA_NOTE,_extra_op
;			move.l	sp,_extra_data

;			move.l	_SysBase,a6
;			lea		_extra_server,a1
;			JSRLIB	Cause

;.1			tst.w	_extra_op							; busy loop until OS
;			bne		.1									;	notices softint

;			move.l	_extra_data,d0						; get result
;			add.w	#nblk_sizeof,sp
;			movem.l	(sp)+,d2/d3/d4/a6
;			rts

*==========================================================================*
*
*	PlaySound - play a sampled sound by stealing an audio channel
*
*
*	LONG PlaySound(void *sample,LONG length,WORD volume,WORD period,WORD side)
*
*==========================================================================*

;			xdef	_PlaySound
;_PlaySound
;			movem.l	d2/d3/a6,-(sp)
;			move.l	4+12(sp),a0
;			move.l	8+12(sp),d0
;			move.l	12+12(sp),d1
;			move.l	16+12(sp),d2
;			move.l	20+12(sp),d3
;			bra.s	ps_merge

;			xdef	PlaySound
;PlaySound
;			movem.l	d2/d3/a6,-(sp)
;ps_merge
;			sub.w	#sblk_sizeof,sp
;			move.l	a0,sblk_Data(sp)
;			move.l	d0,sblk_Length(sp)
;			move.w	d1,sblk_Volume(sp)
;			move.w	d2,sblk_Period(sp)
;			move.w	d3,sblk_Pan(sp)

;			move.w	#EXTRA_PLAYSOUND,_extra_op
;			move.l	sp,_extra_data

;			move.l	_SysBase,a6
;			lea		_extra_server,a1
;			JSRLIB	Cause

;.1			tst.w	_extra_op							; busy loop until OS
;			bne		.1									;	notices softint

;			move.l	_extra_data,d0						; get result
;			add.w	#sblk_sizeof,sp
;			movem.l	(sp)+,d2/d3/a6
;			rts

*==========================================================================*
*
*	StopSound - stop a playing sampled sound
*
*	void StopSound(LONG cookie)
*
*==========================================================================*

;			xdef	_StopSound
;_StopSound
;			move.l	4(sp),d0

;			xdef	StopSound
;StopSound
;			move.w	#EXTRA_STOPSOUND,_extra_op
;			move.l	d0,_extra_data

;			move.l	a6,-(sp)
;			move.l	_SysBase,a6
;			lea		_extra_server,a1
;			JSRLIB	Cause
;			move.l	(sp)+,a6

;.1			tst.w	_extra_op							; busy loop until OS
;			bne		.1									;	notices softint
;			rts

*==========================================================================*
*
*	CheckSound - see if sampled sound still playing
*
*	BOOL check_sound(struct VoiceData *v)
*
*	LONG CheckSound(LONG cookie)
*
*==========================================================================*

;		ifne C_ENTRY
;			DEFX	_check_sound
;_check_sound
;			move.l	4(sp),a0
;		endc
check_sound
			movem.l	d2/a2/a3/a6,-(sp)
			moveq	#0,d2								; result = FALSE
			move.l	a0,a2

			btst.b	#VOICEB_BLOCKED,voice_Flags(a2)		; voice blocked?
			beq.s	.1									; no, so don't check

			move.l	_SysBase,a6							; get an audio block
			move.l	_play_port,a0
			JSRLIB	GetMsg
			tst.l	d0
			beq.s	.1									; failed!

			move.l	d0,a3								; set up audio read call
			move.w	#CMD_READ,IO_COMMAND(a3)
			clr.b	IO_FLAGS(a3)
			clr.b	IO_ERROR(a3)
			moveq	#1,d0
			moveq	#0,d1
			move.b	voice_Number(a2),d1					; set unit number
			lsl.l	d1,d0
			move.l	d0,IO_UNIT(a3)

			move.l	a3,a1
			move.l	_AudioDevice,a6						; do it!
			JSRDEV	BEGINIO

			tst.b	IO_ERROR(a3)		; check conditions matching sound done 
			bne.s	.2
			tst.l	ioa_Data(a3)
			beq.s	.2

			moveq	#1,d2								; voice still going
			bra.s	.1

.2			bclr.b	#VOICEB_BLOCKED,voice_Flags(a2)		; unblock voice
			clr.b	voice_Priority(a2)					; clear priority
			clr.l	voice_UniqueID(a2)					; clear id
			moveq	#0,d0
			move.b	voice_Link(a2),d0
			cmp.b	voice_Number(a2),d0					; is this stereo?
			beq.s	.1									; no, skip

			SCALE	voice_sizeof,d0					; check sound on link
			lea		_voice,a0
			add.w	d0,a0
			jsr		check_sound

.1			move.l	d2,d0
			movem.l	(sp)+,d2/a2/a3/a6
			rts

;xcheck_sound
;			movem.l	d2/a2/a3/a6,-(sp)
;			moveq	#0,d2								; result = FALSE
;			move.l	a0,a2

;			move.l	_SysBase,a6							; get an audio block
;			move.l	_play_port,a0
;			JSRLIB	GetMsg
;			tst.l	d0
;			beq.s	.1									; failed!

;			move.l	d0,a3								; set up audio read call
;			move.w	#CMD_READ,IO_COMMAND(a3)
;			clr.b	IO_FLAGS(a3)
;			clr.b	IO_ERROR(a3)
;			moveq	#1,d0
;			moveq	#0,d1
;			move.b	voice_Number(a2),d1					; set unit number
;			lsl.l	d1,d0
;			move.l	d0,IO_UNIT(a3)

;			move.l	a3,a1
;			move.l	_AudioDevice,a6						; do it!
;			JSRDEV	BEGINIO

;			tst.b	IO_ERROR(a3)		; check conditions matching sound done 
;			bne.s	.2
;			tst.l	ioa_Data(a3)
;			beq.s	.2

;			moveq	#1,d2								; voice still going
;			bra.s	.1

;.2			move.l	voice_Channel(a2),a0				; get channel

;			subq.b	#1,chan_VoicesActive(a0)			; -1 active voice
;		ifeq IN_MUSICX
;			lea		_globaldata,a0
;			subq.b	#1,glob_VoicesActive(a0)
;		endc

;			clr.l	voice_Channel(a2)					; clear voice
;			clr.b	voice_Status(a2)					; ENV_FREE
;			clr.b	voice_Flags(a2)
;			clr.b	voice_Priority(a2)
;			clr.l	voice_UniqueID(a2)

;.1			move.l	d2,d0
;			movem.l	(sp)+,d2/a2/a3/a6
;			rts

;			xdef	_CheckSound
;_CheckSound
;			move.l	4(sp),d0

;			xdef	CheckSound
;CheckSound
;			move.w	#EXTRA_CHECKSOUND,_extra_op
;			move.l	d0,_extra_data

;			move.l	a6,-(sp)
;			move.l	_SysBase,a6
;			lea		_extra_server,a1
;			JSRLIB	Cause
;			move.l	(sp)+,a6

;.1			tst.w	_extra_op							; busy loop until OS
;			bne		.1									;	notices softint

;			move.l	_extra_data,d0						; get result
;			rts

;			xdef	_CheckNote
;_CheckNote
;			move.l	4(sp),d0

;			xdef	CheckNote
;CheckNote
;			move.w	#EXTRA_CHECKNOTE,_extra_op
;			move.l	d0,_extra_data

;			move.l	a6,-(sp)
;			move.l	_SysBase,a6
;			lea		_extra_server,a1
;			JSRLIB	Cause
;			move.l	(sp)+,a6

;.1			tst.w	_extra_op							; busy loop until OS
;			bne		.1									;	notices softint

;			move.l	_extra_data,d0						; get result
;			rts

*==========================================================================*
*
*	ExtraServer - handle non-score features of driver
*
*	void ExtraServer(void)
*
*==========================================================================*

;			DEFX	_ExtraServer
;_ExtraServer
;ExtraServer
;			movem.l	d2-d4/a2/a3/a5/a6,-(sp)
;			move.w	_extra_op,d2						; get op into register
;			clr.w	_extra_op							; clear indicates handled

*	PlayNote

;			cmp.w	#EXTRA_NOTE,d2						; PlayNote operation?
;			bne		.1									; no

;			move.l	_extra_data,a2						; get extra data
;			clr.l	_extra_data							; init result to false

;			bsr		OpenMusic							; be sure driver active
;			tst.l	d0
;			bne		.99									; if not zero, error

;			lea		_xchannel,a0
;			clr.b	chan_Flags(a0)						; clear xchannel flags
;			tst.w	nblk_Pan(a2)						; pan?
;			beq.s	.2									; no, skip
;			move.b	#CHAN_PAN,chan_Flags(a0)			; set to CHAN_PAN

;.2			move.w	nblk_Patch(a2),d0					; which patch?
;			lea		_patch,a1
;			SCALE	patch_sizeof,d0
;			add.w	d0,a1								; get address of patch
;			move.l	a1,chan_Patch(a0)					; put in xchannel

;			subq.w	#2,sp								; space for midi stream
;			move.w	nblk_Note(a2),d0
;			bclr.l	#7,d0
;			move.b	d0,(sp)								; set note #, midi[0]
;			move.b	nblk_Volume+1(a2),1(sp)				; set note volume

;			bclr.b	#MUSICB_ADDED_NOTE,_globaldata+glob_Flags	; reset added
;			move.l	sp,a1								; midi stream
;			moveq	#MUSIC_PRI_NOTE,d0					; priority 1 normally
;			bsr		NoteOn								; note on (channel in a0)

;			btst.b	#MUSICB_ADDED_NOTE,_globaldata+glob_Flags	; note added?
;			beq.s	.3									; nope...

;			moveq	#0,d0								; add note off on voice
;			move.b	_globaldata+glob_LastVoice,d0
;			lea		_globaldata+glob_NoteOff,a0
;			SCALE	sev_sizeof,d0
;			add.w	d0,a0								; get stop event addr

;			move.b	(sp),sev_Command(a0)				; set command
;			move.b	#16,sev_Data(a0)					; set data (channel 16)
;			moveq	#0,d0
;			move.w	nblk_Duration(a2),d0
;			lsl.l	#8,d0
;			move.l	d0,sev_StopTime(a0)					; set stop time

;			moveq	#0,d0
;			move.b	_globaldata+glob_LastVoice,d0
;			addq.b	#1,d0
;			move.l	d0,_extra_data						; indicate ok (voice+1)

;.3			addq.w	#2,sp								; clean up stack
;			bra		.99									; done

*	PlaySound

;.1			cmp.w	#EXTRA_PLAYSOUND,d2					; PlaySound operation?
;			bne		.4									; no

*	a2		= sound block
*	a3/d3	= audio blocks
*	a5/d2	= voice data

;			move.l	_extra_data,a2						; get extra data
;			clr.l	_extra_data							; init result to false

;			bsr		OpenMusic							; be sure driver active
;			tst.l	d0
;			bne		.99									; if not zero, error

;			move.w	sblk_Pan(a2),d1						; get pan value
;			moveq	#1,d4								; default loop value = 1
;			btst.l	#SOUNDB_LOOP,d1						; loop?
;			beq.s	.20

;			move.w	d1,d4
;			lsr.w	#8,d4								; get loop value

;.20			moveq	#LEFT_0,d0
;			btst.l	#SOUNDB_RIGHT_SIDE,d1				; pan?
;			beq.s	.5									; no, use left
;			moveq	#RIGHT_0,d0							; use right
;.5			and.w	#MUST_HAVE_SIDE,d1					; mask all but MUST bit
;			or.w	#MUSIC_PRI_SOUND,d1					; priority 2 normally
;			bsr		pick_voice
;			tst.l	d0									; picked a voice?
;			beq		.99									; nope, done
;			move.l	d0,a5

;			moveq	#0,d3								; init iob2 = NULL
;			moveq	#0,d2								; init voice2 = NULL

;			move.l	_SysBase,a6
;			move.l	_play_port,a0
;			JSRLIB	GetMsg								; get an audio block
;			tst.l	d0
;			beq		.99									; oops, none available??

;			move.l	d0,a3								; set-up iob1

;		ifne FASTSOUND
;			move.l	sblk_Data(a2),a1
;			JSRLIB	TypeOfMem							; is fastmem?
;			btst.l	#MEMB_FAST,d0
;			bne		.40									; yep, play fastsound
;		endc

;			move.w	#CMD_WRITE,IO_COMMAND(a3)			; it's a write
;			move.b	#ADIOF_PERVOL!IOF_QUICK,IO_FLAGS(a3)
;			clr.b	IO_ERROR(a3)
;			move.w	d4,ioa_Cycles(a3)
;			move.w	sblk_Volume(a2),ioa_Volume(a3)
;			move.w	sblk_Period(a2),ioa_Period(a3)
;			moveq	#1,d1
;			moveq	#0,d0
;			move.b	voice_Number(a5),d0
;			lsl.l	d0,d1
;			move.l	d1,IO_UNIT(a3)
;			move.l	sblk_Data(a2),ioa_Data(a3)
;			move.l	sblk_Length(a2),ioa_Length(a3)

;			exg.l	d3,a3								; makes things simpler
;			move.l	a5,a0
;			exg.l	d2,a5

;			move.w	sblk_Pan(a2),d0						; check for stereo
;			and.b	#SOUND_STEREO,d0
;			cmp.b	#SOUND_STEREO,d0
;			bne		.6									; no, didn't want it

;			move.b	voice_Number(a0),d0
;			subq.w	#1,d0						; (v# - 1) & 2
;			btst.l	#1,d0						; did we get LEFT side?
;			bne		.6							; wanted RIGHT, can't have stereo

;			moveq	#LEFT_0,d0
;			move.w	sblk_Pan(a2),d1
;			and.w	#MUST_HAVE_SIDE,d1
;			or.w	#MUSIC_PRI_SOUND,d1			; priority 2 normally
;			bsr		pick_voice
;			tst.l	d0
;			beq.s	.6

;			move.l	d0,a5
;			move.b	voice_Number(a5),d0
;			subq.w	#1,d0						; (v# - 1) & 2
;			btst.l	#1,d0						; did we get LEFT side?
;			beq.s	.6							; if 0, can't have stereo
			
;			move.l	_SysBase,a6
;			move.l	_play_port,a0
;			JSRLIB	GetMsg								; get another audio block
;			tst.l	d0
;			beq.s	.6									; oops, none available??

;			move.l	d0,a3								; use iob2
;			move.w	#CMD_WRITE,IO_COMMAND(a3)			; it's a write
;			move.b	#ADIOF_PERVOL!IOF_QUICK,IO_FLAGS(a3)
;			clr.b	IO_ERROR(a3)
;			move.w	d4,ioa_Cycles(a3)
;			move.w	sblk_Volume(a2),ioa_Volume(a3)
;			move.w	sblk_Period(a2),ioa_Period(a3)
;			moveq	#1,d1
;			moveq	#0,d0
;			move.b	voice_Number(a5),d0
;			lsl.l	d0,d1
;			move.l	d1,IO_UNIT(a3)
;			move.l	sblk_Data(a2),ioa_Data(a3)
;			move.l	sblk_Length(a2),ioa_Length(a3)

;			tst.l	voice_Channel(a5)					; if voice in use, kill
;			beq.s	.7
;			move.l	a5,a0
;			bsr		KillVoice
;.7			bset.b	#VOICEB_BLOCKED,voice_Flags(a5)		; block voice

;.6			exg.l	a5,d2								; switch to voice 1
;			exg.l	a3,d3								; switch to iob 1

;			tst.l	voice_Channel(a5)					; if voice in use, kill
;			beq.s	.8
;			move.l	a5,a0
;			bsr		KillVoice
;.8			bset.b	#VOICEB_BLOCKED,voice_Flags(a5)		; block voice

;			move.l	a3,a1								; send off iob 1
;			move.l	_AudioDevice,a6
;			JSRDEV	BEGINIO

;			tst.l	d3									; stereo?
;			beq.s	.9									; no, skip
;			move.l	d3,a1								; else, send off iob 2
;			JSRDEV	BEGINIO

;.9			tst.b	IO_ERROR(a3)						; error on iob 1?
;			bne.s	.30									; no, skip

;			move.l	a3,a1
;			DMACHECK
;			bra.s	.10

;.30			move.l	a3,a1								; put iob on play_port
;			move.l	_SysBase,a6
;			JSRLIB	ReplyMsg

;			sub.l	a3,a3								; indicate error
;			bclr.b	#VOICEB_BLOCKED,voice_Flags(a5)		; unblock voice

;.10			tst.l	d3
;			beq.s	.11
;			move.l	d3,a1
;			tst.b	IO_ERROR(a1)						; error on iob 1?
;			bne.s	.32									; no, skip & do link

;			DMACHECK
;			bra.s	.12

;.32			move.l	_SysBase,a6							; put iob on play_port
;			JSRLIB	ReplyMsg

;			moveq	#0,d3								; indicate error
;			move.l	d2,a1
;			bclr.b	#VOICEB_BLOCKED,voice_Flags(a1)		; unblock voice
;			bra.s	.11

;.12			move.l	d2,a1
;			move.b	#MUSIC_PRI_SOUND,voice_Priority(a1) ; priority 2 normally
;			move.b	voice_Number(a1),voice_Link(a1)

;.11			move.l	a3,d0								; or iob1 with iob2
;			or.l	d3,d0
;			beq		.99									; zero, no sound made

;			move.b	#MUSIC_PRI_SOUND,voice_Priority(a5) ; priority 2 normally
;			move.b	voice_Number(a5),voice_Link(a5)		; link voices if stereo
;			tst.l	d3
;			beq.s	.13
;			move.l	d2,a0
;			move.b	voice_Number(a0),voice_Link(a5)

;.13			move.l	_globaldata+glob_UniqueID,d0		; set-up UniqueID
;			move.l	d0,d1
;			or.b	voice_Number(a5),d0
;			bset.l	#31,d0
;			move.l	d0,voice_UniqueID(a5)
;			move.l	d0,_extra_data

;			addq.l	#4,d1								; increment UniqueID
;			and.b	#$fc,d1								; safety net
;			move.l	d1,_globaldata+glob_UniqueID
;			bra		.99

;		ifne FASTSOUND
;.40
;		endc

*	StopSound

;.4			cmp.w	#EXTRA_STOPSOUND,d2					; StopSound operation?
;			bne		.14									; no

;			move.l	_extra_data,d0						; get voice # from ID
;			move.l	d0,d1
;			and.w	#3,d0
;			SCALE	voice_sizeof,d0
;			lea		_voice,a2
;			add.w	d0,a2

;			cmp.l	voice_UniqueID(a2),d1				; ID matches with voice
;			bne.s	.15									; nope, skip all

;			btst.b	#VOICEB_BLOCKED,voice_Flags(a2)		; voice actually on?
;			beq.s	.16									; no, check for stereo

;			moveq	#0,d0
;			move.b	voice_Number(a2),d0
;			bsr		stop_audio							; kill voice
;			clr.b	voice_Status(a2)
;			clr.b	voice_Flags(a2)
;			clr.b	voice_Priority(a2)
;			clr.l	voice_UniqueID(a2)

;.16			moveq	#0,d0
;			move.b	voice_Link(a2),d0					; is link = number?
;			cmp.b	voice_Number(a2),d0
;			beq.s	.15									; yes, so not stereo

;			SCALE	voice_sizeof,d0
;			lea		_voice,a2
;			add.w	d0,a2

;			btst.b	#VOICEB_BLOCKED,voice_Flags(a2)		; voice actually on?
;			beq.s	.15									; no, check for stereo

;			moveq	#0,d0
;			move.b	voice_Number(a2),d0
;			bsr		stop_audio							; kill voice
;			clr.b	voice_Status(a2)
;			clr.b	voice_Flags(a2)
;			clr.b	voice_Priority(a2)
;			clr.l	voice_UniqueID(a2)

;.15			clr.l	_extra_data
;			bra	.99

*	CheckSound

;.14			cmp.w	#EXTRA_CHECKSOUND,d2				; CheckSound operation?
;			bne.s	.18									; no

;			move.l	_extra_data,d0						; get voice # from ID
;			move.l	d0,d1
;			and.w	#3,d0
;			SCALE	voice_sizeof,d0
;			lea		_voice,a2
;			add.w	d0,a2

;			cmp.l	voice_UniqueID(a2),d1				; ID matches with voice
;			bne.s	.17									; nope, skip all

;			move.l	a2,a0
;			bsr		check_sound							; check sound on voice
;			tst.l	d0
;			beq.s	.17									; no, return FALSE
;			moveq	#1,d0
;			move.l	d0,_extra_data						; yes, return TRUE
;			bra.s	.99

;.17			clr.l	_extra_data
;			bra.s	.99

*	AdvanceSong

;.18			cmp.w	#EXTRA_ADVANCE,d2					; CheckSound operation?
;			bne.s	.19									; no

;			move.l	_extra_data,d0						; get advance #
;			clr.l	_extra_data							; assume no music playing
;			btst.b	#MUSICB_PLAYING,_globaldata+glob_Flags
;			beq.s	.99									; no music playing, skip

;			bsr		advance_song
;			moveq	#1,d0
;			move.l	d0,_extra_data
;			bra.s	.99

*	CheckNote

;.19			cmp.w	#EXTRA_CHECKNOTE,d2				; CheckNote operation?
;			bne.s	.99									; no

;			move.l	_extra_data,d0						; get voice # + 1
;			subq.b	#1,d0
;			and.w	#3,d0
;			SCALE	voice_sizeof,d0
;			lea		_voice,a2
;			add.w	d0,a2

;			move.l	voice_Channel(a2),a3
;			cmp.b	#16,chan_Number(a3)
;			bne.s	.40

;			move.l	a2,a0
;			bsr		xcheck_sound						; check note on voice
;			tst.l	d0
;			beq.s	.40									; no, return FALSE
;			moveq	#1,d0
;			move.l	d0,_extra_data						; yes, return TRUE
;			bra.s	.99

;.40			clr.l	_extra_data
;			bra.s	.99

;.99			movem.l	(sp)+,d2-d4/a2/a3/a5/a6
;			moveq	#0,d0
;			rts

*==========================================================================*
*
*	AllocSample - allocates & initialized a SampleData structure
*
*	struct SampleData *AllocSample(
*		struct SampleData	*prev_sample,
*		LONG				attacksize,
*		LONG				sustainsize)
*
*==========================================================================*

;		ifne C_ENTRY
;			DEFX	_AllocSample
;_AllocSample
;			move.l	4(sp),a0							; previous sample
;			move.l	8(sp),d0							; attack size
;			move.l	12(sp),d1							; sustain size
;		endc
AllocSample
			movem.l	d2/d3/a2/a3/a6,-(sp)
;			move.l	a0,a2
			move.l	d0,d2
			move.l	d1,d3

;			moveq	#samp_sizeof,d0						; allocate SampleData
;			move.l	#MEMF_CLEAR,d1
;			move.l	_SysBase,a6
;			JSRLIB	AllocMem
;			tst.l	d0
;			beq.s	.99
;			move.l	d0,a3

	MOVE.L	A4,A3

;			move.l	d2,d0								; allocate Waveform
;			add.l	d3,d0
;			move.l	#MEMF_CHIP,d1
;			JSRLIB	AllocMem

;			move.l	d0,samp_Waveform(a3)				; Waveform -> SampleData
;			beq.s	.97									; whoops... no memory

	MOVE.L	A2,samp_Waveform(A3)
	MOVE.L	A0,A2
			
			move.l	d2,samp_AttackSize(a3)				; store sizes
			move.l	d3,samp_SustainSize(a3)
			move.l	a3,d0								; return SampleData

			move.l	a2,d1								; previous sample?
			beq.s	.99									; no, done

			move.l	a3,samp_NextSample(a2)				; link to previous

.99			movem.l (sp)+,d2/d3/a2/a3/a6
			rts

;.97			move.l	#samp_sizeof,d0						; free memory
;			move.l	a3,a1
;			JSRLIB	FreeMem
;			moveq	#0,d0								; return error
;			bra		.99

*==========================================================================*
*
*	FreeSample - frees memory associated with a sample
*
*	void FreeSample(WORD number)
*
*==========================================================================*

;		ifne C_ENTRY
;			DEFX	_FreeSample
;_FreeSample
;			move.w	4(sp),d0							; sample number
;		endc
;FreeSample
;			movem.l	a2/a6,-(sp)	
;			lea		_asample,a0							; calc ptr to sample
;			add.w	d0,d0
;			add.w	d0,d0
;			add.w	d0,a0
;			move.l	(a0),a2								; get sample ptr
;			clr.l	(a0)								; clear saved ptr

;			move.l	_SysBase,a6

;.2			move.l	a2,d0								; still have sample?
;			beq.s	.1									; no, done

;			move.l	samp_Waveform(a2),a1
;			move.l	samp_AttackSize(a2),d0
;			add.l	samp_SustainSize(a2),d0
;			JSRLIB	FreeMem

;			move.l	samp_NextSample(a2),a1				; get next sample
;			exg		a1,a2								; set-up next FreeMem
;			moveq	#samp_sizeof,d0
;			JSRLIB	FreeMem

;			bra		.2

;.1			movem.l	(sp)+,a2/a6
;			rts

*==========================================================================*
*
*	FreePatch - frees memory associated with a patch
*
*	void FreePatch(WORD number)
*
*==========================================================================*

;		ifne C_ENTRY
;			DEFX	_FreePatch
;_FreePatch
;			move.w	4(sp),d0							; patch number
;		endc
;FreePatch
;			movem.l	a2/a6,-(sp)

;			lea		_patch,a2							; calc addr of patch
;			SCALE	patch_sizeof,d0
;			add.w	d0,a2

;			move.l	_SysBase,a6

;			move.l	patch_Attack(a2),d0					; free attack envelopes
;			beq.s	.1
;			move.l	d0,a1
;			move.w	patch_AttackCount(a2),d0
;			mulu.w	#env_sizeof,d0
;			JSRLIB	FreeMem
;			clr.l	patch_Attack(a2)

;.1			move.l	patch_Release(a2),d0				; free release envelopes
;			beq.s	.2
;			move.l	d0,a1
;			move.w	patch_ReleaseCount(a2),d0
;			mulu.w	#env_sizeof,d0
;			JSRLIB	FreeMem
;			clr.l	patch_Release(a2)

;.2			clr.l	patch_Sample(a2)					; clear sample ptr

;			movem.l	(sp)+,a2/a6
;			rts

*==========================================================================*
*
*	UnloadPerf - unloads a MaxTrax performance
*
*	void UnloadPerf(WORD mode)
*
*==========================================================================*

;			xdef	_UnloadPerf
;_UnloadPerf
;			move.l	4(sp),d0							; ALL/SCORE/SAMPLES

;			xdef	UnloadPerf
;UnloadPerf
;			movem.l	d2/d3/a2/a6,-(sp)
;			move.w	d0,d2

;			cmp.w	#PERF_PARTSAMPLES,d2				; partial samples only?
;			beq.s	.9									; yes, don't unload

;			cmp.w	#PERF_SCORE,d2
;			beq.s	.1									; if only scores, skip

;			moveq	#NUM_SAMPLES-1,d3					; free all samples
;.2			move.w	d3,d0
;			bsr		FreeSample
;			dbra	d3,.2

;			moveq	#NUM_PATCHES-1,d3					; free all patches
;.3			move.w	d3,d0
;			bsr		FreePatch
;			dbra	d3,.3

;.1			cmp.w	#PERF_SAMPLES,d2
;			beq.s	.9									; if only samples, skip

;			move.l	_SysBase,a6
;			move.l	_scoreptr,a2
;			move.l	a2,_globaldata+glob_CurrentScore	; reset current score
;			move.w	_globaldata+glob_TotalScores,d3		; get total scores
;			bra.s	.4

;.5			move.l	score_NumEvents(a2),d0				; any events?
;			beq.s	.6									; no, skip

;			moveq	#cev_sizeof,d1						; calc size of events
;			jsr		mulu
;			move.l	score_Data(a2),a1					; free score
;			JSRLIB	FreeMem
;			clr.l	score_Data(a2)						; clear score structure
;			clr.l	score_NumEvents(a2)

;.6			addq.w	#score_sizeof,a2					; next score
;.4			dbra	d3,.5								; loop

;			clr.w	_globaldata+glob_TotalScores
;			clr.w	_maxtrax+mxtx_TotalScores
;			move.w	#120,_globaldata+glob_Tempo
;			move.w	#1,_globaldata+glob_Filter

;.9			movem.l	(sp)+,d2/d3/a2/a6
;			rts

*==========================================================================*
*
*	LoadPerf - load a Music-X score
*
*	BOOL LoadPerf(char *name,WORD mode [,BPTR handle])
*
*==========================================================================*

;CheckRead		macro

;				pea		\2
;				move.l	_maxtrax+mxtx_ReadFunc,-(sp)
;				rts

;\2				cmp.l	d0,d3
;			ifeq NARG-2
;				bne	\1
;			else
;				bne		\1
;			endc
;				endm
				
;				xdef	_LoadPerf
;_LoadPerf
;				move.l	4(sp),a0							; filename
;				move.l	8(sp),d0							; load mode
;				move.l	12(sp),d1							; possible fh

;				xdef	LoadPerf
LoadPerf
;				movem.l	d2-d7/a2/a3/a5/a6,-(sp)
;				sub.w	#dsamp_sizeof+8,sp					; local variables
															; +8 to fix CDTV
				move.l	a0,a2								; a2 = filename
;				move.w	d0,d4								; d4 = mode
;				move.l	d1,d7								; d7 = posible fh

;				bsr		CloseMusic							; close down
;				move.w	d4,d0
;				bsr		UnloadPerf							; unload performance

;				move.l	_DOSBase,a6

;				move.l	a2,d1								; filename
;				beq.s	.30									; none! must have fh

;				moveq	#0,d7								; no fh
;				move.l	#MODE_OLDFILE,d2

;				pea		.32
;				move.l	_maxtrax+mxtx_OpenFunc,-(sp)
;				rts

;.32				tst.l	d0
;				beq		.99									; error! 

;				move.l	d0,a2								; a2 = file handle
;				bra.s	.31

;.30				move.l	d7,a2								; a2 = file handle

;.31				move.l	a2,d1								; get file ID
;				move.l	sp,d2
;				moveq	#4,d3
;				CheckRead .98,.70

	ADDQ.L	#4,A2

;				cmp.l	#'MXTX',(sp)						; a MaxTrax file?
;				bne		.98									; nope, error

;				move.l	a2,d1								; get tempo & filter
;				move.l	sp,d2
;				moveq	#4,d3
;				CheckRead .98,.71
				
;				cmp.w	#PERF_SAMPLES,d4					; samples only?
;				beq.s	.1									; yes, skip
;				cmp.w	#PERF_PARTSAMPLES,d4				; samples only?
;				beq.s	.1									; yes, skip

;				move.w	(sp),_globaldata+glob_Tempo			; set tempo & filter
;				move.w	2(sp),d0


	MOVE.W	(A2)+,_globaldata+glob_Tempo			; set tempo & filter
	MOVE.W	(A2)+,D0

				move.w	d0,d1
				and.w	#1,d0								; low bit is filter
				move.w	d0,_globaldata+glob_Filter

				bclr.b	#MUSICB_VELOCITY,_globaldata+glob_Flags
				bclr.b	#MUSICB_VELOCITY,_maxtrax+mxtx_Flags
				btst.l	#1,d1								; attack volume?
				beq.s	.20
				bset.b	#MUSICB_VELOCITY,_globaldata+glob_Flags
				bset.b	#MUSICB_VELOCITY,_maxtrax+mxtx_Flags

.20
;			ifne HAS_MICROTONAL
;				btst.l	#15,d1								; has microtonal?
;				beq.s	.12

;				move.l	a2,d1								; read in table
;				move.l	#_microtonal,d2
;				moveq	#2+128,d3		; BUG !
;				CheckRead .98,.72

;				bra		.12
;			endc

.1				btst.l	#15,d1								; has microtonal?
				beq.s	.12

;				move.l	a2,d1								; seek past data
;				move.l	#2*128,d2
;				move.l	#OFFSET_CURRENT,d3
;				JSRLIB	Seek

	LEA	256(A2),A2

.12
;				move.l	a2,d1								; get # of scores
;				move.l	sp,d2
;				moveq	#2,d3
;				CheckRead .98,.73

;				move.w	(sp),d5								; use d5 as counter

	MOVE.W	(A2)+,D5

				move.l	_scoreptr,a3
				bra	.2

.3
;				move.l	a2,d1								; get # of events
;				move.l	sp,d2
;				moveq	#4,d3
;				CheckRead .98,.74
;				move.l	(sp),d0

	MOVE.L	(A2)+,D0

				moveq	#cev_sizeof,d1						; x CookedEvent size
				jsr		mulu
;				move.l	d0,4(sp)							; save locally

;				cmp.w	#PERF_SAMPLES,d4					; samples only?
;				beq.s	.14									; no, do load
;				cmp.w	#PERF_PARTSAMPLES,d4				; samples only?
;				beq.s	.14									; no, do load

				move.w	_scoremax,d1
				cmp.w	_globaldata+glob_TotalScores,d1 	; too many scores?
				bne.s	.4									; no, do load

.14				move.l	a2,d1								; seek past data
;				move.l	d0,d2
;				move.l	#OFFSET_CURRENT,d3
;				JSRLIB	Seek

	ADD.L	D0,A2

				bra.s	.2

.4
;				move.l	#MEMF_CLEAR,d1						; get d0 size bytes
;				move.l	_SysBase,a6
;				JSRLIB	AllocMem
;				tst.l	d0
;				beq		.98

;				move.l	d0,score_Data(a3)					; event data
;				move.l	(sp),score_NumEvents(a3)			; number (on stack)

	MOVE.L	A2,score_Data(A3)
	MOVE.L	-4(A2),score_NumEvents(A3)

				addq.w	#1,_globaldata+glob_TotalScores
				addq.w	#1,_maxtrax+mxtx_TotalScores

;				move.l	a2,d1								; load events
;				move.l	d0,d2
;				move.l	4(sp),d3
;				move.l	_DOSBase,a6
;				CheckRead .98,.75

	ADD.L	D0,A2

				addq.w	#score_sizeof,a3					; next score
.2				dbra	d5,.3

;				cmp.w	#PERF_SCORE,d4						; score only?
;				beq		.80									; yes, no samples

;	BEQ.W	.99

	RTS

; END OF SONG INSTALL

INSTALLSAMPLES

;				move.l	a2,d1								; get # of samples
;				move.l	sp,d2
;				moveq	#2,d3
;				CheckRead .98,.76
				
;				move.w	(sp),d5								; number of samples


	MOVE.W	(A2)+,D5
	LEA	SamplesTable,A4
	LEA	TempInfo,A1
	MOVE.L	A1,D4
	MOVEQ	#0,D2

				bra		.5

.6
;				move.l	a2,d1								; get sample header
;				move.l	sp,d2
;				moveq	#dsamp_sizeof,d3
;				CheckRead .98,.78

				lea		_asample,a3							; ptr to sample
;				move.w	dsamp_Number(sp),d0

	MOVE.L	A2,D0
	BTST	#0,D0
	BEQ.B	.even
	LEA	-1(A2),A6
.NextByte
	CMP.L	A2,D3
	BEQ.B	.FileEnd
	MOVE.B	(A2)+,(A6)+
	BRA.B	.NextByte
.FileEnd
	MOVE.L	D0,A2
	SUBQ.L	#1,A2
	ADDQ.L	#1,D2
.even

	LEA	dsamp_sizeof(A2),A0
.COPYINFO
	MOVE.W	(A2)+,(A1)+
	CMP.L	A2,A0
	BNE.B	.COPYINFO
	MOVE.L	D4,A1
	MOVE.W	dsamp_Number(A1),D0

				move.w	d0,d1								; save # in d1
				add.w	d0,d0								; entries are ptrs
				add.w	d0,d0
				add.w	d0,a3
				
				lea		_patch,a5							; ptr to patch
				SCALE	patch_sizeof,d1					; patch size * number
				add.w	d1,a5

;				move.w	dsamp_Tune(sp),patch_Tune(a5)		; copy values
;				move.w	dsamp_Volume(sp),patch_Volume(a5)
;				move.w	dsamp_AttackCount(sp),d6			; # attack env segs

	MOVE.W	dsamp_Tune(A1),patch_Tune(A5)
	MOVE.W	dsamp_Volume(A1),patch_Volume(A5)
	MOVE.W	dsamp_AttackCount(A1),D6

				mulu.w	#env_sizeof,d6						; memory needed
;				move.l	d6,d0
;				moveq	#0,d1
;				move.l	_SysBase,a6
;				JSRLIB	AllocMem
;				tst.l	d0
;				beq		.98

;				move.l	d0,patch_Attack(a5)					; save data
;				move.w	dsamp_AttackCount(sp),patch_AttackCount(a5)

	MOVE.L	A2,patch_Attack(A5)
	MOVE.W	dsamp_AttackCount(A1),patch_AttackCount(A5)
	ADD.L	D6,A2

;				move.l	a2,d1								; read sample
;				move.l	d0,d2
;				move.l	d6,d3
;				move.l	_DOSBase,a6
;				CheckRead .98,.79

;				move.w	dsamp_ReleaseCount(sp),d6			; # release env segs


	MOVE.W	dsamp_ReleaseCount(A1),D6

				mulu.w	#env_sizeof,d6						; memory needed
;				move.l	d6,d0
;				moveq	#0,d1
;				move.l	_SysBase,a6
;				JSRLIB	AllocMem
;				tst.l	d0
;				beq		.98

;				move.l	d0,patch_Release(a5)				; save data
;				move.w	dsamp_ReleaseCount(sp),patch_ReleaseCount(a5)

	MOVE.L	A2,patch_Release(A5)
	MOVE.W	dsamp_ReleaseCount(A1),patch_ReleaseCount(A5)
	ADD.L	D6,A2

;				move.l	a2,d1								; read sample
;				move.l	d0,d2
;				move.l	d6,d3
;				move.l	_DOSBase,a6
;				CheckRead .98,.60

				moveq	#0,d6								; for each octave...
.9
;				tst.w	dsamp_Octaves(sp)

	TST.W	dsamp_Octaves(A1)

				beq.s	.5

				move.l	d6,a0								; alloc sample space
;				move.l	dsamp_AttackLength(sp),d0
;				move.l	dsamp_SustainLength(sp),d1


	MOVE.L	dsamp_AttackLength(A1),D0
	MOVE.L	dsamp_SustainLength(A1),D1

				bsr		AllocSample

	LEA	samp_sizeof(A4),A4

				move.l	d0,d6
;				beq		.98									; opps, error

	BEQ.W	.99

				tst.l	(a3)								; first sample?
				bne.s	.8									; no, skip ahead

				move.l	d6,(a3)								; set sample ptr
				move.l	d6,patch_Sample(a5)					; set sample in patch

.8
;				move.l	a2,d1								; load sample
;				move.l	d6,a0
;				move.l	samp_Waveform(a0),d2
;				move.l	dsamp_AttackLength(sp),d3
;				add.l	dsamp_SustainLength(sp),d3
;				move.l	_DOSBase,a6
;				CheckRead .98,.61


	ADD.L	dsamp_AttackLength(A1),A2
	ADD.L	dsamp_SustainLength(A1),A2

;				move.l	dsamp_AttackLength(sp),d2
;				add.l	d2,dsamp_AttackLength(sp)
;				move.l	dsamp_SustainLength(sp),d2
;				add.l	d2,dsamp_SustainLength(sp)
;				subq.w	#1,dsamp_Octaves(sp)		; decrement octave count


	MOVE.L	dsamp_AttackLength(A1),D0
	ADD.L	D0,dsamp_AttackLength(A1)
	MOVE.L	dsamp_SustainLength(A1),D0
	ADD.L	D0,dsamp_SustainLength(A1)
	SUBQ.W	#1,dsamp_Octaves(A1)


				bra.s	.9									; loop

.5				dbra	d5,.6								; get more samples

	ADD.L	D2,A2

;				tst.l	d7									; fh?
;				bne.s	.33									; don't close it

;				move.l	a2,d1
;				move.l	_DOSBase,a6
;				pea		.33
;				move.l	_maxtrax+mxtx_CloseFunc,-(sp)
;				rts

;.33				moveq	#NUM_PATCHES-1,d2
;				lea		_patch,a2
;.10				tst.l	patch_Sample(a2)
;				bne.s	.11
;				lea		patch_sizeof(a2),a2
;				dbra	d2,.10

;.11				moveq	#1,d0
;				tst.w	d2
;				bpl.s	.99
;				moveq	#0,d0

.99
;				add.w	#dsamp_sizeof+8,sp					; local variables
;				movem.l	(sp)+,d2-d7/a2/a3/a5/a6
				rts

;.80				tst.l	d7									; fh?
;				bne.s	.81									; don't close it

;				move.l	a2,d1								; scores only cleanup
;				move.l	_DOSBase,a6
;				pea		.81
;				move.l	_maxtrax+mxtx_CloseFunc,-(sp)
;				rts

;.81				moveq	#1,d0
;				bra.s	.99

;.98				tst.l	d7									; fh?
;				bne.s	.97									; don't close it

;				move.l	a2,d1								; error cleanup
;				move.l	_DOSBase,a6
;				pea		.97
;				move.l	_maxtrax+mxtx_CloseFunc,-(sp)
;				rts

;.97				moveq	#0,d0
;				bra.s	.99

;StdOpenFunc
;				JSRLIB	Open
;				rts

;StdReadFunc
;				JSRLIB	Read
;				rts

;StdCloseFunc
;				JSRLIB	Close
;				rts

*==========================================================================*
*
*	quick tag finder -- taglist:a0 id:d0, result tagitem:d0
*
*==========================================================================*

;FindTag			move.l	(a0),d1
;				beq.s	.99
;				cmp.l	d0,d1
;				beq.s	.1
;				addq.w	#8,a0
;				bra.s	FindTag
;.1				move.l	a0,d0
;				rts
;.99				moveq	#0,d0
;				rts

*==========================================================================*
*
*	Glue for the three interrupts.
*
*==========================================================================*

* made this as fast as possible if music system off...

;MusicVBlank		tst.l	_AudioDevice		; if audio system off, exit
;				bne.s	.1
;				moveq	#0,d0				; continue chain
;				rts

;.1				move.l	_SysBase,a6
;				lea		_music_server,a1
;				xref	_LVOCause
;				jsr		_LVOCause(a6)		; Cause a softint at IMusicServer...
;				moveq	#0,d0				; continue chain
;				rts

;IMusicServer	bsr		MusicServer			; call music server
;				moveq	#0,d0				; continue chain
;				rts

;IExtraServer	bsr		ExtraServer			; call extra server
;				moveq	#0,d0				; continue chain
;				rts

;*==========================================================================*
;*	Debugging code
;*==========================================================================*
;
;			ifne	DEBUG
;knum			movem.l	d0/d1/a0/a1,-(sp)
;				move.l	20(sp),d0
;				move.l	d0,-(sp)
;				pea		knumtext
;				xref	_kprintf
;				jsr		_kprintf
;				addq.w	#8,sp
;				movem.l	(sp)+,d0/d1/a0/a1
;				rts
;
;knumtext		dc.b	'NUM %ld',10,0
;				ds.w	0
;
;knum3			movem.l	d0/d1/a0/a1,-(sp)
;				move.l	20(sp),d0
;				move.l	24(sp),d1
;				move.l	28(sp),a0
;				movem.l	d0/d1/a0,-(sp)
;				pea		knum3text
;				xref	_kprintf
;				jsr		_kprintf
;				lea		16(sp),sp
;				movem.l	(sp)+,d0/d1/a0/a1
;				rts
;
;knum3text		dc.b	'DATA: %ld,%ld,%ld',10,0
;				ds.w	0
;
;knum3s			movem.l	d0/d1/a0/a1,-(sp)
;				move.l	20(sp),d0
;				move.l	24(sp),d1
;				move.l	28(sp),a0
;				movem.l	d0/d1/a0,-(sp)
;				pea		knum3stext
;				xref	_kprintf
;				jsr		_kprintf
;				lea		16(sp),sp
;				movem.l	(sp)+,d0/d1/a0/a1
;				rts
;
;knum3stext		dc.b	'STOLE: %ld,%ld,%ld',10,0
;				ds.w	0
;
;knum3e			movem.l	d0/d1/a0/a1,-(sp)
;				move.l	20(sp),d0
;				move.l	24(sp),d1
;				move.l	28(sp),a0
;				movem.l	d0/d1/a0,-(sp)
;				pea		knum3etext
;				xref	_kprintf
;				jsr		_kprintf
;				lea		16(sp),sp
;				movem.l	(sp)+,d0/d1/a0/a1
;				rts
;
;knum3etext		dc.b	'ENV: %ld,%ld,%ld   ',0
;				ds.w	0
;			endc
;
;			end


	Section	Buffy,BSS
Buffy
	ds.b	AUDIO_MEM_SIZE
	ds.b	NUM_SCORES*score_sizeof
SamplesTable
	ds.b	NUM_SAMPLES*samp_sizeof
TempInfo
	ds.b	dsamp_sizeof

_patch		ds.b	NUM_PATCHES*patch_sizeof
_voice		ds.b	NUM_VOICES*voice_sizeof
_channel	ds.b	NUM_CHANNELS*chan_sizeof
_xchannel	ds.b	chan_sizeof
_maxtrax	ds.b	mxtx_sizeof
_globaldata	ds.b	glob_sizeof
_extra_op	ds.b	2
_extra_data	ds.b	4

_scoreptr	ds.b	4
_scoremax	ds.b	2

_audio_play
			ds.b	4
_audio_ctrl
			ds.b	4
_audio_stop
			ds.b	4
_audio_env
			ds.b	4
_play_port
			ds.b	4
_temp_port
			ds.b	4
_AudioDevice
			ds.b	4
_asample
			ds.l	NUM_SAMPLES
BuffyEnd
