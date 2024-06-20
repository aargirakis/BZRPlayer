************************************************************************************
*
* The multi-tasking related exec.library stuff.
*
* It would probably be good enough to just have support for cooperative MT in the
* UADE context. But as a proof of concept I'll just try if a minor variation of
* Commodore's original impl will work within UADE. (As of now the interrupt enabling
* /disabling impl is probably unnecessary overkill since UADE's original handling of
* those flags is probably flawed.)
*
* CAUTION: This implementation contains parts of code copyrighted by Commodore
* and it therefore CANNOT be used under a GPL license! It is used here as test code
* that serves an illustrative purpose only, i.e. to show the functionalities that
* still need to be reimplemented in UADE.
*
* note: Most EaglePlayers do not need any multi-tasking at all. Testcases for players
* that do are: FaceTheMusic, MarbleMadness custom player, G&T Game Systems
*
************************************************************************************

multitasking_init
	LVO_JMP FreeSignal,		exec_freesignal
	LVO_JMP AllocSignal,	exec_allocsignal
	LVO_JMP Signal,			exec_signal
	LVO_JMP Wait,			exec_wait
	LVO_JMP WaitPort,		exec_waitport

	LVO_JMP AddTask,		exec_addtask
	LVO_JMP RemTask,		exec_remtask
	LVO_JMP FindTask,		exec_findtask

	LVO_JMP Forbid,			exec_forbid
	LVO_JMP Permit,			exec_permit
;	LVO_JMP ExitIntr,		exec_exitintr
	LVO_JMP Reschedule,		exec_reschedule
	LVO_JMP Schedule,		exec_schedule
	LVO_JMP Switch,			exec_switch
	LVO_JMP Dispatch,		exec_dispatch


	lea defaultasktexit(pc),a0
	move.l		a0,TaskExitCode(a6)

	lea defaulttaskexcept(pc),a0
	move.l		a0,TaskExceptCode(a6)
	move.l		a0,TaskTrapCode(a6)

	move.l		#$ffff,TaskSigAlloc(a6)

	; conflicts with UADE's originally used #$ff! testcase?
	move.w		#$ffff,IDNestCnt(a6)	; interrupt disable level: -1

	move.w		#$c000,$dff09a		; enable interrupt

	lea			TaskWait(a6),a2
	move.b		NT_TASK,LH_TYPE(a2)
	NEWLIST a2

	lea			TaskReady(a6),a2
	move.b		NT_TASK,LH_TYPE(a2)
	NEWLIST a2

	; setup a Task as a base for potential task switches
	lea playlooptask(pc),a1				; just give it a semblance of ligitimacy
	move.l		#NT_TASK,LN_TYPE(a1)
	move.l		#0,LN_PRI(a1)			; what is a good prio for UADE's main playloop?
	move.l		#TS_RUN,TC_STATE(a1)
	move.l		#1,TC_SPLOWER(a1)		; let UADE use whatever it pleases
	move.l		#$fffffffe,TC_SPUPPER(a1)
	move.l		a1,ThisTask(a6)			; marks that this is currently running

;	lea       TaskReady(a6),a0			; seems this must not be put in the Ready list
;	bsr       exec_enqueue
	rts

taskcrashmsg dc.b 'Task crashed',0
	even
defaulttaskexcept
	lea			taskcrashmsg(pc),a0
	bsr 		put_string
	illegal

*******************************************************************
*
*  FreeSignal(signalNum)
*             D0
*
exec_freesignal
	move.l		ThisTask(a6),a1
	move.l		TC_SIGALLOC(a1),d1
	bclr		d0,d1
	move.l		d1,TC_SIGALLOC(a1)
	rts

*******************************************************************
*
*  signalNum = AllocSignal(signalNum)
*  D0                      D0
*

exec_allocsignal
	move.l		ThisTask(a6),a1
	move.l		TC_SIGALLOC(a1),d1
	cmp.b		#$ff,d0
	beq.b		sasearchfree
	bset		d0,d1
	beq.b		sasuccess
	bra.b		saalreadyset

sasearchfree
	moveq		#$1f,d0
satrythis
	bset		d0,d1
	beq.b		sasuccess
	dbra		d0,satrythis
saalreadyset
	moveq		#-1,d0
	bra.b		saend

sasuccess
	move.l		d1,TC_SIGALLOC(a1)
	moveq		#-1,d1
	bclr		d0,d1
	and.l		d1,TC_SIGWAIT(a1)
	and.l		d1,TC_SIGRECVD(a1)
	and.l		d1,TC_SIGEXCEPT(a1)
saend
	rts

****************************************************************************
*
*  signals = Wait( signalSet )
*  d0              d0
*

; ---------------  "just for debugging" start ------------------------
waitstart dc.b  'Wait start '
waitstartv dcb.b  31,0
	even
waitend dc.b  'Wait end '
waitendv dcb.b  31,0
	even

	; in: A1=task
dbgoutstartwait
	push d0-d1/a0-a1

	lea waitstartv(pc),a0

	move.l LN_NAME(a1),d0
	cmp.l #0,d0
	beq noname

	move.l d0,a1
	moveq #20,d0
cpyname
	move.b	(a1)+,d1
	move.b	d1,(a0)+
	cmp.b #0,d1
	beq nameset

	dbf	d0,cpyname

	bra nameset
noname
	move.l a1,d0
	bsr genhexstring
	add.l #9,a0
nameset

	move.b	#32,-1(a0)

	move.l TC_SIGWAIT(a1),d0
	bsr genhexstring

	lea waitstart(pc),a0
	bsr put_string

	pull d0-d1/a0-a1
	rts

	; in: A1=task
dbgoutendwait
	push d0/a0-a1

	lea waitendv(pc),a0

	move.l LN_NAME(a1),d0
	cmp.l #0,d0
	beq noname2

	move.l d0,a1
	moveq #20,d0
cpyname2
	move.b	(a1)+,(a0)+
	dbf	d0,cpyname2

	bra nameset2
noname2
	move.l a1,d0
	bsr genhexstring
nameset2
	lea waitend(pc),a0
	bsr put_string
	pull d0/a0-a1
	rts
; ---------------  "just for debugging" end ------------------------

exec_wait
	move.l		ThisTask(a6),a1
	move.l		d0,TC_SIGWAIT(a1)

;	bsr dbgoutstartwait

	move.w		#$4000,$dff09a     ; disable interrupts
	addq.b		#1,IDNestCnt(a6)
	bra			wloopentry
wloop
	; move task to TaskWait list

	move.b		#TS_WAIT,TC_STATE(a1)
	lea			TaskWait(a6),a0
	lea			LN_PRED(a0),a0
	move.l		LN_PRED(a0),d0
	move.l		a1,LN_PRED(a0)
	move.l		a0,(a1)
	move.l		d0,LN_PRED(a1)
	move.l		d0,a0
	move.l		a1,(a0)

	; switch to another task while waiting

	move.l		a5,a0
	lea			Switch(a6),a5
	jsr			Supervisor(a6)

	; check if signal has meanwhile been received

	move.l		a0,a5
	move.l		ThisTask(a6),a1
	move.l		TC_SIGWAIT(a1),d0

wloopentry
	move.l		TC_SIGRECVD(a1),d1
	and.l		d0,d1
	beq.b		wloop

;	bsr dbgoutendwait

	eor.l		d1,TC_SIGRECVD(a1)
	subq.b		#1,IDNestCnt(a6)
	bge			wskip
	move.w		#$c000,$dff09a	; enable interrupts
wskip
	move.l		d1,d0
	rts

****************************************************************************
*
*  message = WaitPort( port )
*  D0                  A0
*
exec_waitport
	move.l		MP_MSGLIST(a0),a1
	tst.l		(a1)
	bne.b		wpend

	move.b		MP_SIGBIT(a0),d1
	lea			MP_MSGLIST(a0),a0
	moveq		#0,d0
	bset		d1,d0
	move.l		a2,-(sp)
	move.l		a0,a2
wploop
	jsr			Wait(A6)
	move.l		(a2),a1
	tst.l		(a1)
	beq.b		wploop
	move.l		(sp)+,a2
wpend
	move.l		a1,d0
	rts

****************************************************************************
*
*  Signal( task, signals )
*          A1    D0
*

; ---------------  "just for debugging" start ------------------------
sigstart dc.b  'Signal '
sigstartv dcb.b  31,0
	even

	; in: A1=task
dbgoutsignal
	push d0-d2/a0-a1
	move.l d0,d2

	lea sigstartv(pc),a0

	move.l LN_NAME(a1),d0
	cmp.l #0,d0
	beq noname3

	move.l d0,a1
	moveq #20,d0
cpyname3
	move.b	(a1)+,d1
	move.b	d1,(a0)+
	cmp.b #0,d1
	beq nameset3

	dbf	d0,cpyname3

	bra nameset3
noname3
	move.l a1,d0
	bsr genhexstring
	add.l #9,a0
nameset3

	move.b	#32,-1(a0)

	move.l d2,d0
	bsr genhexstring

	lea sigstart(pc),a0
	bsr put_string

	pull d0-d2/a0-a1
	rts
; ---------------  "just for debugging" end ------------------------

exec_signal
;	bsr dbgoutsignal

	lea			TC_SIGRECVD(a1),a0
	move.w		#$4000,$dff09a    		; disable interrupts
	addq.b		#1,IDNestCnt(a6)
	move.l		(a0),-(sp)
	or.l		d0,(a0)

	; XXX ditch exception handling?
	move.l		TC_SIGEXCEPT(a1),d1
	and.l		d0,d1
	bne		sprocessexc

	cmp.b		#TS_WAIT,TC_STATE(a1)
	bne		sdone

	and.l		TC_SIGWAIT(a1),d0
	beq.b		sdone

	; wake up task that has been waiting for this signal
tloop0
	; move task to TaskReady list

	lea			TaskWait(a6),a0
	move.l		a1,d0
	move.l		(a1),a0
	move.l		LN_PRED(a1),a1
	move.l		a0,(a1)
	move.l		a1,LN_PRED(a0)
	move.l		d0,a1

	move.b		#TS_READY,TC_STATE(a1)

	lea			TaskReady(a6),a0
	bsr			exec_enqueue
	cmp.l		TaskReady(a6),a1
	bne.b		sdone
tloop
	subq.b		#1,IDNestCnt(a6)
	bge			senable
	move.w		#$c000,$dff09a
senable
	move.l		(sp)+,d0

	jmp			Reschedule(a6)
sprocessexc
	bset		#5,TC_FLAGS(a1)
	cmp.b		#TS_WAIT,TC_STATE(a1)

	beq.b		tloop0
	bra			tloop

sdone
	subq.b		#1,IDNestCnt(a6)
	bge			senable2
	move.w		#$c000,$dff09a		; enable interrupt
senable2
	move.l		(sp)+,d0
	rts

	; default impl for task exit
defaultasktexit
	move.l		#0,d0     ; remove current task
	move.l		d0,a1
	move.l		4,a6
	bra			exec_remtask




****************************************************************************
*
*  result = AddTask(task, initialPC, finalPC)
*  D0               A1    A2         A3
*
* it seems this is another function that quietly corrupts all kind of
* registers without restoring any.. (works as intended by Commodore) - or
* maybe the API docs found online are just bad
*

; ---------------  "just for debugging" start ------------------------
atinitmsg	dc.b 'AddTask initial PC',0
	even
dbgaddtask
	push d0/a0
	lea atinitmsg(pc),a0
	bsr put_string
	move.l a1,d0		; Task*
	bsr put_value
	move.l a2,d0		; PC
	bsr put_value
	moveq #0,d0			; prio
	move.b LN_PRI(a1),d0
	bsr put_value
	pull d0/a0
	rts
; ---------------  "just for debugging" end ------------------------


exec_addtask
	moveq		#0,d1
	move.b		#TS_ADDED,TC_STATE(a1)		; no need for this tmp state

; fixme: most of the below is probably unused in the UADE context
	move.b		d1,TC_FLAGS(a1)
	move.w		#$ffff,TC_IDNESTCNT(a1)
	move.l		TaskSigAlloc(a6),TC_SIGALLOC(a1)
	move.l		d1,TC_SIGWAIT(a1)
	move.l		d1,TC_SIGRECVD(a1)
	move.l		d1,TC_SIGEXCEPT(a1)

; just ignore: newer code should handle tc_ETask here..
;	move.w		TaskTrapAlloc(a6),TC_TRAPALLOC(a1)
;	move.w		d1,TC_TRAPALLOC+2(a1)

	tst.l		TC_TRAPCODE(a1)
	bne.b		atskiptrapdefault
	move.l		TaskTrapCode(a6),TC_TRAPCODE(a1)
atskiptrapdefault

	tst.l		TC_EXCEPTCODE(a1)
	bne.b		atskipexcepdefault
	move.l		TaskExceptCode(a6),TC_EXCEPTCODE(a1)
atskipexcepdefault

	move.l		TC_SPREG(a1),a0
	move.l		a3,-(a0)			; final PC
	bne.b		atskipexitdefault
	move.l		TaskExitCode(a6),(a0)
atskipexitdefault

	moveq		#$0e,d1
atinitstack
	clr.l		-(a0)
	dbra		d1,atinitstack

	clr.w		-(a0)
	move.l		a2,-(a0)			; initial PC

	; note: with a 68881, an additional long would need to be pushed here

;	bsr dbgaddtask

	move.l		a0,TC_SPREG(a1)        ; restore original sp

	; put task on the TaskReady

	lea			TaskReady(a6),a0

	move.w		#$4000,$dff09a     		; disable int
	addq.b		#1,IDNestCnt(a6)
	move.b		#TS_READY,TC_STATE(a1)
	bsr			exec_enqueue

	; check if new task should be run immediately

	move.l		TaskReady(a6),d0
	subq.b		#1,IDNestCnt(a6)
	bge			atskip
	move.w		#$c000,$dff09a			; endable int
atskip
	cmp.l     a1,d0
	bne.b     atskipreschedule

	; task is in front of the list
	jsr       Reschedule(a6)
atskipreschedule
	rts

****************************************************************************
*
* RemTask( task )
*           A1
*
rmtaskmsg dc.b 'RemTask not implemented',0
	even
exec_remtask		; lets presume this is irrelevant for single song handling
	push a0
	lea rmtaskmsg(pc),a0
	bsr put_string
	pull a0
	rts


*******************************************************************
*
*  task = FindTask(name)
*  D0              A1
*
exec_findtask
	move.l    a1,d0
	bne.b     ftbyname
	move.l    ThisTask(a6),d0
	rts
ftbyname
	; check TaskReady queue
	lea       TaskReady(a6),a0
	move.w    #$4000,$dff09a     	; disable interruots
	addq.b    #1,IDNestCnt(a6)
	jsr       FindName(a6)
	tst.l     d0
	bne.b     ftfound

	; check TaskWait queue
	lea       TaskWait(a6),a0
	jsr       FindName(a6)
	tst.l     d0
	bne.b     ftfound
	move.l    ThisTask(a6),a0
	move.l    LN_NAME(a0),a0
ftnext
	cmpm.b    (a0)+,(a1)+
	bne.b     ftfound
	tst.b     -1(a0)
	bne.b     ftnext

	move.l    ThisTask(a6),d0
ftfound
	subq.b    #1,IDNestCnt(a6)
	bge	      ftdone
	move.w    #$c000,$dff09a		; enable interrupts
ftdone
	rts

*******************************************************************
*
*  Forbid
*
exec_forbid
	addq.b		#1,TDNestCnt(a6)
	rts


*******************************************************************
*
*  Permit
*
exec_permit
	subq.b		#1,TDNestCnt(a6)
	bge.b		peend
	tst.b		IDNestCnt(a6)
	bge.b		peend

	; multitasking reenabled
	btst		#7,SysFlags(a6)      ; attention flag
	beq.b		peend
pestartschedule
	move.l		a5,-(sp)

	lea			pesvstart(pc),a5
	jsr			Supervisor(a6)

	move.l		(sp)+,a5
peend
	rts

pesvstart

	; fixme: UADE's Supervisor impl does not seem to put the stack into the
	; correct state and the respective frame DOES NOT contain the expected
	; SR data..

;	btst		#5,(sp)           	; check supervisor flag
;	beq.b		peschedule
	bra		peschedule				; hack
	rte
peschedule
	jmp			Schedule(a6)


*******************************************************************
*
*  Reschedule
*
exec_reschedule
	bset		#7,SysFlags(a6) 	; attention flag
	sne			d0
	tst.b		TDNestCnt(a6)
	bge.b		reend

; without this hack, MarbleMadness very quickly hangs both Tasks..
;	tst.b		IDNestCnt(a6)   	; interrupts enabled?
;	blt			pestartschedule
	bsr			pestartschedule	; hack

	tst.b		d0
	bne			reend

	; fixme: this is probably useless in UADE env.. but might need replacement?
	move.w		#$8004,$dff09c	; INTREQ (Interrupt request bits) => set "software initiated interrupt"
reend
	rts

*******************************************************************
*
*  ExitIntr
* the respecive stack frame created by UADE's Supervisor impl
* is incorrect and the below code then cannot work.. luckily it seems
* to be unused..
*
;exec_exitintr
;	btst		#5,$18(sp)			; CPU was in supervisor mode?
;	bne.b		eiend
;	move.l		4,a6
;	tst.b		TDNestCnt(a6)		; task switching isabled?
;	bge.b		eiend

;	btst		#7,SysFlags(a6)		; attention flag
;	beq.b		eiend

;	move.w		#$2000,sr			; enable all interrupts
;	bra			scstart
;eiend
;	movem.l		(sp)+,d0/d1/a0/a1/a5/a6
;	rte

*******************************************************************
*
*  Schedule
*
exec_schedule
	movem.l   d0/d1/a0/a1/a5/a6,-(sp)
;scstart

	move.w    #$2700,sr          		; mask maskable interrupts

	bclr      #7,SysFlags(a6)			; attention flag
	move.l    ThisTask(a6),a1
	btst      #5,TC_FLAGS(a1)			; EXCEPT?
	bne       scenqueueready

	lea       TaskReady(a6),a0
	cmp.l     LH_TAILPRED(a0),a0        ; empty?
	beq.b     eiend

	move.l    (a0),a0
	move.b    LN_PRI(a0),d1
	cmp.b     LN_PRI(a1),d1
	bge       scenqueueready

	btst      #6,SysFlags(a6)			; testcase: MarbleMadness
	beq.b     eiend

scenqueueready
	lea       TaskReady(a6),a0
	bsr       exec_enqueue

	move.b    #TS_READY,TC_STATE(a1)

	move.w    #$2000,sr

	; restore current task's saved registers

	movem.l   (sp)+,d0/d1/a0/a1/a5

	move.l    (sp),-(sp)

	move.l    Switch+2(a6),4(sp)
	move.l    (sp)+,a6
	rts    								; i.e. call Switch

eiend
	movem.l		(sp)+,d0/d1/a0/a1/a5/a6
	rte

*******************************************************************
*
*  Switch
*
exec_switch
	move		#$2000,sr
	move.l		a5,-(sp)
	move		usp,a5
	movem.l		d0-d7/a0-a6,-(a5)		; 60 bytes

	move.l		4,a6

	move.w		IDNestCnt(a6),d0
	move.w		#$ffff,IDNestCnt(a6)
	move.w		#$c000,$dff09a			; enable interrupts

	move.l		(sp)+,$34(a5)			; overwrite above saved A5 slot
	move.w		(sp)+,-(a5)				; sr
	move.l		(sp)+,-(a5)				; pc

	lea			restorectx(pc),a4

	move.l		ThisTask(a6),a3
	move.w		d0,TC_IDNESTCNT(a3)
	move.l		a5,TC_SPREG(a3)
	btst		#6,TC_FLAGS(a3)			; SWITCH
	beq			didispatch
	move.l		TC_SWITCH(a3),a5
	jsr			(a5)
	bra		didispatch

*******************************************************************
*
*  Dispatch
*

; ---------------  "just for debugging" start ------------------------
diswitchmsg dc.b 'Switch to',0
	even
dbgdisp
	push d0/a0
	lea diswitchmsg(pc),a0
	bsr put_string
	move.l a3,d0
	bsr put_value
	pull d0/a0
	rts
; ---------------  "just for debugging" end ------------------------

dierrormsg1 dc.b 'Dispatch unexpected lack of runnable',0
dierrormsg2 dc.b 'Dispatch - Task specific exceptions not implemented',0
	even

exec_dispatch
	lea			restorectx(pc),a4
	move.w		#$ffff,IDNestCnt(a6)
	move.w		#$c000,$dff09a			; enable interrupts

didispatch
	lea			TaskReady(a6),a0
dinextrunnable
	move		#$2700,sr				; mask maskable interrupts
	move.l		(a0),a3
	move.l		(a3),d0
	bne.b		dihanderunnable

	; should be dead code in UADE context
	lea			dierrormsg1(pc),a0
	bsr			put_string
	illegal

dihanderunnable
	move.l		d0,(a0)
	move.l		d0,a1
	move.l		a0,4(a1)
;	addq.l		#1,DispCount(a6)		; unused..

	move.l		a3,ThisTask(a6)

;	bsr dbgdisp

;	move.w		Quantum(a6),Elapsed(a6)	; unused
	bclr		#6,SysFlags(a6)			; time slice expired flag; testcase: MarbleMadness

	move.b		#TS_RUN,TC_STATE(a3)

	move.w		TC_IDNESTCNT(a3),IDNestCnt(a6)	; restore interrupt disable level
	tst.b		IDNestCnt(a6)
	bmi			diskipintdisable
	move.w		#$4000,$dff09a			; disable level >= 0

diskipintdisable
	move		#$2000,sr				; enable interrupts
	move.b		TC_FLAGS(a3),d0
	and.b		#$a0,d0
	beq.b		diskip

	; handle LAUNCH & EXCEPTION

	btst		#7,d0					; LAUNCH
	beq.b		skiplaunch
	move.b		d0,d2
	move.l		TC_LAUNCH(a3),a5

	jsr			(a5)
	move.b		d2,d0
skiplaunch
	btst		#5,d0					; EXCEPT
	bne.b		ditaskexec
diskip
	move.l		TC_SPREG(a3),a5
	jmp			(a4)


ditaskexec
	; should be dead code in UADE context
	lea			dierrormsg2(pc),a0
	bsr			put_string
	illegal


restorectx
	lea			$42(a5),a2				; skip stackframe (see below)
	move		a2,usp
	move.l		(a5)+,-(sp)				; pc
	move.w		(a5)+,-(sp)				; sr
	movem.l		(a5),d0-d7/a0-a6
	rte									; run task

* inital task used by UADE (most songs will run in the context of this one task - without any additional tasks)
playlooptask	dcb.b	TC_SIZE,0
