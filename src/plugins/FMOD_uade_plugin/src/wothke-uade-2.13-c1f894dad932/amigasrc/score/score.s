;               T
*
* UADE sound core
* Copyright: Heikki Orsila <heikki.orsila@iki.fi>
*
* with extensions
* Copyright: Juergen Wothke
*
* License: GNU LGPL
* (relicensing is very possible for open source development reasons)

; ps: I merged in some changes from the UADE3 version of this file to potentially
; ease the back-merging of my additions...

; FIXME: having this long flat file is soo annoying/unreadable.. and there are soo many
;        aspects that would need cleaning up: breaking it up into more managable chunks,
;        consistently using ";" instead of "*" for comments (for nicer display in my editor)
;        etc). But for now I still try to keep as much as possible untouched to ease
;        potential merges of changes from UADE's main repository.

; FIXME: UADE's old loadseg impl should be completely ditched/cleaned up (see relocator)

; FIXME: UADE's broken "Supervisor" impl should be replaced


	include	custom.i
	include	exec/exec_lib.i
	include	graphics_lib.i
	include	dos/dos_lib.i
	include	dos/dos.i
	include	dos/dosextens.i
	include	icon_lib.i
	include	rmacros.i

	include	np.i

	include misc/DeliPlayer.i
	include misc/EaglePlayer.i
	include	resources/cia.i
	include	lvo3.0/cia_lib.i
	include	devices/timer.i
	include	devices/audio/audio.i

	include	exec/libraries.i
	include	exec/lists.i
	include	exec/tasks.i
	include "exec/resident.i"
	include "exec/execbase.i"
	include "exec/memory.i"
	include "graphics/gfxbase.i"

; caution: not currently supported in webUADE!
UADE_TEST_LOGGING SET 0

; The multi-tasking implementation activated by the below USE_TEST_ONLY_CODE switch contains
; reverse enigneered parts copyrighted by Commodore and it CANNOT be licensed as GPL!
; It is added here as test-only code, i.e. as a reference illustrating what functionality a
; clean reimplementation will eventually have to provide. This impl is already good enough
; for some usecases but is still is flawed: UADE's Supervisor implementation is still garbage,
; i.e. it doesn't create the correct stack content expected by the "correct" Dispatch logic.

USE_TEST_ONLY_CODE	SET	1

; The new alternative implemention seems to work correctly where UADE's original one doesn't,
; e.g. the original doesn't correctly report the file size and it may have other flaws (after some
; more testing the old impl might be trashed completely)

USE_ALT_LOADSEQ	SET	1

	IFNE	USE_ALT_LOADSEQ
	include	"exec/types.i"
	include "exec/funcdef.i"
	include	"dos/doshunks.i"
	ENDC

; Immediately crash on unimplemented function table entries (instead of showing a
; warning message) .. makes it easier to identity the used offset.

CRASH_ON_UNIMPLEMENTED	SET	0


; set a library's LVO entry (A6 must contain baseaddress)
LVO_JMP     MACRO
	lea	\2(pc),a0
	move.w #$4ef9,\1(a6)
	move.l	a0,\1+2(a6)
	ENDM

; make a object's field point to the specified memory location (A6 must contain object ptr)
SET_PTR     MACRO
	lea	\2(pc),a0
	move.l	a0,\1(a6)
	ENDM

; like LVO_JMP but additionally sets the same function poiner in the library's
; specified field (see EPG_/dtp_ function pointers in Eagleplayer library).
LIB_JMP     MACRO
	lea	\3(pc),a0
	move.l	a0,\2(a6)
	move.w #$4ef9,\1(a6)
	move.l	a0,\1+2(a6)
	ENDM


USE_DBGOUT	SET	0

; ISSUE: put_string & put_value are sending AMIGAMSG_GENERALMSG to the C side thereby
; destroying the "msgptr" of a previous message that may still be in use; see
; "fileload" whose access to "length" had been originally implemented via "msgptr",
; i.e. these APIs are therefore unsuitable/dangerous for adding some quick debug output..

;PRINTSTR     MACRO
;	IFNE	USE_DBGOUT
;	push a0
;	lea	\1(pc),a0
;	bsr	put_string
;	pull a0
;	ENDC
;	ENDM

;PRINTVAL     MACRO
;	IFNE	USE_DBGOUT
;	push d0
;	move.l \1,d0
;	bsr put_value
;	pull d0
;	ENDC
;	ENDM


* reminder:  below numbers MUST correspond to the enum in src/include/amigamsg.h!
AMIGAMSG_SETSUBSONG	equ	1
AMIGAMSG_SONG_END	equ	2
AMIGAMSG_PLAYERNAME	equ	3
AMIGAMSG_MODULENAME	equ	4
AMIGAMSG_SUBSINFO	equ	5
AMIGAMSG_CHECKERROR	equ	6
AMIGAMSG_SCORECRASH	equ	7
AMIGAMSG_SCOREDEAD	equ	8
AMIGAMSG_GENERALMSG	equ	9
AMIGAMSG_NTSC	equ	10
AMIGAMSG_FORMATNAME	equ	11
AMIGAMSG_LOADFILE	equ	12
AMIGAMSG_READ	equ	13
AMIGAMSG_FILESIZE	equ	14
AMIGAMSG_TIME_CRITICAL	equ	15
AMIGAMSG_GET_INFO	equ	16
AMIGAMSG_START_OUTPUT	equ	17
AMIGAMSG_ICON_LOAD	equ	18
AMIGAMSG_ICON_TOOLTYPE	equ	19
AMIGAMSG_TEST_LOGGING	equ	21

	; audio.device functions
AMIGAMSG_AUDIO_DEV_OPEN	equ	50
AMIGAMSG_AUDIO_DEV_BEGINIO	equ	51
AMIGAMSG_AUDIO_DEV_ABORTIO	equ	52


EXECBASE	equ	$0D00
EXECENTRIES	equ	210

DOSCENTRIES	equ	200		; documented range ends at 166

TRAP_VECTOR_0	equ	$80
TRAP_VECTOR_3	equ	$8c	* exec_cause uses this for software interrupt
TRAP_VECTOR_4	equ	$90	* play loop uses this for vbi sync
TRAP_VECTOR_5	equ	$94	* output message trap
TRAP_VECTOR_6	equ	$98	* bin trap

* Base address of the message in memory that is sent to uade.c
* uadecore_get_amiga_message(). A message to emulator is sent with:
*
*     moveq #msgtype,d0
*     bsr put_message_by_value
*
* To add a new trap on the emulator side, add a new enum to
* src/include/amigamsg.h. Then use that enum in
* uade.c:uadecore_get_amiga_message() switch case. The function gets the
* newly defined message type from the base address defined here.
UADECORE_INPUT_MSG	equ	$200
UADECORE_INPUT_MSG_END	equ	$300

UADECORE_DBGINPUT_MSG	equ	$600
UADECORE_DBGINPUT_MSG_END	equ	$700

* uade.library interface
UadeTimeCritical	equ	-6
UadePutString		equ	-12
UadeGetInfo		equ	-18
UadeNewGetInfo		equ	-24

EP_OPT_SIZE		equ	256

* $100	mod address			<-- directly poked here from uade.c
* $104	mod length
* $108	player address
* $10c	reloc address (the player)
* $110	user stack
* $114	super stack
* $118	force by default
* $11C	setsubsong flag (only meaningful on amiga reboot)
* $120	subsong
* $124	ntsc flag	0 = pal, 1 = ntsc
* $128	loaded module name ptr
* $12C	song end bit
* $180	postpause flag
* $184	prepause flag
* $188	delimon flag
* $18C	Exec debug flag (enabled if $18c == 0x12345678)
* $190	unused, used to be volume test flag (enabled if $190 == 0x12345678)
* $194	unused, used to be dma wait constant (number of raster lines + 0x12340000)
* $198	disable EP_ModuleChange

* $1fc  ReplyMsg list filled from C side DMA handling
* $200	output message flag + output message

* $300	input message flag + input message

* $400	module (player) name path

* $600	debug prints: output message flag + output message
* $0800 -
* $0D00	exec base (-6*EXECENTRIES == -6 * 210)
* $0d00-$0f7a note: exec_lib here uses SYSBASESIZE (634 bytes) bytes for Library structure (see execbase.i)
* TODO: Log okprog and default handler offsets

LOG_CALL	macro
	ifne UADE_TEST_LOGGING
	bra.b	.labelskipped\@
.label\@
	dc.b	\1
	dc.b	0
	even
.labelskipped\@
	push	a0-a1
	lea	.label\@(pc),a0
	move.l	msgptr(pc),a1
	move.l	#AMIGAMSG_TEST_LOGGING,(a1)+
	tst.b	(a0)
	bne.b	.stringmsgloop\@
	illegal
.stringmsgloop\@
	move.b	(a0)+,(a1)+
	bne.b	.stringmsgloop\@
	cmp.l	#UADECORE_INPUT_MSG_END,a1
	bls.b	.no_overflow\@
	illegal
.no_overflow\@
	trap	#5
	pull	a0-a1
	endif
	endm


	section	uadesoundcore,code_c

* The org statement acts as a memory poison to catch absolute addressing.
* We need pc relative addressing.
	org	$00fe0000

start	* set super stack and user stack
	move.l	$114,a7
	move.l	#'WALL',(a7)	/* supervisor stack guard */
	move.l	$110,a0
	move.l	#'UALL',(a0)	/* user stack guard */
	move.l	a0,usp

	move	#$7fff,intena+custom
	move	#$7fff,intreq+custom
	move	#$7fff,dmacon+custom
	move.b	#3,$bfe201
	move.b	#2,$bfe001		* filter off

	move.b	#$1f,$bfdd00		* ciab ICR (disable all timers)
	move.b	#$00,$bfef01		* TOD counter rolling on vsync
	move.b	#$00,$bfe801		* order of these is important
	move.b	#$00,$bfdf00		* TOD counter rolling on hsync
	move.b	#$00,$bfd800		* order of these is important

	move	#$200,bplcon0+custom
	move	#$00ff,adkcon+custom

	* patch exception vectors
	move	#8,a0
	lea	default_exception(pc),a1
exloop	move.l	a1,(a0)+
	cmp.l	#$100,a0
	bne.b	exloop

	lea	division_by_zero(pc),a1
	move.l	a1,$14.w

	move	#0,sr			* switch to user level

	lea	zero_sample(pc),a0	* set zero sample
	lea	custom,a2
	moveq	#4-1,d7
zero_sample_l
	clr	aud0vol(a2)
	move.l	a0,aud0lch(a2)
	move	#1,aud0len(a2)
	move	#150,aud0per(a2)
	add	#$10,a2
	dbf	d7,zero_sample_l

	lea	test_copperlist(pc),a0	* set blank copper list
	move.l	a0,cop1lch+custom
	move	d0,copjmp1+custom
	move	#$8280,dmacon+custom

	move	#$c000,intena+custom

	* initialize exec base with failures
	lea	EXECBASE,a0
	move.l	a0,4.w
	lea	exec_unimplemented_call(pc),a2
	move	#EXECENTRIES-1,d7
.exec_jump_table_loop
	subq.l	#6,a0
	move	jmpcom(pc),(a0)
	IFNE	CRASH_ON_UNIMPLEMENTED
	move.w	#$4afc,(a0)			; illegal instr directly in table to identify offset more easily
	ENDC
	move.l	a2,2(a0)
	dbf	d7,.exec_jump_table_loop

		; initialize exec Library struct
	lea	EXECBASE,a0
	push d0/a1
	move.l a0,a1
    move.w  #SYSBASESIZE-1,d0
clearexec
	clr.b    (a1)+
	dbra d0,clearexec

; more stuff is probably needed here
	move.b NT_LIBRARY,LN_TYPE(a0)

	lea MemList(a0),a2		; testcase: PlayAY/ZXAYEMUL
	NEWLIST a2
	pull d0/a1

	* initialize dosbase with failures
	lea	dos_lib_base(pc),a0
	lea	dos_unimplemented_call(pc),a2
	move	#DOSCENTRIES-1,d7
dosdfdf	subq.l	#6,a0
	move	jmpcom(pc),(a0)
	IFNE	CRASH_ON_UNIMPLEMENTED
	move.w	#$4afc,(a0)			; illegal instr directly in table to identify used offset more easily
	ENDC
	move.l	a2,2(a0)
	dbf	d7,dosdfdf

	move.l	4.w,a6
	LVO_JMP SetIntVector,	exec_set_int_vector
	LVO_JMP AddIntServer,	exec_add_int_server
	LVO_JMP AllocMem,		exec_alloc_mem
	LVO_JMP FreeMem,		myfreemem
	LVO_JMP OpenResource,	exec_open_resource
	LVO_JMP OpenDevice,		exec_open_device
	LVO_JMP DoIO,			exec_doio
	LVO_JMP SendIO,			exec_sendio
	LVO_JMP WaitIO,			exec_waitio
	LVO_JMP AbortIO,		myabortio
	LVO_JMP GetMsg,			exec_getmsg
	LVO_JMP Cause,			exec_cause
	LVO_JMP OldOpenLibrary,	exec_old_open_library
	LVO_JMP OpenLibrary,	exec_open_library
	LVO_JMP CloseLibrary,	exec_close_library
	LVO_JMP TypeOfMem,		exec_typeofmem
	LVO_JMP Supervisor,		exec_supervisor
	LVO_JMP SuperState,		exec_super_state
	LVO_JMP UserState,		exec_userstate

	; additional functions used by some audio.device related songs
	LVO_JMP AddHead,		exec_addhead
	LVO_JMP CheckIO,		exec_checkio
	LVO_JMP AddPort,		exec_addport
	LVO_JMP Enqueue,		exec_enqueue
	LVO_JMP FindName,		exec_findname
	LVO_JMP CopyMem,		exec_copymem
	LVO_JMP ReplyMsg,		exec_replymsg
	LVO_JMP PutMsg,			exec_putmsg
	LVO_JMP Remove,			exec_remove
	LVO_JMP InitStruct,		exec_initstruct
	LVO_JMP MakeFunctions,	exec_makefunctions
	LVO_JMP MakeLibrary,	exec_makelibrary
	LVO_JMP AllocAbs,		exec_allocabs

	LVO_JMP InitSemaphore,		exec_initsem
	LVO_JMP AttemptSemaphore,	exec_attemptsem
	LVO_JMP ReleaseSemaphore,	exec_releasesem


	move.b	#$ff,$126(a6)	* fuck med player
	move	#$0003,$128(a6)	* execbase processor flags to 68020+ (AFB_68010|AFB_68020 - NO AFB_68881!)

	IFNE	USE_TEST_ONLY_CODE
	bsr multitasking_init
	ENDC

	bsr audiodevice_init

	lea	dos_lib_base(pc),a6
	LVO_JMP _LVOLoadSeg,		dos_loadseg
	LVO_JMP _LVOOpen,			dos_open
	LVO_JMP _LVOSeek,			dos_seek
	LVO_JMP _LVORead,			dos_read
	LVO_JMP _LVOClose,			dos_close
	LVO_JMP _LVOCurrentDir,		dos_currentdir
;	LVO_JMP _LVOLock,			dos_lock
	LVO_JMP _LVOUnLoadSeg,		dos_unloadseg
	bsr dos_lib_addons

	lea	uade_lib_base(pc),a6
	LVO_JMP UadeTimeCritical,		uade_time_critical
	LVO_JMP UadePutString,			put_string
	LVO_JMP UadeGetInfo,			uade_get_info
	LVO_JMP UadeNewGetInfo,			uade_new_get_info

	move.l	4.w,a6
	lea	rtsprog(pc),a1
	lea	harmlesslist(pc),a0
harmlessloop	move	(a0)+,d0
	beq.b	endharmlessloop
	move	jmpcom(pc),(a6,d0)
	move.l	a1,2(a6,d0)
	bra.b	harmlessloop
endharmlessloop
	lea	dos_lib_base(pc),a6
	lea	rtsnonzeroprog(pc),a1
	lea	harmlessdoslist(pc),a0
harmlessdosloop	move	(a0)+,d0
	beq.b	endharmlessdosloop
	move	jmpcom(pc),(a6,d0)
	move.l	a1,2(a6,d0)
	bra.b	harmlessdosloop
endharmlessdosloop
	bra	contplayer

rtsprog	rts
rtsnonzeroprog	moveq	#-1,d0
	rts

* exec library harmless function list (just do rts)
harmlesslist	dc	CacheClearU,Enable,Disable,0			; StoneTracker uses Disable.. maybe a proper impl is needed?
harmlessdoslist	dc	_LVOUnLock,0		; for debugging it is more useful to have feedback - therefore do not silently ignore UnLoadSeg

zero_sample	dc	0

test_copperlist	dc.l	$01000200,-2

doslibbasemsg	dc.b	'dos_lib baseaddr:',0
exec_error_msg	dc.b	'unimplemented exec function: return address:',0
	even

exec_unimplemented_call	push	all
	lea	exec_error_msg(pc),a0
	bsr	put_string
	move.l	60(a7),d0
	bsr	put_value
	pull	all
	illegal

* TODO: A more comprehensive exception vector offset notification system.
*       Currently it is easy to detect illegal instruction and division by
*       zero, but not all of them.
division_by_zero
	lea	.division_by_zero_msg(pc),a0
	bsr	put_string
	bra	default_exception
.division_by_zero_msg	dc.b	'Exception: Division by zero',0
	even

epg_textrequest			; testcase: RonKlaren player
	move.l #0,d0
	rts

epgloadmsg dc.b 'fatal use of EPG_LoadExecutable: ',0
	even
epg_loadexecutable movem.l d1-a6,-(a7)
	; testcase: RonKlaren player
	lea epgloadmsg(pc),a0
	bsr put_string

	lea	eaglebase(pc),a5
	move.l EPG_ARG1(a5),a0
	bsr put_string

	; no point to actually return the correct result (yet)
	; since player still hangs.. fix would need additional work

;	lea moduleptr(pc),a0
;	move.l (a0),d0
;	asr.l	#2,d0
;	subq.l	#1,d0
;	move.l	d0,EPG_ARG1(a5)
;	movem.l	(a7)+,d1-a6
;	moveq	#0,d0
;	rts
	illegal

*****************************************************************************************************
*
*  InitStruct(initTable, memory, size);
*               a1         A2      d0
*
* testcase: stonepacker.library (verified that all 6 entries of initTable are processed)
*
exec_initstruct
	move.l    a2,a0
	lsr.w     #1,d0
	bra       it1
initclrloop
	clr.w     (a0)+
it1
	dbra      d0,initclrloop

	move.l    a2,a0
initloop
	clr.w     d0
	move.b    (a1)+,d0

	beq       initdone
	bclr      #7,d0
	beq       it2

	; offset command (10ssnnnn or 11ssnnnn).

	bclr      #6,d0
	beq       it3
	subq.l    #1,a1					; back to start
	move.l    (a1)+,d1
	and.l     #$00ffffff,d1			; clear tag
	bra       it4

	; offset/byte command (10ssnnnn).
it3
	moveq     #0,d1
	move.b    (a1)+,d1

it4
	move.l    a2,a0
	add.l     d1,a0
it2
	move.w    d0,d1

	lsr.w     #3,d1
	and.w     #$0e,d1

	and.w     #$0f,d0              ; get (nnnn) of command

	move.w    isswitchtab(pc,d1.w),d1	; get case offset relative to isswitchtab
	jmp       isswitchtab(pc,d1.w)

init_copybyte2
	move.b    (a1)+,(a0)+
	dbra      d0,init_copybyte2
	move.l    a1,d0
	addq.l    #1,d0
	bclr      #0,d0
	move.l    d0,a1
	bra       initloop

init_copylong2
	add.w     d0,d0
	addq.w    #1,d0

init_copyword2
	move.l    a1,d1
	addq.l    #1,d1
	and.b     #$fe,d1
	move.l    d1,a1
initcpyloop
	move.w    (a1)+,(a0)+
	dbra      d0,initcpyloop
	bra       initloop
initdone
	rts

isswitchtab		; 8 offsets for the different copy modes; see labels used below
	dc.w	  init_copylong2-isswitchtab,init_copyword2-isswitchtab,init_copybyte2-isswitchtab,init_invalidstruct-isswitchtab,init_copylongmulti-isswitchtab,init_copywordmulti-isswitchtab,init_copybytemulti-isswitchtab,init_invalidstruct-isswitchtab,0

	; copy one byte several times.
init_copybytemulti
	move.b    (a1)+,d1  .
loop_is1
	move.b    d1,(a0)+
	dbra      d0,loop_is1
	bra       initloop

	; invalid commands  .
init_invalidstruct
	movem.l   d7/a5/a6,-(sp)
	move.l    #$81000007,d7
	move.l    4.w,a6
	jsr       Alert(a6)
	illegal

	; copy one longword several times
init_copylongmulti
	move.l    a1,d1
	addq.l    #1,d1
	and.b     #$fe,d1
	move.l    d1,a1
	move.l    (a1)+,d1
loop_is2
	move.l    d1,(a0)+
	dbra      d0,loop_is2
	bra       initloop

	; copy one word several times
init_copywordmulti
	move.l    a1,d1
	addq.l    #1,d1
	and.b     #$fe,d1
	move.l    d1,a1
	move.w    (a1)+,d1
loop_is3
	move.w    d1,(a0)+
	dbra      d0,loop_is3
	bra       initloop


*****************************************************************************************************
*
*  size = MakeFunctions( vectors, offset )
*  d0                    a1       a2
*
* testcase: stonepacker.library
*
exec_makefunctions
	move.l    a3,-(sp)
	moveq     #0,d0
	move.l    a2,d1
	beq       .absolute_table_loop
.relative_table_loop
	move.w    (a1)+,d1
	cmp.w     #$ffff,d1
	beq       mfend
	lea       0(a2,d1.w),a3    ; rel to abs address
	move.l    a3,-(a0)         ; put addr
	move.w    #$4ef9,-(a0)     ; put "JMP" instr
	addq.l    #6,d0
	bra       .relative_table_loop
.absolute_table_loop
	move.l    (a1)+,d1
	cmp.l     #$ffffffff,d1    ; end marker
	beq       mfend
	move.l    d1,-(a0)         ; put addr
	move.w    #$4ef9,-(a0)     ; put "JMP" instr
	addq.l    #6,d0
	bra       .absolute_table_loop
mfend
	move.l    (sp)+,a3
	rts


*****************************************************************************************************
*
*  library = MakeLibrary( vectors, structure, init, dataSize, segList )
*  d0                     a0       a1         a2    d0        d1
*
* testcase: stonepacker.library
*
exec_makelibrary
	movem.l	d2-d7/a2/a3,-(sp)
	move.l	d0,d2            ; size of library data area
	move.l	a0,d4            ; pointer to library vector table
	move.l	a1,d5            ; pointer to initialization structure
	move.l	a2,d6            ; InitLib function to call
	move.l	d1,d7            ; loaded library file SegList
	move.l	a0,d3            ; pointer to function vector table
	beq.b	.skip_func_vector_table

	move.l	a0,a3
	moveq	#-1,d3
	move.l	d3,d0
	cmp.w	(a3),d0
	bne		.abs_vectors_loop
	addq.l	#2,a3			; skip flag for relative vectors

.rel_vectors_loop
	cmp.w	(a3)+,d0
	dbeq	d3,.rel_vectors_loop
	bra		.vectors_done
.abs_vectors_loop
	cmp.l	(a3)+,d0
	dbeq	d3,.abs_vectors_loop
.vectors_done
	not.w	d3
	mulu	#6,d3

.skip_func_vector_table
	move.l	d2,d0
	add.l	d3,d0            ; total size of lib (vectors + struct)
	move.l	#$010001,d1      ; MEMF_PUBLIC | MEMF_CLEAR
	jsr		AllocMem(a6)
	tst.l	d0
	bne.b	.mem_alloc_ok
	moveq	#0,d0            ; error no mem
	bra		.make_lib_end

.mem_alloc_ok
	move.l	d0,a3            ; Library base addr
	add.l	d3,a3
	move.w	d2,LIB_POSSIZE(a3)
	move.w	d3,LIB_NEGSIZE(a3)
	move.l	a3,a0
	sub.l	a2,a2
	move.l	d4,a1
	cmp.w	#$ffff,(a1)
	bne		.abs_vectors
	addq.l	#2,a1				; skip flag for relative vectors
	move.l	d4,a2

.abs_vectors
	bsr		exec_makefunctions	; add some error check/handling?

	tst.l	d5
	beq		.skip_init_struct

	move.l	a3,a2
	move.l	d5,a1
	moveq	#0,d0
	jsr	InitStruct(a6)
.skip_init_struct
	move.l	a3,d0            ; Library base addr
	tst.l	d6
	beq.b	.make_lib_end

	move.l	d6,a1
	move.l	d7,a0
	jsr		(a1)             ; call InitLib
.make_lib_end
	movem.l	(sp)+,d2-d7/a2/a3
	rts

	; this might be available in some std include - but I did not see it
 STRUCTURE INITTABLE,0
    ULONG ITAB_DATASIZE			; size of the Library struct + proprietary add-ons
    APTR  ITAB_FUNCTABLE		; pointer function table (long word entries) - ascending
    APTR  ITAB_DATAINIT			; pointer to data-init-instructions table (0 terminated)
    APTR  ITAB_INITFUNC			; lib initialization function
    LABEL ITAB_SIZE

**********************************************************************************************
*
* A documentation of the format of shared library files seems to be something that is hard to
* come by (I did not find it). It seems as if the development approach at the time might have
* been: "you need no docs, here, copy/paste this example as a template and that will compile
* to a library..". Looking at examples and comparing them to actual binaries suggests that
* the format is actually rather simple.
*
* First there is the Amiga's hunk-file format to consider. It is used for shared libraries.
* Loading respective files via LoadSeg (which handles relocation of the code according to the
* actually used memory address) produces a linked list of so called SegList structures, each
* of these elements corresponds to one segment from the loaded file (e.g. CODE, BSS, etc). It seems
* that library files start with a CODE segment followed by an optional BSS segment (there might be
* more complicated variations of this). Each SegList starts with a 2 long word header (segment
* length + "next segment" ptr) and the actual segment data after that. One particularity of
* the Amiga APIs is that the pointers used in this context are BCPL based, i.e. they do not
* "count" bytes but long words, i.e. a BPTR*4 = APTR. To make things more "interesting" in the
* case of SegList structs, the pointers do NOT point to the beginning of the struct but instead
* point to the "+1 longword" offset, i.e. the field with the "next segment ptr" - this was probably
* deemed an ingenious performance optimization at the time.
*
* So the actual library stuff can be found in the data part of the loaded CODE segment. And
* the format of that data is this:
*
* 1) 4+ bytes:	usually a mini m68k program that returns -1 (to signal that this is not an executable;
*               other libs may write some log - see stoneplayer.library)
*	 n bytes:	followed by an amount of "arbitrary" bytes
*
* 2) 22 bytes:	a Resident struct (see exec/resident.i / exec/resident.h) - the beginning of this
*               struct must be detected by scanning for the "match word" $4afc. It is unclear
*               if there are any alignment requirements for this dumbshit garbage.
*
* then there are 4 additional structures/pointers that are directly/indirectly linked via the rt_Init
* pointer in the Resident struct. (They are referred to by respective pointers and their
* position within the file is therefore probably not hard coded.)
*
* 3) 12 bytes: INITTABLE (see definition above): gives the length (in bytes) needed to allocate
*    the library (excluding its function-ptr list) and points to the remaining relevant elements:
* 4) a variable length list of function pointers, with a -1 end marker
* 5) a variable length DATAINIT list, that uses a "tag" based config - as used by the InitStuct
*    function. It is used to initialize the allocated Library struct.
* 6) a InitLib function which is called as a final step of the library instantiation
*
* once equipped with this knowledge, the impl gets rather trivial
*
mklibwarn	dc.b	'error: MakeLibrary failed',0
	even

genericlibload
	bsr	send_open_lib_msg

	push a1
	move.l	a0,d1
	bsr dos_loadseg	; returns BPTR to SegList in d0 and d1
	pull a1

	move.l d0,d5		; segment BPRT

	; call lib's init code (easier done here than on C side)
	move.l d5,d4		; calc start of lib-file data:
	lsl.l #2,d4			; BPTR -> APTR
	addi.l #4,d4		; skip "next" segment field (e.g. stonepacker.library has only 1 seg and this is 0 then)
	addi.l #4,d4		; skip "start" prog	=> if there is no additional garbage then the Resident struct start here

searchresident
	move.l d4,a1
	cmp.w #$4afc,(a1)
	beq foundresident
	addi #1,d4
	bra searchresident
foundresident

	; verified: new loadseg impl loaded the lib data and offsets are correct for
	; using the below RT attributes

	move.l RT_INIT(a1),a3	; start of INITTABLE

	; input for MakeLibrary call
	move.l ITAB_FUNCTABLE(a3),a0	; a0 function vectors
	move.l ITAB_DATAINIT(a3),a1		; a1 data initialization list
	move.l ITAB_INITFUNC(a3),a2		; a2 InitLib function
	move.l ITAB_DATASIZE(a3),d0		; d0 size of lib's data area (Library struct etc)
									; d1 already points to SegList

	; FIXME todo: test if MakeLibrary works correctly..
	jsr MakeLibrary(a6)

	tst.l d0
	bne.b mklibdone

	push a0
	lea mklibwarn(pc),a0
	bsr put_string
	pull a0

mklibdone
;	bsr put_value	; print lib address

	move.l d0,a0	; a0 still used by caller..
	rts


********************************************************************
*
*	CloseLibrary(library)
*		     A1
*
;closelibmsg dc.b 'warning: CloseLibrary',0
exec_close_library
;	push a0
;	lea closelibmsg(pc),a0
;	bsr put_string
;	move.l LIB_IDSTRING(a1),a0
;	bsr put_string
;	pull a0
	rts

; hack to make PlayAY's memory allocation happy (why is that fucking shit player
; accessing private system vars anyway?!).. fixme: use for PlayAY only
allocmemhead
	push d1-d6/a0-a6
	move.l	4.w,a6

	move.l d2,d3

	; alloc memheader
	move.l    #MH_SIZE,d0
	move.l    #$010001,d1      ; MEMF_PUBLIC | MEMF_CLEAR
	jsr      AllocMem(a6)
	move.l d0,a1				; the header

	; actual block the above MemHeader will manage  (should be 8-byte aligned)
	add.l     #MC_SIZE,d2		; extra space for header
	add.l     #8,d2				; extra space 8-byte align
	add.l     #$10000,d2		; extra space in front/back of (officially available space)

	move.l    d2,d0
	move.l    #$010001,d1		; MEMF_PUBLIC | MEMF_CLEAR
	jsr       AllocMem(a6)

	add.l     #$10000,d0		; hack: we know we have that reserve space
	move.l    d0,a0				; the memchunk

	; link the shit together
	move.l NT_MEMORY,LN_TYPE(a1)


	move.l d0,MH_FIRST(a1)		; memchunk ptr

	add.l    #MC_SIZE,d0		; 8-byte aligned buffer start
	add.l    #8,d0
	and.l #$fffffff8,d0

	move.l d0,MH_LOWER(a1)		; lower memory bound

	add.l d3,d0
	add.l #1,d0
	move.l d0,MH_UPPER(a1)		; upper memory bound +1

	move.l d3,MH_FREE(a1)

	move.w #$3,MH_ATTRIBUTES(a1)	; MEMF_PUBLIC | MEMF_CHIP

	move.l a1,d0
	pull d1-d6/a0-a6
	rts

alloc_dummy
	push all

	; register free mem blocks where PlayAY will be looking for them

	move.l    #$501e,d2		; PlayAY wants: emullen
	bsr allocmemhead
	move.l d0,a1
	lea	MemList(a6),a0
	ADDHEAD

	move.l    #$10010,d2		; PlayAY wants: zxsize (64k+)
	bsr allocmemhead
	move.l d0,a1

	lea	MemList(a6),a0
	ADDHEAD

	pull all
	rts


; testcase: StoneTracker (stoneplayer.library) uses semaphore
; related APIs but is quite happy with the below dummy impls

****************************************************************************
*
*	    InitSemaphore(signalSemaphore)
*                     A0
*
exec_initsem
	rts

****************************************************************************
*
*	success = AttemptSemaphore(signalSemaphore)
*	D0			   A0
*
exec_attemptsem
	move.l #1,d0	; success: true
	rts

****************************************************************************
*
*   ReleaseSemaphore(signalSemaphore)
*                     A0
*
exec_releasesem
	rts


****************************************************************************
*
*  memoryBlock = AllocAbs(byteSize, location)
*   d0		      			 d0		 a1
*
* "Generally you can't trust the first 8 bytes of anything you AllocAbs."
* => it is amazing what kind of garbage passed as an API in those days..
*
* testcase: PlayAY/ZXAYEMUL
*
allocabserrmsg dc.b 'AllocAbs allocation error',0
allocabsmsg dc.b 'AllocAbs fake implementation',0
	even
exec_allocabs
	push d1-d6/a0

	lea allocabsmsg(pc),a0
	bsr put_string

	move.l a1,d4			; desired location
	move.l d0,d5			; required size

	lea	MemList(a6),a0
	REMHEAD					; hack - counterpart to alloc_dummy above (in order of use)
	move.l d0,a0			; the MemHeader


	; PlayAY's allocation logic seems to be a major dumbfuckery.. it checks the
	; available memblocks to then request an area via AllocAbs that is out of
	; range of the "found" block!
	; maybe the original EXEC impl has some limitations with regard to the block
	; sizes that are actually used here and PlayAY just falls flat on its face
	; since some builtin assumption just isn't generally valid?


	move.l MH_LOWER(a0),d2
	sub.l #$10000,d2 		; we know that we actually have this hidden reserve

	cmp.l d2,d4
	blt	fail

	add.l d5,d4

	move.l MH_UPPER(a0),d2
	add.l #$10000,d2 		; we know that we actually have this hidden reserve

	cmp.l d2,d4
	bge fail

	move.l d4,d0			; the crap actually fits into what we have

	pull d1-d6/a0
	rts
fail
	lea allocabserrmsg(pc),a0
	bsr put_string

	moveq #0,d0
	pull d1-d6/a0
	rts

****************************************************************************
*
*  result = CheckIO(iORequest)
*  d0               a1
*
* testcase: "Digital Sound Creations" player -> (usually combined with WaitIO)
*
checkio_msg_shown
	dc.l	0
checkio_error_msg	dc.b	'using flawed CheckIO impl',0
	even

exec_checkio	push	all
	lea	checkio_msg_shown(pc),a0
	tst	(a0)
	bne.b	skipwarning_checkio
	st	(a0)
	lea	checkio_error_msg(pc),a0
	bsr	put_string
skipwarning_checkio
	pull	all
	moveq #$0,d0		; good enough for above mentioned player
	rts

****************************************************************************
*
*  AddPort(port)
*          a1
*
* testcase: "Digital Sound Creations" player & stoneplayer.library
*
exec_addport	push	all
	; "the List for the MsgPort must be initialized.  This is automatically handled by AddPort(), and amiga.lib/CreatePort."
	lea		MP_MSGLIST(a1),a0
	NEWLIST a0
	pull	all
	; in case FindPort ever were to be implemented, then the created port
	; should also be enqueued into the respective public message port list

;	lea       $0188(A6),A0     ; Address of the public message port list
;	bsr	exec_enqueue
	rts

****************************************************************************
*
*  Enqueue( list, node )
*             a0    a1
*
* put node into list that is sorted by ascending node prio
*
exec_enqueue
	move.b    LN_PRI(a1),d1
	move.l    (a0),d0		; next node - i.e skip header
eqprionotlower
	move.l    d0,a0			; current node

	move.l    (a0),d0		; check if "next" is null.. i.e. last node in the list must be 0 here..
	beq.b     eqendlist

	cmp.b     LN_PRI(a0),d1
	ble.b     eqprionotlower
eqendlist
	; put node between existing ones
	move.l    LN_PRED(a0),d0
	move.l    a1,LN_PRED(a0)
	move.l    a0,(a1)
	move.l    d0,LN_PRED(a1)
	move.l    d0,a0
	move.l    a1,(a0)
	rts


****************************************************************************
*
*  CopyMem( source, dest, size )
*           a0      a1    d0
*
*  general purpose memory copy function found here:
*  http://aminet.net/package/util/boot/CopyMem
*
* testcase: MaxTrax player, e.g. "A-train"
*
	CNOP 0,16
exec_copymem:

	subq.l #4,d0			; size is 4 less than actual!
	bls.b smallcopy		; if size<=4 bytes
	move.l a1,d1
	btst #0,d1
	beq.b daligned2
	move.b (a0)+,(a1)+
	addq.l #1,d1
	subq.l #1,d0
daligned2:					; dest should be WORD aligned now
	btst #1,d1
	beq.b daligned4
	move.w (a0)+,(a1)+
	subq.l #2,d0
	bcs.b last2				; if size<0
daligned4:					; dest should be LONG aligned now
	cmp.l #256-4,d0		; min size for move16, less than 252 is dangerous!
	bcc.b bigmove
move4loop:
	move.l (a0)+,(a1)+
	subq.l #4,d0
	bhi.b move4loop		; if size>0
smallcopy:
	bne.b last2
move4:
	move.l (a0),(a1)
	rts

	CNOP 0,4
bigmove:
	moveq #128-4,d1
	sub.l d1,d0				; size is now 128 less than actual
	addq.l #4,d1			; d1=128=bytes to move per loop
bigmove4:
	move.l (a0)+,(a1)+
	move.l (a0)+,(a1)+
	move.l (a0)+,(a1)+
	move.l (a0)+,(a1)+
	move.l (a0)+,(a1)+
	move.l (a0)+,(a1)+
	move.l (a0)+,(a1)+
	move.l (a0)+,(a1)+
	move.l (a0)+,(a1)+
	move.l (a0)+,(a1)+
	move.l (a0)+,(a1)+
	move.l (a0)+,(a1)+
	move.l (a0)+,(a1)+
	move.l (a0)+,(a1)+
	move.l (a0)+,(a1)+
	move.l (a0)+,(a1)+
	move.l (a0)+,(a1)+
	move.l (a0)+,(a1)+
	move.l (a0)+,(a1)+
	move.l (a0)+,(a1)+
	move.l (a0)+,(a1)+
	move.l (a0)+,(a1)+
	move.l (a0)+,(a1)+
	move.l (a0)+,(a1)+
	move.l (a0)+,(a1)+
	move.l (a0)+,(a1)+
	move.l (a0)+,(a1)+
	move.l (a0)+,(a1)+
	move.l (a0)+,(a1)+
	move.l (a0)+,(a1)+
	move.l (a0)+,(a1)+
	move.l (a0)+,(a1)+
	sub.l	d1,d0				; d0=d0-128
	bcc.b	bigmove4			; if d0>=0
	add.l d1,d0				; d0=d0+128, back to positive
	subq.l #4,d0
	bcs.b last2
lloop:
	move.l (a0)+,(a1)+
	subq.l #4,d0
	bcc.b lloop
last2:
	btst #1,d0
	beq.b last1
	move.w (a0)+,(a1)+
last1:
	btst #0,d0
	bne.b move1
	rts
	CNOP 0,4
move1:
	move.b (a0),(a1)
	rts

****************************************************************************
*
*  PutMsg(port, message)
*         a0    a1
*
* testcase: "Music-X Driver"
* see https://wiki.amigaos.net/wiki/Exec_Messages_and_Ports
*
; ---------------  "just for debugging" start ------------------------
putmsg dc.b "PutMsg called",0
sputmsg dc.b "PutMsg Signal",0
iputmsg dc.b "PutMsg Cause",0
	even

dbgputmsg
	push a0
	move.l		MP_SIGTASK(a0),d0
	lea putmsg(pc),a0
	bsr put_string
	bsr put_value
	pull a0
	rts
; ---------------  "just for debugging" end ------------------------


putmsgerror dc.b "PutMsg illegal input error",0
	even

exec_putmsg
;	bsr dbgputmsg

	move.b		#NT_MESSAGE,d0
pm_begin
	move.w		#$4000,$dff09a		; disable interrupts (Master interrupt)
	addq.b		#1,IDNestCnt(a6)

	move.b		d0,LN_TYPE(a1)

	push a0
	lea			MP_MSGLIST(a0),a0
	ADDTAIL
	pull a0

	move.b		MP_FLAGS(a0),d0
	and.l		#PF_ACTION,d0
	beq.b		pm_pasignal

	cmp.b		#PA_SOFTINT,d0
	beq.b		pm_pasoftint

	cmp.b		#PA_IGNORE,d0
	beq.b		pm_ignore

	lea			putmsgerror(pc),a0
	bsr			put_string
	bra.b		pm_ignore

pm_pasignal
	move.l		MP_SIGTASK(a0),d1
	beq.b		pm_ignore
	move.b		MP_SIGBIT(a0),d0

	move.l		d1,a1
	moveq		#0,d1
	bset		d0,d1
	move.l		d1,d0
	jsr			Signal(a6)

;	lea sputmsg(pc),a0
;	bsr put_string

	bra.b		pm_ignore

pm_pasoftint
	move.l		MP_SIGTASK(a0),d0
	beq.b		pm_ignore

	move.l		d0,a1
	jsr			Cause(a6)

;	lea iputmsg(pc),a0
;	bsr put_string

pm_ignore
	subq.b		#1,IDNestCnt(a6)
	bge.b		pm_skipenable
	move.w		#$C000,$dff09a		; enable interrupts (set Master interrupt)
pm_skipenable
	rts


****************************************************************************
*
*  ReplyMsg(message)
*           a1
*
* testcase: MaxTrax player, e.g. "A-train"
*
exec_replymsg
	move.l		MN_REPLYPORT(a1),d0
	beq			rm_noport
	move.l		d0,a0
	move.b		#NT_REPLYMSG,d0

	bra			pm_begin
rm_noport
	move.b		#NT_FREEMSG,LN_TYPE(a1)
	rts


****************************************************************************
*		Remove(node)
*	       a1
*
* testcase: used by Stonetracker/stonepacker.library
*
exec_remove
	push a0-a1
	REMOVE
	pull a0-a1
	rts


****************************************************************************
*
*  message = GetMsg(port)
*  d0               a0
*
* testcase: MaxTrax player, e.g. "A-train"
*
exec_getmsg
	move.w    #$4000,$dff09a		; disable interrupts
	addq.b    #1,IDNestCnt(a6)

	moveq #$0,d0

	lea		MP_MSGLIST(a0),a0

	IFEMPTY a0,listcont2				; might already be covered by REMHEAD
	REMHEAD								; in: a0=list.. out: result in d0
listcont2

	subq.b    #1,IDNestCnt(a6)  	; enable interrupts
	bge.s     gmend2
	move.w    #$c000,$dff09a
gmend2
	rts

	;------ AllocListData(Size:d0/Flags:d1): This is the memory allocator
	;------ for module specific memory to be used by all players and genies.
	;------ It provides a means of specifying that the allocation should be
	;------ made in a memory area accessible to the chips, or accessible to
	;------ shared system memory. If the allocation is successful,
	;------ DeliTracker will keep track of the new block (GetListData() will
	;------ return the location and size of this block).

; in: d0=length; d1=memtype;
; out: d0=memaddr
alloc_list_data	push	d1-d7/a0-a6

	; testcase: StoneTracker  - requests 2 blocks 1st one of type "1" (i.e. public/fastmem)
	; and 2nd of type "3" (i.e. public/chipmem)

	push d0
	move.l 4.w,a6
	jsr AllocMem(a6)
	move.l d0,d2
	pull d0

	move.l d0,d3

		; hack: copy paste from load_file:
	moveq	#1,d1
	lea	load_file_list(pc),a1
	lea	load_file_list_end(pc),a2
datalistloop21	cmp.l	a1,a2
	bgt.b	no_load_overflow1
	lea	load_file_overflow_msg(pc),a0
	bsr	put_string
	bra	dontplay
no_load_overflow1
	tst.l	(a1)
	bmi.b	enddatalistloop21
	move.l	(a1),d1
	addq.l	#1,d1
	add	#12,a1
	bra.b	datalistloop21
enddatalistloop21

	; store in file list...
	move.l	d1,(a1)		* index
	move.l	d2,4(a1)	* ptr
	move.l	d3,8(a1)	* size
	move.l	#-1,12(a1)	* mark end

	move.l d2,d0		; length

	pull	d1-d7/a0-a6

	tst.l	d0				; needed? Noiseconverter.s does this..
	rts

*******************************************************************
*
*  node = FindName( start, name )
*  d0               a0     a1
*
exec_findname
	push a2
	move.l    a0,a2
	move.l    a1,d1
	move.l    (a2),d0
	beq.s     fnend

fnnextnode
	move.l    d0,a2
	move.l    (a2),d0
	beq.s     fnend

	move.l    LN_NAME(a2),a0
	move.l    d1,a1
fncmpstring
	cmpm.b    (a0)+,(a1)+
	bne.s     fnnextnode
	tst.b     -1(a0)
	bne.s     fncmpstring

	; found it
	move.l    a2,d0
fnend
	move.l    d1,a1
	pull a2
	rts



*******************************************************************
*
*  AddHead(list, node)
*          a0    a1
*
* testcase: CUST.SkyFox
*
exec_addhead
	pushr	d0
	ADDHEAD
	pullr d0
	rts

default_exception	movem.l	d0-d7/a0-a7,$100
	move	#$7fff,intena+custom
	move	#$7fff,intreq+custom
	move	#$7fff,dmacon+custom
	lea	$100.w,a7
	bsr	set_message_traps
	moveq	#AMIGAMSG_SCORECRASH,d0
	bsr	put_message_by_value
	movem.l	$100,d0-d7/a0-a7
.default_exception_loop
	move	d0,$dff180
	not	d0
	bra.b	.default_exception_loop

contplayer
	* initialize messaging trap *
	bsr	set_message_traps

	lea	moduleptr(pc),a0
	lea	modulesize(pc),a1
	move.l	$100.w,(a0)		; poked here from uade_reset (see uade.c)
	move.l	$104.w,(a1)

	move.l	$108.w,a0		* player address

	lea	chippoint(pc),a2
	move.l	$10c.w,(a2)		* reloc address
	bsr	relocator
	tst.l	d0
	beq.b	reloc_success
	lea	reloc_error_msg(pc),a0
	bsr	put_string
	moveq	#AMIGAMSG_SCOREDEAD,d0
	bsr	put_message_by_value
reloc_wait_forever	bra	reloc_wait_forever

reloc_error_msg	dc.b	'reloc error',0
	even

reloc_success
	lea	binbase(pc),a1
	move.l	a0,(a1)		* a0 = player code start relocated

	* TODO: Figure player relocator address range vs module address range
	* and assignment to chippoint based on moduleptr/modulesize.
	*
	* allocate space for dynamic memory operations (allocmem,
	* loadseq, ...)	-> WTF has this comment to do with anything of the below?!
	lea	chippoint(pc),a0
	move.l	moduleptr(pc),d0
	add.l	modulesize(pc),d0
	addi.l	#1024,d0
	clr.b	d0
	move.l	d0,(a0)

	* EP_ModuleChange
	move.l	$198.w,d0
	beq.b	noepmc
	moveq	#-1,d0
	lea	modulechange_disabled(pc),a0
	move.l	d0,(a0)
noepmc
	* initialize intuitionbase with failures
	lea	intuition_lib_base(pc),a0
	lea	intuiwarn(pc),a1
	move.l	#$400,d0
	bsr	init_lib_base

	* initialize intuitionbase functions
	lea	intuition_lib_base(pc),a6
	lea	intuition_alloc_remember(pc),a0
	move	jmpcom(pc),-$18C(a6)
	move.l	a0,-$18C+2(a6)

	* initialize utilitybase with failures
	lea	utility_lib_base(pc),a0
	lea	utilitywarn(pc),a1
	move.l	#426,d0						; see utility_lib.i
	bsr	init_lib_base				; initializes everything with a warn function..

	* initialize gfxsbase with failures (testcase: StoneTracker - uses this
	* lib - but probably only to access the PAL related status field)
	lea	graphics_lib_base(pc),a0
	lea	graphicswarn(pc),a1
	move.l	#1050,d0					; see graphics_lib.i
	bsr	init_lib_base				; initializes everything with a warn function..

	move.w #$26,LIB_VERSION(a0)			; StoneTracker (expects version $20)
	move.w #$ce,gb_DisplayFlags(a0)

	lea utility_lib_base(pc),a1
	move.l a1,gb_UtilityBase(a0)
	move.l $4,gb_ExecBase(a0)


	* initialize iconbase with failures
	lea	icon_lib_base(pc),a0
	lea	iconwarn(pc),a1
	move.l	#$6c,d0						; see icon_lib.i
	bsr	init_lib_base				; initializes everything with a warn function..

	* initialize iconbase functions	- register the few functions actually used by PokeyNoise
	lea	icon_lib_base(pc),a6
	LVO_JMP GetDiskObject,	icon_get_disk_object
	LVO_JMP FreeDiskObject,	icon_free_disk_object
	LVO_JMP FindToolType,	icon_find_tool_type

	* ntsc/pal checking
	moveq	#$20,d1		* default beamcon0 = $20 for PAL
	moveq	#50,d2		* default 50Hz for PAL
	* 709379 / 50
	move.l	#709379,d4
	move	#$376b,d3	* PAL CIA timer value
	tst.l	$124.w
	beq.b	is_pal
	moveq	#0,d1		* NTSC beamcon0 = 0
	moveq	#60,d2		* NTSC 60 Hz
	move.l	#715909,d4
	move	#$2e9c,d3	* NTSC CIA timer value
is_pal	move	d1,beamcon0+custom
	lea	vbi_hz(pc),a0
	move	d2,(a0)
	move.l	4.w,a6

	move.b	d2,$212(a6)	* set execbase vblank frequency

	move.b	d4,ex_EClockFrequency(a6)	; IFF8SVX is using this! (a V36 Exec addition)

	lea	cia_timer_base_value(pc),a0
	move	d3,(a0)

	lea np_warn_once(pc),a0
	move.l #0,(a0)

	* check deliplayer's header tags
	bsr	parse_player_tags

	* initialize delitracker api
	bsr	init_base

	bsr	split_module_name

	bsr	call_config

	* make sure dtg_PathArrayPtr has module name stored
	bsr	copy_module_name

	* check if module corresponds to deliplayer
	bsr	call_check_module

	bsr	call_extload

	* initialize interrupt vectors
	bsr	init_interrupts

	* initialize noteplayer (after configfunc)
	bsr	np_init

	* Set default subsong to zero
	lea	eaglebase(pc),a5
	move	#0,dtg_SndNum(a5)

	* Set default timer value
	move	cia_timer_base_value(pc),dtg_Timer(a5)

	* PlayAY specific hack
	move.l playayhack(pc),d1
	cmp.l #0,d1
	beq skiphack
	bsr alloc_dummy
skiphack

	bsr	call_init_player

	* Initialize amplifier
	bsr	call_amplifier_init

	bsr	get_player_info

	bsr	call_check_subsongs

	* set subsong
	move.l	$11c.w,d1
	beq.b	nospecialsubs
	move.l	$120.w,d0
	cmpi	#2,d1
	bne.b	notrelsubs
	move	minsubsong(pc),d1
	add	d1,d0
notrelsubs	bsr	SetSubSong
	bra.b	dontsetsubsong
nospecialsubs
	lea	eaglebase(pc),a5
	move	dtg_SndNum(a5),d0
	bne.b	dosetsubsong
	move	minsubsong(pc),d0
dosetsubsong	* takes subsong in D0
	bsr	SetSubSong
dontsetsubsong
	bsr	ReportSubSongs

	* filter off
	bset	#1,$bfe001

call_init_volume
	move.l	volumefunc(pc),d0
	beq.b	novolfunc
	LOG_CALL "score:call_init_volume"
	lea	eaglebase(pc),a5
	move.l	d0,a0
	jsr	(a0)
novolfunc

	move.l	balancefunc(pc),d0	; testcase: FaceTheMusic
	beq.b	nobalancefunc
	lea	eaglebase(pc),a5
	move.l	d0,a0
	jsr	(a0)
nobalancefunc

	* call initsound
	bsr	call_init_sound

	* testcase: PaulTonge is using EP_Voices
; fixme: this kills "Mars.mus"! what is the correct way to use this?
;	move.l	voicesfunc(pc),d0
;	beq.b	novoicesfunc
;	lea	eaglebase(pc),a5
;	move.l	d0,a0
;	jsr	(a0)
;novoicesfunc

	* tell the simulator that audio output should start now
	moveq	#AMIGAMSG_START_OUTPUT,d0
	bsr	put_message_by_value

	* CIA/VBI is initialized here, or start_int is called, if
	* necessary
	bsr	set_player_interrupt

playloop	* this is for debugging only
;	bsr	volumetest

	bsr	waittrap		* wait for next frame

	; catch DMA triggered replies - fixme: this should be
	; triggered directly via some interrupt:
	bsr handleDMAreplymsgs

	* check input message
	tst.l 	$300.w
	beq.b	noinputmsgs
	bsr	inputmessagehandler
noinputmsgs
	lea	changesubsongbit(pc),a0
	tst	(a0)
	beq	dontchangesubs
	clr	(a0)

	move	#$4000,intena+custom
	* kill timer device *
	lea	vblanktimerstatusbit(pc),a0
	clr.l	(a0)
	* kill vbi list *
	lea	lev3serverlist(pc),a0
	clr.l	(a0)
	move	#$f,dmacon+custom
	move	#0,aud0vol+custom
	move	#0,aud1vol+custom
	move	#0,aud2vol+custom
	move	#0,aud3vol+custom
	move	#$00ff,adkcon+custom
	bsr	wait_audio_dma
	move	#$800f,dmacon+custom

	lea	songendbit(pc),a0	* clear for short modules
	clr.l	(a0)
	lea	virginaudioints(pc),a0	* audio ints are virgins again
	clr.l	(a0)

change_subsong

	* call nextsongfunc or prevsongfunc if necessary
	move.l	adjacentsubfunc(pc),d0
	beq.b	notadjacentsub
	LOG_CALL "score:call_adjacentsubfunc"
	lea	eaglebase(pc),a5
	move.l	d0,a0
	jsr	(a0)
	lea	cursubsong(pc),a0
	lea	eaglebase(pc),a5
	move	dtg_SndNum(a5),(a0)
	bra.b	adjacentsub
notadjacentsub
	bsr	call_stop_int
	bsr	call_end_sound
	bsr	call_init_sound
	bsr	set_player_interrupt
adjacentsub

	move	#$c000,intena+custom
dontchangesubs
	btst	#6,$bfe001		; issue? UADE3 has changes here
	beq.b	end_song

	* check if song has ended, ignore if songendbit ($12C) = 0
	tst.l	$12C.w
	beq.b	nosongendcheck
	move	songendbit(pc),d0
	bne.b	end_song
nosongendcheck

	; catch DMA triggered replies - fixme: this should be
	; triggered directly via some interrupt:
	bsr handleDMAreplymsgs

	* call dtp_interrupt if dtp_startint function hasn't been inited
	move.l	useciatimer(pc),d0
	bne.b	dontcallintfunc

	move.l	intfunc(pc),d0
	bne.b	dont_check_start_int
	move.l	startintfunc(pc),d1
	beq	dontplay
	bsr	HandleTimerDevice
	bra.b	dontcallintfunc
dont_check_start_int
	* call dtp_interrupt with trap (some require superstate)
	move.l	d0,a0
	move	#$2000,d0
	lea	trapcall(pc),a1
	move.l	a1,TRAP_VECTOR_3
	lea	eaglebase(pc),a5
	trap	#3
dontcallintfunc
	bra	playloop			* loop back

end_song	* FIRST: report that song has ended
	bsr	report_song_end

	* THEN do the deinit stuff...
	bsr	call_stop_int
	bsr	call_end_sound

	* wait for late change of subsong
	lea	songendbit(pc),a0
	clr.l	(a0)
subsongseqloop
	bsr	waittrap		* wait for next frame

	* check input message
	tst.l 	$300.w
	beq.b	subsongseqloop
	bsr	inputmessagehandler
	lea	changesubsongbit(pc),a0
	tst	(a0)
	beq.b	subsongseqloop
	bsr	call_init_sound
	bsr	set_player_interrupt
	bra	playloop

dontplay	* report that score is dead
	bsr	set_message_traps
	move	songendbit(pc),d0
	bne.b	noscoredeadmsg
	moveq	#AMIGAMSG_SCOREDEAD,d0
	bsr	put_message_by_value
noscoredeadmsg
	* check if this is delimon
	cmp.l	#'MONI',$188.w
	bne.b	flashloop
	move	#$7fff,intena+custom
	move	#$7fff,intreq+custom
	lea	smallint(pc),a0
	move.l	a0,$6c.w
	move	#$c020,intena+custom
	bra.b	flashloop
smallint	move	#$0020,intreq+custom
	rte

flashloop	move	$dff006,$dff180
	bra.b	flashloop


jsrcom	jsr	0
jmpcom	jmp	0

waittrap	lea	waittrapvector(pc),a0
	move.l	a0,TRAP_VECTOR_4
	trap	#4
	rts

* wait for the next vertical blanking frame and return
waittrapvector	lea	framecount(pc),a0
	move.l	(a0),d0
newstop	stop	#$2000	; set status register (supervisor state) and wait for exception
						; might the trap based waiting be unnecessarily blocking other tasks?
	move.l	(a0),d1
	cmp.l	d0,d1
	beq.b	newstop
	rte

* dumps memory to output trap
trap_vector_6_bin_trap	push	all
	cmpi.l	#32,d0
	ble.b	nottoobig
	moveq	#32,d0
nottoobig	andi.l	#-4,d0
	push	all
	move.l	a0,d0
	lea	binaddr(pc),a0
	bsr	genhexstring
	pull	all
	lea	bindump(pc),a1
bindumploop	tst.l	d0
	beq.b	endbindumploop
	move.l	(a0)+,d1
	push	all
	move.l	d1,d0
	move.l	a1,a0
	bsr	genhexstring
	pull	all
	subq.l	#4,d0
	addq.l	#8,a1
	move.b	#' ',(a1)+
	bra.b	bindumploop
endbindumploop	clr.b	(a1)+
	lea	binmsg(pc),a0
	sub.l	a0,a1
	move.l	a1,d0
	bsr	put_message
	pull	all
	rte
binmsg	dc.l	AMIGAMSG_GENERALMSG
	dc.b	'MEM '
binaddr	dcb.b	8,0
	dc.b	': '
bindump	dcb.b	100,0
	even

* outputs constant d0 to output trap
put_value
	push	all
	* d0 has input
	lea	pollmsgcode(pc),a0
	bsr	genhexstring
	lea	pollmsg(pc),a0
	bsr	put_string
	pull	all
	rts
pollmsg	dc.b	'debug value '
pollmsgcode	dcb.b	9,0
pollmsge	even

* inputs: d0 = number   a0 = string pointer
* function: generates ascii string from d0 in hexadecimal representation to a0
genhexstring	push	all
	moveq	#8-1,d7
ghsl	rol.l	#4,d0
	moveq	#$f,d1
	and.l	d0,d1
	cmpi	#10,d1
	blt.b	notalfa
	addi.b	#'A'-10-$30,d1
notalfa	addi.b	#$30,d1
	move.b	d1,(a0)+
	dbf	d7,ghsl
	pull	all
	rts

dos_unimplemented_call
	lea	dos_lib_base(pc),a0
	move.l	(a7),a1
	subq.l	#6,a1
	sub.l	a1,a0
	move.l	a0,d0
	lea	doserrorcode(pc),a0
	bsr	genhexstring
	lea	doserrormsg(pc),a0
	bsr	put_string
	illegal
doserrormsg	dc.b	'non-implemented dos.library function called:'
	dc.b	' -$'
doserrorcode	dcb.b	8,0
	dc.b	' => CRASH',0
doserrormsge	even

split_module_name
	move.l	$128.w,d0
	beq.b	nomodulename
	move.l	d0,a0
	lea	loadedmodname(pc),a1
	move.l	#255,d0
	bsr	strlcpy

	lea	loadedmodname(pc),a0
	bsr	strendptr
	move.l	a0,a1
	lea	loadedmodname(pc),a0
separloop1	cmp.b	#'/',-(a1)
	beq.b	endseparloop1
	cmp.l	a0,a1
	bne.b	separloop1
endseparloop1	cmp.b	#'/',(a1)
	bne.b	noslash1
	addq.l	#1,a1
noslash1
	lea	dirarray(pc),a3
patharrayloop1	cmp.l	a0,a1
	beq.b	endpatharrayloop1
	move.b	(a0)+,(a3)+
	bra.b	patharrayloop1
endpatharrayloop1
	clr.b	(a3)+
	move.l	a1,a0
	lea	filearray(pc),a1
	move.l	#255,d0
	bsr	strlcpy

nomodulename	rts

copy_module_name
	lea	eaglebase(pc),a5
	move.l	dtg_PathArrayPtr(a5),a0
	clr.b	(a0)
	bsr	copydir
	bsr	copyfile
	rts

timeventerrmsg1	dc.b	'soundcore: handletimerdevice(): timer function is '
	dc.b	'NULL pointer ...',0
	even
HandleTimerDevice
	move.l	vblanktimerstatusbit(pc),d0
	beq.b	notimerdevcount
	move.l	vblanktimercount(pc),d0
	bne.b	notimerdevevent
	lea	vblanktimerstatusbit(pc),a0
	clr.l	(a0)
	move.l	vblanktimerfunc(pc),d0
	bne.b	timerdevfuncok
	lea	timeventerrmsg1(pc),a0
	bsr	put_string
	bra	dontplay
timerdevfuncok	move.l	timerioptr(pc),a1
	push	all
	move.l	d0,a0
	jsr	(a0)
	pull	all
	bra.b	notimerdevcount
notimerdevevent	lea	vblanktimercount(pc),a1
	tst.l	(a1)
	beq.b	notimerdevcount
	subq.l	#1,(a1)
notimerdevcount	rts


* this function calls startint function if it exists and intfunc doesnt exist
call_start_int	push	all
	move.l	intfunc(pc),d0
	bne.b	intfuncexists
	LOG_CALL "score:call_start_int"
	move.l	startintfunc(pc),d0
	beq	dontplay
	move.l	d0,a0
	lea	eaglebase(pc),a5
	jsr	(a0)
intfuncexists	pull	all
	rts


call_stop_int	push	all
	move.l	stopintfunc(pc),d0
	beq.b	nostopintfunc
	LOG_CALL "score:call_stop_int"
	move.l	d0,a0
	lea	eaglebase(pc),a5
	jsr	(a0)
nostopintfunc
	* Disable player interrupt, if any.
	move.l	intfunc(pc),d0
	beq.b	do_not_disable_player_interrupt
	lea	rem_ciaa_interrupt(pc),a0
	move.l	cia_chip_sel(pc),d0
	beq.b	its_ciaa_2
	lea	rem_ciab_interrupt(pc),a0
its_ciaa_2	move.l	cia_timer_sel(pc),d0
	jsr	(a0)
do_not_disable_player_interrupt
	pull	all
	rts


* A trap vector that does nothing by itself, but the uade emulator does
* specific things depending on what is contained in the address pointed to
* by msgptr pointer.
trap_vector_5_message_trap	rte


inputmessagehandler
	push	all
	lea	$300.w,a0
	cmp.l	#AMIGAMSG_SETSUBSONG,(a0)
	bne.b	nnsubs
	move.l	$120.w,d0
	* call SetSubSong if nextsubsong func is not used
	push	all
	move	d0,d1
	sub	cursubsong(pc),d1
	lea	adjacentsubfunc(pc),a1
	clr.l	(a1)
	cmpi	#1,d1
	bne.b	notnextsub
	move.l	nextsongfunc(pc),d2
	beq.b	notnextsub
	move.l	d2,(a1)
	bra.b	dontcallsetss
notnextsub	cmpi	#-1,d1
	bne.b	notprevsub
	move.l	prevsongfunc(pc),d2
	beq.b	notprevsub
	move.l	d2,(a1)
	bra.b	dontcallsetss
notprevsub	bsr	SetSubSong
dontcallsetss	pull	all
	lea	changesubsongbit(pc),a0
	st	(a0)
	bra	inputmessagehandled
nnsubs
	cmp.l	#AMIGAMSG_NTSC,(a0)
	bne.b	no_ntsc_pal
	move.l	$124.w,d0
	andi.l	#1,d0
	move.l	d0,d1
	eor.b	#1,d0
	lsl	#5,d0
	move	d0,beamcon0+custom
	lea	ntsc_report_bit(pc),a0
	addi.b	#$30,d1
	move.b	d1,(a0)
	lea	ntsc_report_msg(pc),a0
	bsr	put_string
	bra	inputmessagehandled
ntsc_report_msg	dc.b	'ntsc bit set: '
ntsc_report_bit	dc.b	'0', 0

no_ntsc_pal	move.l	$300.w,d0
	lea	inputmsgcode(pc),a0
	bsr	genhexstring
	lea	input_msg_error(pc),a0
	bsr	put_string

inputmessagehandled
	clr.l	$300.w
	pull	all
	rts

input_msg_error	dc.b	'sound core got unknown input message 0x'
inputmsgcode	dcb.b	9,0
	even

put_message
	tst.l	d0
	bne.b	.nonzero
	illegal
.nonzero
	move.l	msgptr(pc),a1
.msgloop
	move.b	(a0)+,(a1)+
	subq.l	#1,d0
	bne.b	.msgloop
	cmp.l	#UADECORE_INPUT_MSG_END,a1
	bls.b	.no_overflow
	illegal
.no_overflow
	trap	#5
	rts

put_message_by_value
	move.l	msgptr(pc),a0
	move.l	d0,(a0)
	trap	#5
	rts


; issue: UADE's original impl uses the same buffer for all types of messages -
; causing simple debug output to interfere with the "parent" message context (see file loading)
put_string	push	a0-a1
	move.l	msgptr(pc),a1
	move.l	#AMIGAMSG_GENERALMSG,(a1)+		; overwriting this should be OK since parent has already evaluated the type

	; hack (use a dedicated buffer for debug message body..)
	move.l	dbgmsgptr(pc),a1
	addq.l #4,a1

.stringmsgloop
	move.b	(a0)+,(a1)+
	bne.b	.stringmsgloop
	cmp.l	#UADECORE_DBGINPUT_MSG_END,a1
	bls.b	.no_overflow
	illegal
.no_overflow
	trap	#5
	pull	a0-a1
	rts

strlen	moveq	#-1,d0
.strlenloop
	addq.l	#1,d0
	tst.b	(a0,d0.w)
	bne.b	.strlenloop
	rts

set_message_traps
	lea	trap_vector_5_message_trap(pc),a0
	move.l	a0,TRAP_VECTOR_5
	lea	trap_vector_6_bin_trap(pc),a0
	move.l	a0,TRAP_VECTOR_6
	rts


dummy_player_header
	PLAYERHEADER	0
dummy_player_header_end

playayhack	dc.l	0
merkpuff	dc.l	0
	even

; testcase: MarbleMadness, SkyFox - player starts timer.device what supposedly signals the
; task with 60Hz freq.. interestingly this does not seem to make any difference (in SkyFox)
; hack: added direct timer signaling by piggy bagging in Eagleplayer's Interrupt call.
timersighack
	push all

	move.l timersigtask(pc),d1
	cmp.l #0,d1
	beq skiptimersig

		; hack... presumably the timer runs once and needs to be relatched
	lea timerioreq(pc),a1
	move.l (a1),a1
	lea IOTV_TIME(a1),a1
	move.l TV_MICRO(a1),d0
	cmp #0,d0
	beq skiptimersig

	lea TV_MICRO(a1),a0
	move.l #0,(a0)			; app seems to do this as well and it might not be necessary here..

	move.l 4.w,a6
	move.l d1,a1
	move.l timersigmask(pc),d0
	jsr Signal(a6)

skiptimersig
	pull all
	rts


origintfunc dc.l 0

intwrapper
	bsr timersighack

	move.l	startintfunc(pc),d0		; testcase: FaceTheMusic
	bne	skipregint

; FaceTheMusic plays at 2x speed when interrupts are also triggered here
; disabling this function completely, makes the FTM songs play at the correct slow speed
; see https://www.youtube.com/playlist?list=PLg4TSh1agl0dcte1mPkcbLOdxiolvpLqZ

	lea origintfunc(pc),a0
	move.l (a0),a0
	jsr (a0)
skipregint
	rts

parse_player_tags
	move.l	binbase(pc),a0
	move.l	dummy_player_header_end-dummy_player_header-4(a0),a0
	
tagloop	move.l	(a0),d0
	tst.l	d0
	beq	endtagloop

	cmpi.l	#DTP_RequestDTVersion,d0
	bne.b	nonov
	lea	dtp_version(pc),a1
	move.l	4(a0),(a1)
nonov
	cmpi.l	#DTP_Check2,d0
	bne.b	nono1
	lea	dtp_check(pc),a1
	move.l	4(a0),(a1)
nono1	cmpi.l	#EP_Check3,d0
	bne.b	nono1_2
	lea	ep_check3(pc),a1
	move.l	4(a0),(a1)
nono1_2	cmpi.l	#EP_Check5,d0
	bne.b	nono1_3
	lea	ep_check5(pc),a1
	move.l	4(a0),(a1)
nono1_3	cmpi.l	#DTP_Interrupt,d0
	bne.b	nono2
	lea	intfunc(pc),a1

;	move.l	4(a0),(a1)			; orig impl

	push a3	; use wrapper for Interrupt func so that other stuff can be piggybacked
	lea origintfunc(pc),a3
	move.l	4(a0),(a3)
	lea intwrapper(pc),a3
	move.l	a3,(a1)
	pull a3

nono2	cmpi.l	#DTP_InitPlayer,d0
	bne.b	nono3
	lea	initfunc(pc),a1
	move.l	4(a0),(a1)
nono3	cmpi.l	#DTP_SubSongRange,d0
	bne.b	nono4
	lea	subsongfunc(pc),a1
	move.l	4(a0),(a1)
nono4	cmpi.l	#DTP_EndSound,d0
	bne.b	nono5
	lea	endfunc(pc),a1
	move.l	4(a0),(a1)
nono5	cmpi.l	#DTP_InitSound,d0
	bne.b	nono6
	lea	initsoundfunc(pc),a1
	move.l	4(a0),(a1)
nono6	cmpi.l	#DTP_CustomPlayer,d0
	bne.b	nono7
	lea	moduleptr(pc),a1
	clr.l	(a1)
	lea	modulesize(pc),a1
	clr.l	(a1)
nono7	cmpi.l	#DTP_Volume,d0
	bne.b	nono8
	lea	volumefunc(pc),a1
	move.l	4(a0),(a1)
nono8	cmpi.l	#DTP_Balance,d0		; added
	bne.b	nono8i
	lea	balancefunc(pc),a1
	move.l	4(a0),(a1)
nono8i	cmpi.l	#EP_Voices,d0		; added
	bne.b	nono9
	lea	voicesfunc(pc),a1
	move.l	4(a0),(a1)
nono9	cmpi.l	#DTP_NextSong,d0
	bne.b	nono10
	lea	nextsongfunc(pc),a1
	move.l	4(a0),(a1)
nono10	cmpi.l	#DTP_PrevSong,d0
	bne.b	nono11
	lea	prevsongfunc(pc),a1
	move.l	4(a0),(a1)
nono11	cmpi.l	#DTP_DeliBase,d0
	bne.b	nono12
	lea	eaglebase(pc),a1
	move.l	4(a0),a2
	move.l	a1,(a2)
nono12	cmpi.l	#DTP_StartInt,d0
	bne.b	nono13
	lea	startintfunc(pc),a1
	move.l	4(a0),(a1)
nono13	cmpi.l	#DTP_StopInt,d0
	bne.b	nono14
	lea	stopintfunc(pc),a1
	move.l	4(a0),(a1)
nono14	cmpi.l	#DTP_Config,d0
	bne.b	nono15
	lea	configfunc(pc),a1
	move.l	4(a0),(a1)
nono15	cmpi.l	#DTP_ExtLoad,d0
	bne.b	nono16_
	lea	extloadfunc(pc),a1
	move.l	4(a0),(a1)

	; PlayAY uses DTP_Process/DTP_StackSize/DTP_MsgPort (which isn't implemented here):
	; for proper support of PlayAY/EMUL two processes would be needed - one that runs the DTP_Process
	; (which waits for messages and then runs whatever code is specified in the message before
	; returning the message).. luckily EMUL is already supported by NEZplug.. so fuck this

nono16_	cmpi.l	#DTP_Process,d0	; hack: for now just use this as a "PlayAY" detection..
	bne.b	nono16
	; hack PlayAY/ZXAYEMUL tries to allocate mem directly via the MemList (just give it that mem)
	lea playayhack(pc),a1
	move.l #1,(a1)

nono16	cmpi.l	#DTP_NewSubSongRange,d0
	bne.b	nono17
	lea	newsubsongarray(pc),a1
	move.l	4(a0),(a1)
nono17	cmpi.l	#DTP_NoteStruct,d0
	bne.b	nono18
	lea	noteplayerptr(pc),a1
	move.l	4(a0),(a1)
nono18	cmpi.l	#DTP_InitNote,d0
	bne.b	nono19
	lea	noteplayersetupfunc(pc),a1
	move.l	4(a0),(a1)
nono19	cmpi.l	#EP_InitAmplifier,d0
	bne.b	nono20
	lea	amplifier_init_func(pc),a1
	move.l	4(a0),(a1)
nono20	cmpi.l	#EP_EagleBase,d0
	bne.b	nono21
	lea	eaglebase(pc),a1
	move.l	4(a0),a2
	move.l	a1,(a2)

	; testcase PlayAY.. InitPlayer/InitSound expect A4 to point to its "Merkpuffer".
	; too bad the fucking Eagleplayer docs do not say what the signatures of their bloody
	; functions actually are!
	push a0
	lea merkpuff(pc),a0
	move.l a2,(a0)
	pull a0

nono21
nexttag	addq.l	#8,a0
	bra	tagloop
endtagloop
	rts


get_player_info	move.l	binbase(pc),a0
	move.l	dummy_player_header_end-dummy_player_header-4(a0),a0
infoloop	move.l	(a0),d0
	tst.l	d0
	beq	endinfoloop
	cmpi.l	#DTP_ModuleName,d0
	bne.b	nodtpmodulename
	move.l	4(a0),a1
	move.l	(a1),d1
	beq.b	nodtpmodulename
	push	all
	move.l	d1,a0
	lea	modulename+4(pc),a1
	move.l	#250,d0
	bsr	strlcpy
	lea	modulename(pc),a0
	move.l	#AMIGAMSG_MODULENAME,(a0)
	move.l	#256,d0
	bsr	put_message
	pull	all
	bra	nextinfoiter
nodtpmodulename	cmpi.l	#DTP_PlayerName,d0
	bne.b	nodtpplayername
	push	all
	move.l	4(a0),a0
	lea	playername+4(pc),a1
	move.l	#25,d0
	bsr	strlcpy
	lea	playername(pc),a0
	move.l	#AMIGAMSG_PLAYERNAME,(a0)
	move.l	#32,d0
	bsr	put_message
	pull	all
	bra	nextinfoiter
nodtpplayername	cmpi.l	#DTP_FormatName,d0
	bne.b	nodtpformatname
	move.l	4(a0),a1
	move.l	(a1),d1
	beq.b	nodtpformatname
	push	all
	move.l	d1,a0
	lea	formatname+4(pc),a1
	move.l	#250,d0
	bsr	strlcpy
	lea	formatname(pc),a0
	move.l	#AMIGAMSG_FORMATNAME,(a0)
	move.l	#256,d0
	bsr	put_message
	pull	all
	bra	nextinfoiter
nodtpformatname
nextinfoiter	addq.l	#8,a0
	bra	infoloop
endinfoloop	rts

* a0 src1 a1 src2
* returns d0: zero => same, non-zero => not same (not usable for sorting)
strcmp	push	a0-a1
strcmp_loop	move.b	(a0)+,d0
	cmp.b	(a1)+,d0
	bne.b	strcmp_notsame
	tst.b	d0
	bne.b	strcmp_loop
	pull	a0-a1
	moveq	#0,d0
	rts
strcmp_notsame	pull	a0-a1
	moveq	#-1,d0
	rts

* a0 src1, a1 src2, d0 max length to compare
* returns d0: zero => same, non-zero => not same (not usable for sorting)
strncmp	movem.l	d1/a0-a1,-(a7)
	move.l	d0,d1
	beq.b	.strncmp_notsame
.strncmp_loop
	move.b	(a0)+,d0
	cmp.b	(a1)+,d0
	bne.b	.strncmp_notsame
	subq.l	#1,d1
	beq.b	.strncmp_same
	tst.b	d0
	bne.b	.strncmp_loop
.strncmp_same
	movem.l	(a7)+,d1/a0-a1
	moveq	#0,d0
	rts
.strncmp_notsame
	movem.l	(a7)+,d1/a0-a1
	moveq	#-1,d0
	rts

* a0 src a1 dst d0 max bytes
strlcpy	subq	#1,d0
	bmi.b	endstrlcpyloop
strlcpyloop	move.b	(a0)+,d1
	move.b	d1,(a1)+
	tst.b	d1
	beq.b	endstrlcpyloop
	dbf	d0,strlcpyloop
endstrlcpyloop	rts

* a0 string returns the zero byte in a0
strendptr	tst.b	(a0)+
	bne.b	strendptr
	subq.l	#1,a0
	rts


hex2int	moveq	#0,d0
	push	d1/a0
hexloop	move.b	(a0)+,d1
	beq.b	endhexloop
	lsl	#4,d0
	cmpi.b	#$61,d1
	blt.b	notsmall
	subi.b	#$61-10,d1
	andi	#$f,d1
	or	d1,d0
	bra.b	hexloop
notsmall	cmpi.b	#$41,d1
	blt.b	notbig
	subi.b	#$41-10,d1
	andi	#$f,d1
	or	d1,d0
	bra.b	hexloop
notbig	subi.b	#$30,d1
	andi	#$f,d1
	or	d1,d0
	bra.b	hexloop
endhexloop	pull	d1/a0
	rts


eaglebase_unimplemented_call
	push	all
	lea	safetymsg(pc),a0
	bsr	put_string
	move.l	60(a7),d0		* return address
	subq.l	#6,d0
	lea	eaglebase(pc),a0
	sub.l	a0,d0
	bsr	put_value
	pull	all
	illegal

eaglebase_unimplemented_fp
	push	all
	lea	safetymsg2(pc),a0
	bsr	put_string
	illegal


safetymsg	dc.b	'fatal error: non-implemented EP function',0
safetymsg2	dc.b	'fatal error: non-set EP function ptr',0
	even

init_base	push a6
	lea	eaglebase(pc),a6

		; set unimplemented ENPP function calls to eaglebase_unimplemented_call()

	move.l	a6,a0
	lea	eaglebase_unimplemented_call(pc),a1
	moveq	#(eaglebase-eaglesafetybase)/6-1,d0
dsbl	subq.l	#6,a0
	move	jmpcom(pc),(a0)
	IFNE	CRASH_ON_UNIMPLEMENTED
	move.w	#$4afc,(a0)			; illegal instr to identify used offset more easily
	ENDC

	move.l	a1,2(a0)
	dbf	d0,dsbl

		; set unimplemented dtp_/EPG_ function pointers

		; note: new eagleplayer redefines fields starting at dtg_NotePlayer.
		; luckily the respective first 3 fields are "reserved" and having the
		; "old player logic" set these should therefore not lead to conflicts.
		; and the extra new EP fields should not bother the old EP

	lea	eaglebase_unimplemented_fp(pc),a1
	lea dtg_GetListData(a6),a0
	moveq	#21-6+18-1,d0
dsbl1
	move.l	a1,(a0)+
	dbf	d0,dsbl1

	move.l	moduleptr(pc),d0
	move.l	d0,dtg_ChkData(a6)

	move.l	modulesize(pc),d1
	move.l	d1,dtg_ChkSize(a6)

	move	cursubsong(pc),dtg_SndNum(a6)
	moveq	#64,d0
	move	d0,dtg_SndVol(a6)
	move	d0,dtg_SndLBal(a6)
	move	d0,dtg_SndRBal(a6)
	move	#15,EPG_Voices(a6)
	move	d0,EPG_Voice1Vol(a6)
	move	d0,EPG_Voice2Vol(a6)
	move	d0,EPG_Voice3Vol(a6)
	move	d0,EPG_Voice4Vol(a6)


	LIB_JMP	ENPP_AllocAudio,	dtg_AudioAlloc,		enpp_alloc_audio
	LIB_JMP	ENPP_FreeAudio,		dtg_AudioFree,		okprog
	LIB_JMP	ENPP_GetListData,	dtg_GetListData,	enpp_get_list_data
	LIB_JMP	ENPP_SongEnd,		dtg_SongEnd,		enpp_song_end


	SET_PTR	dtg_AllocListData,	alloc_list_data
	SET_PTR	EPG_LoadExecutable,	epg_loadexecutable

	lea	defaultstartintfunc(pc),a0
	move.l	startintfunc(pc),d0
	beq.b	nostartintfunc
	move.l	d0,a0
nostartintfunc
	move.l	a0,dtg_StartInt(a6)

	lea	defaultstopintfunc(pc),a0
	move.l	stopintfunc(pc),d0
	beq.b	nostopintfunc2
	move.l	d0,a0
nostopintfunc2
	move.l	a0,dtg_StopInt(a6)


	LIB_JMP	ENPP_SetTimer,		dtg_SetTimer,		enpp_set_timer
	clr	dtg_Timer(a6)

	SET_PTR	dtg_NotePlayer,		np_int
	SET_PTR	EPG_FindAuthor,		epg_findauthor
	SET_PTR	EPG_ModuleChange,	epg_modulechange
	SET_PTR	dtg_WaitAudioDMA,	wait_audio_dma
	SET_PTR	dtg_PathArrayPtr,	patharray
	SET_PTR	dtg_FileArrayPtr,	filearray
	SET_PTR	dtg_DirArrayPtr,	dirarray

	LIB_JMP	ENPP_LoadFile,		dtg_LoadFile,		enpp_load_file
	LVO_JMP	ENPP_NewLoadFile,	enpp_load_file

	SET_PTR	EPG_NewLoadFile,	enpp_load_file

	SET_PTR	dtg_CopyDir,		copydir
	SET_PTR	dtg_CopyFile,		copyfile
	SET_PTR	dtg_CutSuffix,		cutsuffix
	SET_PTR	dtg_CopyString,		copystring

	SET_PTR	dtg_DOSBase,		dos_lib_base
	SET_PTR	dtg_IntuitionBase,	intuition_lib_base

	LVO_JMP	ENPP_Amplifier,		enpp_amplifier_int
	LVO_JMP	ENPP_DMAMask,		enpp_amplifier_dma
	LVO_JMP	ENPP_PokeAdr,		enpp_amplifier_adr
	LVO_JMP	ENPP_PokeLen,		enpp_amplifier_len
	LVO_JMP	ENPP_PokePer,		enpp_amplifier_per
	LVO_JMP	ENPP_PokeVol,		enpp_amplifier_vol
	LVO_JMP	ENPP_PokeCommand,	enpp_amplifier_command
	LVO_JMP	ENPP_FindAuthor,	enpp_find_author

	move.l a6,a5
	pull a6
	rts

	; WTF is this? the ENPP_AllocAudio is supposed to set d0!
	; the tst.l is done in the code that calls it! testcase: PlayAY
enpp_alloc_audio
	LOG_CALL	"eagle:ENPP_AllocAudio"
	move.l #0,d0	; added
	tst.l	d0
	rts

fawarn	dc.b	'ENPP_FindAuthor not implemented',0
	even
enpp_find_author
	LOG_CALL	"eagle:ENPP_FindAuthor"
	push	a0
	lea	fawarn(pc),a0
	bsr	put_string
	pull	a0
	rts

wait_audio_dma	push	d0-d1
	move.l	dmawaitconstant(pc),d0
	subq	#1,d0
	bmi.b	wad_nowait
wad_loop	move.b	$dff006,d1
wad_loop_1	cmp.b	$dff006,d1
	beq.b	wad_loop_1
	dbf	d0,wad_loop
wad_nowait	pull	d0-d1
	rts


enpp_set_timer
	LOG_CALL	"eagle:ENPP_SetTimer"
	push	all
	move.l	cia_chip_sel(pc),d0
	move.l	cia_timer_sel(pc),d1
	bsr	set_cia_timer_value
	pull	all
	rts


set_player_interrupt
	push	all
	move.l	intfunc(pc),d0
	beq	.try_start_int
	LOG_CALL "score:set_player_interrupt:use_cia_timer"
	lea	useciatimer(pc),a0
	st	(a0)
	lea	add_ciaa_interrupt(pc),a0
	move.l	cia_chip_sel(pc),d0
	beq.b	.its_ciaa
	lea	add_ciab_interrupt(pc),a0
.its_ciaa
	lea	tempciabtimerstruct(pc),a1
	move.l	intfunc(pc),$12(a1)
	move.l	cia_timer_sel(pc),d0
	jsr	(a0)

	move.l	startintfunc(pc),d0		; testcase: FaceTheMusic
	beq	.skipstart
	lea	eaglebase(pc),a5
	move.l	dtg_StartInt(a5),a5
	jsr (a5)
.skipstart
	pull	all
	rts
.try_start_int
	move.l	startintfunc(pc),d0
	beq	dontplay
	* call startint (player initializes own interrupts here)
	bsr	call_start_int
	pull	all
	rts

tempciabtimerstruct	dcb.b	$20,0

* D0 CIA A or CIA B: 0 = CIA A, 1 = CIA B
* D1 Timer A or timer B: 0 = Timer A, 1 = Timer B
set_cia_timer_value
	push	d0-d1/a0-a1
	btst	#0,d0
	bne.b	set_ciab_timer
	lea	$bfe001,a0
	bra.b	set_ciaa_timer
set_ciab_timer	lea	$bfd000,a0
set_ciaa_timer	andi	#1,d1
	lsl	#8,d1
	add	d1,d1
	add	d1,a0
	lea	eaglebase(pc),a1
	move	dtg_Timer(a1),d0
	move.b	d0,$400(a0)
	ror	#8,d0
	move.b	d0,$500(a0)
	pull	d0-d1/a0-a1
	rts


modulechangemsg	dc.b	'epg_modulechange: patched the player', 0
findauthormsg	dc.b	'epg_findauthor notice', 0
	even

epg_modulechange	push	all
	move.l	modulechange_disabled(pc),d0
	bne	modulechange_not_enabled
	cmp.l	#1,EPG_ARG4(a5)
	bne	modulechange_not_enabled
	cmp.l	#-2,EPG_ARG5(a5)
	bne.b	modulechange_not_enabled
	cmp.l	#5,EPG_ARGN(a5)
	bne.b	modulechange_not_enabled
	moveq	#0,d7			* number of patches applied
	move.l	EPG_ARG3(a5),a1		* patch table
mc_pl_1	move	(a1)+,d2		* pattern offset
	beq.b	end_mc_pl_1
	move	(a1)+,d3		* (pattern_len / 2) - 1
	move	(a1)+,d4		* patch offset
	move.l	EPG_ARG1(a5),a0		* dst address
	move.l	EPG_ARG2(a5),d0		* dst len
mc_pl_2	tst.l	d0
	ble.b	end_mc_pl_2
	move	d3,d5			* patch dbf len
	move.l	EPG_ARG3(a5),a2		* patch table
	add	d2,a2			* + pattern offset
	move.l	a0,a3			* dst
mc_pl_3	cmpm	(a2)+,(a3)+
	bne.b	mc_pl_3_no
	dbf	d5,mc_pl_3
	* a pattern match => patch it
	move.l	a0,a2			* dst address
	move	d3,d5			* patch dbf len
mc_pl_3_nop	move	#$4e71,(a2)+		* put nops to old code
	dbf	d5,mc_pl_3_nop
	move.l	EPG_ARG3(a5),a2		* patch table
	add	d4,a2			* patch code address
	move	jsrcom(pc),(a0)		* put jump to patch code
	move.l	a2,2(a0)
	addq.l	#1,d7			* number of patches applied
	move	d3,d5
	ext.l	d5
	add.l	d5,d5
	sub.l	d5,d0
	add.l	d5,a0			* skip dbf_len*2+2-2
mc_pl_3_no	addq.l	#2,a0
	subq.l	#2,d0
	bra.b	mc_pl_2
end_mc_pl_2	bra.b	mc_pl_1
end_mc_pl_1	tst.l	d7
	beq.b	modulechange_not_enabled
	lea	modulechangemsg(pc),a0
	bsr	put_string
modulechange_not_enabled
	pull	all
	rts
epg_findauthor	push	all
	lea	findauthormsg(pc),a0
	bsr	put_string
	pull	all
	rts

defaultstartintfunc	lea	startintwarning(pc),a0
	bsr	put_string
	rts
defaultstopintfunc	lea	stopintwarning(pc),a0
	bsr	put_string
	rts
startintwarning	dc.b	'warning: default start int func called', 0
stopintwarning	dc.b	'warning: default stop int func called', 0
	even

noinitfuncwarn	dc.b	'warning: the player is unorthodox (no InitPlayer())',0
eaglelibbasemsg	dc.b	'eagle_lib baseaddress:', 0
	even

call_init_player
	lea	eaglebase(pc),a5

	IFNE	CRASH_ON_UNIMPLEMENTED
	; print base addr to ease calculation of crash offsets
	lea	eaglelibbasemsg(pc),a0
	bsr put_string
	move.l	a5,d0
	bsr put_value

	lea	dos_lib_base(pc),a5
	lea	doslibbasemsg(pc),a0
	bsr put_string
	move.l	a5,d0
	bsr put_value

	lea	eaglebase(pc),a5
	ENDC

	move.l	initfunc(pc),d0
	bne.b	hasinitfunction
	lea	noinitfuncwarn(pc),a0
	bsr	put_string
	rts
hasinitfunction
	LOG_CALL "score:call_init_player"
	move.l	d0,a0
	; testcase PlayAY: initSound expects A4 to be set to ..
	push a0
	lea merkpuff(pc),a0
	move.l (a0),a4
	pull a0

	jsr	(a0)
	tst.l	d0
	beq.b	initwasok
	lea	initerrmsg(pc),a0
	bsr	put_string
initwasok	rts
initerrmsg	dc.b	'InitPlayer function returned fail',0
	even

checkwarn	dc.b	'warning: both DTP_Check2 and EP_Check5 available: '
	dc.b	'using DTP_Check2',0
epcheck3warn	dc.b	'warning: using EP_Check3',0
	even

* call check module: Priority for checks (first is highest):
*	     DTP_Check2, EP_Check5, EP_Check3
call_check_module
	lea	eaglebase(pc),a5
	move.l	dtp_check(pc),d0
	beq.b	no_dtp_ep_conflict
	move.l	ep_check5(pc),d0
	beq.b	no_dtp_ep_conflict
	lea	checkwarn(pc),a0
	bsr	put_string
no_dtp_ep_conflict

* reminder: UADE's below impl is garbage: always using DTP_Check2 when EP_Check5 also is available
* IS NOT how it works in the original eagleplayer! testcase: old MusicMaker_4V. it seems that the
* check to be used somehow is linked to the memory region that the song expects to be loaded into.
* in the test-case one of the two always succeeds while the other fails - which one depends on
* the song that either want to reside in chip-mem or in fast-mem! (IFF8SVX uses DTP_Check1 - which
* isn't handled at all in UADE..)

	move.l	dtp_check(pc),d0
	bne.b	do_ep_check
	move.l	ep_check5(pc),d0
	bne.b	do_ep_check
	move.l	ep_check3(pc),d0
	beq.b	do_no_check
	lea	epcheck3warn(pc),a0
	bsr	put_string
do_ep_check
	LOG_CALL "score:call_check_module"
	move.l	d0,a0
	* must be called even when force_by_default is enabled
	jsr	(a0)
	tst.l	d0
	beq.b	song_ok
	tst.l	$118.w
	bne.b	song_ok
	moveq	#AMIGAMSG_CHECKERROR,d0
	bsr	put_message_by_value
.loopforever
	bra	.loopforever
do_no_check
song_ok	rts

call_config	lea	eaglebase(pc),a5
	move.l	configfunc(pc),d0
	beq.b	noconfig
	LOG_CALL "score:call_config"
	move.l	d0,a0
	jsr	(a0)
	tst.l	d0
	bne	dontplay
noconfig	rts

call_extload	* load external data if requested
	move.l	extloadfunc(pc),d0
	beq.b	noextloadfunc
	LOG_CALL "score:call_extload"

	lea	eaglebase(pc),a5	; extloadfunc input parm
	move.l	d0,a0
	jsr	(a0)
	tst.l	d0
	beq.b	noextloadfunc
	lea	extloaderrmsg(pc),a0
	bsr	put_string
	bra	dontplay
extloaderrmsg	dc.b	'ExtLoad failed',0
	even
noextloadfunc	rts

report_song_end	moveq	#AMIGAMSG_SONG_END,d0
	bsr	put_message_by_value
	rts

call_check_subsongs
	push	all
	move.l	subsongfunc(pc),d0
	beq.b	.NoSubSongFunc
	LOG_CALL "score:call_check_subsongs"
	move.l	d0,a0
	lea	eaglebase(pc),a5
	jsr	(a0)
	lea	subsongrange(pc),a0
	movem	d0-d1,(a0)
.NoSubSongFunc
	move.l	newsubsongarray(pc),d0
	beq.b	.nonewsubsongarr
	move.l	d0,a0
	movem	(a0),d0-d2   * d0 = default, d1 = min, d2 = max subsong
	lea	eaglebase(pc),a5
	move	d0,dtg_SndNum(a5)
	lea	subsongrange(pc),a1
	movem	d1-d2,(a1)
.nonewsubsongarr
	pull	all
	rts

ReportSubSongs	move	minsubsong(pc),d0
	move	maxsubsong(pc),d1
	move	cursubsong(pc),d2
	ext.l	d0
	ext.l	d1
	ext.l	d2
	lea	subsonginfo(pc),a0
	move.l	d0,4(a0)
	move.l	d1,8(a0)
	move.l	d2,12(a0)
	moveq	#16,d0
	bsr	put_message
	rts
subsonginfo	dc.l	AMIGAMSG_SUBSINFO
	dc.l	0,0,0


SetSubSong	push	d0-d1/a0-a2/a5
	lea	cursubsong(pc),a0
	lea	eaglebase(pc),a5
	tst	d0
	bpl.b	notnegative
	moveq	#0,d0
notnegative	move	d0,(a0)
	move	d0,dtg_SndNum(a5)
nonewsubsong	pull	d0-d1/a0-a2/a5
	rts


* TODO: BUG: internal register value is not moved to d1 if initsoundfunc is 0.
call_init_sound
	lea	eaglebase(pc),a5
	move.l	initsoundfunc(pc),d0
	beq.b	.initsoundintena
	LOG_CALL "score:call_init_sound"
	move.l	d0,a0
	move	intenar+custom,d1
	pushr	d1

	; testcase PlayAY: initSound expects A4 to be set to:
	push a0
	lea merkpuff(pc),a0
	move.l (a0),a4
	pull a0

	jsr	(a0)
	pullr	d1
.initsoundintena
	* this hack overcomes fatality in SynTracker deliplayer
	* SynTracker does move #$4000,intena+custom in InitSound
	* function and does not re-enable interrupts
	move	intenar+custom,d2
	andi	#$4000,d1
	beq.b	.nointenaproblem
	andi	#$4000,d2
	bne.b	.nointenaproblem
	lea	intenamsg(pc),a0
	bsr	put_string
	move	#$c000,intena+custom	* re-enable intena
.nointenaproblem
	tst.l	d0
	rts
intenamsg	dc.b	'Stupid deliplayer: disables interrupts',0
	even


call_end_sound	push	all
	move.l	endfunc(pc),d0
	beq.b	noendsound
	LOG_CALL "score:call_end_sound"
	move.l	d0,a0
	lea	eaglebase(pc),a5
	jsr	(a0)
	pull	all
	rts
noendsound	move	#15,dmacon+custom
	move	#0,aud0vol+custom
	move	#0,aud1vol+custom
	move	#0,aud2vol+custom
	move	#0,aud3vol+custom
	pull	all
	rts


enpp_song_end
	LOG_CALL	"eagle:ENPP_SongEnd"
	pushr	a0
	lea	songendbit(pc),a0
	st	(a0)
	pullr	a0
	rts


okprog	moveq	#0,d0
	rts


*** MIX MODULE SIZE LATER ****


************************************************************************
* GetListData(Num:d0): This function returns the memorylocation
* of a loaded file in a0 and its size in d0. Num starts with 0
* (the selected module). Example: GetListData(2) returns the
* start of the third file loaded (via ExtLoad) in a0 an its size
* in d0.
*
* in: D0=num of loaded file
* out: A0=addr of file data; D0=size (0 means error)
*
enpp_get_list_data
	LOG_CALL	"eagle:ENPP_GetListData"
	tst.l	d0
	bne.b	dontreturnmodule
	move.l	moduleptr(pc),a0
	move.l	modulesize(pc),d0
	rts
dontreturnmodule
	push	d1/a1
	lea	load_file_list(pc),a1
datalistloop1
	move.l	(a1),d1				; idx
	bmi.b	endloopillegal
	cmp.l	d0,d1
	bne.b	notthisdata
	move.l	4(a1),a0			; mem ptr
	move.l	8(a1),d0			; length
	bra.b	enddatalistloop1
notthisdata	add	#12,a1
	bra.b	datalistloop1
enddatalistloop1
	pull	d1/a1
	rts
endloopillegal	addi.b	#$30,d0
	lea	errorloadindex(pc),a1
	move.b	d0,(a1)
	lea	getlistdataerror(pc),a0
	bsr	put_string
	move.l #0,d0		; signal the error!
	pull	d1/a1
	rts
getlistdataerror	dc.b	'Tried to get list data with index number '
errorloadindex	dc.b	'0, but it does not exist!',0
getlistdataerrore	even

copydir	push	d0/a0-a1/a5
	lea	eaglebase(pc),a5
	move.l	dtg_PathArrayPtr(a5),a0
	bsr	strendptr
	move.l	a0,a1
	lea	dirarray(pc),a0
	move.l	#255,d0
	bsr	strlcpy
	pull	d0/a0-a1/a5
	rts

copyfile	push	d0/a0-a1/a5
	lea	eaglebase(pc),a5
	move.l	dtg_PathArrayPtr(a5),a0
	bsr	strendptr
	move.l	a0,a1
	lea	filearray(pc),a0
	move.l	#128,d0
	bsr	strlcpy
	pull	d0/a0-a1/a5
	rts

cutsuffix	rts

copystring	pushr	a5
	pushr	a0
	move.l	dtg_PathArrayPtr(a5),a0
	bsr	strendptr
	move.l	a0,a1
	pullr	a0
	move.l	#128,d0
	bsr	strlcpy
	pullr	a5
	rts

icon_free_disk_object
	rts						* memory leak.. does not matter

icontooltypemsg	dc.l	AMIGAMSG_ICON_TOOLTYPE
	dc.l	0,0,0,0	* key ptr, chip dest ptr, size in msgptr(pc)+12
icontooltypemsge
	even
icon_find_tool_type										* char *FindToolType(char **list, char *key); d0 = (a0, a1) (presumably a0 points to garbage here since respective parsing is not implemented)
	push	d1-d7/a0-a6
	lea	icontooltypemsg(pc),a0
	move.l	a1,4(a0)								* a1 points to the search key string
	move.l	#icontooltypemsge-icontooltypemsg,d0
	bsr	put_message									* trigger "C" side handling
	move.l	msgptr(pc),a0
	move.l	8(a0),d0								* pointer to found value
	pull	d1-d7/a0-a6
	rts

loadiconmsg	dc.l	AMIGAMSG_ICON_LOAD
	dc.l	0,0,0,0	* name ptr, dest ptr, size in msgptr(pc)+12
loadiconmsge
	even

icon_get_disk_object	push	d1-d7/a0-a6
	lea	loadiconmsg(pc),a0
	pushr   a0
	move.l	dtg_PathArrayPtr(a5),4(a0)		* name ptr
	move.l	chippoint(pc),d2
	move.l	d2,8(a0)						* dest ptr
	pushr	d2
	clr.l	12(a0)
	move.l	#loadiconmsge-loadiconmsg,d0
	bsr	put_message							* call the uade "C" code via message
	move.l	msgptr(pc),a0
	move.l	12(a0),d3
	pullr	d2
	tst.l	d3
	beq.b	loadiconerror
	lea	chippoint(pc),a2					* memory mgmt?
	add.l	d3,(a2)
	and.l	#-16,(a2)
	add.l	#16,(a2)
	pullr	a0
	move.l	8(a0),d0 			* function returns pointer to buffer in d0
loadiconerror
	pull	d1-d7/a0-a6
	rts

loadfilesize
	dc.l 0
loadfilemsg	dc.l	AMIGAMSG_LOADFILE
	dc.l	0,0,0,0,0	* name ptr, dest ptr, size in msgptr(pc)+12, added: destSize ptr
loadfilemsge
load_file_overflow_msg	dc.b	'load file list overflow!',0
	even

enpp_load_file
	LOG_CALL	"eagle:ENPP_LoadFile"
	push	d1-d7/a0-a6
	lea	loadfilemsg(pc),a0
	move.l	dtg_PathArrayPtr(a5),4(a0)
	move.l	chippoint(pc),d2
	move.l	d2,8(a0)
	pushr	d2
	clr.l	12(a0)

	lea	loadfilesize(pc),a2	; fixme: added more robust way to return file size
	clr.l (a2)
	move.l a2,16(a0)

	move.l	#loadfilemsge-loadfilemsg,d0
	bsr	put_message
	move.l	msgptr(pc),a0
	move.l	12(a0),d3
	pullr	d2
	tst.l	d3
	beq.b	loadfileerror
	moveq	#1,d1
	lea	load_file_list(pc),a1
	lea	load_file_list_end(pc),a2
datalistloop2	cmp.l	a1,a2
	bgt.b	no_load_overflow
	lea	load_file_overflow_msg(pc),a0
	bsr	put_string
	bra	dontplay
no_load_overflow
	tst.l	(a1)
	bmi.b	enddatalistloop2
	move.l	(a1),d1
	addq.l	#1,d1
	add	#12,a1
	bra.b	datalistloop2
enddatalistloop2
	move.l	d1,(a1)		* index
	move.l	d2,4(a1)	* ptr
	move.l	d3,8(a1)	* size
	move.l	#-1,12(a1)	* mark end

	lea	chippoint(pc),a2
	add.l	d3,(a2)
	and.l	#-16,(a2)
	add.l	#16,(a2)

loadfileerror	moveq	#0,d0
	tst.l	d3
	seq		d0
	pull	d1-d7/a0-a6
	tst.l	d0
	rts

	include	relocator.s

************************************************************************
*  memoryBlock = AllocMem(byteSize, attributes)
*    d0                     d0        D1
*
exec_alloc_mem
	LOG_CALL	"exec:AllocMem"
	push	d1-d7/a0-a6
	lea	chippoint(pc),a0
	move.l	d0,d2
	move.l	(a0),d0
	move.l	d0,d3
	add.l	d2,d3
	andi.b	#$f0,d3
	addi.l	#16,d3
	move.l	d3,(a0)
	* test if MEMF_CLEAR is set
	btst	#16,d1
	beq.b	nomemclear
	move.l	d2,d1
	beq.b	nomemclear
	move.l	d0,a0
memclearloop
	clr.b	(a0)+
	subq.l	#1,d1
	bne.b	memclearloop
nomemclear	pull	d1-d7/a0-a6
	rts

myfreemem	rts

clearmem	movem.l	d0-d2/a0,-(a7)
	move.l	d0,d1
	lsr.l	#2,d0
	beq.b	noltr1
ltr1	clr.l	(a0)+
	subq.l	#1,d0
	bne.b	ltr1
noltr1	andi	#$3,d1
	subq	#1,d1
	bmi.b	nobs1
ybs1	clr.b	(a0)+
	dbf	d1,ybs1
nobs1	movem.l	(a7)+,d0-d2/a0
	rts

memcopy	push	d0-d1/a0-a1
	move.l	d0,d1
	lsr.l	#2,d0
	beq.b	noltr2
ltr2	move.l	(a0)+,(a1)+
	subq.l	#1,d0
	bne.b	ltr2
noltr2	andi	#$3,d1
	subq	#1,d1
	bmi.b	nobs2
ybs2	move.b	(a0)+,(a1)+
	dbf	d1,ybs2
nobs2	pull	d0-d1/a0-a1
	rts


dos_lock	push	all
	pushr	d1
	move.l	d1,a0
	lea	lastlock(pc),a1
	move.l	#256,d0
	bsr	strlcpy
	pullr	d1
	* d1 = filename
	move.l	#1005,d2	* MODE_OLDFILE
	bsr	dos_open
	tst.l	d0
	beq.b	dos_lock_failure
	move.l	d0,d1
	bsr	dos_close
	pull	all
	move.l	#$f0000000,d0
	rts
dos_lock_failure
	pull	all
	moveq	#0,d0
	rts

dos_currentdir	push	all
	lea	lastlock(pc),a0
	lea	curdir(pc),a1
	move.l	#256,d0
	bsr	strlcpy
	lea	curdirwarning(pc),a0
	bsr	put_string
	pull	all
	rts
curdirwarning	dc.b	'warning: using dos.library/CurrentDir()',0
fixfilewarning	dc.b	'warning: fixfilename',0
	even

* puts CurrentDirectory in front of the name if there is no : character in
* the name. Allocates a new name pointer if necessary.
* filename in a0. returns a (new) name in a0.
dos_fixfilename	push	d0-d7/a1-a6
	move.l	a0,a4
dos_ffn_sloop	move.b	(a0)+,d0
	cmpi.b	#':',d0
	beq.b	dos_ffn_nothing
	tst.b	d0
	bne.b	dos_ffn_sloop
	move.l	#256,d0
	moveq	#0,d1
	bsr	exec_alloc_mem
	move.l	d0,a5
	lea	curdir(pc),a0
	move.l	a5,a1
	move.l	#256,d0
	bsr	strlcpy
	move.l	a5,a0
	bsr	strendptr
	move.l	a0,a1
	move.l	a4,a0
	move.l	#256,d0
	bsr	strlcpy
	move.l	a5,a4
	lea	curdir(pc),a0
	tst.b	(a0)
	beq.b	dos_ffn_nothing
	lea	fixfilewarning(pc),a0
	bsr	put_string
	move.l	a4,a0
	bsr	put_string
dos_ffn_nothing	move.l	a4,a0
	pull	d0-d7/a1-a6
	rts

********************************************************************************
*
*  file = Open( name, accessMode )
*  d0           D1    D2
*
*  file - BCPL pointer to a file handle
*
* contrary to amigaos convention dos_open returns a positive index to the
* file that is opened (zero on failure)	-> pitfall: if there is any code
* that directly wants to work with the "BCPL file handle" (not just passing it
* as-is to some patched dos_lib API, that code will crash miserably)
*
dos_open	push	d1-d7/a0-a6
	move.l	d1,a0
	bsr	dos_fixfilename
	move.l	a0,d1

	lea	.dosopenmsg(pc),a0
	move.l	d1,4(a0)
	clr.l	8(a0)
	clr.l	12(a0)						; 'file exists' marker
	moveq	#.dosopenmsge-.dosopenmsg,d0
	bsr	put_message

	move.l	msgptr(pc),a1
	lea	.dosopenmsg(pc),a0
	move.l	4(a1),4(a0)
	move.l	8(a1),8(a0)
	move.l	12(a1),12(a0)
	tst.l	12(a0)
	bne.b	.dos_open_not_fail
	pull	d1-d7/a0-a6
	moveq	#0,d0
	rts
.dos_open_not_fail
	* get free file index
	lea	dos_file_list(pc),a2
.filelistloop1
	tst.l	(a2)
	beq.b	.fileopenerror
	tst.l	8(a2)
	beq.b	.usethisfileindex
	add	#16,a2
	bra.b	.filelistloop1
.usethisfileindex
	* save a2
	* alloc mem for name
	move.l	#128,d0
	moveq	#0,d1
	bsr	exec_alloc_mem
	lea	.dosopenmsg(pc),a0
	move.l	4(a0),d2	* name
	move.l	d0,d3		* new name space
	move.l	8(a0),d4	* filesize
	* copy file name
	move.l	d2,a0
	move.l	d3,a1
	moveq	#127,d0
	bsr	strlcpy
	move.l	(a2),d0		* get free file index
	clr.l	4(a2)		* clear file offset
	move.l	d3,8(a2)	* put name space ptr
	move.l	d4,12(a2)
	pull	d1-d7/a0-a6
	rts
.fileopenerror
	lea	.tablefullmsg(pc),a0
	bsr	put_string
	pull	d1-d7/a0-a6
	rts
.tablefullmsg	dc.b	'error: file table full',0
	even
.dosopenmsg	dc.l	AMIGAMSG_FILESIZE
	dc.l	0	* file name ptr
	dc.l	0	* file length
	dc.l	0	* file exists (uae returns, see msgptr+12)
.dosopenmsge
dos_file_list	dc.l	1,0,0,0		* index, filepos, filenameptr, filesize
	dc.l	2,0,0,0
	dc.l	3,0,0,0
	dc.l	4,0,0,0
	dc.l	5,0,0,0
	dc.l	0

dos_seek	push	d1-d7/a0-a6
	andi	#15,d1
	subq	#1,d1
	lsl	#4,d1
	lea	dos_file_list(pc),a2
	add	d1,a2
	move.l	8(a2),d0
	bne.b	seek_is_opened
	pull	d1-d7/a0-a6
	moveq	#-1,d0
	rts
seek_is_opened	move.l	4(a2),d0
	cmpi	#1,d3
	bne.b	seek_not_end
	add.l	12(a2),d2
	move.l	d2,4(a2)
	bra.b	seek_done
seek_not_end	cmpi	#-1,d3
	bne.b	seek_not_start
	move.l	d2,4(a2)
	bra.b	seek_done
seek_not_start	tst	d3
	bne.b	seek_not_cur
	add.l	d2,4(a2)
	bra.b	seek_done
seek_not_cur
	pull	d1-d7/a0-a6
	moveq	#-1,d0
	rts
seek_done	pull	d1-d7/a0-a6
	rts


dosreadmsg	dc.l	AMIGAMSG_READ
	* name ptr, dest, offset, length, r. length
	dc.l	0,0,0,0,0
dosreadmsge

***********************************************************************
*    NAME
*	Read -- Read bytes of data from a file
*
*    SYNOPSIS
*	actualLength = Read( file, buffer, length )
*	d0		             D1    D2	   D3
*
*	LONG Read(BPTR, void *, LONG)
*
*  potential issue: D1 is abused for a replacement "handle".. any
*  code actually depending on a "BPTR to filehandle" will crash
*
dos_read	push	d1-d7/a0-a6
	andi	#15,d1
	subq	#1,d1
	lsl	#4,d1
	lea	dos_file_list(pc),a2
	add	d1,a2
	move.l	8(a2),d0
	bne.b	read_is_opened
	pull	d1-d7/a0-a6
	moveq	#0,d0
	rts
read_is_opened	lea	dosreadmsg(pc),a0
	move.l	8(a2),4(a0)		* name ptr
	move.l	d2,8(a0)		* dest
	move.l	4(a2),12(a0)		* offset
	move.l	d3,16(a0)		* length
	clr.l	20(a0)			* clear actually read len
	moveq	#dosreadmsge-dosreadmsg,d0
	bsr	put_message
	move.l	msgptr(pc),a0
	move.l	20(a0),d0
	add.l	d0,4(a2)		* udpate opened file offset
	pull	d1-d7/a0-a6
	rts

dos_close	push	all
	andi	#15,d1
	subq	#1,d1
	lsl	#4,d1
	lea	dos_file_list(pc),a2
	add	d1,a2
	move.l	8(a2),d0
	bne.b	close_is_opened
	bra.b	close_is_not_opened
close_is_opened	clr.l	8(a2)
close_is_not_opened
	pull	all
	moveq	#0,d0
	rts

	include	dos/dos_lib.s

	IFNE	USE_ALT_LOADSEQ
	include	dos/loadseg.s
	ENDC
	IFEQ	USE_ALT_LOADSEQ

loadsegwarnmsg	dc.b	'warning: this deliplayer uses loadseg()/dos.library',0
loadsegerrmsg	dc.b	'loadseg relocation error',0
	even


**************************************************************************
*
*  seglist = LoadSeg( name )
*  d0                 D1
*
*  RESULTS
*     seglist - BCPL pointer to a seglist.. (BCPL pointer is not measured in bytes
*               but in longwords - therefor APTR/4 = BPTR)
*               http://aminet.net/package/dev/misc/BCPL the structure of a seglist
*               is then: 1 long (length of the segment) + 1 long (next segment ptr) +
*				then the data. The BCPL pointer then always points to the "next
*               segment ptr" field.
*               also see https://wiki.amigaos.net/wiki/AmigaDOS_Data_Structures
*
* ISSUE: unfortunately UADE's dos_loadseg impl is defective.. see alternative
* implementation for comparison: 1) "length" field of the returned SegList incorrectly
* is always 0 2) BSS segments (and maybe others?) and/or the chaining of multiple
* SegList seems to be broken: Example file containing a CODE and a BSS segement should
* result in 2 SeqList structs - one for the CODE and one for the BSS. Each of these
* should start with the 8 bytes header field for the "length" and the "next" pointer.
* And the "next" field of the CODE segment should BCPL point to the address of the
* "next" field of the BSS segment (whose "next" pointer should be 0). UADEs impl
* only returns 1 SeqList with no "next" pointer nor "length".
*

dos_loadseg	push	d1-d7/a0-a6
	push	d0/a0
	lea	loadsegwarnmsg(pc),a0
	bsr	put_string
	pull	d0/a0
	move.l	d1,a0
	tst.b	(a0)
	bne.b	myloadseg_loadfile
	move.l	moduleptr(pc),a0			; note: this handling is not part of the new impl (issue?)
	bra.b	myloadseg_noloading
myloadseg_loadfile

	lea	loadfilemsg(pc),a0
	move.l	d1,4(a0)
	move.l	chippoint(pc),8(a0)			; original impl

	; added for robust "file length" retrieval
	lea	loadfilesize(pc),a2
	clr.l (a2)
	move.l a2,16(a0)

	move.l	#loadfilemsge-loadfilemsg,d0
	bsr	put_message

	; ISSUE: the "anti pattern" of using the "msgptr" to retrieve results that
	; the C code has written directly into its "msg-buffer copy" is FRAGILE SHIT:
	; it is fragile since any use of put_string/put_value (e.g. for debug outout)
	; triggers a AMIGAMSG_GENERALMSG which will overwrite the "msgptr" and render the
	; original response data unuseable. better add a separate destination pointer
	; field for the length of the loaded file to the loadfilemsg

	move.l	chippoint(pc),d0
	move.l	d0,a0
	pushr	a0

	; at this point both of the below two impls still return the correct length,
	; but adding a (temporary) put_value debug output on the line above would immediately
	; corrupt the 1st impl
;	move.l	msgptr(pc),a0
;	move.l	12(a0),d1
	lea	loadfilesize(pc),a0
	move.l (a0),d1				; src file length in bytes

	add.l	d1,d0
	andi.l	#-16,d0				; WTF is this? seglist needs 8 additional bytes + <=3 for proper alignment
	addi.l	#16,d0
	lea	chippoint(pc),a0
	move.l	d0,(a0)
	pullr	a0					; original input file buffer
myloadseg_noloading

	bsr	relocator
	tst.l	d0
	beq.b	loadsegsuccess
	lea	loadsegerrmsg(pc),a0
	bsr	put_string
	pull	d1-d7/a0-a6
	moveq	#0,d0
	rts
loadsegsuccess	move.l	a0,d0
	subq.l	#4,d0
	lsr.l	#2,d0
	pull	d1-d7/a0-a6
	tst.l	d0

	; see http://amigadev.elowar.com/read/ADCD_2.1/Includes_and_Autodocs_3._guide/node0194.html
	; result is returned in BOTH d0 AND d1...

	move.l d0,d1
	rts

	ENDIF

exec_supervisor_msg	dc.b	'warning: supervisor called',0
exec_super_state_msg	dc.b	'warning: superstate called',0
	even
exec_supervisor
	LOG_CALL	"exec:Supervisor"
	push	all
	lea	exec_supervisor_msg(pc),a0
; testcase: Archon & MarbleMadness do call this..	
;	bsr	put_string
	pull	all
	move.l	a5,TRAP_VECTOR_0
	trap	#0

	; FIXME: above impl is incorrect and causes problems with the "original"
	; MT impl. 	here is what the a5 program should be seeing on the stack:

	;	or.w		#$2000,sr
	;	subq.l		#8,sp
	;	move		sr,(sp)
	;	move.l		pointer to a dummy "rts" prog,2(sp)
	;	move.w		#$20,6(sp)        ; 68010/020 stack frame type

	; the original MT logic check the sr info on the stack to check
	; if Supervisor mode was reached..
	rts

exec_super_state
	LOG_CALL	"exec:SuperState"
	push	all
	lea	exec_super_state_msg(pc),a0
	bsr	put_string
	lea	superstate_kick(pc),a0
	move.l	a0,TRAP_VECTOR_0
	pull	all
	trap	#0
	illegal

superstate_kick	move.l	a7,d0		* d0 = SSP
	move.l	usp,a7
	rts

exec_userstate
	LOG_CALL	"exec:UserState"
	move.l	a7,usp
	move.l	d0,a7
	move	#0,sr
	rts

timername1	TIMERNAME
audioname1	AUDIONAME
tderrmsg1	dc.b	'OpenDevice: unknown device',0
tderrmsg2	dc.b	'OpenDevice: only timer.device UNIT_VBLANK supported',0
tdmsg1	dc.b	'timer.device UNIT_VBLANK was opened',0
	even


tdnotimplmsg dc.b 'timer.device BeginIO - CMD not implemented',0
tdhackmsg dc.b 'timer.device BeginIO/TR_ADDREQUEST using hack',0
	even

timersigmask	dc.l 0
timersigtask	dc.l 0
timerioreq		dc.l 0


; fixme: see redundancy to sendIO
; input a1=ioRequest
timer_beginio
	push d0/a0-a2

	move.w IO_COMMAND(a1),d0
	cmp.w #TR_ADDREQUEST,d0
	beq skiptbi
unimpltbi
	lea tdnotimplmsg(pc),a0
	bsr put_string

	pull d0/a0-a2
	push d1/a0-a2

	bra endtbi
skiptbi

	moveq #0,d0
	move.b MP_FLAGS(a1),d0

	cmp.l #PA_SIGNAL,d0
	bne unimpltbi

	; testcase: SkyFox - uses timer signaling to wake some
	; task with 60fps freq. hack: Instead of using the actually
	; specified timing (see TV_MICRO(a1)) this impl presumes that
	; the same 60fps already used by EaglePlayer's Interrupt can
	; can be reused here instead.

	lea tdhackmsg(pc),a0
	bsr put_string

	lea timerioreq(pc),a2
	move.l a1,(a2)

;	lea IOTV_TIME(a1),a2			; could check this for $411A to be sure...
;	move.l TV_MICRO(a2),d0
;	bsr put_value

	pull d0/a0-a2
	push d1/a0-a2

	move.l MN_REPLYPORT(a1),d0
	beq endtbi

	move.l d0,a1

	move.b		MP_SIGBIT(a1),d1
	moveq		#0,d0
	bset		d1,d0

	lea timersigmask(pc),a0
	move.l d0,(a0)
;	bsr put_value

	lea timersigtask(pc),a0
	move.l MP_SIGTASK(a1),d0
	move.l d0,(a0)
;	bsr put_value

endtbi
	pull d1/a0-a2
	rts

*******************************************************************
*
*  error = OpenDevice(devName, unitNumber, iORequest, flags)
*  d0                 a0       d0          a1         D1
*
exec_open_device
	LOG_CALL	"exec:OpenDevice"
	push	all
	push	d0/a1
	lea	timername1(pc),a1	* timer.device support
	bsr	strcmp
	tst.l	d0
	beq.b	istimerdev
	lea	audioname1(pc),a1
	bsr	strcmp
	tst.l	d0
	beq.b	isaudiodev
	pull	d0/a1
	lea	tderrmsg1(pc),a0
	bsr	put_string
	bra	dontplay

isaudiodev	pull	d0/a1
	bra openaudiodev

istimerdev	pull	d0/a1

	lea	timer_device_base(pc),a6	; testcase: SkyFox, etc
	LVO_JMP DEV_BEGINIO,	timer_beginio
	move.l	a6,IO_DEVICE(a1)

	cmpi.b	#UNIT_VBLANK,d0
	beq.b	novblerr
	lea	tderrmsg2(pc),a0
	bsr	put_string
	bra	dontplay
novblerr	move.l	d0,IO_UNIT(a1)
	lea	tdmsg1(pc),a0
	bsr	put_string
	pull	all
	push	all
	lea	timerioptr(pc),a0
	move.l	a1,(a0)
	lea	vblanktimerbit(pc),a0
	st	(a0)
	lea	vblanktimerstatusbit(pc),a0
	clr.l	(a0)
	pull	all
	clr.b	IO_ERROR(a1)		* no error
	moveq	#0,d0
	rts


*******************************************************************
*
* error = DoIO(iORequest)
* D0           A1
*
* waits until the I/O request is fully complete!
*
*  testcase: cust.Astate
*

doiowarnmsg	dc.b	'warning: doing fake exec.library/DoIO',0
	even

exec_doio
	LOG_CALL	"exec:DoIO"
	push	all

	lea	audio_device_base(pc),a6
	cmp.l	IO_DEVICE(a1),a6
	bne.b doiofake

	cmp	#11,IO_COMMAND(a1)	; ADCMD_FINISH
	bne.b	doiofake

	bsr adbeginio	; ADCMD_FINISH used in the testcase should already be synchronous
					; so it shouldn't matter that this impl here ignores completion
	bra doiodone
doiofake:
	lea	doiowarnmsg(pc),a0
	bsr	put_string
doiodone:
	pull	all
	clr.b	IO_ERROR(a1)		* no error
	moveq	#0,d0
	rts


*******************************************************************
*
* void SendIO(struct IORequest *);
*             A1
*
* returns control without waiting
*

sendioerrmsg1	dc.b	'SendIO(): Unknown IORequest pointer...',0
sendioerrmsg2	dc.b	'SendIO(): Unknown IORequest command',0
sendiomsg1	dc.b	'SendIO(): TR_ADDREQUEST',0
	even

exec_sendio
	LOG_CALL	"exec:SendIO"
	push	all
	lea	timerioptr(pc),a0
	move.l	(a0),a0
	cmp.l	a0,a1
	beq.b	itstimerdev_1
	lea	sendioerrmsg1(pc),a0
	bsr	put_string
	bra	dontplay
itstimerdev_1	cmp	#TR_ADDREQUEST,IO_COMMAND(a1)
	beq.b	itstraddreq
	lea	sendioerrmsg2(pc),a0
	bsr	put_string
	bra	dontplay
itstraddreq
	* check if sendio general msg has already been sent once
	lea	sendiomsgbit(pc),a0
	tst	(a0)
	bne.b	dontsendiomsg
	st	(a0)
	lea	sendiomsg1(pc),a0
	bsr	put_string
dontsendiomsg	pull	all
	push	all
	move.l	IOTV_TIME+TV_SECS(a1),d0
	move.l	IOTV_TIME+TV_MICRO(a1),d1
	mulu	vbi_hz(pc),d0
	move.l	#1000000,d2
	divu	vbi_hz(pc),d2	* frame time in microsecs
	divu	d2,d1
	ext.l	d1
	add.l	d1,d0
	lea	vblanktimercount(pc),a0
	move.l	d0,(a0)
	lea	vblanktimerstatusbit(pc),a0
	st	(a0)
	lea	vblanktimerfunc(pc),a2
	move.l	MN_REPLYPORT(a1),a0

	move.l	MP_SIGTASK(a0),a0	; a0=object to be signaled (i.e. a Task)
	move.l	$12(a0),(a2)		; TC_SIGALLOC(a0) WTF? how can this ever be a valid function ptr!
	pull	all
	clr.b	IO_ERROR(a1)		* no error
	rts


waitiomsg1	dc.b	'warning: WaitIO is not implemented!',0
abortiomsg1	dc.b	'AbortIO is not implemented!',0
	even

exec_waitio
	LOG_CALL	"exec:WaitIO"
	push	all
	lea	waitiomsg1(pc),a0
	bsr	put_string
	pull	all
	clr.b	IO_ERROR(a1)		* no error
	moveq	#0,d0
	rts

********************************************************************
*
*  AbortIO(iORequest)
*	       a1
*
myabortio	push	all
	move.l	IO_DEVICE(a1),a6
	move.l a6,d0
	cmp.l #$0,d0
	beq .oldabort
	jsr DEV_ABORTIO(a6)
	pull	all
	rts
.oldabort
	lea	abortiomsg1(pc),a0
	bsr	put_string
	pull	all
	moveq	#0,d0
	rts


* exec.library: _LVOCause (a1 = struct Interrupt *)
exec_cause
	LOG_CALL	"exec:Cause"
	push	all

	cmp.b     #NT_SOFTINT,LN_TYPE(a1)
	beq.s     not_soft_irq          ; if node type is already NT_SOFTINT do nothing.


	move.b    #NT_SOFTINT,LN_TYPE(a1)


	move.l	IS_CODE(a1),d0			; Interrupt code..
	beq.b	not_soft_irq

	move.l	IS_DATA(a1),a3			; Interrupt data was missing in UADE!
	move.l	d0,a5					; IS_CODE should be in a5!
	move	#$2100,d0
	lea	trapcall(pc),a4
	move.l	a4,TRAP_VECTOR_3
	trap	#3
not_soft_irq	pull	all
	rts

trapcall	move	d0,sr

	move.b    #NT_INTERRUPT,LN_TYPE(a1)

	move.l a3,a1	; Interrupt code expects the data in A1!
	move.l 4.w,a6	; just in case
	lea	$dff000,a0	; testcase: stoneplayer.library

	jsr	(a5)

	move	#$2000,sr
	rte


dos_library_name	DOSNAME
uade_library_name	dc.b	'uade.library',0
icon_library_name	dc.b	'icon.library',0
graphics_library_name	dc.b	'graphics.library',0
utility_library_name	dc.b	'utility.library',0
	even


********************************************************************
*
*  library = OpenLibrary(libName, version)
*  d0                    a1       d0
*
exec_old_open_library
exec_open_library
	LOG_CALL	"exec:OpenLibrary"
;	bsr put_value		; lets see what version is asked for

	push	d1-d7/a0-a6
	moveq	#0,d0

	lea	graphics_library_name(pc),a0
	bsr	strcmp
	tst.l	d0
	bne.b	not_graphics_lib
	bsr	send_open_lib_msg

	lea	graphics_lib_base(pc),a0
	move.l	a0,d0
	bra	return_open_lib
not_graphics_lib
	lea	utility_library_name(pc),a0
	bsr	strcmp
	tst.l	d0
	bne.b	not_util_lib
	bsr	send_open_lib_msg
	lea	utility_lib_base(pc),a0
	move.l	a0,d0
	bra.b	return_open_lib
not_util_lib
	lea	dos_library_name(pc),a0
	bsr	strcmp
	tst.l	d0
	bne.b	not_dos_lib
	bsr	send_open_lib_msg

	IFNE	CRASH_ON_UNIMPLEMENTED
	; print base addr to ease calculation of crash offsets
	lea	doslibbasemsg(pc),a0
	bsr put_string
	lea	dos_lib_base(pc),a0
	move.l	a0,d0
	bsr put_value
	ENDC

	lea	dos_lib_base(pc),a0
	move.l	a0,d0
	bra.b	return_open_lib
not_dos_lib
	lea	uade_library_name(pc),a0
	bsr	strcmp
	tst.l	d0
	bne.b	not_uade_lib
	bsr	send_open_lib_msg
	lea	uade_lib_base(pc),a0
	move.l	a0,d0
	bra.b	return_open_lib
not_uade_lib
	lea	icon_library_name(pc),a0
	bsr	strcmp
	tst.l	d0
	bne.b	not_icon_lib
	bsr	send_open_lib_msg
	lea	icon_lib_base(pc),a0
	move.l	a0,d0
	bra.b	return_open_lib
not_icon_lib

	move.l	a1,a0

	; just just generic lib loader
	bsr genericlibload

return_open_lib
	move.l a1,LIB_IDSTRING(a0)
	move.w #$101,LIB_OPENCNT(a0)	; hack: don't want anyone to expunge "unused" library

	pull	d1-d7/a0-a6
	tst.l	d0
	rts

send_open_lib_msg	push	all
	lea	openlibname(pc),a1
	moveq	#31,d0
	bsr	strlcpy
	lea	openlibmsg(pc),a0
;	bsr	put_string
	pull	all
	rts

openlibmsg	dc.b	'open: '
openlibname	dcb.b	32,0
openlibwarnmsg	dc.b	'warning: couldnt open library '
openlibwarnname	dcb.b	32,0
	even

liboffscheck	pushr	d0
	move.l	8(a7),d0
	push	all
	move.l	d0,a0
	subq.l	#4,a0
	move	(a0),d0
	andi	#$4ea0,d0
	cmpi	#$4ea0,d0
	bne.b	nolibjsr
	move	2(a0),d0
	ext.l	d0
	neg.l	d0
	bsr	put_value
nolibjsr	pull	all
	pullr	d0
	rts

intuiwarn	bsr	liboffscheck
	push	all
	lea	intuiwarnmsg(pc),a0
	bsr	put_string
	pull	all
	rts
intuiwarnmsg	dc.b	'warning: intuition library function not implemented',0
	even

iconwarn	bsr	liboffscheck
	push	all
	lea	iconwarnmsg(pc),a0
	bsr	put_string
	pull	all
	rts
iconwarnmsg	dc.b	'warning: icon.library function not implemented',0
	even

utilitywarn	bsr	liboffscheck
	push	all
	lea	utilitywarnmsg(pc),a0
	bsr	put_string
	pull	all
	rts
utilitywarnmsg	dc.b	'warning: utility.library function not implemented',0
	even

graphicswarn	bsr	liboffscheck
	push	all
	lea	graphicswarnmsg(pc),a0
	bsr	put_string
	pull	all
	rts
graphicswarnmsg	dc.b	'warning: graphics.library function not implemented',0
	even

* a0 base, a1 warn funct, d0 = abs(minimum offset)
init_lib_base
	push	all
	move.l	d0,d6
	moveq	#6,d0
	subq.l	#6,a0
initlibloop	cmp.l	d6,d0
	bgt.b	endlibloop
	move	jmpcom(pc),(a0)
	move.l	a1,2(a0)
	subq.l	#6,a0
	addq.l	#6,d0
	bra.b	initlibloop
endlibloop	pull	all
	rts

***********************************************************************
*  attributes = TypeOfMem(address)
*    d0                     a1
*
* testcase: stonetracker
*
exec_typeofmem
	LOG_CALL	"exec:TypeOfMem"
	move.l	a1,d0
	bmi.b	.exec_typeofmem_fail
	cmpi.l	#$200000,d0
	bge.b	.exec_typeofmem_fail
	moveq	#3,d0			* MEMF_PUBLIC | MEMF_CHIP
	rts
.exec_typeofmem_fail
	moveq	#0,d0
	rts


* intuition_library:AllocRemember
intuition_alloc_remember
	move.l	#$666,(a0)	* mark success ;-)
	bra	exec_alloc_mem

uade_time_critical
	push	all
	lea	uade_tc_msg(pc),a0
	move.l	d0,4(a0)
	moveq	#8,d0
	bsr	put_message
	pull	all
	rts

uade_tc_msg	dc.l	AMIGAMSG_TIME_CRITICAL,0

uade_get_info
	push	d1-d7/a0-a6
	lea	uade_gi_msg(pc),a2
	move.l	a0,4(a2)
	move.l	a1,8(a2)
	move.l	d0,12(a2)
	move.l	a2,a0
	moveq	#16,d0
	bsr	put_message
	move.l	msgptr(pc),a0
	move.l	12(a0),d0
	pull	d1-d7/a0-a6
	rts

* a0 = option array for returned values
* array format:
* dc.l	opttype,optname,optreceived,optvalue
* opttype = 0 (end of list), 1 (string), 2 hexadecimal value (at most 32 bits),
*           3 (flag)
*
* optname is a pointer to the name of the value
*
* optreceived is a pointer to a byte that indicates whether or not that
* option was found. 0 means not found, otherwise found.
*
* optvalue is a pointer to a place where the value should be stored.
* if option type is "flag", optvalue is not needed, because optreceived
* already indicates if that flag was received. "hexadecimal value" is
* returned as a 32 bit integer despite the length of hexadecimal string
* given by the user.
uade_new_get_info
	push	d1-d7/a0-a6
	move.l	a0,a6			* option array for eagleplayer
	lea	uade_gi_msg(pc),a2
	lea	ep_opt_request(pc),a0
	move.l	a0,4(a2)
	lea	uadelibmsg(pc),a1
	move.l	a1,8(a2)
	move.l	#EP_OPT_SIZE,d0
	move.l	d0,12(a2)
	move.l	a2,a0
	moveq	#16,d0
	bsr	put_message
	move.l	msgptr(pc),a0	* fetch result
	move.l	12(a0),d0
	tst.l	d0
	ble.b	no_info

	move.l	d0,d2

	lea	uadelibmsg(pc),a3
get_ep_info_loop
	move.l	a6,a2
get_ep_info_loop2
	move.l	(a2)+,d7	* opt type
	beq.b	end_get_ep_info_loop2

	; the eagleoption in eagleoptlist can be shorter than the
	; actual given option given from uade. this happens when
	; eagleoption gets a value, like ciatempo=150. to handle
	; this, eagleopt list would only contain "ciatempo" and
	; the following code only compares strlen("ciatempo")
	; amount of bytes from the data given from uade.
	move.l	(a2)+,a0	* opt name
	bsr	strlen

	move.l	(a2)+,a4	* opt received
	
	move.l	(a2)+,a5	* opt value

	; We now have eagleopt length in d0 and the eagle option name
	; pointer is still in a0 (strlen preservers registers).
	; Do a limited string comparison against the eagleoption
	; and data given from uade
	move.l	a3,a1		; uade lib message
	bsr	strncmp
	bne.b	get_ep_info_loop2

	; woah! equal! celebrate it by spamming the user:
	move.l	a3,a0
	bsr	put_string

	; string options may not be longer than 31 characters (32 bytes with
	; null byte)
	move.l	a3,a0
	bsr	strlen
	cmpi.l	#31,d0
	bgt	invalid_ep_opt

	;  mark option as received
	st	(a4)

	cmpi	#1,d7
	bne.b	ep_opt_not_a_string
	move.l	a3,a0
	move.l	a5,a1
	moveq	#32,d0
	bsr	strlcpy
	bra.b	end_get_ep_info_loop2
ep_opt_not_a_string
	cmpi	#2,d7
	bne.b	end_get_ep_info_loop2
	; skip "foo=" string for a hexadecimal value, where foo is an option
	; name
	move.l	-12(a2),a0	;  get option name
	bsr	strlen
	addq.l	#1,d0		;  add one more to skip '='
	lea	(a3,d0),a0
	bsr	hex2int
	move.l	d0,(a5)

end_get_ep_info_loop2
	; get next option from response, or quit if no more eagleoptions.
	move.l	a3,a0
	bsr	strlen
	addq.l	#1,d0
	add.l	d0,a3
	sub.l	d0,d2
	bpl.b	get_ep_info_loop

	pull	d1-d7/a0-a6
	moveq	#1,d0
	rts
no_info	pull	d1-d7/a0-a6
	moveq	#0,d0
	rts

invalid_ep_opt
	lea	invalid_ep_opt_msg(pc),a0
	bsr	put_string
	bra	end_get_ep_info_loop2

uade_gi_msg	dc.l	AMIGAMSG_GET_INFO,0,0,0
ep_opt_request	dc.b	'eagleoptions',0
invalid_ep_opt_msg	dc.b	'invalid ep option received',0

ciareswarnmsg	dc.b	'exec.library/OpenDevice: unknown resource',0
	even

*********************************************************************************
*
*  resource = OpenResource(resName)
*  D0	                   A1
*
exec_open_resource
	LOG_CALL	"exec:OpenResource"
	cmp.b	#'c',(a1)
	bne.b	nociabresource
	cmp.b	#'i',1(a1)
	bne.b	nociabresource
	cmp.b	#'a',2(a1)
	bne.b	nociabresource
	cmp.b	#'b',3(a1)
	beq.b	isciabresource
	cmp.b	#'a',3(a1)
	beq.b	isciaaresource
nociabresource	lea	ciareswarnmsg(pc),a0
	bsr	put_string
	moveq	#0,d0
	rts


; see "Amiga Intern (Data Becker)":
;Der Aufruf der Funktionen kann nur fehlerfrei erfolgen , wenn der
;Zeiger auf die Resource-Struktur in A6 steht. Mit move.b #$02,d0
;move.l ResourceBase,a6
;jsr -18(a6)

isciaaresource	lea	ciaaresjmptab(pc),a0
	lea	illegalciaaresource(pc),a1
	moveq	#10-1,d0
ciaaresjmptabl	move.l	a1,2(a0)
	addq.l	#6,a0
	dbf	d0,ciaaresjmptabl
	lea	ciaaresource(pc),a0
	lea	sys_add_ciaa_interrupt(pc),a1
	move.l	a1,_LVOAddICRVector+2(a0)
	lea	rem_ciaa_interrupt(pc),a1
	move.l	a1,_LVORemICRVector+2(a0)
	lea	ciaaresource(pc),a0
	move.l	a0,d0
	rts

isciabresource
	lea	ciabresjmptab(pc),a0
	lea	illegalciabresource(pc),a1
	moveq	#10-1,d0
ciabresjmptabl	move.l	a1,2(a0)
	addq.l	#6,a0
	dbf	d0,ciabresjmptabl
	lea	ciabresource(pc),a0
	lea	add_ciab_interrupt(pc),a1
	move.l	a1,_LVOAddICRVector+2(a0)
	lea	rem_ciab_interrupt(pc),a1
	move.l	a1,_LVORemICRVector+2(a0)
	lea	ciab_seticr(pc),a1
	move.l	a1,_LVOSetICR+2(a0)
	lea	ciab_ableicr(pc),a1
	move.l	a1,_LVOAbleICR+2(a0)
	lea	ciabresource(pc),a0
	move.l	a0,d0
	rts

ciab_ableicr
ciab_seticr	push	all
	lea	icrwarnmsg(pc),a0
	bsr	put_string
	pull	all
	moveq	#0,d0
	rts

icrwarnmsg	dc.b	'warning: not implemented ciab.resource/SetICR or '
	dc.b	'AbleICR was used',0
illciaamsg	dc.b	'ciaaresource: resource is not implemented!', 0
illciabmsg	dc.b	'ciabresource: resource is not implemented!', 0
	even
illegalciaaresource	push	all
	lea	illciaamsg(pc),a0
	bra.b	disillciamsg
illegalciabresource	push	all
	lea	illciabmsg(pc),a0
disillciamsg	bsr	put_string
	pull	all
	rts

ciaaresjmptab	rept	10
	jmp	0
	endr
ciaaresource
ciabresjmptab	rept	10
	jmp	0
	endr
ciabresource
	dcb.b LIB_SIZE+88,0		; space used by the real cia-resource structure (alloc space just in case)



sys_add_ciaa_interrupt
	moveq	#-1,d0
	rts

* AddICRVector() for ciaa.resource and ciab.resource
* sets hw int vector, cia registers, and enables a ciab interrupt
*
* deliciabdata is passed in a1 to deliciabint function
*
* SHOULD WE READ deliciabdata from interrupt structure every time we do the
* interrupt?


**********************************************************
* AddICRVector(resource,iCRBit,interrupt)
*              a6       d0     a1
*
add_ciab_interrupt
	push	all
	lea	ciabdatas(pc),a2
	lea	ciabints(pc),a3
	lea	$bfd000,a4
	move	#$2000,d2
	lea	ciab_interrupt(pc),a5
	move.l	a5,$78.w
	bra.b	add_cia_interrupt

add_ciaa_interrupt
	push	all
	lea	ciaadatas(pc),a2
	lea	ciaaints(pc),a3
	lea	$bfe001,a4
	move	#$0008,d2
	lea	ciaa_interrupt(pc),a5
	move.l	a5,$68.w
add_cia_interrupt
	moveq	#1,d1
	and.l	d0,d1
	lsl	#2,d1
	move.l	IS_DATA(a1),(a2,d1)
	move.l	IS_CODE(a1),(a3,d1)

	lea	eaglebase(pc),a5
	move	dtg_Timer(a5),d1

	btst	#0,d0
	bne.b	bit0one
	move.b	d1,$400(a4)		* Set A Timer value
	rol	#8,d1
	move.b	d1,$500(a4)
	move.b	#$81,$d00(a4)		* set timer A ON
	move.b	#$11,$e00(a4)		* A timer on
	bra.b	bit0zero
bit0one	move.b	d1,$600(a4)		* Set B Timer value
	rol	#8,d1
	move.b	d1,$700(a4)
	move.b	#$82,$d00(a4)		* set timer B ON
	move.b	#$11,$f00(a4)		* B timer on
bit0zero
	ori	#$8000,d2
	move	d2,intena+custom	* enable cia interrupt
	pull	all
	moveq	#0,d0
	rts


rem_ciaa_interrupt
	push	all
	lea	ciaadatas(pc),a2
	lea	ciaaints(pc),a3
	lea	$bfe001,a4
	bra.b	ciaremint

rem_ciab_interrupt
	push	all
	lea	ciabdatas(pc),a2
	lea	ciabints(pc),a3
	lea	$bfd000,a4
ciaremint	andi	#1,d0
	moveq	#1,d1
	lsl	d0,d1
	move.b	d1,$d00(a4)		* Disable A or B timer in ICR
	lsl	#2,d0
	move.l	#0,(a2,d0)
	move.l	#0,(a3,d0)
	pull	all
	moveq	#0,d0
	rts


* register setup for calling ciab interrupt
* a1 = ciab interrupt data pointer
* a6 = exec base
* IS THIS RIGHT? Should we hit intreq+custom after the interrupt is executed?

ciaa_interrupt	push	all
	move	#$0008,intreq+custom * quit the int to be sure
	move.b	$bfed01,d0	* quit int (reading should do it)
	andi	#3,d0
	btst	#0,d0
	beq.b	ciaa_not_timer_a
	move.l	ciaaints(pc),a0
	move.l	ciaadatas(pc),a1
	move.l	4.w,a6
	move.l	d0,-(a7)
	jsr	(a0)
	move.l	(a7)+,d0
ciaa_not_timer_a
	btst	#1,d0
	beq.b	ciaa_not_timer_b
	move.l	ciaaints+4(pc),a0
	move.l	ciaadatas+4(pc),a1
	move.l	4.w,a6
	move.l	d0,-(a7)
	jsr	(a0)
	move.l	(a7)+,d0
ciaa_not_timer_b
	pull	all
	rte

ciab_interrupt	push	all
	move	#$2000,intreq+custom * quit the int to be sure
	move.b	$bfdd00,d0	* quit int (reading should do it)
	andi	#3,d0
	btst	#0,d0
	beq.b	ciab_not_timer_a
	move.l	ciabints(pc),a5		; handler expects this in a5!
	move.l	ciabdatas(pc),a1
	move.l	4.w,a6
	move.l	d0,-(a7)
	lea	$dff000,a0	; testcase: stoneplayer.library
	jsr	(a5)

	move.l	(a7)+,d0
ciab_not_timer_a
	btst	#1,d0
	beq.b	ciab_not_timer_b
	move.l	ciabints+4(pc),a5
	move.l	ciabdatas+4(pc),a1
	move.l	4.w,a6
	move.l	d0,-(a7)
	lea	$dff000,a0	; testcase: stoneplayer.library
	jsr	(a5)
	move.l	(a7)+,d0
ciab_not_timer_b
	pull	all
	rte

AMP_ADR_CHANGED	equ	1
AMP_LEN_CHANGED	equ	2
AMP_PER_CHANGED	equ	4
AMP_VOL_CHANGED	equ	8

	rsreset
amp_adr	rs.l	1
amp_len	rs.l	1
amp_per	rs	1
amp_vol	rs	1
amp_changes	rs.l	1
amp_entry_size	rs.b	0

amp_dma	dc	0,0	* disable/enable masks
amp_struct	dcb.b	amp_entry_size*4,0

amplifierwarn	dc.b	'warning: amplifier used',0
ampinitfailure	dc.b	'error: amplifier init failed',0
	even

* amplifier_init must be called after DTP_InitPlayer. See Future Composer 1.4
call_amplifier_init
	move.l	amplifier_init_func(pc),d0
	bne.b	is_an_amp
	rts
is_an_amp
	LOG_CALL "score:call_amplifier_init"
	push	all
	lea	amplifierwarn(pc),a0
	bsr	put_string
	move.l	amplifier_init_func(pc),a0
	jsr	(a0)
	tst.l	d0
	beq.b	amp_init_ok
	lea	ampinitfailure(pc),a0
	bsr	put_string
amp_init_ok	pull	all
	rts

amp_is_valid_channel
	cmpi	#4,d1
	bhs.b	amp_not_valid_channel
	rts
amp_chan_warning	dc.b	'warning: illegal chan number (amp)',0
	even
amp_not_valid_channel
	push	all
	lea	amp_chan_warning(pc),a0
	bsr	put_string
	pull	all
	rts

enpp_amplifier_dma
	push	all
	lea	amp_dma(pc),a0
	andi	#$000f,d1
	tst	d0
	bpl.b	amp_dma_not_neg
	ori	#$8000,d1
amp_dma_not_neg	move	d1,dmacon+custom
	tst	d0
	bpl.b	amp_dma_not_neg_2
	bsr	wait_audio_dma
amp_dma_not_neg_2
	pull	all
	rts

enpp_amplifier_adr	push	all
	bsr	amp_is_valid_channel
	move.l	d1,d2
	lsl	#4,d2
	lea	aud0lch+custom,a0
	move.l	d0,(a0,d2)
	pull	all
	rts

enpp_amplifier_len	push	all
	bsr	amp_is_valid_channel
	move.l	d1,d2
	lsl	#4,d2
	lea	aud0len+custom,a0
	move	d0,(a0,d2)
	pull	all
	rts

enpp_amplifier_per	push	all
	bsr	amp_is_valid_channel
	move.l	d1,d2
	mulu	#$10,d2
	lea	aud0per+custom,a0
	move	d0,(a0,d2)
	pull	all
	rts

enpp_amplifier_vol	push	all
	bsr	amp_is_valid_channel
	move.l	d1,d2
	lsl	#4,d2
	lea	aud0vol+custom,a0
	move	d0,(a0,d2)
	pull	all
	rts

amp_com_unk_msg	dc.b	'unknown ENPP_PokeCommand() command',0
amp_com_filt_msg	dc.b	'unknown ENPP_PokeCommand() filter command',0
	even

enpp_amplifier_command
	push	all
	cmpi.l	#1,d0
	bne.b	amp_com_unknown
	cmpi.l	#0,d1
	bne.b	amp_com_not_filt_off
	bset	#1,$bfe001
	bra.b	amp_com_end
amp_com_not_filt_off
	cmpi.l	#1,d1
	bne.b	amp_com_not_filt_on
	bclr	#1,$bfe001
	bra.b	amp_com_end
amp_com_not_filt_on
	cmpi.l	#-1,d1
	bne.b	amp_com_not_filt_toggle
	bchg	#1,$bfe001
	bra.b	amp_com_end
amp_com_not_filt_toggle
	lea	amp_com_filt_msg(pc),a0
	bsr	put_string
	bra.b	amp_com_end
amp_com_unknown	lea	amp_com_unk_msg(pc),a0
	bsr	put_string
amp_com_end	pull	all
	rts

enpp_amplifier_int
	push	all
	lea	amp_struct(pc),a0
	moveq	#4-1,d7
amp_clr_loop	clr.l	amp_changes(a0)
	add	#amp_entry_size,a0
	dbf	d7,amp_clr_loop
	lea	amp_dma(pc),a0
	clr.l	(a0)		* disable / enable
	pull	all
	rts

* Noteplayer initialization *
np_init	push	all
	move.l	noteplayerptr(pc),d0	; this is a "**noteStruct"
	beq	np_not_an_np

	push	all
	lea	noteplayerwarn(pc),a0
	bsr	put_string

	move.l	noteplayersetupfunc(pc),d0
	beq.b	no_np_setup
	LOG_CALL "score:call_noteplayersetupfunc"
	move.l	d0,a0
	jsr	(a0)			; the player directly uses its internal ptr to the struct

	tst.l	d0
	beq.b	no_np_setup
	move.l	#$06660666,d0
	bsr	put_value
no_np_setup
	pull	all
	move.l	d0,a0				; original **noteStruct
	move.l	(a0),a0
	lea	notestructptr(pc),a1
	move.l	a0,(a1)
	move.l	(a0),a1
	lea	np_chanlist(pc),a2
	move.l	a1,(a2)
	moveq	#0,d0
	move	6(a0),d0
	andi	#$0020,d0
	lea	np_longsamples(pc),a2
	move.l	d0,(a2)
	move.l	#$00010000,d2			* short sample (1 word)
	tst	d0
	beq.b	np_not_long
	moveq	#2,d2				* long sample (2 bytes)
np_not_long	lea	np_zerosample(pc),a2
	moveq	#1,d1
np_count_channels
;	move.b	#2+8+$10,npc_modified(a1)	* set sample, per, vol
;	move.l	a2,npc_sampleptr(a1)
;	move.l	d2,npc_samplelen(a1)
;	clr.l	npc_srepeatptr(a1)
;	clr.l	npc_srepeatlen(a1)
;	move	#200,npc_period(a1)
;	move	#64,npc_volume(a1)
	move.l	(a1),d0
	beq.b	np_end_channel_count
	move.l	d0,a1
	addq.l	#1,d1
	bra.b	np_count_channels
np_end_channel_count
	lea	np_chans(pc),a2
	move.l	d1,(a2)
np_not_an_np	pull	all
	rts

noteplayerwarn	dc.b	'noteplayer warning',0
np_multichan_warning	dc.b	'noteplayer error: multichannel song',0
	even

np_warn_once	dc.l	0
np_zerosample	dc.l	0

* noteplayer interrupt routine *
np_int	push	all

	lea	custom,a6

	move.l	np_counter(pc),d0
	bne.b	np_counter_nz
	move	#$000f,dmacon(a6)
	bsr	wait_audio_dma
	lea	np_zerosample(pc),a1
	move.l	a6,a5
	moveq	#4-1,d7
np_zsloop	move.l	a1,aud0lch(a5)
	move	#1,aud0len(a5)
	move	#0,aud0vol(a5)
	move	#200,aud0per(a5)
	add	#$10,a5
	dbf	d7,np_zsloop
	move	#$800f,dmacon(a6)
	bsr	wait_audio_dma
np_counter_nz	lea	np_counter(pc),a0
	addq.l	#1,(a0)

	move.l	np_chanlist(pc),d0
	move.l	np_longsamples(pc),d5
	moveq	#0,d6		* dma on mask
	moveq	#0,d7		* audio channel bit number
	lea	np_chanset(pc),a5
np_int_loop	move.l	d0,a0
	cmp	#$8000,npc_chanpos(a0)
	beq.b	np_not_active
	move.b	npc_modified(a0),d0
	beq.b	np_no_changes

	btst	#1,d0
	beq.b	np_no_sample
	move.l	npc_sampleptr(a0),aud0lch(a6)
	tst.l	d5		* check if long samples
	bne.b	np_l_sample_1
	move	npc_samplelen(a0),aud0len(a6)
	bra.b	np_s_sample_1
np_l_sample_1	move.l	npc_samplelen(a0),d1
	lsr.l	#1,d1
	move	d1,aud0len(a6)
np_s_sample_1	st	(a5,d7)		* set sample repeat boolean
	bset	d7,d6		* set audio channel dma bit
np_no_sample
	btst	#2,d0
	beq.b	np_no_repeat
;	btst	#1,d0
;	bne.b	np_there_was_a_sample
;	move.l	npc_srepeatptr(a0),aud0lch(a6)
;	tst.l	d5		* check if long samples
;	bne.b	np_l_sample_2
;	move	npc_srepeatlen(a0),aud0len(a6)
;	bra.b	np_s_sample_2
;np_l_sample_2	move.l	npc_srepeatlen(a0),d1
;	lsr.l	#1,d1
;	move	d1,aud0len(a6)
;np_s_sample_2	bset	d7,d6		* set audio channel dma bit
;	bra.b	np_no_repeat
;np_there_was_a_sample
	st	(a5,d7)		* set sample repeat boolean
np_no_repeat
	btst	#3,d0
	beq.b	np_no_period
	move	npc_period(a0),aud0per(a6)
np_no_period
	btst	#4,d0
	beq.b	np_no_volume
	move	npc_volume(a0),aud0vol(a6)
np_no_volume
np_no_changes	clr.b	npc_modified(a0)
	add	#$10,a6
	addq	#1,d7
np_not_active	move.l	(a0),d0
	bne	np_int_loop

	cmpi	#4,d7
	ble.b	np_4_chan

	push d0
	lea np_warn_once(pc),a0
	move.l (a0),d0
	bne .np_skip_warn

	move.l #1,(a0)
	lea	np_multichan_warning(pc),a0
	bsr	put_string
.np_skip_warn
	pull d0


	andi	#$000f,d6
np_4_chan
	tst.b	d6
	beq.b	np_no_dma_set
	move	d6,dmacon+custom
	bsr	wait_audio_dma
	ori	#$8000,d6
	move	d6,dmacon+custom	* set relevant audio channels
np_no_dma_set
	move.l	np_longsamples(pc),d5
	lea	np_chanset(pc),a5
	move.l	np_chanlist(pc),d0
	lea	custom,a6
	moveq	#0,d7			* audio dma wait boolean
np_rloop	move.l	d0,a0
	cmp	#$8000,npc_chanpos(a0)
	beq.b	np_not_active_2
	tst.b	(a5)			* check if audxlch was set
	beq.b	np_no_repeat_2		* not set => dont set repeat
	move.l	npc_srepeatptr(a0),d0
	beq.b	np_no_repeat_2
	tst.l	d7		* check if audio dma has been waited
	bne.b	np_r_no_wait	* (audio dma should waited only once)
	st	d7
	bsr	wait_audio_dma
np_r_no_wait	move.l	d0,aud0lch(a6)
	tst.l	d5		* check if long samples
	bne.b	np_l_sample_3
	move	npc_srepeatlen(a0),aud0len(a6)
	bra.b	np_s_sample_3
np_l_sample_3	move.l	npc_srepeatlen(a0),d1
	lsr.l	#1,d1
	move	d1,aud0len(a6)
np_s_sample_3
np_no_repeat_2	clr.b	(a5)+			* clear channel sample repeat
	add	#$10,a6
np_not_active_2	move.l	(a0),d0
	bne.b	np_rloop
	pull	all
	rts


init_interrupts	push	all
	lea	mylevel2(pc),a0		* set CIAA int vector
	move.l	a0,$68.w
	lea	mylevel3(pc),a0		* set VBI vector
	move.l	a0,$6c.w
	lea	mylevel6(pc),a0		* set CIAB int vector
	move.l	a0,$78.w
	move	#$c000,d0
	ori	#$0020,d0		* enable CIAA, CIAB and VBI
	move	d0,intena+custom

	lea	handlertab(pc),a0
	lea	mylevel1(pc),a1
	move.l	a1,(a0)+
	move.l	a1,(a0)+
	move.l	a1,(a0)+
	lea	mylevel2(pc),a1
	move.l	a1,(a0)+
	lea	mylevel3(pc),a1
	move.l	a1,(a0)+
	move.l	a1,(a0)+
	move.l	a1,(a0)+
	lea	mylevel4(pc),a1
	move.l	a1,(a0)+
	move.l	a1,(a0)+
	move.l	a1,(a0)+
	move.l	a1,(a0)+
	lea	mylevel5(pc),a1
	move.l	a1,(a0)+
	move.l	a1,(a0)+
	lea	mylevel6(pc),a1
	move.l	a1,(a0)+
	pull	all
	rts

intvecmsg	dc.b	'setintvector(): Tried to set unauthorized interrupt '
	dc.b	'vector !',0
	even

********************************************************************************
*
*  oldInterrupt = SetIntVector(intNumber, interrupt)
*  D0			               D0         A1
*
* testcase: FaceTheMusic
*			stoneplayer.library sets 4 vectors (intNumber: 7-10)
*
exec_set_int_vector
	LOG_CALL	"exec:SetIntVector"
	push	d1-d7/a0-a6
	cmpi	#7,d0		* validity check
	blt.b	int_vec_error
	cmpi	#10,d0
	ble.b	int_level_ok
int_vec_error	lea	intvecmsg(pc),a0
	bsr	put_string
	bra	dontplay
int_level_ok
	move	#$8000,d6
	bset	d0,d6		* enabling value for intena
	lea	vectab(pc),a0	* table containing vector addresses
	lea	irqlines(pc),a2	* table containing vectors
	lea	handlertab(pc),a3 * table containing my handlers
	lea	isdatapointers(pc),a4

	add	d0,d0
	move	(a0,d0),d2
	andi.l	#$ff,d2		* get int vec address
	move.l	d2,a5		* a5 = hw interrupt pointer
				* eg. $6C == VBI interrupt pointer
	add	d0,d0
	move.l	(a3,d0),(a5)	* set my own int handler as hw int ptr

; A7=stackpointer... using stack as temp storage
	move.l	(a4,d0),-(a7)	* get old IS_DATA (webUADE added)
	move.l	(a2,d0),-(a7)	* get old int vector

	move.l	IS_DATA(a1),(a4,d0)	* copy is_data pointer
	move.l	IS_CODE(a1),(a2,d0)	* put new vector into list

	lea	oldstructs(pc),a0
	add	d0,a0

	; UADE's original handling again missing the IS_DATA handling...
	move.l	(a7)+,IS_CODE(a0)
	move.l	(a7)+,IS_DATA(a0)		; webUADE added
	move.l	a0,d0		* return old int structure

	move	d6,intena+custom

	pull	d1-d7/a0-a6
	rts

addintmsg	dc.b	'addintserver(): Tried to add unauthorized interrupt '
	dc.b	'server !',0
	even

exec_add_int_server
	LOG_CALL	"exec:AddIntServer"
	push	all
	cmpi.b	#5,d0
	beq.b	servlevok
	lea	addintmsg(pc),a0
	bsr	put_string
	bra	dontplay
servlevok	lea	lev3serverlist(pc),a0
skipslistl	move.l	(a0),d0
	addq.l	#8,a0
	tst.l	d0
	bne.b	skipslistl
	move.l	IS_CODE(a1),-8(a0)	* interrupt vector pointer
	move.l	IS_DATA(a1),-4(a0)	* interrupt data pointer
	clr.l	(a0)
	pull	all
	moveq	#0,d0
	rts


mylevel1	move	#$0007,intreq+custom
	rte


mylevel2	move	#$0008,intreq+custom
	pushr	d0
	move.b	$bfed01,d0
	pullr	d0
	rte

mylevel3
	btst	#5,intreqr+custom+1
	beq.b	.is_not_vbi
	push	d0/a0/a5
	* add frame counter
	lea	framecount(pc),a0
	addq.l	#1,(a0)
	* interrupt server
	lea	lev3serverlist(pc),a0
.server5loop
	move.l	(a0),d0			* interrupt server pointer
	beq.b	.endserver5list
	push	all
	move.l	4(a0),a1		* interrupt server data pointer
	move.l	d0,a5
	jsr	(a5)
	pull	all
	addq.l	#8,a0
	bra.b	.server5loop
.endserver5list
	pull	d0/a0/a5
.is_not_vbi
	move	#$0070,intreq+custom
	rte

mylevel4	push	all
	lea	$dff000,a2
	move	intenar(a2),d2
	btst	#14,d2
	bne.b	mylevel4_ints_enabled
	lea	mylevel4_dismsg(pc),a0
	bsr	put_string
	pull	all
	rte
mylevel4_dismsg	dc.b	'audio interrupt taken but interrupts enabled.. hmm.. '
	dc.b	'please report this!',0
	even

mylevel4_ints_enabled
	lea	irqlines(pc),a3
	lea	isdatapointers(pc),a4
mylevel4_begin	moveq	#0,d2
	move	intenar(a2),d2
	and	intreqr(a2),d2
	moveq	#0,d7
mylevel4_loop	move	mylevel4_int_seq(pc,d7),d6	* aud0int bit
	bmi.b	mylevel4_end_loop
	addq	#2,d7
	btst	d6,d2
	beq.b	mylevel4_loop
	move.l	d6,d1
	lsl	#2,d1
	move.l	(a3,d1),d0
	beq.b	mylevel4_no_int_handler
	move.l	d0,a5			* a5 = IS_CODE
	move.l	(a4,d1),a1		* a1 = IS_DATA
	move.l	a2,a0			* a0 = $dff000
	move.l	d2,d1			* d1 = intena & intreq
	move.l	4.w,a6			* a6 = exec base

	jsr	(a5)
mylevel4_end_loop
	move	intenar(a2),d0
	and	intreqr(a2),d0
	andi	#$0780,d0
	bne.b	mylevel4_begin
endmylevel4	pull	all
	rte

mylevel4_no_int_handler
	moveq	#0,d0
	bset	d6,d0
	move	d0,intreq(a2)
	lea	virginaudioints(pc),a0
	tst.l	(a0)
	bne.b	mylevel4_end_loop
	st	(a0)
	lea	mylevel4_msg(pc),a0
	bsr	put_string
	bra.b	mylevel4_end_loop

mylevel4_int_seq
	dc	8, 10, 7, 9, -1	* order of audio interrupt execution

mylevel4_msg	dc.b	'audio interrupt not handled',0
	even


mylevel5	move	#$1800,intreq+custom
;	rts
	rte		; why was this rts in original UADE?


mylevel6	move	#$2000,intreq+custom
	pushr	d0
	move.b	$bfdd00,d0	* quit int (reading should do it)
	pullr	d0
	rte

	IFNE	USE_TEST_ONLY_CODE
	include	exec/test/multitasking.s
	ENDC


*		0 1 2 3 4 5 6 7 8 9 A B C D
irqtab	dc	1,1,1,2,3,3,3,4,4,4,4,5,5,6
vectab	dcb	3,$64
	dcb	1,$68
	dcb	3,$6c
	dcb	4,$70
	dcb	2,$74
	dcb	1,$78
* FOLLOWING LINES MUST BE SET TO MyLevel1,MyLevel2, ... (in init_interrupts())
handlertab	dcb.l	3,0
	dcb.l	1,0
	dcb.l	3,0
	dcb.l	4,0
	dcb.l	2,0
	dcb.l	1,0
irqlines	dcb.l	14,0
isdatapointers	dcb.l	14,0
oldstructs	dcb.b	$12+16*4,0

lev3serverlist	dcb.l	32,0

* Hunk relocator variables. chippoint tracks where the next available free
* memory starts. This variable is monotonously incremented, i.e. there is no
* memory freeing.
chippoint	dc.l	0

;loadbase	dc.l	0
binbase	dc.l	0	* contains pointer to relocated player code
moduleptr	dc.l	0
modulesize	dc.l	0
* number of raster lines to wait for dma by default
dmawaitconstant	dc.l	10

framecount	dc.l	0
songendbit	dc.l	0

tagarray	dc.l	0
dtp_version	dc.l	0		* DTP_RequestDTVersion
dtp_check	dc.l	0		* DTP_Check2
ep_check3	dc.l	0		* EP_Check3
ep_check5	dc.l	0		* EP_Check5
startintfunc	dc.l	0
stopintfunc	dc.l	0
intfunc	dc.l	0
initfunc	dc.l	0
initsoundfunc	dc.l	0
endfunc	dc.l	0
volumefunc	dc.l	0
balancefunc	dc.l	0
voicesfunc	dc.l	0
cia_chip_sel	dc.l	0		* 0 = CIA A, 1 = CIA B
cia_timer_sel	dc.l	1		* 0 = Timer A, 1 = Timer B
ciabase	dc.l	0		* CIA base pointer: bfe001 or bfd000
ciatimerbase	dc.l	0		* Pointer to low byte of CIA timer
ciaadatas	dcb.l	2,0		* data pointers for CIA A timer A and B
ciaaints	dcb.l	2,0		* interrupt vectors for CIA A timers
ciabdatas	dcb.l	2,0		* data pointers for CIA B timer A and B
ciabints	dcb.l	2,0		* interrupt vectors for CIA B timers
configfunc	dc.l	0

* must be called before init player
extloadfunc	dc.l	0
* format is: dc.l index,pointer,len (last index is -1)
* maximum of 64 files can be loaded
load_file_list	dcb.l	64*3,-1
load_file_list_end
	dc.l	-1

msgptr	dc.l	UADECORE_INPUT_MSG
dbgmsgptr	dc.l	UADECORE_DBGINPUT_MSG	; hack: dedicated to debug output

messagebit	dc.l	0

nextsongfunc	dc.l	0
prevsongfunc	dc.l	0
subsongfunc	dc.l	0
newsubsongarray	dc.l	0
cursubsong	dc	0
subsongrange
minsubsong	dc	0
maxsubsong	dc	0
changesubsongbit	dc.l	0

adjacentsubfunc	dc.l	0

vblanktimerstatusbit	dc.l	0
vblanktimercount	dc.l	0
vblanktimerbit	dc.l	0
vblanktimerfunc	dc.l	0
timerioptr	dc.l	0

vbi_hz	dc	50		* PAL 50Hz is default
cia_timer_base_value	dc	$376b	* PAL 50Hz

useciatimer	dc.l	0

modulechange_disabled	dc.l	0

getmsgbit	dc.l	0
sendiomsgbit	dc.l	0

virginaudioints	dc.l	0

noteplayerptr	dc.l	0			; noteStruct**
noteplayersetupfunc	dc.l	0
notestructptr	dc.l	0			; noteStruct*
np_chanlist	dc.l	0
np_chans	dc.l	0
np_longsamples	dc.l	0
np_counter	dc.l	0
np_chanset	dcb.b	32,0

amplifier_init_func	dc.l	0

* audio.device
	include	devices/audio/audio_dev.s



loadedmodname	dcb.b	256,0

playername	dcb.b	256,0
modulename	dcb.b	256,0
formatname	dcb.b	256,0

lastlock	dcb.b	256,0
curdir	dcb.b	256,0

dirarray	dcb.b	256,0
filearray	dcb.b	256,0
patharray	dcb.b	256,0

exec_dumpsignal	dcb.b	128,0

* WARNING a buggy asmone might not tolerate "\2 - debug_info" so spaces are
* removed from the macro
debuginfo	macro
	dc.b	\1, 0
	even
	dc.l	\2-debug_info
	endm

debug_info	debuginfo	'uade debug info', debug_info
	debuginfo	'config', call_config
	debuginfo	'extload', call_extload
	debuginfo	'check module', call_check_module
	debuginfo	'init player', call_init_player
	debuginfo	'check sub songs', call_check_subsongs
	debuginfo	'init volume', call_init_volume
	debuginfo	'init sound', call_init_sound
	debuginfo	'start int', call_start_int
	debuginfo	'interrupt', trapcall
	debuginfo	'amp init', call_amplifier_init
	debuginfo	'amp interrupt', enpp_amplifier_int
	debuginfo	'amp adr', enpp_amplifier_adr
	debuginfo	'amp per', enpp_amplifier_per
	debuginfo	'amp len', enpp_amplifier_len
	debuginfo	'amp vol', enpp_amplifier_vol
	debuginfo	'find author', enpp_find_author
	debuginfo	'relocator', relocator
	debuginfo	'load file', enpp_load_file
	debuginfo	'audio int', mylevel4
	debuginfo	'ciaa int', ciaa_interrupt
	debuginfo	'ciab int', ciab_interrupt
	debuginfo	'set player interrupt', set_player_interrupt
	debuginfo	'superstate', exec_super_state
	debuginfo	'change subsong', change_subsong
	dc.l	0

* dos.library
	dcb.b	$800,0
dos_lib_base
	dcb.b	$200,0

* uade.library
uadelibmsg	dcb.b	EP_OPT_SIZE,0
		dcb.b	24,0
uade_lib_base
	dcb.b LIB_SIZE,0			* minimum(!) size of a Library struct

* intuition.library
	dcb.b	$400,0
intuition_lib_base
	dcb.b	$200,0

* icon.library
	dcb.b	$6c,0		* allocate 108 bytes intialized to 0 (LVO table: see vectors in icon_lib.i)
icon_lib_base
	dcb.b LIB_SIZE,0			* minimum(!) size of a Library struct, i.e. $22 bytes

* utility.library
	dcb.b	426,0		* allocate 426 bytes intialized to 0 (LVO table: see vectors in utility_lib.i)
utility_lib_base
	dcb.b LIB_SIZE,0			* minimum(!) size of a Library struct

* graphics.library (testcase: StoneTracker)
	dcb.b	1050,0		* allocate 1050 bytes intialized to 0 (LVO table: see vectors in graphics_lib.i)
graphics_lib_base
	dcb.b gb_SIZE,0

* timer.device
		dcb.b	$24,0		* allocate 36 bytes for 6 function pointers: 4 built-in (open, close, etc) and 2 lib-specific (BeginIO, AbortIO)
timer_device_base
	dcb.b LIB_SIZE,0			* minimum(!) size of a Library struct



; see NEWEP_GLOBALS in eagleplayer.i - function vectors of "eagleplayer lib" (64 functions *6 = $180 needed)
eaglesafetybase	dcb.b	$200,0	* see ENPP_SizeOf
eaglebase
	dcb.b	$200,0

end
