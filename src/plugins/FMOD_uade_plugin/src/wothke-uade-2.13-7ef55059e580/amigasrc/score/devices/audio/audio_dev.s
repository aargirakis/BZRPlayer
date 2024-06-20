****************************************************************************
**
** poor man`s audio.device implementation 
**
** see https://wiki.amigaos.net/wiki/Audio_Device
**
** reminder: the messages sent to the C side are copied, i.e. the C code
** cannot return any data directly within the message (the hack of using
** "msgptr" should be avoided due to the pitfalls caused by debug outbut
** created via put_string/put_value)!
**
** Copyright 2022, Juergen Wothke
**

audiodevice_init
	; install vectors in audio.device function table
	; each lib function entry is 6 bytes and this device has 6 functions
	
	lea	audio_device_base(pc),a6
		; the 4 generic lib funcs (all libs have these)
		
	LVO_JMP LIB_OPEN,	adopen
	LVO_JMP LIB_CLOSE,	adclose
	LVO_JMP LIB_EXPUNGE,	adexpunge
	LVO_JMP LIB_EXTFUNC,	adextfunc
	LVO_JMP LIB_EXTFUNC,	adextfunc
		
		; user funcs
	LVO_JMP DEV_BEGINIO,	adbeginio
	LVO_JMP DEV_ABORTIO,	adabortio
	rts


adclosemsg1	dc.b	'audio.device Close called but not implemented!',0
adexpungemsg1	dc.b	'audio.device Expunge called but not implemented!',0
adextfuncmsg1	dc.b	'audio.device ExtFunc called but not implemented!',0
	even
	
****************************************************************************
*
*	Open
*
*   SYNOPSIS
*  Open(ioRequest)
*  	    A1   
*
* testcase: "marble madness"
*

ad_openmsg	dc.l	UADE_AUDIO_DEV_OPEN			; input: request type
	dc.l	0									; input: IOAudio ptr
ad_openmsge

adopen	push	all	
; impl delegates all the handling to the C-code side.
; does not have a return value, i.e. respective return value of
; OpenDevice needs to be calculated from the resulting IOAudio fields..

	lea	ad_openmsg(pc),a0
	move.l	a1,4(a0)							; IOAudio ptr
	moveq	#ad_openmsge-ad_openmsg,d0
	bsr	put_message
	pull	all
	; note: status is returned in the IOAudio
	rts
	
	
adclose	push	all
	lea	adclosemsg1(pc),a0
	bsr	put_string
	pull	all
	rts
adexpunge	push	all
	lea	adexpungemsg1(pc),a0
	bsr	put_string
	pull	all
	rts
adextfunc	push	all
	lea	adextfuncmsg1(pc),a0
	bsr	put_string
	pull	all
	rts
	
	
****************************************************************************
*
*	Open
*
*   SYNOPSIS
*  BeginIO(ioRequest)
*  	       A1   
*

ad_beginiomsg	dc.l	UADE_AUDIO_DEV_BEGINIO			; input: request type
	dc.l	0											; input: IOAudio ptr
ad_beginiomsge
	even

adbeginio	push	all	
; this is an async call, except if there is an error - see io_error.
; impl delegates all the handling to the C-code side:
	lea	ad_beginiomsg(pc),a0	
	
	move.l	a1,4(a0)									; input: IOAudio ptr			
	moveq	#ad_beginiomsge-ad_beginiomsg,d0			; len of the message
	bsr	put_message
	
	pull	all
	rts
	

ad_abortiomsg	dc.l	UADE_AUDIO_DEV_ABORTIO			; input: request type
	dc.l	0											; input: IOAudio ptr
ad_abortiomsge
	even
	
adabortio	push	all
	lea	ad_abortiomsg(pc),a0	
	
	move.l	a1,4(a0)									; input: IOAudio ptr			
	moveq	#ad_abortiomsge-ad_abortiomsg,d0			; len of the message
	bsr	put_message
	pull	all
	rts


	
*******************************************************************
*
*	audio.device specific part of exec_lib`s OpenDevice 
*
*   SYNOPSIS
*  error = OpenDevice(devName, unitNumber, iORequest, flags)
*  D0                 A0       D0          A1         D1
*
* note: the previously made `push all` must be undone in this impl
*

openaudiodev
	lea	audio_device_base(pc),a6	
	move.l	a6,IO_DEVICE(a1)		; install the audio.device "base addr"

;	jsr LIB_OPEN(a6)
	bsr adopen

	
; below logic was moved into the device`s above "adopen" function
; fixme: need test-case to check if this still works
		
;	cmp	#32,IO_COMMAND(a1)	* ACMD_ALLOCATE
;	bne.b	addone	
;	tst.l	IO_LENGTH(a1)
;	beq.b	addone
;	moveq	#0,d0			* cook up a valid result
;	move.l	IO_DATA(a1),a0
;	move.b	(a0),d0			* first chan mask succeeds ;)
;	move.l	d0,IO_UNIT(a1)
;	tst.l	32(a1)		* give it a "unique" IOA_ALLOCKEY if needed
;	bne.b	addone
;	move.l	#'uniq',32(a1)	
;addone	lea	tdmsg2(pc),a0
;	bsr	put_string
		
	pull	all
	
; todo: some status checks of iORequest fields might be used to properly 
; calc the below.. or just use the IO_ERROR
;	clr.b	IO_ERROR(a1)		* no error
	moveq	#0,d0				* always report success..
	rts




; note/reminder regarding memory layout: it seems the "base address" of a library/device points to
; a Library struct but right in front of that struct is always the associated table with the function
; vectors. (the lib_NegSize/lib_PosSize attributes suggest that there might actually be more than just 
; that..)
	
	dcb.b	$24,0		* allocate 36 bytes for 6 function pointers: 4 built-in (open, close, etc) and 2 lib-specific (BeginIO, AbortIO)
audio_device_base
;	dcb.b	LIB_SIZE,0 * just allocate enough space for the Library struct
	dcb.b	$22,0		* same as LIB_SIZE.. problem: "audio.device" might be using/expecting extended struct with added trailing fields?
