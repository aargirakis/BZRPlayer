**
**  $Filename: misc/DeliPlayer.i $
**  $Release: 2.0 $
**  $Revision: 2.15$
**  $Date: 02/06/95$
**
**	Definitions and Macros for creating DeliTracker Player&Genie-modules
**
**	(C) Copyright 1991, 1992, 1993, 1994, 1995 Delirium Softdesign
**	    All Rights Reserved
**

	IFND	DELITRACKER_PLAYER_I
DELITRACKER_PLAYER_I	SET	1

	IFND EXEC_PORTS_I
	    INCLUDE "exec/ports.i"
	ENDC

	IFND EXEC_TYPES_I
	    INCLUDE "exec/types.i"
	ENDC

	IFND UTILITY_TAGITEM_I
	    INCLUDE "utility/tagitem.i"
	ENDC

;----------------------------------------------------------------------------

DELIVERSION	EQU	17			; Current Version of DeliTracker
DELIREVISION	EQU	4			; Current Revision of DeliTracker


;------------------------ Player Function Offsets ---------------------------

 STRUCTURE DeliTrackerPlayer,0

	STRUCT	dtp_RTS_code,4			; RTS for security (private !)
	STRUCT	dtp_ID,8			; Identifier (private !)
	APTR	dtp_TagArray			; pointer to TagItem array
	LABEL	dtp_SIZE

* The TagItem ID's (ti_Tag values) for the player interface follow.

DTP_TagBase	EQU	TAG_USER+"DT"

	ENUM	DTP_TagBase			; TagBase

	EITEM	DTP_InternalPlayer		; obsolete
	EITEM	DTP_CustomPlayer		; player is a customplayer

	EITEM	DTP_RequestDTVersion		; minimum DeliTracker version needed
	EITEM	DTP_RequestKickVersion		; minimum KickStart version needed
DTP_RequestV37	EQU	DTP_RequestKickVersion	; obsolete

	EITEM	DTP_PlayerVersion		; actual player version & revision
	EITEM	DTP_PlayerName			; name of this player
	EITEM	DTP_Creator			; misc string

	EITEM	DTP_Check1			; Check Format before loading
	EITEM	DTP_Check2			; Check Format after file is loaded
	EITEM	DTP_ExtLoad			; Load additional files
	EITEM	DTP_Interrupt			; Interrupt routine
	EITEM	DTP_Stop			; Clear Patterncounter
	EITEM	DTP_Config			; Config Player
	EITEM	DTP_UserConfig			; User-Configroutine
	EITEM	DTP_SubSongRange		; Get min&max subsong number

	EITEM	DTP_InitPlayer			; Initialisize the Player
	EITEM	DTP_EndPlayer			; Player clean up
	EITEM	DTP_InitSound			; Soundinitialisation routine
	EITEM	DTP_EndSound			; End sound
	EITEM	DTP_StartInt			; Start interrupt
	EITEM	DTP_StopInt			; Stop interrupt

	EITEM	DTP_Volume			; Set Volume
	EITEM	DTP_Balance			; Set Balance
	EITEM	DTP_Faster			; Incease playspeed
	EITEM	DTP_Slower			; Decrease playspeed
	EITEM	DTP_NextPatt			; Jump to next pattern
	EITEM	DTP_PrevPatt			; Jump to previous pattern
	EITEM	DTP_NextSong			; Play next subsong
	EITEM	DTP_PrevSong			; Play previous subsong

	;--- functions in revision 14 (distributed as Release 1.35) ---

	EITEM	DTP_SubSongTest			; Test, if given subsong is vaild

	;--- functions in revision 16 (distributed as Release 2.01) ---

	EITEM	DTP_NewSubSongRange		; enhanced replacement for DTP_SubSongRange

	EITEM	DTP_DeliBase			; the address of a pointer where DT
						; stores a pointer to the DeliGlobals

	EITEM	DTP_Flags			; misc Flags (see below)

	EITEM	DTP_CheckLen			; Length of the Check Code

	EITEM	DTP_Description			; misc string

	EITEM	DTP_Decrunch			; pointer to Decrunch Code
	EITEM	DTP_Convert			; pointer to Converter Code

	EITEM	DTP_NotePlayer			; pointer to a NotePlayer Structure
	EITEM	DTP_NoteStruct			; the address of a pointer to the
						; NoteStruct Structure
	EITEM	DTP_NoteInfo			; a pointer where DT stores a pointer
						; to the current NoteStruct Structure
	EITEM	DTP_NoteSignal			; pointer to NoteSignal code

	EITEM	DTP_Process			; pointer to process entry code
	EITEM	DTP_Priority			; priority of the process
	EITEM	DTP_StackSize			; stack size of the process
	EITEM	DTP_MsgPort			; a pointer where DT stores a pointer
						; to a port to send its messages

	EITEM	DTP_Appear			; open your window, if you can
	EITEM	DTP_Disappear			; go dormant

	EITEM	DTP_ModuleName			; get the name of the current module
	EITEM	DTP_FormatName			; get the name of the module format
	EITEM	DTP_AuthorName			; not implemented yet

	;--- functions in revision 17 (distributed as Release 2.07) ---

	EITEM	DTP_InitNote			; NoteStruct initialization

*** end of player interface enumeration ***


; --- various flags ---------------------------------------------------------

	BITDEF	PLY,CUSTOM,0			; the player is a customplayer
	BITDEF	PLY,SONGEND,1			; this player supports songend

	;--- flags defined in revision 17 (distributed as Release 2.07) ---

	BITDEF	PLY,ANYMEM,2			; modules of this player don't require chipmem


; --- DeliTracker message ---------------------------------------------------

   STRUCTURE DeliMessage,MN_SIZE
	ULONG	DTMN_Function			; function pointer
	ULONG	DTMN_Result			; store the result here
   LABEL DTMN_SIZE


;------------------------------ Player Header -------------------------------
;
; Here is the MACRO for creating the player header structure. Use this MACRO !!!

PLAYERHEADER	MACRO
	IFC	'\2',''
	moveq	#-1,d0			; this should return an error
	rts				; in case someone tried to run it
	ELSE
	bra.w	\2			; branch to startupcode
	ENDC
	dc.b	'DELIRIUM'		; identifier
	dc.l	\1			; ^tagitem array
	ENDM


	;------ When a subroutine in the player is called, A5 will contain
	;------ the pointer to the DeliTrackerGlobals, the only exeption is
	;------ of course the interrupt routine.
	;------ The interruptroutine is called every 1/50 sec (via timerint).

	;------ When Check is called, supply d0=0 if the format is ok
	;------ else d0<>0.

	;------ Check1 is called before loading the complete file, you can
	;------ check in the first 1024 Bytes of the file. If the file is
	;------ smaller than 1kB, the remaining space will contain zero.

	;------ Check2 is called after the complete file is loaded, you
	;------ can use dtg_ChkSize to determine the length of the file.
	;------ If you supply this tag the file can be crunched.

	;------ ExtLoad: routine for loading additional files (instruments).
	;------	If successful, you must return d0=0 else d0<>0. In case of
	;------ an error DeliTracker frees all memory used for this module.

	;------	InitPlayer: Here you should allocate the audio channels.
	;------ In case the player supports multi-modules, you must set here
	;------ dtg_SndNum to the minimal subsong number (not necessary if
	;------ you have supplied a DTP_SubSongRange routine!).
	;------ If successful, you must return d0=0 else d0<>0.

	;------	EndPlayer: Here you should free the audio channels.

	;------ InitSound: If you want to use the internal interrupt but don't
	;------ need the default 50 Hz frequency, you can write another timer
	;------ value into dtg_Timer.

	;------ It is recommended to use DTP_SubSongRange/DTP_SubSongTest
	;------ instead of DTP_NextSong/DTP_PrevSong.

	;------ Volume usually only copies the values dtg_Volume, dtg_SndLBal
	;------ and dtg_SndRBal to an internal buffer. The interrupt code has
	;------ access to this buffer and can set the volume correct.

	;------ CheckLen: This tag is only allowed for players. If you supply
	;------ this tag, the player will be unloaded by DeliTracker in low
	;------ memory situations. When needed, it will be loaded again
	;------ automatically.

	;------ Decrunch: Supply d0=0 if you could decrunch the file else
	;------ d0<>0.

	;------ Convert: Supply d0=0 if you converted the file to another
	;------ format, else d0<>0.

	;------ Appear: Supply d0<>0 if the window was already opend, else
	;------ d0=0.

	;------ Disappear: Supply d0=0 if the window was already closed, else
	;------ d0<>0.

	;------ ModuleName: This tag is only allowed for players. It contains
	;------ the address of a pointer to the module name (must be null
	;------ terminated). The tag is evaluated after the InitPlayer function
	;------ was called.

	;------ FormatName: This tag is only allowed for convert genies. It
	;------ contains the address of a pointer to the name of the module
	;------ format (must be null terminated). The tag is evaluated after
	;------ the InitPlayer function was called.

	;------ AuthorName: This tag is only allowed for players. It contains
	;------ the address of a pointer to the authorname (must be null
	;------ terminated). The tag is evaluated after the InitPlayer function
	;------ was called.

	;------ Note: the Player can consist of more Hunks. That means you
	;------ can seperate CHIP DATA form CODE (and you should do this!).


;---------------------------- Global Variables ------------------------------

 STRUCTURE DeliTrackerGlobals,0

	;------ if you use dtg_AslBase, make sure that
	;------ DTP_RequestDTVersion is at least 13 !

	APTR	dtg_AslBase		; librarybase don't CloseLibrary()

	APTR	dtg_DOSBase		; librarybase -"-
	APTR	dtg_IntuitionBase	; librarybase -"-
	APTR	dtg_GfxBase		; librarybase -"-
	APTR	dtg_GadToolsBase	; librarybase -"- (NULL for Kick 1.3 and below)
	APTR	dtg_ReservedLibraryBase	; reserved for future use

	APTR	dtg_DirArrayPtr		; Ptr to the directory of the current module
	APTR	dtg_FileArrayPtr	; Ptr to the filename of the current module
	APTR	dtg_PathArrayPtr	; Ptr to PathArray (e.g used in LoadFile())

	APTR	dtg_ChkData		; pointer to the module to be checked
	ULONG	dtg_ChkSize		; size of the module

	UWORD	dtg_SndNum		; current sound number
	UWORD	dtg_SndVol		; volume (ranging from 0 to 64)
	UWORD	dtg_SndLBal		; left volume (ranging from 0 to 64)
	UWORD	dtg_SndRBal		; right volume (ranging from 0 to 64)
	UWORD	dtg_LED			; filter (0 if the LED is off)
	UWORD	dtg_Timer		; timer-value for the CIA-Timers

	FPTR	dtg_GetListData		;
	FPTR	dtg_LoadFile		;
	FPTR	dtg_CopyDir		;
	FPTR	dtg_CopyFile		;
	FPTR	dtg_CopyString		;
	FPTR	dtg_AudioAlloc		;
	FPTR	dtg_AudioFree		;
	FPTR	dtg_StartInt		;
	FPTR	dtg_StopInt		;
	FPTR	dtg_SongEnd		; save to call from interrupt code !
	FPTR	dtg_CutSuffix		;

	;------ extension in revision 14

	FPTR	dtg_SetTimer		; save to call from interrupt code !

	;------ extension in revision 15

	FPTR	dtg_WaitAudioDMA	; save to call from interrupt code !

	;------ extension in revision 16

	FPTR	dtg_LockScreen
	FPTR	dtg_UnlockScreen
	FPTR	dtg_NotePlayer		; save to call from interrupt code !
	FPTR	dtg_AllocListData
	FPTR	dtg_FreeListData

	FPTR	dtg_Reserved1		; do not use !!!
	FPTR	dtg_Reserved2		; do not use !!!
	FPTR	dtg_Reserved3		; do not use !!!

	; There is no dtg_SIZEOF cause ...


	;------ GetListData(Num:d0): This function returns the memorylocation
	;------ of a loaded file in a0 and its size in d0. Num starts with 0
	;------ (the selected module). Example: GetListData(2) returns the
	;------ start of the third file loaded (via ExtLoad) in a0 an its size
	;------ in d0.

	;------ LoadFile(): this function may only be called in the ExtLoad
	;------ routine. file/pathname must be in dtg_PathArrayPtr then
	;------ this function will attempt to load the file into CHIPMEM
	;------ (and DECRUNCH it). If everything went fine, d0 will be zero.
	;------ If d0 is not zero this indicates an error (e.g. read error,
	;------ not enough memory, ...).

	;------ CopyDir(): this function copies the pathname at the end
	;------ of the string in dtg_PathArrayPtr(a5).

	;------ CopyFile(): this function copies the filename at the end
	;------ of the string in dtg_PathArrayPtr(a5).

	;------ CopyString(Ptr:a0): this function copies the string in a0
	;------ at the end of the string in dtg_PathArrayPtr(a5).

	;------ AudioAlloc(): this function allocates the audiochannels
	;------ (only necessary if the player doesn't supply a NoteStruct
	;------ tag). If d0=0 all is ok, d0<>0 indicates an error.

	;------ AudioFree(): this function frees the audiochannels allocated
	;------ with AudioAlloc().

	;------ StartInt(): this function starts the timer-interrupt.

	;------ StopInt(): this function stops the timer-interrupt started
	;------ with StartInt().

	;------ SongEnd(): signal the songend to DeliTracker.
	;------ This call is guaranteed to preserve all registers.

	;------ CutSuffix(): this function removes the suffix '.xpk' or '.pp'
	;------ from the string in dtg_PathArrayPtr(a5).

	;------ SetTimer(): programs the CIA-Timer with the value supplied
	;------ in dtg_Timer(a5). Only useful, if the internal timer-interrupt
	;------ is used. This call is guaranteed to preserve all registers.

	;------ WaitAudioDMA(): DMA delay wait. Only allowed, if the internal
	;------ timer-interrupt is used. This call is guaranteed to preserve
	;------ all registers.

	;------ LockScreen(): this function tries to lock DeliTracker's screen.
	;------ It returns the screenpointer in d0 or NULL on failure.

	;------ UnlockScreen(): this function unlocks DeliTracker's screen.
	;------ do not unlock a screen more times than it was locked!

	;------ NotePlayer(): this call plays the notes specified in the
	;------ current NoteStruct structure. This function call is not allowed
	;------ if the active player doesn't have a valid NotePlayer structure.
	;------ do not call this function in interrupt code at interrupt
	;------ level 5 or higher! This call is guaranteed to preserve
	;------ all registers.

	;------ AllocListData(Size:d0/Flags:d1): This is the memory allocator
	;------ for module specific memory to be used by all players and genies.
	;------ It provides a means of specifying that the allocation should be
	;------ made in a memory area accessible to the chips, or accessible to
	;------ shared system memory. If the allocation is successful,
	;------ DeliTracker will keep track of the new block (GetListData() will
	;------ return the location and size of this block).
	;------ byteSize - the size of the desired block in bytes.
	;------ Flags - the flags are passed through to AllocMem().
	;------ A pointer to the newly allocated memory block is returned in d0.
	;------ If there are no free memory regions large enough to satisfy the
	;------ request, zero will be returned. The pointer must be checked
	;------ for zero before the memory block may be used!

	;------ FreeListData(MemBlock:a1): Free a region of memory allocated
	;------ with AllocListData(), returning it to the system pool from which
	;------ it came.
	;------ memoryBlock - pointer to the memory block to free, or NULL.


	ENDC	; DELITRACKER_PLAYER_I



