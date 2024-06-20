; The original version of this file can be found here: https://sintonen.fi/src/loadseg/LS.asm

; FILE: Source:LS.asm          REV: 116 --- LoadSeg routines
; History
;  66     22nd Dec 1995: Got debug and symbol stuff working.
;  74     Got HUNK_RELOC32SHORT working. Logoff 3.00AM.
;  78     Minor fixes.
;  79     Added LS_DOSERROR defination.
;  93     23rd: Got LS_DOSERROR really function.
;  94     3rd Jan 1996: Added HookData.
;  98     20th Jan: Fixed two major bugs.
;  100    22nd Oct: Fixed to skip HUNK_HEADER hunk names.
;  108    22nd: Added full support for external allocreq thing,
;         HUNK_RELRELOC32 hunk and HUNKF_ADVISORY flag. Logoff 5.00AM.
;  109    Fixed *not* to skip HUNK_HEADER hunk names! (OS doesn't!)
;  110    Fixed errorcode bug.
;  111    Added process test to LS_(Set|Get)Error.
;  114    10th July 1997: Cleaned up a bit.
;  115    30.12.2014: Clean up, rewrote the documentation and released.
;         Just took 18 years. ;-)
;  116    Dual licensed now.
;

;--------------------------------------------------------------------------

; quick hacks: added mappings to the respective names used in UADE

_LVOFreeMem  equ  FreeMem
_LVOAllocMem  equ  AllocMem
_LVOOpenLibrary  equ  OpenLibrary
_LVOCloseLibrary  equ  CloseLibrary

;--------------------------------------------------------------------------

LS_TESTCODE	SET	0

	IFNE	LS_TESTCODE

	include	"dos/dosextens.i"
	include	"dos/doshunks.i"

Main	moveq	#RETURN_ERROR,d7
	lea	(FileName,pc),a0
	move.l	a0,d1
	bsr	LS_LoadSeg
	beq.b	.noseg

	move.l	d0,a0
	add.l	a0,a0
	add.l	a0,a0

	bsr	LS_UnLoadSeg
	moveq	#RETURN_OK,d7

.noseg	move.l	d7,d0
	rts

FileName
	dc.b	'c:dir',0
	ENDC

;--------------------------------------------------------------------------
; LoadSeg routines for non-overlaid executables.
; Written by Harry Sintonen <sintonen@iki.fi>.
;
; Kickstart 1.2+ (V33.x) compatible code.
; This code is capable of loading Kickstart release 2 (V37) and 3 (V39)
; hunks.
;
; NOTE THAT YOU MUST CLEAR CPU CACHES BEFORE YOU ATTEMPT TO EXECUTE THE
; LOADED CODE. THIS LOADSEG ROUTINE DOES *NOT* DO THAT AUTOMAGICALLY, SO
; YOU MUST DO IT BY HAND! [See exec.library/CacheClearE(), or if running
; pre pre-V37 add manual instruction cache clearing code, with support for
; both 020/030 and 040/060 CPUs].
;
; Copyright (C) 1995-1997,2014,2022 Harry Sintonen.
; Released under dual license. Pick whichever license suits you and your
; project:
;
; Option 1: Creative Commons license: CC BY-NC-SA 4.0
; See: https://creativecommons.org/licenses/by-nc-sa/4.0/
;
; Option 2: GNU Lesser General Public License, version 2.1
;
;    This library is free software; you can redistribute it and/or
;    modify it under the terms of the GNU Lesser General Public
;    License as published by the Free Software Foundation; either
;    version 2.1 of the License, or (at your option) any later version.
;
;    This library is distributed in the hope that it will be useful,
;    but WITHOUT ANY WARRANTY; without even the implied warranty of
;    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
;    Lesser General Public License for more details.
;
;    You should have received a copy of the GNU Lesser General Public
;    License along with this library; if not, write to the Free Software
;    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
;    02110-1301  USA
;
; See: COPYING.LGPL
;
; Generic implementation details
; ==============================
;
; The loader routine supports the following hunk types:
;
; HUNK_HEADER        processed normally
; HUNK_CODE          processed normally
; HUNK_DATA          processed normally
; HUNK_BSS           processed normally
; HUNK_DEBUG         hooked
; HUNK_NAME          skipped
; HUNK_RELOC32       processed normally
; HUNK_DEBUG         hooked
; HUNK_SYMBOL        hooked
; HUNK_RELOC32SHORT  processed normally
; HUNK_DREL32        identical to HUNK_RELOC32SHORT
; HUNK_RELRELOC32    processed normally
;
; Note: HUNKF_CHIP|HUNKF_FAST memory type is supported.
;
; High level functions
; ====================
;
; LS_LoadSeg - Load amiga executable to memory
; --------------------------------------------
;   IN: d1 = filename
;  OUT: z,d0 = d1 = seglist or NULL
; NOTE: This function uses dos.library to load the file, and AllocMem/FreeMem
;       to handle memory.
;
; LS_UnLoadSeg - Free amiga executable
; ------------------------------------
;   IN: d1 = seglist or NULL
;  OUT: z,d0 = success (always succeeds unless if seglist is NULL)
;
;
; Low level functions
; ===================
;
; LS_InternalLoadSeg - Load amiga executable to memory
; ----------------------------------------------------
;   IN: d0 = handle
;       a0 = funcarray (readfunc, allocfunc, freefunc, debugfunc, symbolfunc, HookData)
;       a6 = userdata (passed to readfunc in a6)
;  OUT: z,d0 = seglist or NULL, d1 = doserror, if seglist is NULL
;
; LS_InternalLoadSeg is passed an array of 6 items. First 5 are the
; hook function pointers, as follows:
;
; 1. readfunc
;   IN: d1 = handle, d2 = buffer, d3 = len, a6 = userdata
;  OUT: d0 = bytes read, -1 for error
;
; 2. allocfunc
;   IN: d0 = len, d1 = flags, a6 = execbase
;  OUT: d0 = memory or NULL if no memory is available
;
; 3. freefunc
;   IN: a1 = memory, d0 = len, a6 = execbase
;  OUT: -
;
; 4. debugfunc
;   IN: a0 = debugdata, a1 = HookData, d0 = datalen, d1 = hunk number, a6 = execbase
;  OUT: -
;
; 5. symbolfunc
;   IN: a0=symbol c-string, a1 = HookData, d0 = offset, d1 = hunk number, a6 = execbase
;  OUT: -
; NOTE: You *must* copy the string if you intend to use it after the hook call.
;
; 6. hookdata - the value passed to debugfunc and symbolfunc in register a1.
;
; LS_InternalUnLoadSeg - Free amiga executable
; --------------------------------------------
;   IN: d1 = seglist or NULL, a1 = freefunc
;  OUT: z,d0 = success
;

; Following three flags can be modified (set 0 or 1):

LS_USEINCLUDES	SET	0		; Use includes.
LS_DOSERROR	SET	0		; Set pr_Result2 (ioerr)
LS_USEXDEFS	SET	0		; Enable to have XDEFs for external linking.


; Internal default values for flags:

;	IFND	LS_USEINCLUDES
;LS_USEINCLUDES	SET	1		; Use includes.
;	ENDC
	IFND	LS_DOSERROR
LS_DOSERROR	SET	1		; Set pr_Result2 correctly.
	ENDC
	IFND	LS_USEXDEFS
LS_USEXDEFS	SET	1		; Have XDEFs for external linking.
	ENDC

	IFNE	LS_USEINCLUDES
	include	"exec/types.i"
	include	"exec/libraries.i"
	include "exec/funcdef.i"
	include	"exec/exec_lib.i"
	include	"dos/dos.i"
	include	"dos/dosextens.i"
	include	"dos/doshunks.i"
	include	"dos/dos_lib.i"
	ENDC


	IFNE	LS_USEXDEFS
	XDEF	_LS_LoadSeg
	XDEF	_LS_UnLoadSeg
	XDEF	_LS_InternalLoadSeg
	XDEF	_LS_InternalUnLoadSeg
	ENDC

	CNOP	0,4

;  IN: d1=filename
; OUT: z,d0=d1=seglist or NULL
	IFNE	LS_USEXDEFS
_LS_LoadSeg
	ENDC

***********************************************************************
*   seglist = LoadSeg( name )
*    D0                 D1
*
*    Scatterload a loadable file into memory
*
LS_LoadSeg
dos_loadseg			; for UADE use
	movem.l	d2-a6,-(sp)
	move.l	(4).w,a6
	IFNE	LS_DOSERROR
	moveq	#-1,d3			; Magic error.
	ENDC
	moveq	#0,d7
	tst.l	d1
	beq.b	.exitr
	move.l	d1,a2
	lea	(LS_DosName,pc),a1
	moveq	#33,d0
	jsr	(_LVOOpenLibrary,a6)
	tst.l	d0
	beq.b	.exit
	move.l	d0,a6

	move.l	a2,d1
	move.l	#MODE_OLDFILE,d2
	jsr	(_LVOOpen,a6)

	; UADE issue: normally d0 would now contain a "BCPL pointer to a file handle"
	; but UADE just uses an index into a file table instead.. since impl uses UADE's
	; hacked Read, it should be OK here

	move.l	d0,d6
	beq.b	.closedos

	move.l	d6,d0
	pea	(LS_symbol,pc)		; note: uses random HookData
	pea	(LS_debug,pc)
	pea	(LS_free,pc)
	pea	(LS_alloc,pc)
	pea	(LS_read,pc)
	move.l	sp,a0
	bsr.b	LS_InternalLoadSeg
	lea	(5*4,sp),sp
	move.l	d0,d7

	IFNE	LS_DOSERROR
	move.l	d1,d3
	ENDC

	move.l	d6,d1
	jsr	(_LVOClose,a6)
.closedos
	move.l	a6,a1
	move.l	(4).w,a6
	jsr	(_LVOCloseLibrary,a6)
.exitr	move.l	d7,d0
	IFNE	LS_DOSERROR
	bne.b	.exit
	move.l	d3,d1
	bmi.b	.exit
	bsr	LS_SetError
	ENDC
.exit	move.l	d0,d1
	movem.l	(sp)+,d2-a6
	rts


;  IN: d1=fh, d2=buffer, d3=len, a6=xyzbase
; OUT: d0=bytes read, -1 for error
LS_read	jmp	(_LVORead,a6)

;  IN: d0=len, d1=requirements, a6=execbase
; OUT: d0=memory array or NULL
LS_alloc
	jmp	(_LVOAllocMem,a6)

;  IN: a1=array, d0=len, a6=execbase
; OUT: -
LS_free	jmp	(_LVOFreeMem,a6)

;  IN: a0=debugdata, a1=HookData, d0=datalen, d1=hunk number, a6=execbase
; OUT: -
;NOTE: Hunk number is counted from hunk 0 on, even if hunk numbering really
;      doesn't start from hunk 0.
LS_debug
;;	rts

;  IN: a0=symbol c-string, a1=HookData, d0=offset, d1=hunk number, a6=execbase
; OUT: -
;NOTE: You *must* copy the string to use it, as it's freed after this hook!
;      Hunk number is counted from hunk 0 on, even if hunk numbering really
;      doesn't start from hunk 0.
LS_symbol
	rts


;  IN: d0=fh
;      a0=funcarray (readfunc, allocfunc, freefunc, debugfunc, symbolfunc, HookData)
;      a6=xyzbase (usually dosbase)
; OUT: z,d0=seglist or NULL, d1=doserror, if seglist=NULL (LS_DOSERROR!!)
	IFNE	LS_USEXDEFS
_LS_InternalLoadSeg
	ENDC
LS_InternalLoadSeg
	movem.l	d2-a6,-(sp)
	moveq	#0,d7
	move.l	d0,d6
	move.l	a0,a5

	lea	(-5*4,sp),sp		; Read header:
	lea	(.clearstack,pc),a0
	move.l	sp,d2
	moveq	#5,d3			; 5 longwords.
	bsr	LS_Read

	IFNE	LS_DOSERROR
	move.l	#ERROR_BAD_HUNK,d5
	ENDC

	cmp.l	#HUNK_HEADER,(sp)	; Is it executable?
	bne.b	.clearstack
	tst.l	(4,sp)
	bne.b	.clearstack

	movem.l	(8,sp),d5/a3/a4		; Get number of hunks/1st hunk/last hunk

	move.l	a4,d5			; Is it valid?
	sub.l	a3,d5
	addq.l	#1,d5

	IFNE	LS_DOSERROR
	move.l	d5,d1
	move.l	#ERROR_BAD_HUNK,d5
	ENDC

	bsr.b	.ils_sub1

.clearstack
	lea	(5*4,sp),sp
	move.l	d7,d0
	IFNE	LS_DOSERROR
	bne.b	.ret_ok
	move.l	d5,d1
	bne.b	.ret_ok
	bsr	LS_GetError
.ret_ok
	ENDC

	movem.l	(sp)+,d2-a6
	rts

; Subpart1
.ils_sub1
	IFNE	LS_DOSERROR
	move.l	d1,d5
	ENDC

	move.l	d5,d0			; Read all hunk lens:
	bsr	LS_InternalAllocMem
	beq.b	.exits1
	move.l	d0,-(sp)

	lea	(.freehlenm,pc),a0
	move.l	(sp),d2
	move.l	d5,d3
	bsr	LS_Read

	move.l	d5,d2			; Allocate all hunks
	move.l	(sp),a2			; and link them together
	clr.l	-(sp)
	move.l	sp,a1
.ils_alloc
	move.l	(a2)+,d0
	addq.l	#1,d0
	bsr	LS_InternalAllocMem
	beq.b	.free_list
	move.l	d0,d1
	lsr.l	#2,d0
	move.l	d0,(a1)
	move.l	d1,a1
	subq.l	#1,d2
	bne.b	.ils_alloc

	move.l	(sp)+,d7
	bsr.b	.ils_sub2
	move.l	(sp)+,a1
	bra	LS_InternalFreeMem

.freehlenm
	move.l	(sp)+,a1
	bsr	LS_InternalFreeMem
.exits1
	IFNE	LS_DOSERROR
	moveq	#0,d5
	ENDC
	rts

.free_list
	move.l	(sp)+,d1
	bsr	LS_UnLoadSeg
	bra.b	.freehlenm


; Main loader part:

.ils_sub2
	subq.l	#8,sp
	move.l	d7,d0
	lsl.l	#2,d0
	move.l	d0,a2			; a2=thishunk
	moveq	#0,d5			; d5=hunk number (counted from hunk 0 onwards)

.more_hunks
	lea	(.badexe,pc),a0		; Get main type:
	move.l	sp,d2
	moveq	#2,d3
	bsr	LS_Read
	and.w	#$1FFF,(sp)
	cmp.l	#HUNK_BSS,(sp)
	beq.b	.get_sub
	cmp.l	#HUNK_CODE,(sp)
	beq.b	.valid
	cmp.l	#HUNK_DEBUG,(sp)
	beq	.is_debug
	cmp.l	#HUNK_DATA,(sp)
	bne.b	.badexe
.valid	move.l	(4,sp),d3		; Get hunk len.
	move.l	(-4,a2),d0		; Is it too long?
	subq.l	#8,d0
	lsr.l	#2,d0
	cmp.l	d0,d3
	bhi.b	.badexe

;;	lea	(.badexe,pc),a0		; Read hunk:
	move.l	a2,d2
	addq.l	#4,d2
	bsr	LS_Read

.get_sub
	lea	(.badexe,pc),a0		; Get sub hunk type:
	move.l	sp,d2
	moveq	#1,d3
	bsr	LS_Read

	move.w	#HUNK_DEBUG,d0
	btst	#HUNKB_ADVISORY-3*8,(sp)
	bne.b	.is_advisory
	tst.w	(sp)			; Is it bad?
	bne.b	.badexe
	move.l	(sp),d0
.is_advisory
	bsr.b	.ils_sub3		; Handle sub hunk.
	bne.b	.get_sub		; If not HUNK_END get more.

	addq.l	#1,d5			; Bump counter to next hunk.
	move.l	(a2),d0
	lsl.l	#2,d0
	move.l	d0,a2
	bne.b	.more_hunks

.exits2	addq.l	#8,sp
	rts


.badexe2_frm
	move.l	d2,a1			; Free reloc buffer memory:
	bsr	LS_InternalFreeMem
.badexe2
	addq.l	#8,sp			; Pop buffer longs.
	addq.l	#4,sp			; Pop return address.
.badexe	move.l	d7,d1
	bsr	LS_UnLoadSeg
	moveq	#0,d7

	IFNE	LS_DOSERROR
	move.l	#ERROR_BAD_HUNK,d5
	ENDC

	bra.b	.exits2


.is_debug
	move.l	(4,sp),d2		; Get debug data len.
	lea	(.badexe,pc),a0
	lea	(LS_HandleDebug,pc),a1
	bsr	LS_DebugSkip
	bra	.more_hunks


; D0.w=subhunk type
.ils_sub3
	subq.l	#8,sp
	move.l	sp,d2
	moveq	#1,d3
	lea	(.badexe2,pc),a0

	moveq	#0,d4			; Word size reloc (magic).
	cmp.w	#HUNK_RELRELOC32,d0
	beq.b	.handle_relocs
	moveq	#1,d4			; Word size reloc.
	cmp.w	#HUNK_RELOC32SHORT,d0
	beq.b	.handle_relocs
	cmp.w	#HUNK_DREL32,d0
	beq.b	.handle_relocs

.hskip04
	moveq	#2,d4			; Long size reloc.
	cmp.w	#HUNK_RELOC32,d0
	bne	.hskip02

.handle_relocs
	clr.l	(4,sp)			; Clear reloc counter.
.do_relocs
	move.l	sp,d2
	lea	(.badexe2,pc),a0
	moveq	#1,d3
	bsr	LS_SizeRead
	move.l	sp,a0			; Get number of relocs.
	bsr	.getsized
	move.l	d0,d3
	beq.b	.relocs_done

	add.l	d3,(4,sp)		; Update reloc counter.
	addq.l	#1,d3			; Add space for dest hunk info.
	move.l	d3,d0			; Allocate reloc buffer memory:
	bsr	LS_AllocVecS
	beq.b	.badexe2
	move.l	d0,d2			; ...and load relocs...
	lea	(.badexe2_frm,pc),a0
	bsr	LS_SizeRead
	move.l	d2,a0
	bsr.b	.getsized
	cmp.l	d0,a3			; Test for invalid hunk number.
	bhi	.badexe2_frm
	cmp.l	a4,d0
	bhi	.badexe2_frm

	sub.l	a3,d0
	move.l	d7,d1			; locate the hunk pointer
.find_hptr
	move.l	d1,a1
	add.l	a1,a1
	add.l	a1,a1
	move.l	(a1),d1
	subq.l	#1,d0
	bpl.b	.find_hptr
	move.l	a1,d1
	addq.l	#4,d1

	subq.l	#1,d3			; d3=number of relocs
.relocate
	bsr.b	.getsized
	lea	4(a2,d0.l),a1
	tst.l	d4
	bne.b	.no_relrel
	move.l	(a1),d0
	sub.l	a1,d0
	move.l	d0,(a1)
.no_relrel
	add.l	d1,(a1)
	subq.l	#1,d3
	bne.b	.relocate

	move.l	d2,a1			; Free reloc buffer memory:
	bsr	LS_InternalFreeMem
	bra.b	.do_relocs

.getsized
	move.l	(a0)+,d0
	cmp.w	#2,d4
	beq.b	.gs_islong
	clr.w	d0
	swap	d0
	subq.l	#2,a0
.gs_islong
	rts

.relocs_done
	cmp.w	#2,d4			; Is is long reloc?
	beq.b	.r_noskip		; Yup, skip.
	move.l	(4,sp),d0		; Get reloc counter.
	ror.l	#1,d0			; Is it even?
	bcs.b	.r_noskip		; It is, don't skip.
	move.l	sp,d2			; Nope, skip 2 bytes:
	lea	(.badexe2,pc),a0
	moveq	#1,d3
	bsr	LS_SizeRead
.r_noskip
	bra.b	.exits3


.hskip02
	cmp.w	#HUNK_NAME,d0		; Skip HUNK_NAME:
	bne.b	.hskip01
	bsr	LS_Read
	move.l	(sp),d2
	bsr	LS_Skip
	bra.b	.exits3
.hskip01
	cmp.w	#HUNK_DEBUG,d0
	bne.b	.hskip03
	bsr	LS_Read
	move.l	(sp),d2
	lea	(LS_HandleDebug,pc),a1
	bsr	LS_DebugSkip
	bra.b	.exits3

.hskip03
	cmp.w	#HUNK_SYMBOL,d0
	beq	.handle_symbols

	cmp.w	#HUNK_END,d0		; Handle HUNK_END:
	bne	.badexe2
	moveq	#0,d0
	bra.b	.s3_endskip
.exits3	moveq	#1,d0
.s3_endskip
	addq.l	#8,sp
	rts


.handle_symbols
.do_symbols
	move.l	sp,d2			; Read lenght of BSTR:
	moveq	#1,d3
	lea	(.badexe2,pc),a0
	bsr	LS_Read
	move.l	(sp),d3			; If len=NULL then we're done!
	beq.b	.exits3
	move.l	d3,d0
	addq.l	#1,d0			; Add space for CSTR null.
	bsr	LS_InternalAllocMem
	beq	.badexe2
	move.l	d0,d2			; Read BSTR:
	lea	(.badexe2_frm,pc),a0
	bsr	LS_Read			; d3=len
	move.l	d2,a1			; For FreeMem!

	move.l	sp,d2
	moveq	#1,d3
	lea	(.badexe2,pc),a0
	bsr	LS_Read
	move.l	(sp),d0			; d0=offset
	move.l	a1,a0			; a0=c-str
	move.l	d5,d1			; d1=hunk number

	movem.l	a1/a2/a6,-(sp)
	move.l	(4).w,a6		; a6=execbase
	move.l	(20,a5),a1		; a1=HookData
	move.l	(16,a5),a2		; Call symbol hook
	jsr	(a2)
	movem.l	(sp)+,a1/a2/a6

	bsr	LS_InternalFreeMem
	bra.b	.do_symbols


; A0=error entry, d2=num of longs to skip
LS_Skip	sub.l	a1,a1
; A0=error entry, a1=special code d2=num of longs to skip
LS_DebugSkip
	move.l	a0,-(sp)
	move.l	d2,d0
	bsr	LS_InternalAllocMem
	beq.b	lss_exiterr
	move.l	d2,d3
	add.l	d3,d3			; Read x4
	add.l	d3,d3
	move.l	d6,d1
	move.l	d0,d2
	move.l	(a5),a0
	move.l	a1,-(sp)
	jsr	(a0)			; Read
	move.l	(sp)+,a1
	move.l	a1,d1
	beq.b	.skip
	cmp.l	d0,d3
	bne.b	.skip
	jsr	(a1)
.skip	move.l	d2,a1
	move.l	d0,d2
	bsr	LS_InternalFreeMem
	cmp.l	d2,d3
	bne.b	lss_exiterr
	move.l	(sp)+,a0
	rts
lss_exiterr
	move.l	(sp)+,a0
	move.l	a0,(sp)
	rts


; A0=error entry, d2=buffer, d3=num of units to read, d4=readsize, a5=funcarray
LS_SizeRead
	move.l	d4,d0
	bne.b	LS_SRSkip
	moveq	#1,d0
	bra.b	LS_SRSkip

; A0=error entry, d2=buffer, d3=num of longs to read, a5=funcarray
LS_Read	moveq	#2,d0
LS_SRSkip
	move.l	a0,-(sp)
	movem.l	d1/d3/a1,-(sp)
	move.l	d6,d1
	lsl.l	d0,d3
	move.l	(a5),a0
	jsr	(a0)			; Read
	cmp.l	d0,d3
	movem.l	(sp)+,d1/d3/a1		; cond codes not changed!
	bne.b	lss_exiterr
	move.l	(sp)+,a0
	rts


;  IN: d1=seglist or NULL
; OUT: z,d0=success
	IFNE	LS_USEXDEFS
_LS_UnLoadSeg
	ENDC
LS_UnLoadSeg
	lea	(LS_free,pc),a1
;;	bsr.b	LS_InternalUnLoadSeg


;  IN: d1=seglist or NULL, a1=freefunc (as in LS_InternalLoadSeg)
; OUT: z,d0=success
	IFNE	LS_USEXDEFS
_LS_InternalUnLoadSeg
	ENDC
LS_InternalUnLoadSeg
	movem.l	d7/a1/a5,-(sp)
	lea	(4-8,sp),a5		; hack to get "funcarray" with freefunc
	moveq	#0,d7			; in the correct position.
.freenext
	lsl.l	#2,d1
	beq.b	.noseg
	moveq	#1,d7
	move.l	d1,a1
	move.l	(a1),d1
	bsr	LS_InternalFreeMem
	bra.b	.freenext
.noseg	move.l	d7,d0
	movem.l	(sp)+,d7/a1/a5
	rts


;  IN: d0=memory len, d4=size, a5=funcarray
; OUT: z,d0=memory block allocated or NULL
LS_AllocVecS
	lsl.l	d4,d0
	addq.l	#3,d0
	lsr.l	#2,d0
;;	bra	LS_InternalAllocMem

;  IN: d0=packed memory req/len, a5=funcarray
; OUT: z,d0=memory block allocated or NULL
LS_InternalAllocMem
	movem.l	d1-d3/a0-a1/a6,-(sp)
	move.l	d0,d1
	rol.l	#3,d1
	moveq	#%110,d2		; Get MEMB_CHIP & MEMB_FAST
	and.l	d2,d1
	cmp.l	d2,d1
	bne.b	.noextmagic
	move.l	d0,-(sp)		; Load allocmem requirements...
	subq.l	#4,sp
	move.l	sp,d2
	moveq	#1,d3
	lea	(.amreadf,pc),a0
	bsr	LS_Read
	move.l	(sp)+,d1
	bclr	#30,d1
	move.l	(sp)+,d0
.noextmagic
	or.l	#$10001,d1		; MEMF_PUBLIC!MEMF_CLEAR
	lsl.l	#2,d0
;;	bclr	#29+2,d0		; Mask out possible HUNKB_ADVISORY
	addq.l	#4,d0
	move.l	(4).w,a6
	move.l	d0,d2
	move.l	(4,a5),a0		; AllocMem
	jsr	(a0)
	tst.l	d0
	beq.b	.nomem
	move.l	d0,a0
	move.l	d2,(a0)+
	move.l	a0,d0
.nomem	movem.l	(sp)+,d1-d3/a0-a1/a6
;;	tst.l	d0
	rts
.amreadf
	addq.l	#8,sp
	moveq	#0,d0
	bra.b	.nomem

;  IN: a1=memory block to free, a5=funcarray
; OUT: -
LS_InternalFreeMem
	movem.l	d0-d1/a0-a1/a6,-(sp)
	move.l	(4).w,a6
	move.l	a1,d0
	beq.b	.nofree
	move.l	-(a1),d0
	move.l	(8,a5),a0
	jsr	(a0)
.nofree	movem.l	(sp)+,d0-d1/a0-a1/a6
	rts


;  IN: d2=buffer, d0=d3=len of buffer
LS_HandleDebug
	movem.l	d0/a2/a6,-(sp)
	move.l	(4).w,a6		; a6=execbase
	move.l	d2,a0			; a0=debugdata
	move.l	d0,d1			; d1=datalen
	move.l	d5,d0			; d0=hunk number
	move.l	(20,a5),a1		; a1=HookData
	move.l	(12,a5),a2		; Call debug hook
	jsr	(a2)
	movem.l	(sp)+,d0/a2/a6
	rts


	IFNE	LS_DOSERROR
LS_GetError
	moveq	#0,d1
LS_SetError
	move.l	(4).w,a0
	move.l	($114,a0),a0		; ThisTask
	cmp.b	#NT_PROCESS,(LN_TYPE,a0)
	bne.b	.exit
	lea	($94,a0),a0		; pr_Result2
	move.l	(a0),-(sp)
	move.l	d1,(a0)
	move.l	(sp)+,d1
.exit	rts
	ENDC


LS_DosName
	dc.b	'dos.library',0
	CNOP	0,4