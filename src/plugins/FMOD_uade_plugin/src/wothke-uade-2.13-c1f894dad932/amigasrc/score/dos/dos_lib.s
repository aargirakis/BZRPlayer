***************************************************************************************
*
* Just some dummy dos_lib function impls that where not present in UADE 2.13
*
* Most of these will not provide the correct functionality but they are actually
* called by PlayAY player and might be useful to analyze and eventually fix
* the support for this player (and maybe others).
*
* Copyright 2022, Juergen Wothke
*
***************************************************************************************


dos_lib_addons	push all
	lea	dos_lib_base(pc),a6

	LVO_JMP _LVOSetIoErr,			dos_setioerr
	LVO_JMP _LVOMakeLink,			dos_makelink
	LVO_JMP _LVOGetConsoleTask,		dos_getconsoletask
	LVO_JMP _LVOSetConsoleTask,		dos_setconsoletask
	LVO_JMP _LVOGetFileSysTask,		dos_getfstask
	LVO_JMP _LVORunCommand,			dos_run
	LVO_JMP _LVOSetMode,			dos_setmode
	LVO_JMP _LVOCli,				dos_cli

	LVO_JMP _LVOLockRecords,		dos_lockrecords
	LVO_JMP _LVOUnLockRecords,		dos_unlockrecords
	LVO_JMP _LVOSelectOutput,		dos_selectoutput
	LVO_JMP _LVOOutput,				dos_output
	LVO_JMP _LVOIsFileSystem,		dos_isfilesystem
	LVO_JMP _LVOFreeDosEntry,		dos_freedosentry
	LVO_JMP _LVORelabel,			dos_relabel
	LVO_JMP _LVONewLoadSeg,			dos_newloadseg
	LVO_JMP _LVOReadArgs,			dos_readargs
	LVO_JMP _LVOFreeArgs,			dos_freeargs
	LVO_JMP _LVOMatchFirst,			dos_matchfirst
	LVO_JMP _LVOCreateNewProc,		dos_createnewproc
	LVO_JMP _LVOGetVar,				dos_getvar
	
	; IFF8SVX is the only player that seems to be using these (and
	; since it doesn't work - see multi-processing - these are 
	; currently rather useless)
	LVO_JMP _LVOLock,				dos_lock2
	LVO_JMP _LVOOpenFromLock,		dos_openfromlock
	LVO_JMP _LVODupLock,			dos_duplock
	LVO_JMP _LVOExamine,			dos_examine

	; this error handling stuff would probably be dead code once the players work correctly
	LVO_JMP _LVOFault,				dos_fault
	LVO_JMP _LVOErrorReport,		dos_errorreport
	LVO_JMP _LVOPrintFault,			dos_printfault

	LVO_JMP _LVOVFPrintf,			dos_vfprintf

	LVO_JMP _LVOFoobar1,			dos_foobar1

	pull all
	rts

unloadsegmsg	dc.b	'UnLoadSeg called',0
	even

dos_unloadseg
	push	a0
	lea unloadsegmsg(pc),a0
	bsr put_string
	pull	a0
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
dos_setmode
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
*    lock  = Lock( name, accessMode )
*    D0            D1        D2
*    BPTR    Lock(STRPTR, LONG)
*
* testcase: IFF8SVX
*
dos_lock2
	bsr dos_lock		; use original UADE impl for compatibility..
	
	tst.l	d0
	beq	dos_lock2_fail

	; hack directly return the file handle (which is already fucked up in UADE impl!
	; no point adding additional Lock wrapper infrastructure)
	bsr dos_open
	
dos_lock2_fail	
	rts

******************************************************************************
*
*    BPTR OpenFromLock(BPTR)
*    D0                 D1
*
* Opens a file you have a lock on (V36)
*
* testcase: IFF8SVX
*
dos_openfromlock
	move.l d1,d0 	; hack: see dos_lock
	rts

******************************************************************************
*
*	lock = DupLock( lock )
*	D0		D1
*
* testcase: IFF8SVX
*
dos_duplock
	move.l d1,d0 	; hack: see dos_lock
	rts

******************************************************************************
*
*	success = Examine( lock, FileInfoBlock )
*	D0                 D1	      D2
*
* testcase: IFF8SVX
*
dos_examine
	; player expects file length to be stored in fib_Size(d2)
	
	push d1-d7/a0-a6
	
	; hack; "lock" here is actually UADE's "filepointer" - which is actually just an index
	lea	dos_file_list(pc),a2

	sub.l #1,d1		; numbering starts at 1
	
exa_loop	tst.l	d1
	beq.b	gotfile
	add.l	#16,a2
	sub.l	#1,d1
	bra.b	exa_loop
gotfile		
	move.l d2,a1
	move.l	12(a2),fib_Size(a1)	;  filesize

	pull	d1-d7/a0-a6
	
	move.l #1,d0	; success
	rts

******************************************************************************
*
*	process = CreateNewProc(tags)
*	D0                       D1
*
* testcase: PlayAY/ZXAYEMUL
*
newprocmsg
	dc.b 'error: CreateNewProc not implemented',0
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
clistruct	dcb.b 	cli_SIZEOF,0		; see dosextensions.i (i.e. $40)
