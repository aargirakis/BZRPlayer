*****************************************************************************
**           IFF 8SVX Music Format Replayer for Eagleplayer 2.0+           **
**  (C) Henryk "Buggs" Richter ,all copyright notices have to stay intact  **
**  Needs ASM-One 1.29 !                                                   **
*****************************************************************************
;
	;
	incdir	include:
	include	exec/exec_lib.i
	include	exec/execbase.i
	include	exec/memory.i
	include	exec/types.i
	include	dos/dos_lib.i
	include	dos/dostags.i
	include	dos/dos.i
	include	"misc/EaglePlayer.i"
	include	"misc/EaglePlayerengine.i"

; issue: uses DTP_Check1 which isn't handled at all in UADE	
; issue: relies on Eagleplayer Amplifier APIs - might not be available in UADE..
	
; uses exec/CreateNewProc,  dos/_LVOLock, _LVOOpenFromLock, _LVOUnLock, _LVODupLock,_LVODelay
	
; XXX added
MEMF_Public	equ 1<<0
MEMF_Fast	equ 1<<2

; XXX reminder: main issue of the original code seem to be various upper/lowercase mismatches..
; which vasm doesn't seem to appreciate
	
	
; XXX inlined buggsmacros.i:
	ifnd	push
push	macro
	ifc	"\1","all"
	movem.l	d0-a6,-(SP)
	else	
	movem.l	\1,-(SP)
	endc
	endm
	endc
pop	macro
	ifc	"\1","all"
	movem.l	(SP)+,d0-a6
	else	
	movem.l	(SP)+,\1
	endc
	endm	
	
	
	
test		=	0
loadbuffersize	=	32768			;Ladepuffergröße in Bytes (größer als Playbuffersize !!)
Playbuffersize	=	1024			;Abspielpuffersize in Bytes
MinLoadBuffers	=	6			;min. 196 kB Ladepuffer
MaxLoadBuffers	=	6			;max. Anzahl der Ladepuffer (~196 kB)

 STRUCTURE	LoadBuffer,0
 	WORD	LB_Number		;Nummer des Puffers, beginnend mit 0, wird für den
 					;Fall benötigt, daß das gesamte File beim Start
 					;eingeladen wird
	BYTE	LB_LastBuffer		;dies ist der letzte Puffer, durch Ladetask gesetzt (!=0)
	BYTE	LB_Pad			;align auf Longwortgröße
	LONG	LB_Playoffset		;aktuelle Position beim Lesen, wird durch Laderoutine
					;auf 0 rückgesetzt
 	LONG	LB_LoadSize		;Anzahl der geladenen Bytes
 	STRUCT	LB_Datas,loadbuffersize	;Daten
	LABEL	LB_Sizeof
 	STRUCT	LB_StereoDatas,loadbuffersize	;Daten bei Stereo-Betrieb
	LABEL	LB_StereoSizeof

ID_8SVX 	equ	'8SVX'
ID_VHDR		equ	'VHDR'
ID_CHAN		equ	'CHAN'
ID_BODY		equ	'BODY'
ID_NAME		equ	'NAME'
ID_ANNO		equ	'ANNO'

;typedef LONG Fixed;	
;	 * A fixed-point value, 16 bits to the left of the point and 16 
;	 * to the right. A Fixed is a number of 216ths, i.e. 65536ths.	*/
;#define Unity 0x10000L	/* Unity = Fixed 1.0 = maximum volume	*/

* sCompression: Choice of compression algorithm applied to the samples.
sCmpNone	equ	0	* not compressed
sCmpFibDelta	equ	1	* Fibonacci-delta encoding
* Can be more kinds in the future.

 STRUCTURE	I8SVX_VHDR,0
	ULONG	VHDR_ID
	ULONG	VHDR_Size
	ULONG	VHDR_oneShotHiSamples	* # samples in the high octave 1-shot part */
        ULONG	VHDR_repeatHiSamples	* # samples in the high octave repeat part */
        ULONG	VHDR_samplesPerHiCycle	* # samples/cycle in high octave, else 0   */
	UWORD	VHDR_samplesPerSec	* data sampling rate	*/
	UBYTE	VHDR_ctOctave		* # octaves of waveforms	*/
      	UBYTE	VHDR_sCompression	* data compression technique used	*/
	LONG	VHDR_volume		* playback volume from 0 to Unity (full 
				 	* volume=$10000).
	LABEL	VHDR_Sizeof

;----------------------------------------------------------------------------------------------------
	ifne	test
Start:
		bsr	ClearBSS
		bsr	I8SVX_Check
		bsr	InitPlay
		bsr	InitSound
		bsr	EndSound
		bsr	Endplay
		illegal
	else
		section	0,code
	endc

		PLAYERHEADER PlayerTags

		dc.b '$VER: 8SVX Eagleplayer V0.84 (08.01.97)',0
		even

PlayerTags
		dc.l	EP_PlayerVersion,EAGLEVERSION
;XXXX		dc.l	EP_Flags,EPB_CalcDuration!EPB_ModuleInfo!EPB_VolVoices!EPB_Restart!EPB_Songend!EPB_Volume!EPB_Balance!EPB_Voices!EPB_Analyzer
		dc.l	EP_Flags,EPB_CalcDuration+EPB_ModuleInfo+EPB_VolVoices+EPB_Restart+EPB_Songend+EPB_Volume+EPB_Balance+EPB_Voices+EPB_Analyzer
		dc.l	DTP_PlayerVersion,2
		dc.l	DTP_PlayerName,MI_Name
		dc.l	DTP_Creator,MI_Buggs
; XXX		dc.l	DTP_Check1,I8SVX_Check
		dc.l	DTP_Check2,I8SVX_Check	; XXX hack
		dc.l	DTP_Config,ClearBSS

		dc.l	DTP_InitPlayer,InitPlay
		dc.l	DTP_EndPlayer,Endplay

		dc.l	DTP_InitSound,InitSound
		dc.l	DTP_EndSound,EndSound

		dc.l	DTP_StartInt,StartPlay

		dc.l	DTP_Volume,SetVoices
		dc.l	DTP_Balance,SetVoices
		dc.l	EP_Voices,SetVoices

		dc.l	EP_SetSpeed,Speedit

		dc.l	EP_NewModuleInfo,MI_InfoBuffer

		dc.l	EP_InitAmplifier,InitAudstruct
		dc.l	0

MI_Name		dc.b	'IFF 8SVX',0
MI_Buggs	dc.b	'IFF 8SVX specifications © Electronic Arts, player written by Buggs of Defect',0

	even
MI_InfoBuffer:	
	dc.l	MI_Samples,1			;4
	dc.l	MI_Duration,0			;12
	dc.l	MI_PlayFrequency,0		;20
	dc.l	MI_Calcsize,0			;28
	dc.l	MI_LoadSize,0			;36
	dc.l	MI_Voices,0			;44
	dc.l	0
EPBase		dc.l	0
;----------------------------------------------------------------------------------------------------
ClearBSS:
	lea	Datas,a0
	move	#DataSize/4-1,d0
clr$
	clr.l	(a0)+
	dbf	d0,clr$
	
	moveq #0,d0	; XXX added: UADE expects this - otherwise "score died"
	rts
;----------------------------------------------------------------------------------------------------
Speedit:
	lea	$dff0a0,a5
	move.l	I8SVX_SamplePer,d0
	bsr	PokePer
	lea	$10(a5),a5
	bsr	PokePer
	rts
;----------------------------------------------------------------------------------------------------
InitAudstruct:
	basereg	Datas,a4
		lea	Datas,a4

		lea	audstruct0,a0		;Audio Struktur vorbereiten

		move.l	#EPAMB_AudioInts!EPAMB_Direct!EPAMB_8Bit,d7
		move.l	d7,NPFlags

		lea	(a0),a1
		move.w	#AS_Sizeof*2-1,d0
.clr		clr.b	(a1)+
		dbf	d0,.clr

		move.w	#01,AS_LeftRight(a0)			;1. Kanal links
		move.w	#-1,AS_LeftRight+AS_Sizeof*1(a0)	;2. Kanal rechts

		move.l	#AudioINT,AS_Int(a0)

		lea	AudTagliste(pc),a0
		move.l	a0,EPG_AmplifierTagList(a5)

		moveq	#0,d0
SetVoices
		rts
.err
		moveq	#-1,d0
myrts		rts

	endb	a4
AudTagliste
	dc.l	EPAMT_NumStructs,2
	dc.l	EPAMT_AudioStructs,audstruct0
	dc.l	EPAMT_Flags
NPFlags
	dc.l	0
	dc.l    TAG_DONE,0
;--------------------------------------------------------------------------------------------------
	basereg	Datas,a6
INT_Aud:
		push	all
		lea	Datas,a6
		tst.b	I8SVXR_NoPlay(A6)
		bne	fatal$

		tst.b	I8SVXR_WaitPlay(A6)
		bne	not$

		moveq	#0,d0
		move	PlaybufNext(a6),d0
		lea	PlaybufL(a6),a1
		lea	PlaybufR(a6),a2
		add.l	d0,a1
		add.l	d0,a2
		eor.w	#Playbuffersize,d0
		move	d0,PlaybufNext(a6)

		move.l	EPBase,a5
		move.l	a1,d0
		moveq	#0,d1
		jsr	ENPP_PokeAdr(a5)
		move.l	#Playbuffersize/2,d0
		jsr	ENPP_PokeLen(a5)
;		move.l	I8SVX_SamplePer,d0
;		jsr	ENPP_PokePer(a5)

		move.l	a2,d0
		addq	#1,d1
		jsr	ENPP_PokeAdr(a5)
		move.l	#Playbuffersize/2,d0
		jsr	ENPP_PokeLen(a5)
;		move.l	I8SVX_SamplePer,d0
;		jsr	ENPP_PokePer(a5)

;-------------------- Puffer holen -----------------------
		move.l	I8SVXR_CurrentBuf(a6),d0
		bne.s	take$

		suba.l	a0,a0				;kein alter Buffer
		bsr	NewCurrentBuffer

		move.l	I8SVXR_CurrentBuf(a6),d0
		beq.w	fail$

take$		move.l	d0,a0

;---------------------------- Offset und Restlänge berechnen ------------------------

		lea	LB_Datas(a0),a3
		move.l	a0,a4
		add.l	#LB_StereoDatas,a4

		move.l	LB_Playoffset(a0),d0
		move.l	LB_LoadSize(a0),d1
		sub.l	d0,d1
		bgt.s	goon$

		bsr	NewCurrentBuffer
		move.l	I8SVXR_CurrentBuf(a6),d0
		beq.w	fail$
		move.l	d0,a0
		lea	LB_Datas(a0),a3
		move.l	a0,a4
		add.l	#LB_StereoDatas,a4

		move.l	LB_Playoffset(a0),d0	;sollte 0 sein, siehe unten
		move.l	LB_LoadSize(a0),d1
goon$
		move	I8SVX_BytesperFrame(a6),d2
		mulu	I8SVX_ReadPerInt(A6),d2

		cmp.l	d2,d1
		blt.s	abnormal$
		moveq	#0,d1
		move	I8SVX_ReadPerInt(A6),d1
		bra	normal$
abnormal$
		divu	I8SVX_BytesperFrame(a6),d1
normal$
		add.l	d0,a3
		add.l	d0,a4

;-------------------------------- eigentliche Abspielroutine -------------------------
		move	I8SVX_Sampletype(a6),d0
		beq	do8Bit$

		move	I8SVX_NumChannels(a6),d0
		subq	#1,d0
		bne	fibstereo8$
		bsr	FillBufFibMono8
		bra	end$
fibstereo8$
		bsr	FillBufFibStereo8
		bra	end$
do8Bit$
		move	I8SVX_NumChannels(a6),d0
		subq	#1,d0
		bne	dostereo8$
		bsr	FillBufMono8
		bra	end$
dostereo8$
		bsr	FillBufStereo8
end$
		move.l	I8SVXR_CurrentBuf(a6),d0
		beq	not$
		move.l	d0,a0
		lea	LB_Datas(a0),a1
		sub.l	a1,a3
		move.l	a3,LB_Playoffset(a0)
not$
;------------------------------------------------------------------------------------
		sf	I8SVXR_WaitPlay(A6)
		pop	all
		rts
fail$
		move.w	#Playbuffersize,d2
		bsr	CLR_Rest
		pop	all
		rts
fatal$
		moveq	#$3,d0
		bsr	PokeDMA

		pop	all
		rts

FillBufFibMono8:
		move	I8SVX_ReadPerInt(A6),d2
		sub.w	d1,d2

		tst	d1
		ble	dorest$

		push	a0/a2
		move.l	a3,a0
		lea	I8SVXR_FibvalueL(A6),a2	;letzter Fibdelta-Wert
		bsr	Delta_2_linear
		pop	a0/a2

		moveq	#0,d0
		move	d1,d0
		add.l	d0,a3
		add.l	d0,a4

		subq	#1,d0
mono1$		move.w	(a1)+,(a2)+
		dbf	d0,mono1$
dorest$
		tst	d2
		ble	fertig$

		bsr	NewCurrentBuffer
		move.l	I8SVXR_CurrentBuf(a6),d0
		beq	CLR_Rest
		move.l	d0,a0
		lea	LB_Datas(a0),a3
		move.l	a0,a4
		add.l	#LB_StereoDatas,a4

		move	d2,d1

		push	a0/a2
		move.l	a3,a0
		lea	I8SVXR_FibvalueL(A6),a2	;letzter Fibdelta-Wert
		bsr	Delta_2_linear
		pop	a0/a2

		moveq	#0,d0
		move	d2,d0
		add.l	d0,a3

		subq	#1,d0
mono2$		move.w	(a1)+,(a2)+
		dbf	d0,mono2$

fertig$		rts


FillBufFibStereo8:
		move	I8SVX_ReadPerInt(A6),d2
		sub.w	d1,d2

		tst	d1
		beq	dorest$

		push	a0/a2
		move.l	a3,a0
		lea	I8SVXR_FibvalueL(A6),a2	;letzter Fibdelta-Wert
		bsr	Delta_2_linear
		pop	a0/a2

		push	a0/a1/a2
		move.l	a4,a0
		move.l	a2,a1
		lea	I8SVXR_FibvalueR(A6),a2	;letzter Fibdelta-Wert
		bsr	Delta_2_linear
		pop	a0/a1/a2

		moveq	#0,d0
		move	d1,d0
		add.l	d0,a3
		add.l	d0,a4
		add.l	d0,d0
		add.l	d0,a1
		add.l	d0,a2
dorest$
		tst	d2
		beq	fertig$

		bsr	NewCurrentBuffer
		move.l	I8SVXR_CurrentBuf(a6),d0
		beq	CLR_Rest
		move.l	d0,a0
		lea	LB_Datas(a0),a3
		move.l	a0,a4
		add.l	#LB_StereoDatas,a4

		move	d2,d1

		push	a0/a2
		move.l	a3,a0
		lea	I8SVXR_FibvalueL(A6),a2	;letzter Fibdelta-Wert
		bsr	Delta_2_linear
		pop	a0/a2

		push	a0/a1/a2
		move.l	a4,a0
		move.l	a2,a1
		lea	I8SVXR_FibvalueR(A6),a2	;letzter Fibdelta-Wert
		bsr	Delta_2_linear
		pop	a0/a1/a2

		moveq	#0,d0
		move	d2,d0
		add.l	d0,a3		;für Weiterverarbeitung
fertig$		rts

FillBufStereo8
		move	I8SVX_ReadPerInt(A6),d2
		sub.w	d1,d2

		subq	#1,d1
		bmi	dorest$
copy$
		move.b	(a3)+,(a1)+		;Kanal 1
		move.b	(a4)+,(a2)+		;Kanal 2
		dbf	d1,copy$
dorest$
		subq	#1,d2
		bmi	fertig$

		bsr	NewCurrentBuffer
		move.l	I8SVXR_CurrentBuf(a6),d0
		beq	CLR_Rest
		move.l	d0,a0
		lea	LB_Datas(a0),a3
		move.l	a0,a4
		add.l	#LB_StereoDatas,a4
copyrest$
		move.b	(a3)+,(a1)+
		move.b	(a4)+,(a2)+
		dbf	d2,copyrest$
fertig$		rts

FillBufMono8
		move	I8SVX_ReadPerInt(A6),d2
		sub.w	d1,d2

		subq	#1,d1
		bmi	dorest$
copy$
		move.b	(a3)+,d0
		move.b	d0,(a1)+
		move.b	d0,(a2)+
		dbf	d1,copy$
dorest$
		subq	#1,d2
		bmi	fertig$

		bsr	NewCurrentBuffer
		move.l	I8SVXR_CurrentBuf(a6),d0
		beq	CLR_Rest
		move.l	d0,a0
		lea	LB_Datas(a0),a3
copyrest$
		move.b	(a3)+,d0
		move.b	d0,(a1)+
		move.b	d0,(a2)+
		dbf	d2,copyrest$
fertig$		rts

;----------------------- restliche Bytes löschen, wenn kein Puffer mehr bereit -----------------
CLR_Rest
		subq	#1,d2
		bmi	fertig$
CLR_Rest$
		clr.b	(a1)+
		clr.b	(a2)+
		dbf	d2,CLR_Rest$
fertig$		rts

CLR_Rest16
		subq	#1,d2
		bmi	fertig$
CLR_Rest$
		clr.w	(a1)+
		clr.w	(a2)+
		dbf	d2,CLR_Rest$
fertig$		rts

;--------------------------------------------------------------------------------------------
;              converts 4 Bit FibDelta Samples 2 normal 8 Bit signed PCM Samples
;Input:
;  a0   - input data
;  a1   - output data
;  a2   - pointer to initial value (.w!!), -1 at the location A2 points to indicates
;                                          start or restart of the sample
;  d1.w - input length (output length is 2*d1)
;Output:
;  a2   - initial value at (a2) updated
Delta_2_linear:
	push	all

	move	(a2),d0
	bpl.s	normal$
	clr	(a2)

	move	(a0)+,d0
	move	d0,(a1)+
	and	#$ff,d0

	subq	#2,d1
normal$

	lea	fibnumbertab(pc),a3
	subq	#1,d1		;Input length <= 0  ?
	bmi	fail$

	moveq	#0,d2
	moveq	#%00001111,d4
loop$	move.b	(a0)+,d2
	move.l	d2,d3
	lsr	#4,d2
	add.b	(a3,d2.l),d0
	move.b	d0,d5
	lsl	#8,d5
	and	d4,d3
	add.b	(a3,d3.l),d0
	move.b	d0,d5
	move.w	d5,(a1)+
	dbf	d1,loop$
fail$
	move	d0,(a2)

	pop	all
	rts

;------------------------------------------------------------------------------------------
;- Input:  A0 - alter Puffer oder 0
;- Output: A0 - neuer Puffer oder 0 (selbiges gilt für I8SVXR_CurrentBuf)
;
NewCurrentBuffer:
		push	d0-d7/a1-a6

		move.l	a0,d0
		beq.w	noold$

		lea	I8SVX_ReadyBufList1(a6),a1
such$		tst.l	(a1)+			;einer ist garantiert frei, da genau der jetzige
		bne.s	such$			;Puffer in eine Liste soll, die für alle ausreicht

		move.l	a0,-4(a1)		;und in Readyliste eintragen

		tst.b	I8SVXR_LoadOnce(a6)	;müssen wir darauf achten, daß der Puffer erhalten
		bne.s	keepNum$		;bleibt ?
		clr.w	LB_Number(a0)		;nein, Puffer freigeben
keepNum$
		tst.b	LB_LastBuffer(a0)	;zuende gespielt ?
		beq.s	nosongend$

		moveq	#-1,d0
		move	d0,I8SVXR_FibvalueL(A6)		;FibDelta-Initialwert Kanal 1
		move	d0,I8SVXR_FibvalueR(A6)		;FibDelta-Initialwert Kanal 2

		move.l	EPBase(pc),a2		;Eagleplayer benachrichtigen, daß das
		jsr	ENPP_SongEnd(a2)	;Songende erreicht wurde

		tst.b	I8SVXP_NoLoad(A6)	;Test, ob noch irgend etwas nachgeladen wurde
		beq.s	nosortlist$

		move	LB_Number(a0),d0	;höchste Nummer (die des letzten Puffers)
		moveq	#0,d3				;aktuelle Nummer
		lea	I8SVX_ReadyBufList1(a6),a1	;Liste 1
		lea	I8SVX_LoadedBufList1(a6),a2	;Liste 2
sortlist$						;
		move.l	a1,a0				;Anfang der Liste
		moveq	#MaxLoadBuffers-1,d1		;max. Anzahl
suchentry$
		move.l	(a0)+,d2			;Eintrag ?
		bne.s	foundentry$			;yup
suchweiter$
		dbf	d1,suchentry$			;weitersuchen

		st	I8SVXR_NoPlay(a6)		;Fatal Error, Replay anhalten

		suba.l	a0,a0				;sorry, wir haben nix zum Füttern des Players
		clr.l	I8SVXR_CurrentBuf(a6)		;kein aktueller Puffer
		bra	fail$
foundentry$
		move.l	d2,a3				;
		cmp.w	LB_Number(a3),d3		;Stimmt Nummer ?
		bne	suchweiter$			;nein, weitersuchen

		clr.l	LB_Playoffset(a3)		;Offset rücksetzen

		clr.l	-4(a0)				;Eintrag aus Liste entfernen
		move.l	a3,(a2)+			;in andere Liste einklinken
		addq	#1,d3
		dbf	d0,sortlist$			;weiter

		moveq	#MaxLoadBuffers-1,d1		;max. Anzahl
		lea	I8SVX_ReadyBufList1(a6),a1	;Liste 1
clrentries$						;
		clr.l	(a1)+				;nur zur Sicherheit alle möglichen Einträge
		clr.l	(a2)+				;beider Listen
		dbf	d1,clrentries$			;löschen
nosortlist$
nosongend$

noold$
		lea	I8SVX_LoadedBufList1(a6),a1 ;Liste der fertig geladenen Puffer
		moveq	#MaxLoadBuffers-1,d0	;
findfree$
		move.l	(a1)+,d1		;falls einmal aus Handshaking-Gründen ein Puffer
		bne	found$			;ausgelassen wurde, muss die gesamte Liste
		dbf	d0,findfree$		;durchsucht werden

		suba.l	a0,a0			;sorry, wir haben nix zum Füttern des Players
		clr.l	I8SVXR_CurrentBuf(a6)	;kein aktueller Puffer

		bsr	signal_load$		;aber Hallo! Sofort Daten Laden, wird`s bald ?
		bra	fail$
found$
		move.l	d1,a0			;neuer Puffer

		lea	I8SVX_LoadedBufList1(a6),a2 ;alles bis nach oben durchscrollen
		moveq	#MaxLoadBuffers-1,d0
copylist$
		move.l	(a1)+,(a2)+
		dbf	d0,copylist$

		move.l	a0,I8SVXR_CurrentBuf(a6)	;eintragen in Globals

		move.b	I8SVXR_CountBufferchg(A6),d0
		addq.b	#1,d0
		cmp.b	I8SVXR_NumCountBuffer(A6),d0
		blt	nosig$
		bsr	signal_load$		;Signal nur alle 2 mal schicken, das sollte reichen

		moveq	#0,d0
nosig$		move.b	d0,I8SVXR_CountBufferchg(A6)

fail$
		pop	d0-d7/a1-a6
		rts

signal_load$
		push	all
		
		tst	I8SVXP_Ready(A6)		;ist der Lade-Task noch da ?
		beq	local_fail$		;

		move.l	I8SVXP_Address(A6),a1	;ok, geben wir dem mal was zu tun
		move.l	I8SVXP_SignalLoad(A6),d1	;
		moveq	#0,d0			;
		bset	d1,d0			;
		move.l	myExecbase(A6),a6	;
	endb	a6
		jsr	Signal(a6)		;
local_fail$
		pop	all
		rts
;----------------------------------- Checkroutine ------------------------------------------------
I8SVX_Check:
	basereg	Datas,a4
		lea	Datas,a4

	ifne	test
		lea	testsample,a0
		move.l	#testsize,d7
	else
		move.l	dtg_ChkData(A5),a0
		move.l	dtg_ChkSize(A5),d7
	endc

		move.l	a0,a1
		move.l	a0,d6
		add.l	d7,a1

		cmp.l	#'FORM',(a0)
		bne.w	fail$
		addq.l	#4,a0
		move.l	(a0)+,TMP_FileSize(a4)
		cmp.l	#ID_8SVX,(a0)+
		bne.w	fail$
		move.l	a0,a2

		moveq	#1,d4
		move.l	a2,a0
		move.l	#ID_CHAN,d0
		bsr	FindIFFchunk			;optional
		beq	mono$
		cmp.l	#6,8(A0)			;Left=4 + Right=2 -> 6
		bne	mono$				;wenn nur Left oder nur Right angegeben, wird
		moveq	#2,d4				;das Ganze wie ein normales Mono-Sample
mono$							;abgespielt, PAN-Chunks werden ignoriert
		move	d4,TMP_numChannels(A4)

		move.l	#ID_VHDR,d0
		move.l	a2,a0
		bsr	FindIFFchunk			;required
		beq	fail$

		moveq	#0,d0
		move.b	VHDR_sCompression(A0),d0
;		cmp.b	#sCmpNone,d0
		beq.s	none$
		cmp.b	#sCmpFibDelta,d0		;fibdelta und ungepackt wird unterstützt
		bne.s	fail$
none$
		move	d0,TMP_sampletype(a4)		;entweder 4 Bit FibDelta oder 8 Bit unpacked
		move	#1,TMP_BytesperFrame(a4)	;immer 1 Byte auf einmal lesen (für einen Kanal)

		moveq	#0,d0
		move	VHDR_samplesPerSec(a0),d0
		move.l	d0,TMP_sampleRate(A4)

		move.l	#ID_BODY,d0
		move.l	a2,a0
		bsr	FindIFFchunk
		beq	fail$				;muß drin sein

		lea	8(a0),a0
		sub.l	d6,a0
		move.l	a0,TMP_BodyOffset1(A4)
		clr.l	TMP_BodyOffset2(A4)		;bei Stereo 2 Files öffnen, 2. Offset hier
		subq	#1,d4
		beq.s	mono1$
		lsr.l	d4,d0
		add.l	d0,a0
		move.l	a0,TMP_BodyOffset2(A4)
mono1$
		move.l	d0,TMP_BodySize(A4)

		cmp	#sCmpNone,TMP_sampletype(a4)	;entweder 4 Bit FibDelta oder 8 Bit unpacked
		beq.s	nodouble$
		add.l	d0,d0
nodouble$	move.l	d0,TMP_numSampleFrames(A4)


;		move.l	#ID_NAME,d0
;		move.l	a2,a0
;		bsr	FindIFFchunk
;		beq	noname$
;		move.l	#ID_ANNO,d0
;		move.l	a2,a0
;		bsr	FindIFFchunk
;		beq	noANNO$

		moveq	#0,d0
		rts

fail$		moveq	#-1,d0
		rts

;-------------------------------------------------------------------------------
;                    Suche angegebenen IFF Chunk
;Input:  A0 - aktuelle Position im IFF File 
;        D0 - zu suchender Chunk
;        A1 - geladene Länge (Overflow check)
;Output: A0 - Pointer auf den Chunk
;        D0 - Suxxess, 0=fail, sonst Länge des Chunks 
FindIFFchunk:
	push	d1
loop$
		cmp.l	(a0),d0
		beq	ok$
		move.l	4(a0),d1
		add.l	#8+1,d1
		and.l	#~1,d1
		add.l	d1,a0
;		addq.l	#4,a0
		cmp.l	a0,a1
		bgt	loop$
	pop	d1
		moveq	#0,d0
		rts
ok$
	pop	d1
		move.l	4(a0),d0
		rts
;-------------------------------------------------------------------------------
;		Convert an 80 bit IEEE Standard 754 floating point number
;		into an integer value.
;Format: WORD Exponent, Bit 15 ist `±`
;        LONG Mantisse[2]
;
;Input:  A0 - Pointer auf IEEE Zahl
;Output: D0 - Zahl als Integer
;
Extended2Long:
		move.b	(a0),d0			;Zahl negativ ?
		smi	d1
		ext.w	d1
		ext.l	d1
		or.b	#1,d1			;bei positiv -> 1, negativ -> -1

		move.l	2(a0),d0

		move	(a0),d2			;Exponent
		and	#$7fff,d2
		sub	#$3fff,d2
		bmi.s	ret_0$			;<0 ? -> raus

						; Special meaning?
		sub	#31,d2
						; Overflow?
		bpl.s	ret_maxInt$

		neg	d2
		lsr.l	d2,d0			; in D0 Samplerate
		rts
ret_0$
		moveq	#0,d2
		rts
ret_maxInt$	move.l	#$7FFFFFFF,d2
		rts
;--------------------------------------------------------------------------------------------------
StartPlay:
		lea	Datas,a4

		moveq	#$3,d0
		bsr	PokeDMA

		lea	PlaybufL(A4),a0
		lea	PlaybufR(a4),a2

		lea	PlaybufL(a4),a1
;		move	#[Playbufend-PlaybufL]/4-1,d0
		move	#(Playbufend-PlaybufL)/4-1,d0
clr$		clr.l	(a1)+
		dbf	d0,clr$

		lea	$dff0a0,a5
		move.l	a0,d0
		bsr	PokeAdr
		move	#Playbuffersize/2,d0
		bsr	PokeLen
		move.l	I8SVX_SamplePer(A4),d0
		bsr	PokePer
		moveq	#60,d0
		bsr	PokeVol

		lea	$dff0b0,a5
		move.l	a2,D0
		bsr	PokeAdr
		move	#Playbuffersize/2,d0
		bsr	PokeLen
		move.l	I8SVX_SamplePer,d0
		bsr	PokePer
		moveq	#60,d0
		bsr	PokeVol

		move	#Playbuffersize,PlaybufNext

		move.b	#1,I8SVXR_WaitPlay

		move	#$8003,d0
		bsr	PokeDMA

		moveq	#0,d0
		rts

;-----------------------------------------------------------------------------------
PokeVol:			;in D0 Lautstärke
	movem.l	d1/a5,-(sp)
	move.w	a5,d1		;Dff0a0/b0/c0/d0
	sub.w	#$f0a0,d1
	lsr.w	#4,d1		;Nummer der Stimme von 0-3
	move.l	EPBase,a5
	jsr	ENPP_PokeVol(a5)
	movem.l	(sp)+,d1/a5
	rts
PokePer:			;in D0 Periodenwert
		push	d1/d2/a5

		move.w	a5,d1		;Dff0a0/b0/c0/d0
		sub.w	#$f0a0,d1
		lsr.w	#4,d1		;Nummer der Stimme von 0-3
		and.w	#$f,d1

		move.l	EPBase,a5

		move	EPG_Speed(a5),d2
		beq.s	.speed0
	
		bmi.s	.slower
		mulu	#20,d0
		add	#20,d2
		divu	d2,d0
		bra.s	.speed0
.slower
		neg	d2
		add	#20,d2
		mulu	d2,d0
		divu	#20,d0
.speed0
		jsr	ENPP_PokePer(a5)
		pop	d1/d2/a5
		rts
PokeLen:			;in D0 Länge in Worten
	movem.l	d1/a5,-(sp)
	move.w	a5,d1		;Dff0a0/b0/c0/d0
	sub.w	#$f0a0,d1
	lsr.w	#4,d1		;Nummer der Stimme von 0-3
	and.l	#$ffff,d0

	move.l	EPBase,a5
	jsr	ENPP_PokeLen(a5)

	movem.l	(sp)+,d1/a5
	rts
PokeAdr:			;in D0 Adresse
	movem.l	d1/a5,-(sp)
	move.w	a5,d1		;Dff0a0/b0/c0/d0
	sub.w	#$f0a0,d1
	lsr.w	#4,d1		;Nummer der Stimme von 0-3
	move.l	EPBase,a5
	jsr	ENPP_PokeAdr(a5)
	movem.l	(sp)+,d1/a5
	rts
PokeDMA:
	movem.l	d0/d1/d2/a5,-(sp)
	move.l	EPBase,a5
	move.w	d0,d1
	and.w	#$8000,d0	;D0.w neg=enable ; 0/pos=disable
	and.l	#15,d1		;D1 = Maske (LONG !!)
	jsr	ENPP_DMAMask(a5)
	movem.l	(sp)+,d0/d1/d2/a5
	rts

;-----------------------------------------------------------------------------------
;
;
;
InitPlay:
		lea	Datas,a4
		move	TMP_numChannels(A4),I8SVX_NumChannels(A4)	;temporäre Variablen kopieren
		move	TMP_sampletype(a4),I8SVX_Sampletype(A4)		;
		move	TMP_BytesperFrame(A4),I8SVX_BytesperFrame(a4)	;
		move.l	TMP_numSampleFrames(A4),I8SVX_numSampleFrames(A4);
		move.l	TMP_sampleRate(a4),I8SVX_sampleRate(a4)		;
		move.l	TMP_BodyOffset1(A4),I8SVX_BodyOffset1(A4)	;
		move.l	TMP_BodyOffset2(A4),I8SVX_BodyOffset2(A4)	;
		move.l	TMP_BodySize(A4),I8SVX_BodySize(A4)		;

		move.l	4,myExecbase(A4)
		move.l	a5,EPBase					;Eagleplayer Base sichern
		
		move.b	#2,I8SVXR_NumCountBuffer(A4)		;Warteanzahl für das Neuladen der
								;Puffer
		sf	I8SVXR_LoadOnce(A4)			;Datei nur einmal laden `aus`

		move.l	#LB_Sizeof*MinLoadBuffers,d0		;Mindestanzahl an Puffern allozieren

		tst.l	I8SVX_BodyOffset2(A4)			;Stereo Sample ?
		beq.s	puffer_nostereo$			;

		move.l	#LB_StereoSizeof*MinLoadBuffers,d0	;bei Stereo größeren Puffer nehmen
puffer_nostereo$
		move.l	d0,I8SVX_LoadBufSize(A4)		;Größe sichern

		moveq	#MEMF_Fast!MEMF_Public,d1		;
		move.l	myExecbase(A4),a6			;
		jsr	AllocMem(A6)			;
		move.l	d0,I8SVX_LoadBuf(A4)			;
		beq.w	fail$					;
								;
		cmp.l	#loadbuffersize*MinLoadBuffers,I8SVX_BodySize(A4) ;reichen Puffer bereits
		bgt.s	puffer_nosecond$			; für gesamtes File aus ?

		st	I8SVXR_LoadOnce(A4)			;Datei nur einmal laden an
puffer_nosecond$						;

	ifeq	test

		move.l	dtg_DOSBase(a5),MyDosBase(A4)		;DosBase holen

		move.l	dtg_PathArrayPtr(a5),a0			;Filepfad zusammensetzen
		clr.b	(a0)					;
		move.l	dtg_CopyDir(a5),a0			;
		jsr	(a0)					;
		move.l	dtg_CopyFile(a5),a0			;
		jsr	(a0)					;

		move.l	MyDosBase(A4),a6				;File Lock holen, der dann
		move.l	dtg_PathArrayPtr(a5),d1			;an den Playprozeß weitergereicht
		moveq	#-2,d2					;wird
		jsr	_LVOLock(a6)				;
		move.l	d0,PR_Filelock(a4)			;
		beq.w	fail$					;

;		lea	-[fib_SIZEOF+20](sp),sp			;Filelänge rauskriegen
		lea	-(fib_SIZEOF+20)(sp),sp			;Filelänge rauskriegen
		move.l	sp,d2					;kein Allocmem, einfach Stack benutzt
		addq.l	#4,d2					;Adresse auf 
		and.l	#~3,d2					; Langwortgröße align
		move.l	d2,a3					;Adr sichern
		move.l	d0,d1					;Filelock
		jsr	_LVOExamine(a6)				;Examine
		tst.l	d0					;
		beq.s	.ItsAll					;
		move.l	fib_Size(a3),MI_InfoBuffer+36		;Wert sichern
.ItsAll								;
		lea	fib_SIZEOF+20(sp),sp			;Filelänge rauskriegen
	else
		lea	dos(pc),a1
		moveq	#37,d0
		move.l	myExecbase(A4),a6
		jsr	_LVOopenlibrary(a6)
		move.l	d0,MyDosBase(A4)

		move.l	MyDosBase(A4),a6
		move.l	#filepath,d1
		moveq	#-2,d2
		jsr	_LVOLock(a6)			;File Lock
		move.l	d0,PR_Filelock(a4)
		beq.s	fail$

;		lea	testloadbuff,a0
;		move.l	a0,I8SVX_LoadBuf(A4)
	endc

		move	#Playbuffersize,d0
		cmp	#sCmpNone,I8SVX_Sampletype(a4)		;entweder 4 Bit FibDelta oder 8 Bit unpacked
		beq	oksize$
		move	#Playbuffersize/2,d0
oksize$
		move	d0,I8SVX_ReadPerInt(A4)

		move.l	I8SVX_sampleRate(A4),d1
		move.l	d1,MI_InfoBuffer+20	;Set Playfrequency

		move.l	I8SVX_numSampleFrames(a4),d0
		bsr.s	longdiv
		move.l	d0,MI_InfoBuffer+12	;Set Duration

		move.l	TMP_FileSize(a4),d0
		addq.l	#8,d0
		move.l	d0,MI_InfoBuffer+28	;Set Size

		moveq	#0,d0
		move	I8SVX_NumChannels(A4),d0
		move.l	d0,MI_InfoBuffer+44	;Wert sichern

		moveq	#0,d0
		rts
fail$
		bsr	Endplay
		
		moveq	#-1,d0
		rts

* divu_32 --- d0 = d0/d1, d1=jakojäännös
longdiv
	move.l	d3,-(a7)
	swap	d1
	tst	d1
	bne.b	lb_5f8c
	swap	d1
	move.l	d1,d3
	swap	d0
	move	d0,d3
	beq.b	lb_5f7c
	divu	d1,d3
	move	d3,d0
lb_5f7c	swap	d0
	move	d0,d3
	divu	d1,d3
	move	d3,d0
	swap	d3
	move	d3,d1
	move.l	(a7)+,d3
	rts	

lb_5f8c	swap	d1
	move	d2,-(a7)
	moveq	#16-1,d3
	move	d3,d2
	move.l	d1,d3
	move.l	d0,d1
	clr	d1
	swap	d1
	swap	d0
	clr	d0
lb_5fa0	add.l	d0,d0
	addx.l	d1,d1
	cmp.l	d1,d3
	bhi.b	lb_5fac
	sub.l	d3,d1
	addq	#1,d0
lb_5fac	dbf	d2,lb_5fa0
	move	(a7)+,d2
	move.l	(a7)+,d3
	rts	
;------------------------------------------------------------------------------------------
;
;
;
Endplay:
		lea	Datas,a4
;	ifeq	test
		move.l	I8SVX_LoadBuf(A4),d0		;Puffer alloziert ?
		beq.s	exit1$				;nein -> raus
		move.l	d0,a1				;
		move.l	I8SVX_LoadBufSize(A4),d0	;Größe holen
		move.l	myExecbase(A4),a6		;
		jsr	FreeMem(a6)			;
exit1$		clr.l	I8SVX_LoadBuf(A4)		;Puffer leer setzen

;	endc
		bsr	UnlockFile

		moveq	#0,d0
		rts

;------------------------------------------------------------------------------------------
;
;
;
InitSound:
		bsr	EndSound

		lea	Datas,a4

		move.l	myExecbase(A4),a1		;
		move.l	ex_EClockFrequency(a1),d2	;
		move.l	d2,d0				;
		lsl.l	#2,d2				;mal 4
		add.l	d0,d2				;mal 4+1 = mal 5!
	;	move.l	#3546895,d2			;
		move.l	I8SVX_sampleRate(a4),d0		;
		divu	d0,d2				;
		and.l	#$ffff,d2			;
		move.l	d2,I8SVX_SamplePer(A4)		;

		moveq	#-1,d0				;Signal allozieren, 
		move.l	myExecbase(A4),a6		;für Returnmeldungen vom Prozeß
		jsr	AllocSignal(a6)		;
		move.l	d0,I8SVXP_SignalReturn(a4)	;
		bmi	fail$				;

		suba.l	a1,a1				;Task suchen
		move.l	myExecbase(A4),a6		;
		jsr	FindTask(A6)		;
		move.l	d0,I8SVXP_SigTask(A4)		;

		move.l	I8SVX_LoadBuf(A4),a0		;alle Puffer in Readylist setzen, sprich
		lea	I8SVX_ReadyBufList1(A4),a1	;alle können vom LadeProzeß voll Daten
		moveq	#MinLoadBuffers-1,d0		;geladen werden

		move.l	#LB_Sizeof,d1			;Pufferlänge bei Mono
		tst.l	I8SVX_BodyOffset2(A4)		;
		beq.s	initreadybuffers1$		;
		move.l	#LB_StereoSizeof,d1		;Pufferlänge bei Stereo
initreadybuffers1$					;
		clr	LB_Number(a0)			;
		sf	LB_LastBuffer(a0)		;
		move.l	a0,(a1)+			;
		add.l	d1,a0				;
		dbf	d0,initreadybuffers1$		;

		move.l	#-1,I8SVX_DummyBuf(a4)		;siehe in Datenarea
		
		moveq	#MaxLoadBuffers*2-1,d0		;kein Puffer ist `ready to play`
		lea	I8SVX_LoadedBufList1(A4),a2	;
initloadedbuffer$					;
		clr.l	(a2)+				;
		dbf	d0,initloadedbuffer$		;

		clr.l	I8SVXR_CurrentBuf(a4)

		move.l	#-1,I8SVXP_SignalQuit(a4)	;Initialwerte für den LadeProzeß
		move.l	#-1,I8SVXP_SignalLoad(a4)	;
		clr.l	I8SVXP_Fail(A4)			;
		sf	I8SVXP_Ready(A4)			;
		sf	I8SVXP_NoLoad(A4)		;
		sf	I8SVXP_ReadyToPlay(A4)		;

		sf	I8SVXR_NoPlay(A4)		;<- für Replay
		moveq	#-1,d0
		move	d0,I8SVXR_FibvalueL(A4)		;FibDelta-Initialwert Kanal 1
		move	d0,I8SVXR_FibvalueR(A4)		;FibDelta-Initialwert Kanal 2
		
		lea	mytags(A4),a0			;LadeProzeß starten
		move.l	a0,d1				;
		move.l	#NP_Entry,(a0)+			;
		move.l	#Process,(a0)+			;
		move.l	#NP_Name,(A0)+			;
		move.l	#PR_Name,(A0)+			;
		move.l	#NP_Priority,(a0)+		;
		move.l	#5,(a0)+			;
		clr.l	(a0)				;
		move.l	MyDosBase(a4),a6			;
		jsr	_LVOCreateNewProc(a6)		;
		move.l	d0,I8SVXP_Address(a4)		;
		beq.s	fail$				;

		moveq	#0,d0				;Auf Meldung vom Prozeß warten
		move.l	I8SVXP_SignalReturn(a4),d3	;
		bset	d3,d0				;
		move.l	myExecbase(A4),a6		;
		jsr	Wait(A6)			;

		tst.l	I8SVXP_Fail(A4)			;Prozeßfehler ?
		bne	fail$				;

		tst.b	I8SVXP_Ready(a4)			;Prozeß hat sich -warum auch immer-
		beq	EndSound			;selbst beendet ?

		move.l	I8SVXP_Address(A4),a1		;alle Puffer mit Stuff füllen lassen
		move.l	I8SVXP_SignalLoad(A4),d1		;
		moveq	#0,d0				;
		bset	d1,d0				;
		move.l	myExecbase(A4),a6		;
		jsr	Signal(a6)			;wir wollen Daten !!

		moveq	#0,d0				;Auf Meldung vom Prozeß warten, dann kann`s
		move.l	I8SVXP_SignalReturn(a4),d3	;mit dem Abspielen losgehen
		bset	d3,d0				;
		move.l	myExecbase(A4),a6		;
		jsr	Wait(A6)			;
							;
wart$							;nun noch warten, bis die Puffer alle
		tst.b	I8SVXP_ReadyToPlay(A4)		;voll sind (wegen möglichem LoadBefore)
		bne.s	fertig$				;
		move.l	MyDosBase(a4),a6			;
		moveq	#5,d1				;
		jsr	_LVODelay(a6)			;

		tst.b	I8SVXP_Ready(a4)			;Prozeß hat sich aufgrund eines Dateifehlers
		beq	EndSound			;selbst beendet ?
		bra	wart$				;
fertig$
		moveq	#0,d0
		rts
fail$
fail2$
		moveq	#-1,d0
		rts
EndSound:
		lea	Datas,a4

		lea	PlaybufL(A4),a0			;Datenmüll raus aus den Abspielpuffern
;XXX		move.w	#[Playbufend-PlaybufL]/4-1,d0	;
		move.w	#(Playbufend-PlaybufL)/4-1,d0	;
.clr		clr.l	(a0)+				;
		dbf	d0,.clr				;

		move.l	I8SVXP_SignalReturn(a4),d0	;Signal freigeben
		bmi	nosig$				;
		move.l	myExecbase(A4),a6		;
		jsr	FreeSignal(a6)		;
nosig$		move.l	#-1,I8SVXP_SignalReturn(a4)	;

		lea	PR_Name(pc),a1			;Nachsehen, ob der LadeTask noch da ist
		move.l	myExecbase(A4),a6		;
		jsr	FindTask(a6)		;
		tst.l	d0				;
		beq	ende$				;
		move.l	d0,a1				;
		
		move.l	I8SVXP_SignalQuit(A4),d1		;LadeTask mit freundlichem Nachdruck beenden
		moveq	#0,d0				;
		bset	d1,d0				;
		move.l	myExecbase(A4),a6		;
		jsr	Signal(a6)			;

loopwait$
		move.l	MyDosBase(a4),a6			;kurz warten
		moveq	#1,d1				;
		jsr	_LVODelay(a6)			;

		lea	PR_Name(pc),a1			;Nachsehen, ob der LadeTask noch da ist
		move.l	myExecbase(A4),a6		;
		jsr	FindTask(a6)		;
		tst.l	d0				;
		beq	ende$				;

		bra	loopwait$			;und nachschauen, ob die Aufforderung
							;erfolgreich war
ende$
		moveq	#-1,d0				;-1, falls hier von Initsound aufgrund eines
							;Fehlers herverzweigt wurde
		rts
UnlockFile:
		push	all
		lea	Datas,a4
		move.l	MyDosBase(A4),a6

		move.l	PR_Filelock(a4),d1
		beq	fail$
		jsr	_LVOUnLock(A6)

fail$		clr.l	PR_Filelock(A4)
		pop	all
		rts
*****************************************************************************************
*          Prozeß, der die Daten auf Kommando asynchron zum Abspielen liest             *
*****************************************************************************************
;Rules:
;1. nach dem Start dieses Prozesses wird auf jeden Fall ein Signal an den Parent Task
;   geschickt, falls I8SVXP_Fail dabei != 0 ist, wurde der Prozeß bereits aufgrund eines
;   Fehlers beendet
;2. jedes Signal vom Parent wird erwidert, sobald es übernommen wurde, es sei denn,
;   der Parent-Task löscht die `I8SVXP_SignalReturn` Speicherzelle mit -1
;3. sollte I8SVXP_Ready = 0 sein, wurde der Prozeß beendet und darf naturgemäß nicht
;   mehr vom Parent angesprochen werden
;4. die Sicherheit der Datenübernahme wird dadurch gewährleistet, daß nur shared
;   Memory Areas beschrieben werden, die 0 sind und an denen nur einer der beiden
;   (Int oder Prozess) ein Interesse haben
;5. nach dem Setzen des `I8SVXP_NoLoad`-Flags gibt der Lade-Prozeß die gesamte Kontrolle
;   über die Pufferlisten ab, die dann komplett von der Abspielroutine verwaltet werden,
;   der Lade-Prozeß hat dann dort nichts mehr zu suchen
;
;mögliche Fehlercodes in I8SVXP_Fail:
;0 - kein Fehler :-)
;1 - konnte Signal nicht bekommen (sehr unwahrscheinlicher Fehler)
;2 - Dateifehler beim Duplizieren des Locks, dem Dateiöffnen vom Lock aus oder
;    beim Dateilesen
;
Process:
		lea	Datas,a4

;---------------------------- Init: Signale allozieren ----------------------------------
		moveq	#-1,d0
		move.l	myExecbase(A4),a6
		jsr	AllocSignal(a6)	;Signal allozieren
		move.l	d0,I8SVXP_SignalQuit(a4)
		blt	PR_Fail1

		moveq	#-1,d0
		move.l	myExecbase(A4),a6
		jsr	AllocSignal(a6)	;2. Signal allozieren (für Interrupt)
		move.l	d0,I8SVXP_SignalLoad(a4)
		blt	PR_Fail1

;---------------- Init: File öffnen und Seek auf richtige Position ----------------------

		move.l	PR_Filelock(a4),d1
		beq	PR_Fail2
		move.l	MyDosBase(A4),a6
		jsr	_LVODupLock(A6)
		move.l	d0,d1
		beq	PR_Fail2
		move.l	d1,d6
		move.l	MyDosBase(A4),a6
		jsr	_LVOOpenFromLock(A6)
		move.l	d0,I8SVXP_File1(A4)
		bne.s	weiter$

		move.l	d6,d1
		jsr	_LVOUnLock(A6)
		bra	PR_Fail2
weiter$	
		move.l	d0,d1
		move.l	I8SVX_BodyOffset1(a4),d2;Offset für die Rohdaten
		moveq	#OFFSET_BEGINNING,d3	;Modus: von Dateistart aus
		jsr	_LVOSeek(A6)		;

		clr.l	I8SVXP_File2(A4)	;

		tst.l	I8SVX_BodyOffset2(A4)	;brauchen wir 2. Filehandle ?
		beq.s	nostereo1$		;nein

		move.l	PR_Filelock(a4),d1	;dasselbe Spiel nochmal
		move.l	MyDosBase(A4),a6		;
		jsr	_LVODupLock(A6)		;
		move.l	d0,d1			;
		beq	PR_Fail2		;
		move.l	d1,d6			;
		move.l	MyDosBase(A4),a6		;
		jsr	_LVOOpenFromLock(A6)	;
		move.l	d0,I8SVXP_File2(A4)	;
		bne.s	weiter2$		;

		move.l	d6,d1			;
		jsr	_LVOUnLock(A6)		;
		bra	PR_Fail2		;
weiter2$	
		move.l	d0,d1
		move.l	I8SVX_BodyOffset2(a4),d2;Offset für die Rohdaten Kanal 2
		moveq	#OFFSET_BEGINNING,d3	;Modus: von Dateistart aus
		jsr	_LVOSeek(A6)		;
nostereo1$
		clr.l	PR_Bytesread(a4)		;0 Bytes bisher gelesen (für Restart-Check)
		clr.w	I8SVXP_CurrentNumber(A4)	;aktuelle Nummer des Puffers

		st	I8SVXP_Ready(A4)		;`hallo ! wir sind da`

		bsr	PR_SendSignal			;Signal: wir sind da
		bne	PR_ret				;huch! Antwort unmöglich -> Prozeß beenden
;---------------------------------- Warteschleife -----------------------------------------
PR_WaitLoop
		moveq	#0,d0
		move.l	I8SVXP_SignalQuit(a4),d3	;Quit-Kommando
		bset	d3,d0
		move.l	I8SVXP_SignalLoad(a4),d4	;Lade-Kommando
		bset	d4,d0

		move.l	myExecbase(A4),a6
		jsr	Wait(A6)

		btst	d3,d0		;Quit-Kommando austesten (hätte auch SIGF_Break sein können, aber was solls)
		bne	PR_ret		;<- beim Verlassen des Prozesses wird das Signal quittiert

		bsr	PR_SendSignal	;Kommando erwidern
		bne	PR_ret			;huch! Antwort unmöglich -> Prozeß beenden

	;------------- hier nun Datei in einen der Puffer laden --------------------------
loadnext$
		tst.b	I8SVXP_NoLoad(A4)	;nix mehr laden ?
		bne.s	PR_WaitLoop		;ok, weiterwarten, bis Prozeß beendet wird

		lea	I8SVX_ReadyBufList1(a4),a1
		moveq	#MaxLoadBuffers-1,d0
searchempty$
		move.l	(a1)+,d1
		bne.s	foundempty$
searchnextempty$
		dbf	d0,searchempty$

		st	I8SVXP_ReadyToPlay(a4)	;alle Puffer voll, Abspielen kann beginnen
		
		bra	PR_WaitLoop		;keinen leeren gefunden -> also sind alle Puffer
						;voll und einer beim Abspielen und wir können
						;"weiterpennen"
foundempty$
		move.l	d1,a0

		tst.b	I8SVXR_LoadOnce(a4)	;müssen wir darauf achten, daß der Puffer 
		beq.s	takeit$			;erhalten bleibt ?

		tst.w	LB_Number(A0)		;Nummer des Puffers testen
		bne	searchnextempty$	;-> nicht leer, anderen Suchen
takeit$
		clr.l	-4(a1)			;aus Readyliste rausnehmen

		clr.l	LB_Playoffset(a0)			;Offset rücksetzen
		clr.l	LB_LoadSize(a0)				;Länge rücksetzen
		sf	LB_LastBuffer(a0)			;Flag für Songende löschen 
		move	I8SVXP_CurrentNumber(A4),LB_Number(a0)	;Nummer des Puffers
		addq.w	#1,I8SVXP_CurrentNumber(A4)		;Nummer für nächsten Puffer erhöhen
		
		tst.l	I8SVXP_File2(a4)	;2. File vorhanden ?
		beq	nofile2_2$		;
						;
		push	a0			;
		lea	(a0),a1			;
		add.l	#LB_StereoDatas,a1	;
						;Versuch, 32 kB an Daten
		move.l	a1,d2			;zu lesen
		move.l	I8SVXP_File2(a4),d1	;
		move.l	#loadbuffersize,d3	;
		move.l	MyDosBase(A4),a6		;
		jsr	_LVORead(A6)		;
		pop	a0			;
		move.l	d0,LB_LoadSize(a0)	;wieviel wurde wirklich geladen ?
		bmi	PR_Fail2		;nix-> Fehler und raus aus dem Programm
nofile2_2$					;
		push	a0			;
		lea	LB_Datas(a0),a1		;Versuch, 32 kB an Daten
		move.l	a1,d2			;zu lesen
		move.l	I8SVXP_File1(a4),d1	;
		move.l	#loadbuffersize,d3	;
		move.l	MyDosBase(A4),a6		;
		jsr	_LVORead(A6)		;
		pop	a0			;
		tst.l	LB_LoadSize(a0)		;
		bne.s	skipset$		;
		move.l	d0,LB_LoadSize(a0)	;wieviel wurde wirklich geladen ?
skipset$					;
		tst.l	d0			;
		bmi	PR_Fail2		;nix-> Fehler und raus aus dem Programm

		move.l	LB_LoadSize(a0),d0	;
		add.l	d0,PR_Bytesread(A4)	;Anzahl der gelesenen Bytes updaten

		move.l	PR_Bytesread(A4),d0	;
		cmp.l	I8SVX_BodySize(A4),d0	;alle Bytes gelesen ?
		blt.s	ok$

		push	a0			;
		move.l	I8SVXP_File1(a4),d1	;Filezeiger auf Start setzen
		move.l	I8SVX_BodyOffset1(a4),d2;
		moveq	#OFFSET_BEGINNING,d3	;
		move.l	MyDosBase(A4),a6		;
		jsr	_LVOSeek(A6)		;
		clr.l	PR_Bytesread(A4)	;
						;
		move.l	I8SVXP_File2(a4),d1	;Filezeiger auf Start setzen
		beq.s	nofile2_1$		;
		move.l	I8SVX_BodyOffset2(a4),d2;
		moveq	#OFFSET_BEGINNING,d3	;
		jsr	_LVOSeek(A6)		;
nofile2_1$					;
		pop	a0			;

		st	LB_LastBuffer(a0)	;ok, also ist der aktuelle auch der letzte Puffer,
						;der gelesen wird
		st	I8SVXP_ReadyToPlay(a4)	;alle Puffer voll, Abspielen kann beginnen

		tst.b	I8SVXR_LoadOnce(A4)	;Datei nur einmal komplett lesen ?
		beq.s	ok1$			;
		st	I8SVXP_NoLoad(A4)	;Ok, also nix mehr laden
		bra.s	ok$
ok1$
		clr.w	I8SVXP_CurrentNumber(A4)	;aktuelle Nummer des Puffers rücksetzen
ok$

weiterwarten2$
		lea	I8SVX_LoadedBufList1+MaxLoadBuffers*4(A4),a1 ;letzter Puffer+1
		tst.l	-4(a1)			;letzte Position leer ? (sollte eigentlich sein)
		beq.s	onefound$		;einer ist also auf jeden Fall frei

		bsr	PR_Delay		;warten, bis letzte Position frei wird
		bne	PR_ret			;-> raus, `QUIT`-Signal erhalten

		bra	weiterwarten2$		;und wir warten geduldig auf den Playtask
						;(sehr unwahrscheinlich)
onefound$

eintragenloop$				;solange suchen, bis volle Position gefunden, sprich
		move.l	-(a1),d1	;den letzten freien Puffer spezifizieren
		beq	eintragenloop$	;die Routine sollte Deadlock- bzw. Crashsicher sein,
					;da im schlimmsten Falle plötzlich eine Position mehr 
					;frei sein kann, was aber vom AudioInt-Task abgefangen
					;wird und zu keinerlei Problemen führen kann, außer
					;daß, wenn`s dumm läuft die `weiterwarten2$`-Routine
					;doch noch zum Tragen kommen kann und das ist auch nicht
					;problembehaftet

		move.l	a0,4(a1)	;in Liste einfügen

		bra	loadnext$	;versuchen, gleich noch einen Puffer voll zu laden,
					;sonst warten

;---------------------------------- Programmende ---------------------------------------------
PR_ret
		move.l	I8SVXP_File1(a4),d1
		beq	nofile1$
		move.l	MyDosBase(A4),a6
		jsr	_LVOClose(A6)
		clr.l	I8SVXP_File1(a4)
nofile1$
		move.l	I8SVXP_File2(a4),d1
		beq	nofile2$
		move.l	MyDosBase(A4),a6
		jsr	_LVOClose(A6)
		clr.l	I8SVXP_File2(a4)
nofile2$

		moveq	#0,d0
		sf	I8SVXP_Ready(A4)

		bsr	PR_SendSignal
		rts
;------------------------------- Signalfehler ------------------------------------------------
PR_Fail1:
		moveq	#1,d0
		move.l	d0,I8SVXP_Fail(A4)
		sf	I8SVXP_Ready(A4)

		bsr	PR_SendSignal
		rts
;-------------------------------- Dateifehler ------------------------------------------------
PR_Fail2:
		move.l	I8SVXP_File1(a4),d1
		beq	nofile1$
		move.l	MyDosBase(A4),a6
		jsr	_LVOClose(A6)
		clr.l	I8SVXP_File1(a4)
nofile1$
		move.l	I8SVXP_File2(a4),d1
		beq	nofile2$
		move.l	MyDosBase(A4),a6
		jsr	_LVOClose(A6)
		clr.l	I8SVXP_File2(a4)
nofile2$


		moveq	#2,d0
		move.l	d0,I8SVXP_Fail(A4)
		sf	I8SVXP_Ready(A4)

		bsr	PR_SendSignal
		rts
;---------------------- sende Handshake-Signal an Master-Task -------------------------------
PR_SendSignal:
		push	d1-a6
		move.l	I8SVXP_SignalReturn(A4),d1	;wir sind schon abgeschrieben ?
		bmi	failsignal$			;ok, also `QUIT` !
		moveq	#0,d0
		bset	d1,d0
		move.l	I8SVXP_SigTask(A4),a1
		move.l	myExecbase(A4),a6
		jsr	Signal(a6)
		moveq	#0,d1
failsignal$	move.l	d1,d0
		pop	d1-a6
		tst.l	d0
		rts
;-- warte 1/50 Sekunde, weil kein Puffer frei war zum Lesen/Schreiben und teste auf Kill-Signal ----
;Input: -
;Output: d0 != 0 -> Programm beenden
PR_Delay:
		push	d1-a6

		move.l	MyDosBase(A4),a6
		moveq	#1,d1
		jsr	_LVODelay(A6)

		moveq	#0,d0
		moveq	#0,d1
		move.l	myExecbase(A4),a6
		jsr	SetSignal(A6)

		moveq	#-1,d7
		move.l	I8SVXP_SignalQuit(a4),d1
		btst	d1,d0
		bne	quit$

		moveq	#0,d7

quit$		move.l	d7,d0
		pop	d1-a6
		tst.l	d0
		rts

	endb	a4


PR_Name:	dc.b	'Eagleplayer IFF 8SVX Playtask',0
	ifne	test
dos		dc.b	'dos.library',0

filepath
;	dc.b	`modules:other/anrufb.vogel`,0
;	dc.b	`modules:test/verylongstereo`,0
	dc.b	'modules:test/verylongstereo.fib',0

	endc
fibnumbertab
	dc.b	-34,-21,-13,-8,-5,-3,-2,-1,0,1,2,3,5,8,13,21
exp_tab	dc.b	-128,-64,-32,-16,-8,-4,-2,-1,0,1,2,4,8,16,32,64

	even
AudioINT
	dc.l	0			;letzter Node
	dc.l	0			;nächster Node
	dc.b	2			;Node Type = Interrupt
	dc.b	1			;Priorität
	dc.l	0			;Name
	dc.l	0			;Zeiger auf Daten
	dc.l	INT_Aud			;Interrupt Routine

	Section	1,bss_p
Datas:
MyDosBase		ds.l	1
myExecbase		ds.l	1

;----------------------- Samplespezifische Daten --------------------------------
I8SVX_Convert_to_8	ds.w	1	;16 Bit -> 8 Bit ?
I8SVX_NumChannels	ds.w	1	;1 oder 2 Kanäle
I8SVX_numSampleFrames	ds.l	1	;Anzahl der SampleBlöcke
I8SVX_Sampletype	ds.w	1	;8 bit = 0, 16 Bit = 1
I8SVX_sampleRate	ds.l	1	;Samplerate in Bytes pro Sekunde
I8SVX_BytesperFrame	ds.w	1	;Bytes pro Einheit
I8SVX_BodySize		ds.l	1	;Länge der Audiodaten in Bytes
I8SVX_BodyOffset1	ds.l	1	;Offset für die Audiodaten in der Datei
I8SVX_BodyOffset2	ds.l	1	;Offset für die Audiodaten in der Datei

I8SVX_SamplePer		ds.l	1	;Period value (aus Samplerate berechnet)
I8SVX_ReadPerInt		ds.w	1	;bei 16 Bit muß nur die Hälfte aus dem Puffer gelesen werden

;-------------------------- Player- und Ladetaskdaten ---------------------------

I8SVX_LoadBuf:		ds.l	1	;Pufferadresse
I8SVX_LoadBufSize:	ds.l	1

I8SVXR_CurrentBuf	ds.l	1	;aktueller Puffer beim Abspielen

I8SVX_DummyBuf		ds.l	1	;damit die Suchroutine des Prozesses richtig arbeitet, 
					;wird hier einfach ein Wert von -1 eingetragen
I8SVX_LoadedBufList1	ds.l	MaxLoadBuffers*2;Puffer, die komplett ins Ram geladen wurden
			;Übergabe: von hinten an letzte freie Speicherzelle suchen und Daten dort reinlegen,
			;          wenn letzte Position belegt, warten und danach nochmal versuchen
			;Übernahme: erste belegte Speicherzelle suchen, rauslesen und gesamten Block
			;           nach oben scrollen
			;
			;das *2 gilt nur für das Upscrolling, nach der Anzahl von NumLoadBuffers
			;dürfen keine Daten abgelegt werden, reine Speedup Storage

I8SVX_ReadyBufList1	ds.l	MaxLoadBuffers	;Puffer, die abgespielt wurden und nun wieder vollgepackt werden können,
				;Übergabe: leere Speicherzelle suchen (min. eine ist immer frei) und Adr reinschreiben
				;Übernahme: volle Speicherzelle suchen, auslesen und danach
				;           auf 0 rücksetzen

I8SVXP_SignalLoad:	ds.l	1	;Signalbit: Lade nächsten Puffer
I8SVXP_SignalQuit:	ds.l	1	;Signalbit: Beende Prozeß
I8SVXP_SignalReturn:	ds.l	1	;Signalbit: Kommando erhalten
I8SVXP_SigTask:		ds.l	1	;Task, der das Returnsignal erhält
I8SVXP_Fail:		ds.l	1	;Flag: Prozeßfehler
I8SVXP_Address:		ds.l	1	;Adresse des Prozesses
I8SVXP_File1:		ds.l	1	;Filehandle Datei 1
I8SVXP_File2:		ds.l	1	;Filehandle Datei 2

I8SVXP_Ready:		ds.b	1	;Prozeß ist bereit
I8SVXP_NoLoad:		ds.b	1	;alles wurde genau einmal eingeladen und ist komplett im
I8SVXP_CurrentNumber:	ds.w	1	;aktuelle Nummer des Puffers
I8SVXR_FibvalueL:	ds.w	1	;FibDelta-Initialwert Kanal 1
I8SVXR_FibvalueR:	ds.w	1	;FibDelta-Initialwert Kanal 2

I8SVXR_CountBufferchg:	ds.b	1	;Zähler: alle x abgespielten Puffer werden welche neu geladen
I8SVXR_NumCountBuffer:	ds.b	1	;Anzahl: alle x abgespielten Puffer werden welche neu geladen
I8SVXR_LoadOnce:	ds.b	1	;Kram nur einmal laden, da alles in den Puffer paßt
I8SVXR_NoPlay:		ds.b	1	;Totaler Fehler beim Listensortieren, Replay anhalten
I8SVXR_WaitPlay:	ds.b	1	;kurz beim Start des Replays warten
I8SVXP_ReadyToPlay:	ds.b	1	;alle Puffer voll, Abspielen kann beginnen

			
;---------- Werte, die bereits in der Checkroutine herausgesucht werden, ------------------------
;-------- wg. Möglichkeit des Doublebufferings der Samples durch den EP zwischengespeichert -----
TMP_numChannels		ds.w	1
TMP_sampletype		ds.w	1
TMP_BytesperFrame	ds.w	1
TMP_numSampleFrames	ds.l	1
TMP_sampleRate		ds.l	1
TMP_BodyOffset1		ds.l	1
TMP_BodyOffset2		ds.l	1
TMP_BodySize		ds.l	1
TMP_FileSize		ds.l	1	;Größe des FORM Files von Offset 4 gelesen

PR_Filelock		ds.l	1	;Lock für zu ladendes File, wird vom Prozeß dupliziert
PR_Bytesread		ds.l	1	;Anzahl der vom Prozeß gelesenen Bytes

PlaybufNext		ds.l	1			;welcher Abspielpuffer gerade benutzt wird
PlaybufL		ds.b	Playbuffersize*2	;Puffer  für Daten links (double buffered)
PlaybufR		ds.b	Playbuffersize*2	;Puffer  für Daten rechts (double buffered)
Playbufend		ds.b	0
			ds.b	100

mytags:			ds.l	10*2

audstruct0		ds.b	AS_Sizeof*4	;*4 nur zur Sicherheit, man weiss ja nie bei Betas

DataSize = *-Datas

	ifne	test

		section	2,data
testsample
		incdir	modules:problem-modules/
		incbin	eav_rapunzel,30000

		incdir	modules:other/
;		incbin	anrufb.vogel,30000

		incdir	modules:test/
;		incbin	verylongstereo,30000
;		incbin	verylongstereo.fib,30000
;		incbin	brother.I8SVX,30000
;		incbin	distain.I8SVX,30000
;		incbin	HS_Plastic_fantastic.I8SVX,30000
testsize = *-testsample

;testloadbuff	ds.b	LB_StereoSizeof*MinLoadBuffers
	endc
