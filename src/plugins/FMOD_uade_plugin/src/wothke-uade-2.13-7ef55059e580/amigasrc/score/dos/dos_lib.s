***************************************************************************************
*
* Just some dummy dos_lib function impls that where not present in UADE 2.13
*
* Most of these will not provide the correct functionality but they are actually 
* called by PlayAY player and might be useful to analyze and eventually fix
* the support for this player (and maybe others). 
*
* So far the attempts have not been successful and it is possible that some 
* of these APIs would not actually be called except for some earlier "init" problem,
* see  "InitPlayer function returned fail". (Though it is possible that PlayAY is
* actually relying on CLI based command execution - which isn`t properly supported 
* here.)
*
* Depending how this turns out the below code might either be improved of 
* thrown out completely again.
*
* Copyright 2022, Juergen Wothke
*
***************************************************************************************

USE_DBGOUT	SET	0

; ISSUE: put_string & put_value are sending UADE_GENERALMSG to the C side thereby 
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
	
;PRINT_HUNKNEXT     MACRO 	; input	BCPL ptr to SegList - any register except D7
;	IFNE	USE_DBGOUT
;	push all
;	move.l \1,d7
;	lsl.l	#2,d7	
;	move.l d7,a0
;	move.l (a0),d0		; next hunk	
;	bsr put_value
;	pull all
;	ENDC
;	ENDM
	
;PRINT_HUNKLEN     MACRO 
;	IFNE	USE_DBGOUT
;	push all
;	move.l \1,d7
;	lsl.l	#2,d7	
;	move.l d7,a0
;	move.l -4(a0),d0	; hunk length
;	bsr put_value
;	pull all
;	ENDC
;	ENDM


dos_lib_addons	push all
	lea	dos_lib_base(pc),a6
	
	LVO_JMP SetIoErr,			dos_setioerr
	LVO_JMP MakeLink,			dos_makelink
	LVO_JMP GetConsoleTask,		dos_getconsoletask
	LVO_JMP SetConsoleTask,		dos_setconsoletask
	LVO_JMP GetFileSysTask,		dos_getfstask
	LVO_JMP RunCommand,			dos_run
	LVO_JMP SetMode,			dos_setmode
	LVO_JMP Cli,				dos_cli
	
	LVO_JMP LockRecords,		dos_lockrecords
	LVO_JMP UnLockRecords,		dos_unlockrecords
	LVO_JMP SelectOutput,		dos_selectoutput
	LVO_JMP Output,				dos_output
	LVO_JMP IsFileSystem,		dos_isfilesystem
	LVO_JMP FreeDosEntry,		dos_freedosentry
	LVO_JMP Relabel,			dos_relabel
	LVO_JMP NewLoadSeg,			dos_newloadseg
	LVO_JMP ReadArgs,			dos_readargs
	LVO_JMP FreeArgs,			dos_freeargs
	LVO_JMP MatchFirst,			dos_matchfirst
	LVO_JMP CreateNewProc,		dos_createnewproc
	LVO_JMP GetVar,				dos_getvar
	
	; XXX Input seems to be referenced from Stonetracker... unused code? or garbage Ghidra output?
	
	; this error handling stuff would probably be dead code once the players work correctly
	LVO_JMP Fault,				dos_fault
	LVO_JMP ErrorReport,		dos_errorreport
	LVO_JMP PrintFault,			dos_printfault
	
	LVO_JMP VFPrintf,			dos_vfprintf
	
	LVO_JMP Foobar1,			dos_foobar1

	pull all
	rts

	
loadsuccmsg	dc.b	'loadseg hunk-file completed',0
	even

; tmp hack
hunk_dbg_print0	push all	

;	PRINTSTR	loadsuccmsg

	subq.l	#8,d0	; this should now point to the size?
	move.l d0,a1

;	move.l a1,d0
;	bsr put_value
	
	move.l #$5c7,(a1)	; AMAD hack - seems to make no difference
	
;	move.l (a1),d0
;	bsr put_value
;	move.l 4(a1),d0
;	bsr put_value
;	move.l 8(a1),d0
;	bsr put_value
;	move.l 12(a1),d0
;	bsr put_value
	
	pull all
	rts
	
;se_next_msg	dc.b	'next segment: ',0
;se_len_msg	dc.b	'segment len: ',0
;	even

; tmp hack
; input: d0 (BCSP ptr to SegList)	
hunk_dbg_print
	
;	PRINTSTR	loadsuccmsg
;	PRINTSTR	se_next_msg
;	PRINT_HUNKNEXT d0	
;	PRINTSTR	se_len_msg
;	PRINT_HUNKLEN d0	
	rts
	
**************************************************************************
*
*  oldcode = SetIoErr(code)
*  D0                  D1
*
*	testcase: PlayAY - last "unimplemented" dos_lib function called for AMAD module
*             
*
dos_setioerr
	; todo: actually store the "current" error somwhere..
	moveq #0,d0
	rts

**************************************************************************
*
*  success = MakeLink( name, dest, soft )
*  D0                   D1    D2    D3
*
*	testcase: PlayAY - WTF? again, the player first complained when this wasnt implemented.
*   now that it is, the below put_string never seems to be reached (no outpput!)
*
dos_makelink	
;	push all
;	move.l d1,a0
;	bsr put_string	
;	pull all
	moveq #-1,d0		; success (should this be a BCPL thing?)
	rts

	
**************************************************************************
*
*  port = GetFileSysTask()
*  D0
*
*  (hack: same for GetConsoleTask)
*
*  RESULT
*  port - The pr_MsgPort of the filesystem, or NULL.
*
*
*	testcase: PlayAY - this is more& more looking like random dos_lib calls being made.. due to some inconstistent memory state
*
dos_getconsoletask
dos_getfstask
	moveq #0,d0				; fail
	rts


**************************************************************************
*	
*  oldport = SetConsoleTask(port)
*  D0                       D1
*
dos_setconsoletask
	moveq #0,d0			; no old port..
	rts

	
**************************************************************************
*
*  rc = RunCommand(seglist, stacksize, argptr, argsize)
*  D0                D1         D2       D3      D4
*
*	testcase: PlayAY
*
dos_run	
;	push all	
;	move.l d4,d0
;	bsr put_value
;	move.l d3,a0
;	bsr put_string
;	pull all	
	moveq #-1,d0		; fail
	rts
	
**************************************************************************
*
*  len = Fault(code, header, buffer, len)
*  D0           D1     D2      D3    D4
*
*	testcase: PlayAY -> might be a phantom problem since as soon as I provided this supposedly missing function, it is no longer called
*             and instead another "unimplemented function" pops up..WTF?
*
dos_fault	
;	push all
;	move.l #$7171,d0	; hack: just a recongizable pattern	WTF: adding these 2 instructions makes the other code behave differently!!
;	bsr put_value
;	move.l d2,a0
;	lea dosfaultwarning(pc),a0		; triggers "Illegal instructions"... which are NOT there with move 
;	bsr put_string	; lets just see what is input
;	move.l d1,d0
;	bsr put_value
;	pull all
	moveq #0,d0
	rts

**************************************************************************
*
*  status = ErrorReport(code, type, arg1, device)
*  D0                    D1    D2    D3     A0
*
*	testcase: PlayAY - if root cause gets fixed this probably becomes dead code (CLI related?)
*
dos_errorreport
;	move.l #$5151,d0	; hack: just a recongizable pattern	WTF: adding these 2 instructions makes the other code behave differently!!
;	bsr put_value
	moveq #-1,d0	;	 DOS_TRUE here means "cancel"
	rts
	
**************************************************************************
*
*  success = PrintFault(code, header)
*  D0                    D1     D2
*
*	testcase: PlayAY
*	
dos_printfault
;	move.l #$6161,d0	; hack: just a recognizable pattern	WTF: adding these 2 instructions makes the other code behave differently!!
;	bsr put_value
	moveq #0,d0
	rts

	
**************************************************************************
*
*  count = VFPrintf(fh, fmt, argv)
*  D0               D1  D2    D3
*
*	testcase: PlayAY/ZXAYEMUL
*
dos_vfprintf
	push all
	move.l d2,a0
	bsr put_string
	pull all
	rts
	
**************************************************************************
*
*  success = SetMode(fh, mode)
*  D0                D1  D2
*
* testcase: PlayAY/AMAD
*
;setmode_CON	dc.b	'SetMode CON',0
;setmode_RAW	dc.b	'SetMode RAW',0
;setmode_ERR	dc.b	'SetMode error',0
	even

dos_setmode
;	push all
;	cmpi #0,d2
;	beq mode_CON
;	cmpi #1,d2
;	beq mode_RAW
;	lea setmode_ERR(pc),a0
;	bra setmode_print
;mode_CON
;	lea setmode_CON(pc),a0
;	bra setmode_print
;mode_RAW
;	lea setmode_RAW(pc),a0
;setmode_print
;	bsr put_string
;	pull all
	moveq #0,d0	
	rts

**************************************************************************
*
*	Cli -- Returns a pointer to the CLI structure of the process (V36)
*
*   FUNCTION
*	Returns a pointer to the CLI structure of the current process, 
*	or NULL if the process has no CLI structure.
*
*	testcase: PlayAY -> WTF? this code no longer even seems to be reached
*             (though the emulation had previously complained about it not
*             being implemented..) but using the 'no CLI' put_string instead of
*             put_value leads to the code behaving differently elsewhere
*

dos_cli		
;	push all
;	move.l #$999,d0
;	bsr put_value
;	pull all
	
	push a0
	lea clistruct(pc),a0
	move.l a0,d0
	pull a0
	
	; testcase PlayAY; if 0 is returned here then this triggers a call of -486(doslibbase)
	; which seems to be an undocumented dos_lib function.. 
;	move.l #0,d0
	rts

**************************************************************************
*
*  success = LockRecords(record_array,timeout)
*  D0                    D1           D2
*
* testcase: "PlayAY" - 
*
dos_lockrecords	
	moveq	#1,d0
	rts

dos_unlockrecords	
	moveq	#1,d0
	rts

**************************************************************************
*
*  old_fh = SelectOutput(fh)
*  D0                    D1
* 
* testcase: "PlayAY"
*
defaultoutchnl
	dc.l	0
	
dos_selectoutput	
	push	d1/a0
	lea defaultoutchnl(pc),a0
	move.l	(a0),d0
	move.l d1,(a0)
	pull	d1/a0
	rts

**************************************************************************
*
*  file = Output()
*  D0
* 
* testcase: not much point in using SelectOutput without having Output..
*
dos_output	
	push	a0
	lea defaultoutchnl(pc),a0
	move.l	(a0),d0
	pull	a0
	rts

**************************************************************************
*
*  result = IsFileSystem(name)
*  D0                     D1
*
* testcase: PlayAY
* 
dos_isfilesystem	
;	push	all		; just some dbg out	
;	move.l d1,a0
	; WTF? another one of those "unimplemented" calls that "disappear" when the impl is there.. no print to be seen! 
;	bsr put_string
;	pull	all
	move.l #1,d0
	rts

**************************************************************************
*
*  FreeDosEntry(dlist)
*               D1
*
* testcase: PlayAY -> why is this called then MakeDosEntry has never been used?
* 
dos_freedosentry	
	rts

**************************************************************************
*
*  success = Relabel(volumename,name)
*  D0                    D1      D2
*
* testcase: PlayAY
*
dos_relabel
;	push	all	; just some dbg out	
;	move.l d2,a0
	; WTF? anotherone of those "unimplemented" calls that "disappear" when the impl is there.. no print to be seen! 
;	bsr put_string
;	pull all	
	move.l #1,d0
	rts

**************************************************************************
*
*  seglist = NewLoadSeg(file, tags)
*  D0			        D1    D2
*
* testcase: PlayAY
*
dos_newloadseg
; fixme: tags are just ignored for now
;	push a0
;	move.l d1,a0
;	bsr put_string	; this probably again will not print ... why?
;	pull a0

	push d1
	;  "NewLoadSeg() does NOT return seglist in both D0 and D1, as LoadSeg does."
	bsr dos_loadseg
	pull d1
	rts

**************************************************************************
*
*  result = ReadArgs(template, array, rdargs)
*  D0                   D1      D2      D3
*
* testcase: PlayAY
*
dos_readargs
	; if player really depends on this we better call it a day..

	move.l #0,d0	; fail
	rts

**************************************************************************
*
*  FreeArgs(rdargs)
*	        D1
*
* testcase: PlayAY
*
dos_freeargs
	rts

**************************************************************************
*
*  error = MatchFirst(pat, AnchorPath)
*  D0                 D1       D2
*
* testcase: PlayAY
*
dos_matchfirst
;	push a0
;	move.l d1,a0
;	bsr put_string	; this probably again will not print ... why?
;	pull a0
	; the chain of dos calls just keeps getting longer - with no meaningful
	; debug output available; the next unimplemented func is -858(doslibbase)
	; which is some undocumented vector.. fuck this
	move.l #0,d0	; error
	rts		

******************************************************************************
*
*	len = GetVar( name, buffer, size, flags )
*	D0	       D1     D2     D3    D4
*
* testcase: PlayAY/ZXAYEMUL - (called in preparation of calling CreateNewProc)
*
getvarmsg
	dc.b 'GetVar not implemented',0
		even
dos_getvar
	push all
	lea getvarmsg(pc),a0
	bsr put_string	
	move.l d1,a0
	bsr put_string
	pull all
	move.l #0,d0
	rts
	
******************************************************************************
*
*	process = CreateNewProc(tags)
*	D0                       D1
*
* testcase: PlayAY/ZXAYEMUL
*
newprocmsg
	dc.b 'CreateNewProc not implemented',0
		even
dos_createnewproc
	push all
	lea newprocmsg(pc),a0
	bsr put_string	
	pull all
	move.l #0,d0
	rts
	
	
**************************************************************************
*
*   some undocumented function that is actually called by PlayAY
*
dos_foobar1
	; this is currently the last one in the chain.. the next one goes to 
	; an entry >160 .. which is again in the undocumented range...
	move.l #0,d0
	rts		

	
* a dummy struct for all those that might need it (testcase; PlayAY)
clistruct	dcb.b 	$40,0	; see cli_SIZEOF in dosextensions.i - problem: include of dos.i doesn't work
;clistruct	dcb.b 	cli_SIZEOF,0
	