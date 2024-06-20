**
**	$Filename: misc/EaglePlayerEngine.i
**	$Release: 2.01 $
**	$Revision: 16$
**	$Date: 03/16/98$
**
** Definitions and Macros for creating EaglePlayer Engines & Players
**
**	(C) Copyright 1994-98 by DEFECT
**		All Rights Reserved
**
**

	IFND	EAGLEPLAYERENGINE_I
EAGLEPLAYERENGINE_I SET	1

	IFND	EXEC_PORTS_I
		INCLUDE "exec/ports.i"
	ENDC

	IFND	EXEC_TYPES_I
		INCLUDE "exec/types.i"
	ENDC

	IFND	UTILITY_TAGITEM_I
		INCLUDE "utility/tagitem.i"
	ENDC

	IFND	EXEC_NODES_I
		INCLUDE "exec/nodes.i"
	ENDC

	IFND	EAGLEPLAYER_I
		INCLUDE "Misc/EaglePlayer.i"
	ENDC

*---------------------------- macros --------------------------------------*
RS_RESET	MACRO
RSOFFSET	SET	0
		ENDM

RS_BYTE		MACRO
\1		EQU	RSOFFSET
RSOFFSET	SET	RSOFFSET+\2
		ENDM

RS_WORD		MACRO
\1		EQU	RSOFFSET
RSOFFSET	SET	RSOFFSET+2*\2
		ENDM

RS_LONG		MACRO
\1		EQU	RSOFFSET
RSOFFSET	SET	RSOFFSET+4*\2
		ENDM
*---------------------------- PlayerInfo Tags -----------------------------*
EPPI_TagBase	EQU TAG_USER+20553		;TAG_USER+"PI"

	ENUM	EPPI_TagBase			; TagBase

	EITEM	EPPI_Creator			;Ptr to Creator
	EITEM	EPPI_CreatorYearPrg		;Date for Program
	EITEM	EPPI_CreatorYearPlayer		;Date for Eagleplayer
	EITEM	EPPI_Adapted			;Ptr to Adaptor
	EITEM	EPPI_About			;TI_Data points on a string
						;telling something about this
						;Soundsystem
	EITEM	EPPI_MaxPattern			;Max Pattern (SoundTr. 64)
	EITEM	EPPI_MaxPatternLength		;Max PatternLen (SoundTr. 64)
	EITEM	EPPI_MaxLength			;Max Length (SoundTr. 128)
	EITEM	EPPI_MaxTracks			;Max Tracks (SonicA.)
	EITEM	EPPI_MaxVoices			;Max Voices
	EITEM	EPPI_MaxSubSongs			;Max SubSongs
	EITEM	EPPI_MaxSteps			;Max Steps
	EITEM	EPPI_MaxSamples			;Max Samples
	EITEM	EPPI_MaxSynthSamples		;Max SynthSamples
	EITEM	EPPI_SpecialInfo			;Pointer to InfoText
	EITEM	EPPI_UnPackedSystem		;MIUS (see below) or APTR
						;to Soundsysname
	EITEM	EPPI_Prefix			;Save-Prefix (e.g. "MOD." or
						;"Mdat.")
	EITEM	EPPI_Prefix2			;for SampleData ("Smpl.")
	EITEM	EPPI_NeededProzessor
	EITEM	EPPI_NeededKickStart
	EITEM	EPPI_ReplayerSizeChip
	EITEM	EPPI_ReplayerSizeFast
	EITEM	EPPI_Flags
	EITEM	EPPI_AssessmentFileFormat		;Note from 0 to 6
	EITEM	EPPI_AssessmentMultitaskingSure
	EITEM	EPPI_AssessmentBugs
	EITEM	EPPI_AssessmentTempo
	EITEM	EPPI_AssessmentReplayer


*--------------------------- PlayerInfo-Flags ----------------------------*
EPPIF_SelfModify	EQU	1
EPPIF_ReplayerInFront	EQU	2
EPPIF_FilesSpitted	EQU	3
EPPIF_CiaTiming		EQU	4
EPPIF_RealPattern	EQU	5
EPPIF_MidiSupport	EQU	6
EPPIF_AudioInterrupt	EQU	7
EPPIF_SoftInt		EQU	8




**----------------------- User-Program-Start-Structure -----------------**
**	This Structure must be initialized in the executable file	**
**	EUS_Next,EUS_UserNr don`t use, important !!!			**
**	EUS_Flags,EUS_WinX,EUS_WinY,EUS_Special can be initialized	**
**	(will be changed if prefs are loaded :)-=-=	)			**

 STRUCTURE Engine_START,0
	ULONG	EUS_Jmp			;bra.s (bra.b) to the real start of the
					;userprogram (usually direct after this
					;struct	(MUST be a BRA.W !!!!!)
	STRUCT	EUS_Identifier,8	;Identifier "EPEngine"
					;Eagleplayer can use this (Ampifiers)
	APTR	EUS_Next		;Don`t Use !!! only for Eagleplayer
	UWORD	EUS_UserNr		;don`t change, only for Eagleplayer

	APTR	EUS_EPBase		;Pointer to the Eagleplayerglobals
	APTR	EUS_FreeTable
	APTR	EUS_TaskAdr		;if zero=kein Task
	APTR	EUS_Unused1		;Future use (don't change)
	APTR	EUS_Unused2		;Future use (don't change)
	APTR	EUS_SpecialJumpTab	;private Jumptab (e.g Amplifiers)
	APTR	EUS_TagList
	UWORD	EUS_Ticks		;Ticks for Interrupt (0=no Interrupt)
	UWORD	EUS_TickCounter
	ULONG	EUS_TickFlags
	ULONG	EUS_MsgFlags

	APTR	EUS_PName		;Processname of this Engine
					;e.g. 'Analyzer.Task'
	APTR	EUS_Creator		;Creatorname e.g. "BUGGS" or "Eagleeye"
	APTR	EUS_AboutEngine		;tells something about this Engine.
	UWORD	EUS_KickStart		;min Kickversion 0=All , 37 = OS 2.0
	ULONG	EUS_EPVersion		;min. EagleplayerVersion
	UWORD	EUS_Version		;Engine. Version	eg. 37 \not very
	UWORD	EUS_Revision		;Engine. Revision eg.175 /important
	APTR	EUS_UserName		;Name for PullDownMenu (max.14 Chars)
	UWORD	EUS_WinX		;User-Window X-Position
	UWORD	EUS_WinY		;User-Window Y-Position
	UWORD	EUS_Flags		;Flags (Iconify/ZipWindow,Window Open),
					;see below
	ULONG	EUS_Special		;SpecialInfos (e.g. AnalyzerMode)
	ULONG	EUS_Special2		;SpecialInfos (e.g. Windowsize)
	ULONG	EUS_Special3		;more SpecialInfos

	ULONG	EUS_CreatorDate		;Creatordate (e.g 14.b 6. 1993.w)
	UBYTE	EUS_Priority		;TaskPriority of Engine
	UBYTE	EUS_Type		;Userprogramtype (0=unknown)

	UWORD	EUS_Reserved2		;set zero, don`t change
	ULONG	EUS_AMIDNr		;ID Kennung des Amplifiers,darf nur einmal vorkommen
	ULONG	EUS_Reserved4		;set zero, don`t change
	ULONG	EUS_AMUPSStruct		;set zero, don`t change
	ULONG	EUS_Reserved6		;set zero, don`t change

	LABEL	EUS_SizeOF

EUS_EngineNr	EQU	EUS_UserNr
EUS_EngineName	EQU	EUS_UserName
EUS_AboutUPrg	EQU	EUS_AboutEngine

EUS_AMPriority	EQU	EUS_Special	;Priorität des Amplifiers
					;(zum Einsortieren)
EUS_AMFlags	EQU	EUS_Special+2	;Flags des Amplifiers (Enable/Disable)

*--------------------------------- EUS-Flags --------------------------------*
EUSF_Openwin	EQU	0	;Window to be opened when started ?
				;Bit set = yes
EUSF_Zipwin	EQU	1	;Zipwindow/Iconify when started ?
				;Bit set = yes
EUSF_Disable	EQU	2	;Amplifier disabled ?
				;Bit set = yes
EUSF_Show	EQU	3	;Show (e.g. Extractor)

EUSF_Activate	EQU	EUSF_Openwin

EUSB_Openwin	EQU	1<<EUSF_Openwin
EUSB_Zipwin	EQU	1<<EUSF_Zipwin
EUSB_Disable	EQU	1<<EUSF_Disable
EUSB_Show	EQU	1<<EUSF_Show
EUSB_Activate	EQU	1<<EUSF_Activate


*--------------------------------- EUS-NPFlags ------------------------------*

ENPF_NoWindow	EQU	0		;no Window to show/hide ?
ENPB_NoWindow	EQU	1<<ENPF_NoWindow


*----------------------------- EUS-InterruptFlags ----------------------------*
EUIF_Always	EQU	1
EUIF_OnlyPlay	EQU	2
EUIF_OnlyActive	EQU	3
EUIF_TickCounter	EQU	4

EUIB_Always	EQU	1<<EUIF_Always
EUIB_OnlyPlay	EQU	1<<EUIF_OnlyPlay
EUIB_OnlyActive	EQU	1<<EUIF_OnlyActive
EUIB_TickCounter	EQU	1<<EUIF_TickCounter

*------------------------ Amplifiers direct Jumps ---------------------------*

 STRUCTURE Amplifier_Jumps,0
	FPTR	AMJ_CheckFeatures	;Test, ob der Amplifier Spezialfeatures wie z.B. 16 Bit Samples
					;unterstützt, optional

	FPTR	AMJ_Init		;Übergabe der Amplifiertagliste
	FPTR	AMJ_StartInt		;Alloc Audio & Start Int
	FPTR	AMJ_StopInt		;Free Audio & Stop Int
	FPTR	AMJ_End			;mem freigeben, etc.
	FPTR	AMJ_Amplifier		;Übergabe einer neu gefüllten AS_-Struktur an den Amplifier durch das Replay

	FPTR	AMJ_PokeAdr		;nur für Amplifier initialisieren,
	FPTR	AMJ_PokeLen		;die auf die Hardware poken, wie
	FPTR	AMJ_PokePer		;z.B. der Chipram Player, sonst
	FPTR	AMJ_PokeVol		;auf 0 setzen, diese Jumps werden mit
	FPTR	AMJ_DMAMask		;den selben Parametern aufgerufen, wie die ENPP_Pokes

	FPTR	AMJ_Command		;spezielle Kommandos ,z.B Filter
					;Aufruf: D0 - Command
					;        D1 - Argument

AMCMD_Filter	EQU	1;D1: 0=off;1=on;-1=toggle

*------------------------------ EUS-Enginetypes -----------------------------*
EUTY_Unknown		EQU	0
EUTY_Manager		EQU	1
EUTY_SampleInfo		EQU	2
EUTY_EngineInfo		EQU	3
EUTY_PlayerInfo		EQU	4
EUTY_Decruncher		EQU	5
EUTY_Ripper		EQU	6
EUTY_MainWindow		EQU	7
EUTY_ScreenManager	EQU	8
EUTY_DirViewer		EQU	9
EUTY_Analyzer		EQU	10
EUTY_Amplifier		EQU	11
EUTY_PatternScroll	EQU	12
EUTY_MessageWindow	EQU	13
EUTY_ListAdministration	EQU	14
EUTY_AmplifierManager	EQU	15
EUTY_PlayerLoader	EQU	16
EUTY_Extractor		EQU	17
EUTY_Scope		EQU	18
EUTY_ModuleInfo		EQU	19
EUTY_Converter		EQU	20
EUTY_FormatLoader	EQU	21
EUTY_SampleSaver	EQU	22


*------------------------------- Engine MsgFlags ----------------------------*
*------- If you want to get special Messages you must set this flags --------*
USMF_Zipwin		EQU	0
USMF_NewEngineLoaded	EQU	1
USMF_ChangeConfig	EQU	2	*for WinPos,ZipPos
USMF_Command		EQU	3
USMF_NewModule		EQU	4
USMF_NewSong		EQU	5
USMF_NewPreference	EQU	6
USMF_ChangeInterrupt	EQU	7
USMF_PlaySample		EQU	8
USMF_NewScrollText	EQU	9
USMF_NewDirectory	EQU	10
USMF_NewPlayer		EQU	11
USMF_KillModule		EQU	12
USMF_Surface		EQU	13
USMF_ActiveSurface	EQU	14
USMF_AmplifiersChanged	EQU	15	;Änderung in Amplifierlist
USMF_RemPlayerList	EQU	17
USMF_RemEnginesList	EQU	18
USMF_RemModulesList	EQU	19
USMF_SaveConfig		EQU	20	;for Amplifierconfig
USMF_Configuration	EQU	21	;Configwindow (e.g. Extractor)
USMF_Show		EQU	22
USMF_LoadConfig		EQU	23
USMF_WaitPointer	EQU	24

USMB_Zipwin		EQU	1<<USMF_Zipwin
USMB_NewEngineLoaded	EQU	1<<USMF_NewEngineLoaded
USMB_ChangeConfig	EQU	1<<USMF_ChangeConfig
USMB_Command		EQU	1<<USMF_Command
USMB_NewModule		EQU	1<<USMF_NewModule
USMB_NewSong		EQU	1<<USMF_NewSong
USMB_NewPreference	EQU	1<<USMF_NewPreference
USMB_ChangeInterrupt	EQU	1<<USMF_ChangeInterrupt
USMB_PlaySample		EQU	1<<USMF_PlaySample
USMB_NewScrollText	EQU	1<<USMF_NewScrollText
USMB_NewDirectory	EQU	1<<USMF_NewDirectory
USMB_NewPlayer		EQU	1<<USMF_NewPlayer
USMB_KillModule		EQU	1<<USMF_KillModule
USMB_Surface		EQU	1<<USMF_Surface
USMB_ActiveSurface	EQU	1<<USMF_ActiveSurface
USMB_AmplifersChanged	EQU	1<<USMF_AmplifiersChanged
USMB_RemPlayerList	EQU	1<<USMF_RemPlayerList
USMB_RemEnginesList	EQU	1<<USMF_RemEnginesList
USMB_RemModulesList	EQU	1<<USMF_RemModulesList
USMB_SaveConfig		EQU	1<<USMF_SaveConfig
USMB_Configuration	EQU	1<<USMF_Configuration
USMB_Show		EQU	1<<USMF_Show
USMB_LoadConfig		EQU	1<<USMF_LoadConfig
USMB_WaitPointer	EQU	1<<USMF_WaitPointer

*----------------------------- EP-Userprogram-Tags ---------------------------*
EUT_TagBase	EQU	TAG_USER+21843	;TAG_USER+"US"		;EagleplayerUserTags
	ENUM	EUT_TagBase		;EaglePlayer-TagBase


	EITEM	EUT_EngineFlags		*Userflags

	EITEM	EUT_InitEngine		*Wird aufgerufen, wenn ein Engine ge-
					*laden wird und kein Task ist.
					*Die richtige Konfiguration ist bereits
					*in der EUS-Strukture enthalten !
					*Bei einem Error wird das Engine
					*entfernt !!!!!!!!!!!!!!!!!!
					*INPUT : -
					*OUTPUT: d0=NULL oder Error
	EITEM	EUT_ExitEngine		*Das Engine wird beendet
					*INPUT : -
					*OUTPUT: d0=NULL oder Error
	EITEM	EUT_Activate		*Das Engine startet seine Arbeit
					*INPUT : -
					*OUTPUT: d0=NULL oder Error
	EITEM	EUT_DeActivate		*Das Engine beendet seine Arbeit
					*INPUT : -
					*OUTPUT: d0=NULL oder Error
	EITEM	EUT_Show		*Open Window
					*output:d0=Null oder Error
	EITEM	EUT_Hide		*Open Window
	EITEM	EUT_Config		*Das Engine startet eine Configuration
					*INPUT : -
					*OUTPUT: d0=NULL oder Error
	EITEM	EUT_GetConfig		*Das UserProgramm soll seine aktuelle
					*Configuration in den Merkzellen der
					*EUS-Strukture schreiben
					*INPUT : -
					*OUTPUT: d0=Error oder NULL
					*	EUS-Strukture gefüllt
	EITEM	EUT_NewConfig		*Eine neue Config für dieses Engine
					*wurde festgelegt (->EUS-Strukture)
					*INPUT : EUS-Strukture gefüllt
					*OUTPUT: d0=Error oder NULL

	EITEM	EUT_OpenAWindow		*Öffnet ein Window vom angegebenen Type
					*z.B. Surface, Playerwindow,TextrEQUest
					*INPUT : d0=Type des Windows
					*	d1=Pos X oder -1 für Unknown
					*	d2=Pos Y oder -1 für Unknown
					*	a0=Title des Windows
					*	d3-a3=SpecialInfo
					*OUTPUT: d0=NULL oder Error
	EITEM	EUT_CloseAWindow	*Öffnet ein Window vom angegebenen Type
					*z.B. Surface, Playerwindow,TextrEQUest
					*INPUT : d0=Type des Windows
					*	d1=Pos X oder -1 für Unknown
					*	d2=Pos Y oder -1 für Unknown
					*	d3-a3=SpecialInfo
					*OUTPUT: d0=NULL oder Error
	EITEM	EUT_GetWindowHandle	*es handelt sich um ein normales Int.
					*Window. AppWindow,WaitPointer,Menues
					*werden automatisch bearbeitet.
					*Input:	d0=Windowtype
					*Output: d0=WinHandle oder Error
	EITEM	EUT_SetWaitPointer	*Input:	-
					*Output: d0=Error oder NULL
	EITEM	EUT_ClearWaitPointer	*Input:	-
					*Output: d0=Error oder NULL
	EITEM	EUT_ChangeAWindowPosition * Das Window wird in seiner Position
					* im Screen verschoben
					*INPUT : d0=Type des Windows
					*	d1=abs. X-Position
					*	d2=abs. Y-Position
					*OUTPUT: d0=NULL oder Error
	EITEM	EUT_Iconify		*Die Engine soll in den Iconify-Mode
					*usefull für EP-Surfaces
					*INPUT : d0=interne EP-IconifyMode
					*OUTPUT: d0=NULL oder Error
					*	d1=NULL (EP-Mode)
					*		<> NULL (SurfaceMode)
	EITEM	EUT_UnIconify		*Die Engine soll zurück in Normal-
					*Mode
					*usefull für EP-Surfaces
					*INPUT : d0=IconifyMode
					*OUTPUT: d0=NULL oder Error
	EITEM	EUT_ZipAWin		*Das Window soll in den ZipMode
					*INPUT : d0=Type des Windows
					*	d1=Pos X oder -1 für Unknown
					*	d2=Pos Y oder -1 für Unknown
					*	d3-a3=SpecialInfo
					*OUTPUT: d0=NULL oder Error
	EITEM	EUT_UnZipAWin		*Das Window soll in den UnZipMode
					*INPUT : d0=Type des Windows
					*	d1=Pos X oder -1 für Unknown
					*	d2=Pos Y oder -1 für Unknown
					*	d3-a3=SpecialInfo
					*OUTPUT: d0=NULL oder Error

	EITEM	EUT_Changelocale	*Die Localisation wird geändert
					*INPUT : ?
					*OUTPUT: d0=NULL oder Error
	EITEM	EUT_UpdateVER		*Updated alle Versionsnummern des EP
					*in den Texten des Userprogramms
					*INPUT : d0=0 -> nicht registriert
					*		1 -> registriert
					*	a0=Zeiger auf Versionstring
					*		4 Zeichen ("1.60")
					*OUTPUT: d0=Error oder NULL

	EITEM	EUT_DisableAll		*Alle Gadgets und Menustrips werden
					*disabled. Vorbereitung für ChangeGui !
					*INPUT : -
					*OUTPUT: d0=Error oder NULL
	EITEM	EUT_EnableAll		*Alle Gadgets und Menustrips werden
					*enabled. Vorbereitung für ChangeGui !
					*INPUT : -
					*OUTPUT: d0=Error oder NULL
	EITEM	EUT_SignalReceived	*INPUT:	d0=SignalSet
					*Output: d0=ActionNr oder NULL
					*Es wurden Signale empfangen. Die zur
					*angegebenen WaitMask gehören
					*Hinweis, EUT_GetMSG wird danach vom
					*EP aufgerufen
	EITEM	EUT_GetWaitMask		*Gibt dem EP die Signalbits auf die
					*zusätzlich zu den EP-internen gewartet
					*werden soll
					*INPUT : -
					*OUTPUT: d0=SignalSet oder -1
	EITEM	EUT_GetInterruptMask	*Diese SignalMaske wird beim Ablauf des
					*Interrupts (EUS_Count) gesendet.
					*Input: --
					*Output: d0=SignalNr
	EITEM	EUT_GetMsg		*Testen im Engine, ob ein Gadget ge-
					*drückt wurde (für Surface)
					*INPUT : d0=Signal oder -1
					*OUTPUT: d0=EPNr oder NULL
					*	 a0=Hook
					*	 d1=?
					*	 d2/a2 = Arg1
					*	 d3/a3 = Arg2
	EITEM	EUT_ClearMenuStrip	*Löscht die entsprechenden MenuStrips
					*in allen Engines
					*INPUT : a1=MenuStrip (Test auf Zero)
					*OUTPUT: d0=NULL oder Error
	EITEM	EUT_SetMenuStrip	*Setzt die MenuStrips in allen
					*offenen Engines
					*INPUT : a1=MenuStrip (Test auf Zero)
					*OUTPUT: d0=NULL oder Error
	EITEM	EUT_Unused5
	EITEM	EUT_Unused4
	EITEM	EUT_Command		*Ein Commando aus der UCM_Tabelle wird
					*oder soll ausgeführt werden
					*INPUT : d0=Nr der Aktion
					*	d1-d7=Parameter
					*	a0-a3=Parameter
					*OUTPUT: d0=Error oder NULL
	EITEM	EUT_NewModule		*Es wurde ein neues Module geladen
					*INPUT : d0=Size des Modules
					*	a0=Name des Modules
					*	a1=SoundSystem
					*	a2=PlayerTagListe
					*	a3=ModListeStructure
					*OUTPUT: -
	EITEM	EUT_NewSong		*Der angegebene Song wird abgespielt
					*INPUT : d0=Nr des Songs
					*OUTPUT: -
	EITEM	EUT_NewPreference	*In der Eagleplayer-Preference wurde
					*eine Einstellung geändert. Die Para
					*meter werden je nach Aktion genutzt
					*INPUT : d0=Nr der Aktion
					*	d1-d7=Parameter
					*	a0-a3=Parameter
					*OUTPUT: d0=Error oder NULL
	EITEM	EUT_StartInt		*Der EP-Interrupt wurde gestartet
					*INPUT : -
					*OUTPUT: -
	EITEM	EUT_StopInt		*Der EP-Interrupt wurde gestoppt
					*INPUT : -
					*OUTPUT: -
	EITEM	EUT_PlaySample		*Das angegebene Sample wird abgespielt
					*Audiokanäle sind allokiert !
					*INPUT : d0=Nr des Samples
					*	d1=PlayMode (Loop,NoLoop)
					*	d2=Offset vom Start
					*	d3=Offset vom End
					*	a0=SampleInfostrukture
					*OUTPUT: d0=Error oder NULL
	EITEM	EUT_ClearTextWindow	*Das TextWindow wird gelöscht.
	EITEM	EUT_NewScrollText	*Der angegebene ScrollText soll ge-
					*scrollt werden. Er kann unendlich lang
					*sein
					*INPUT : a0=ScrollText
					*OUTPUT: d0=Error oder NULL
	EITEM	EUT_PrintText		*Printe Text ins Statusdisplay
					*INPUT : a0=Text für ScrollWindow
					*OUTPUT: d0=NULL oder Error
	EITEM	EUT_NewDirectory	*Es wurde ein neues Directory gelesen
					*INPUT : d0=Dirs
					*	d1=Files
					*OUTPUT: -
	EITEM	EUT_NewPlayer		*Es wurde ein Player hinzugeladen
					*INPUT : d0=Anz der Player im EP
					*OUTPUT: -
	EITEM	EUT_NewEngineLoaded	*Es wurde ein neues Engine geladen
					*INPUT : -
					*OUTPUT: -
	EITEM	EUT_KillModule		*Das geladene Module wurde entfernt
					*INPUT : -
					*OUTPUT: -
	EITEM	EUT_NewAmplifierList	*Die Amplifierliste ist geändert
					*worden
					*INPUT:	a0=AmplifierList
					*OUTPUT: -
	EITEM	EUT_CheckForPlayer	*Testet den angegebenen Speicherbe-
					*reich nach möglichen Playern und gibt
					*die Namen in einer EPT-Textstructure
					*zurück.
					*INPUT : d0=Size des Speicherbereichs
					*	d1=Size des Files
					*	a0=Adresse des Files
					*	a1=Name des Files
					*	a2=PlayerList
					*OUTPUT: d0=Error oder NULL
					*	a0=EPT-Textstucture
	EITEM	EUT_Ripp		*Input: a0=Memstart
					*	d0=MemSize
					*	a1=Filename
					*Output:d0=Error oder NULL
					*	d1=ModuleSize
					*	a0=Modulestart
					*	a1=Ripperstruct (read only)
	EITEM	EUT_RippSegment		*Such in einem Segment nach Modulen.
					*(unbenutzt in V2.0)
					*Input: d0=Segment
					*Output:d0=Error oder NULL
					*	d1=ModuleSize
					*	a0=Modulestart
					*	a1=Ripperstruct (read only)
	EITEM	EUT_RippCont		*Führt die Suche nach Modulen fort.
					*Input: a0=Ripperstruct
					*Output:d0=Error oder NULL
					*	d1=ModuleSize
					*	a0=Modulestart
					*	a1=Ripperstruct (read only)
	EITEM	EUT_RippExt		*Dient zum Suchen der Samples von
					*Modulen (z.B. Smpl.TRSI)
					*Input: a0=Ripperstruct
					*Output:d0=Error oder NULL



					*------------ stored !!!! -----------*
					*	d1=Samplesize
					*	a0=Samplestart
					*	a1=Ripperstruct (read only)
	EITEM	EUT_Unused6
	EITEM	EUT_ConvertModule	*Convertiert ein Module in das angege-
					*bene Format.
					*Der alte Speicherbereich wird NICHT!!!
					*freigegeben !!!!!!!!!!!
					*INPUT : d0=Size des Modules
					*	d1=DefaultMemory
					*	d2=Offset zur Size
					*	a0=DataPtr
					*OUTPUT: d0=NULL oder Error
					*	d1=Targetfiles
					*	d2=Flags
					*		0=Module nun gesplittet
					*		a0=SongAdr a1=Sample
					*		1=Kein OriginalMod mehr
					*		2=Sample im OriginalMod
					*		3=Sample im FastRAM
					*	a0=EPT_TestSt. Playerloading
					*	a1=PlayerTaglist
					*	a2=LFPuffer (max 100 Files)
	EITEM	EUT_Interrupt		*(APTR) Intereinsprung (nach Angaben von
					*EUS_Ticks/EUS_Counter ...
					*d0=0	Scroller ist fertig
					*sonst d0<>0

	EITEM	EUT_InitDisplay		*Bar für's Packen & Entpacken
					*Input: a0=HintergrundText
					*Output:d0?Null oder Error
	EITEM	EUT_FillDisplay		*Input: d0=Füllen in Prozent
					*Output --
	EITEM	EUT_RemoveDisplay	*Input: --
					*Output: --
	EITEM	EUT_StringGadget	*Input:	d0=Mode 0=Ziffern
					*		1=Text
					*		!2=Invisible
					*	d1=MinValue
					*	d2=MaxValue od Maxlen
					*	a0=Puffer oder Adr
					*	a1=TextFmt
					*	a2=TextFmtArgs
					*	a3=REQUestertitle
	EITEM	EUT_AddAbortGadget	*Output:d0=Error oder NULL
	EITEM	EUT_RemoveAbortGadget	*Input :--
	EITEM	EUT_GetAbortMsg		*Input: --
					*Output:d0=Error oder NULL
					*	d1=0 keine Reaktion
					*	d1=1 Abort gedrückt
	EITEM	EUT_ChangeGui		*Das Gadgets des Gui's werden erlaubt
					*bzw erlaubt. Die Gadgets sind removed.
					*kein Refresh (wird über AddGList ge-
					*macht)
					*Input:	d0= Aktionnr.
					*	d1=Zustand 0=enable 1=disable
					*Output:--
	EITEM	EUT_ChangePrefs		*Es wurde im EP irgendetwas geändert.
					*z.B Volume über Arexx oder Zustand
					*Es muß refresht werden.
					*INPUT : d0=Nr der Aktion
					*		d1=Zustand 0=Enable 1=Disable
					*			-1=keine Änderung
					*		d2= * Wert z.B. Volumewert
					*		* 0=Aus 1=Ein
					*	a0-a3=Parameter
					*OUTPUT: d0=Error oder NULL
	EITEM	EUT_GetGuiMenuStrip	*Es wird ein Menustrip erzeugt, der
					*im Eagleplayermenu mit aufgenommen
					*wird. Der Menustrip wird initialisiert
					*Das Layout erledigt der EP
					*Input: --
					*Ouput: d0=Error oder NULL
					*	a0=MenuStrip
	EITEM	EUT_FreeGuiMenuStrip	*Der benutzte Menuzstrip kann freige-
					*geben werden.
					*Input: a0=MenuStrip
					*Ouput: d0=Error oder NULL
	EITEM	EUT_GetPrefs		*Der Eagleplayer fragt etwas vom Gui
					*ab
					*Input: d0=EPNr ???
					*Output:
	EITEM	EUT_SaveConfig		*Hier wird hingesprungen, wenn der EP
					*Saveconfig aufruft.

	EITEM	EUT_TestValidArchive	*Teste, on File ein Archive ist
					*Input:	a0=Pfad des Archives
					*Output:d0=Error oder NULL=Ok
	EITEM	EUT_OpenArchive		*Öffne ein Archive zum Dirscannen
					*Input: a0=Pfad des Archives
					*	d0=Error oder NULL
					*	a0=Archiveinfo
	EITEM	EUT_CloseArchive	*Schließe Archive wieder
					*Input:	a0=Archiveinfo
	EITEM	EUT_ArchiveExNext	*Gibt den nächsten Eintrag aus dem
					*Archive im WorkPuffer mit WSize
					*zurück
					*Input:	a0=ArchiveInfo
					*	a1=ZielPuffer
					*	d1=ZielPufferSize
					*Output:d0=Error oder NULL
	EITEM	EUT_ExtractArchiveEntry	*Entpackt einen Eintrag aus dem Archive
					*nach T: + Pfad(!!!) des Entries
					*Input:	a0=Pfad des Archives
					*	a1=Pfad des Namens im Archive
					*	a2=WorkPuffer für Befehlspfad
					*	d2=WorkPufferSize

	EITEM	EUT_RemPlayerList
	EITEM	EUT_RemEnginesList
	EITEM	EUT_RemModulesList
	
	EITEM	EUT_Unused1
	EITEM	EUT_Unused2
	EITEM	EUT_Unused3

	EITEM	EUT_EPSubItems		*TI_Data= Ptr to Menustrip for SubItem
					*	  in Enginemenu in EPFormat

	EITEM	EUT_LocaleTable		*TI_Data=Ptr to Table of
					*.w Localenr + .l Ptr to orig. Text
					*+ .l to localeText
					*EP will check the locale-catalogs and
					*wil store a Pointer to .l
					*End with ZERO

	EITEM	EUT_LoadConfig		*Hier wird hingesprungen, wenn der EP
					*Loadconfig macht.
	EITEM	EUT_UpdateEngineMenu	*Hier wird hingesprungen, wenn der EP
					*im Helpmodus ist und die Hilfe eines
					*Schalters angewählt wird. In dieser
					*Routine wird der Haken wieder richtig
					*gesetzt. 
	EITEM	EUT_HelpNodeName	*Name der Helpnode für das Engine

	EITEM	EUT_SpecialinfoLNr	*Helpnummer für die Specialinfos der Engines
	EITEM	EUT_SpecialinfoArgs	*Argumente für die Specialinfos

	EITEM	EUT_AttnFlags		*rEQUired CPU/FPU type, Exec style

	EITEM	EUT_EngineCommand
	EITEM	EUT_FreeRipperstruct	*a0=Ripperstruct

*----------------------------------------------------------------------------*
*
*	EITEM	EUT_AddModulesMenu	*Das intern geladen Directory wird in
*					*das Menu eingebaut
*					*INPUT : d0=Anzahl der Einträge
*					*	a0=1. Modulesstrukture
*					*OUTPUT: d0=NULL oder Error
*					*	d1=Anzahl der angezeigten
*					*		Eintrge
*	EITEM	EUT_RemoveModulesMenu	*Das intern geladene Directory wird
*					*wieder aus dem Menu ausgebaut.
*					*INPUT : -
*					*OUTPUT: d0=NULL oder Error


*------- end of player interface enumeration for EaglePlayer-Engine ------*


*---------------------------- Windowtypes - Types ---------------------------*
EPST_AllWindows		EQU	0
EPST_MainWindow		EQU	1
EPST_PlayerWindow	EQU	2
EPST_UserprogramWindow	EQU	3
EPST_PreferenceWindow	EQU	4
EPST_ModuleInfoWindow	EQU	5
EPST_SampleInfoWindow	EQU	6
EPST_TextREQUest	EQU	7		;intern !!!

*------------------------------- Gadgettypes --------------------------------*
EUTGTY_AbortGadget	EQU	1
EUTGTY_StringGadget	EQU	2
EUTGTY_YesNoGadget	EQU	3
EUTGTY_RetryCancelGadget	EQU	4



*----------------------- EagleUserProgram-Identifier ------------------------*
EUSN_Identifier MACRO
		dc.b	'EPEngine'
		ENDM


*------------------------- Userprogramm FreeTabelle -------------------------*
*- Here now a Table for all libraries that the userprograms use.		-*
*- If an userprogram crashes the eagleplayer will close these windows/screens*
*- libraries/Interrupts/subtasks and will free the Memory at MemAdr		-*

EUSM_ListEnd	EQU	0
EUSM_Ignore	EQU	1
EUSM_Arg1	EQU	2
EUSM_Arg2	EQU	3
EUSM_Arg3	EQU	4
EUSM_Arg4	EQU	5
EUSM_Interrupt	EQU	10
EUSM_SubTask	EQU	11
EUSM_Segment	EQU	12
EUSM_Port13	EQU	13		;Remport (no freeMem)
EUSM_MsgPort	EQU	14		;deleteMsgPort
EUSM_Msg	EQU	15		;will replyed
EUSM_Lock	EQU	16
EUSM_Device	EQU	17
EUSM_LibraryBase	EQU	18
EUSM_Window	EQU	19
EUSM_Screen	EQU	20
EUSM_Raster	EQU	21		;FreeRaster
EUSM_Font	EQU	22
EUSM_MemAdr	EQU	23
EUSM_MemListAdr	EQU	24
EUSM_MenuStrip	EQU	25
EUSM_GadgetAdr	EQU	26
EUSM_VisualInfo	EQU	27
EUSM_PubScreenAdr	EQU	28
EUSM_MaxNummer	EQU	28


*----------------- Analyzer and Eagleplayer Message-Structure ---------------*
 STRUCTURE UM_Message,0
	STRUCT	UM_ExecMessage,20	;Exec-Message
	APTR	UM_TaskAdr		;Engine-TaskAdr
	ULONG	UM_Signal		;Engine-Signal for Interrupt-Signal
	ULONG	UM_Type			;MessageType
	UWORD	UM_Class		;Message-Class
	APTR	UM_StructAdr		;UPS_StructAdr
	APTR	UM_UserPort		;Pointer to UserProgram-Port
	UWORD	UM_UserNr		;Engine-Nr
	UWORD	UM_Enabled		;1=Enable (Window close->no Signal)
	ULONG	UM_Command		;Command from Userprog to EP
	ULONG	UM_Argstring		;Ptr to Argstruct
	ULONG	UM_Result		;Result
	APTR	UM_UserWindow		;Pointer to Window of Userprogram
	LABEL	UM_SizeOf

UM_EnginePort	EQU	UM_UserPort	;Pointer to UserProgram-Port
UM_EngineNr	EQU	UM_UserNr
UM_EngineWindow	EQU	UM_UserWindow
UM_Arglist	EQU	UM_Argstring





*------------------------------- Messagetypes -----------------------------*
USM_UserPrg		EQU	1	;Message from UserPrg to EP
USM_Eagleplayer		EQU	2	;Message from Eagleplayer to ...
USM_Externalprg		EQU	3	;Message from an External Program to EP

USM_Engine		EQU	USM_UserPrg	;Message from Engine to EP

*----------------------------- Message-Classes ----------------------------*
USClass_Dummy		EQU	0	;no class (maybe new StructAdr)
USClass_Activate	EQU	1	;Start Work e.g. Open Window
USClass_DeActivate	EQU	2	;e.g. Close Window (just Pause,NO Exit!)
USClass_Exit		EQU	3	;Exit Engine (only to Engine)
USClass_Zipwin		EQU	4
USClass_Unzipwin	EQU	5
USClass_NewEngine	EQU	6	;Return-Message bei Engine-Start
USClass_GetConfig	EQU	7	;EUS_Flags,EUS_Winx,EUS_Winy,EUS_special have
					;to be initialized by Userprogram	before
					;replying the Message
USClass_NewConfig	EQU	8	;EUS_Flags,EUS_Winx,EUS_Winy,EUS_special
					;countain new Values (the Userprogram has to
					;utilize this new datas)
USClass_LockEP		EQU	9	;Eagleplayer sets Waiting Pointer and
					;does nothing until USCLASS_UNLOCKEP
					;comes, be shure to unlock the EP
					;again (be carefully, call UnLock only
					;as often as you called Lock)
USClass_UnLockEP	EQU	10	;Free Eagleplayer to work on
USClass_Command		EQU	11	;Command to Eagleplayer, future
					;use (e.g. NextPattern,Stop)
USClass_Answer		EQU	12	;Command came back, in UM_Arg and
					;UM_Arg2 results
USClass_QuitEagle	EQU	13	;Makes the Eagleplayer quit


USClass_NewModule	EQU	14
USClass_NewSong		EQU	15
USClass_NewPreference	EQU	16
USClass_StartInt	EQU	17
USClass_StopInt		EQU	18
USClass_PlaySample	EQU	19
USClass_NewScrollText	EQU	20
USClass_NewDirectory	EQU	21
USClass_NewPlayer	EQU	22
USClass_NewEngineLoaded EQU	23	;for Engine-Manager
USClass_KillModule	EQU	24
USClass_Surface		EQU	25
USClass_ActiveSurface	EQU	26
USClass_NewAmplifierlist EQU	27
USClass_Show		EQU	28	;only show Window
USClass_Hide		EQU	29	;only hide Window
USClass_SaveConfig	EQU	30
USClass_RemPlayerList	EQU	31
USClass_RemEnginesList	EQU	32
USClass_RemModulesList	EQU	33
USClass_LoadConfig	EQU	34
USClass_SetWaitPointer	EQU	35
USClass_ClearWaitPointer EQU	36
USClass_EngineCommand	EQU	37
USClass_SizeOf		EQU	37


USClass_NewUserPrg	EQU	USClass_NewEngine
USClass_NewUserPrgLoaded EQU	USClass_NewEngineLoaded

*----------------------------------------------------------------------------*
 STRUCTURE EaglePlayerText,0
	APTR	EPT_Next		;Pointer to Next EPT-Struct
	LONG	EPT_Result1		;Arg or Returnvalue
	LONG	EPT_Result2		;Arg or Returnvalue
	LONG	EPT_StringSize		;Size of allocated Mem (Struct+String)
	LABEL	EPT_String		;String Itself

 STRUCTURE EngineArgs,0
	APTR	ENA_Next		;Pointer to Next EngineArgs
	LONG	ENA_Arg1		;Argument 1
	LONG	ENA_Arg2		;Argument 2
	LONG	ENA_Size		;Size of EngineArg-Struct
	LABEL	ENA_String		;Stringstart, if (ENA_Size)>ENA_SizeOf

**------------------------- Userprogram-Commandos ---------------------------*
*UCM_Status		EQU	1		;String
*UCM_LoadModuleNr	EQU	2		;-1 = FilerEQUester
*UCM_CloseAllOtherEngine	EQU	3
*UCM_OpenAllOtherEngine	EQU	4
*UCM_PubScreen		EQU	5
*UCM_Quit		EQU	6		;Quit Eagleplayer
*UCM_AboutEP		EQU	7
*UCM_ScrollText		EQU	8
*UCM_Iconify		EQU	9
*UCM_HelpMode		EQU	10		;not imlemented
*UCM_HelpMe		EQU	11		;not imlemented
*UCM_DeleteFile		EQU	12
*UCM_Engine		EQU	13
*UCM_LoadConfig		EQU	14
*UCM_SaveConfig		EQU	15
*UCM_AddEngine		EQU	16
*UCM_HelpFile		EQU	17		;not imlemented
*UCM_NewFont		EQU	18
*UCM_LoadModule		EQU	19
*UCM_SaveModule		EQU	20
*UCM_Eject		EQU	21
*UCM_PrevSong		EQU	22
*UCM_NextSong		EQU	23
*;UCM_PlaySong		EQU	24		;not implemented
*UCM_PrevPatt		EQU	25
*UCM_NextPatt		EQU	26
*UCM_PrevModule		EQU	27
*UCM_NextModule		EQU	28
*UCM_Pause		EQU	29
*UCM_Play		EQU	30
*UCM_Stop		EQU	31
*UCM_ReplaySong		EQU	32
*UCM_AboutModule		EQU	33
*UCM_OpenMainWindow	EQU	34
*UCM_OpenPlWindow	EQU	35
*UCM_OpenSIWindow	EQU	36		;not imlemented
*UCM_OpenMIWindow	EQU	37		;not imlemented
*UCM_CloseMainWindow	EQU	38
*UCM_ClosePlWindow	EQU	39
*UCM_CloseSIWindow	EQU	40		;not imlemented
*UCM_CloseMIWindow	EQU	41		;not imlemented
*UCM_AddPlayer		EQU	42
*UCM_AddPlDir		EQU	43
*UCM_DelPlayer		EQU	44
*UCM_DeleteAll		EQU	45
*UCM_Enable		EQU	46
*UCM_Disable		EQU	47
*UCM_LoadPlConfig	EQU	48
*UCM_SavePlConfig	EQU	49
*UCM_DefaultSpeed	EQU	50
*UCM_SlowerSpeed		EQU	51
*UCM_FasterSpeed		EQU	52
*UCM_Filter		EQU	53
*UCM_FadeOut		EQU	54
*UCM_FadeIn		EQU	55
*UCM_QuickStart		EQU	56
*UCM_SetProgramMode	EQU	57
*UCM_SongEnd		EQU	58
*UCM_TimeOut		EQU	59
*UCM_ScrollInfos		EQU	60
*UCM_LoadDir		EQU	61
*UCM_AutoDir		EQU	62
*UCM_FlashPointer	EQU	63
*UCM_RandomSong		EQU	64
*UCM_MasterVolume	EQU	65
*UCM_Overwrite		EQU	66
*UCM_LoadAlways		EQU	67
*UCM_UseSongName		EQU	68
*UCM_GetModList		EQU	69		;not imlemented
*UCM_GetPlList		EQU	70		;not imlemented
*UCM_GetEngineList	EQU	71		;not imlemented
*UCM_GetSampleList	EQU	72		;not imlemented
*UCM_SetIconifyMode	EQU	73
*UCM_AutoSubSong		EQU	74
*UCM_ShowDirNames	EQU	75
*UCM_SetCrunchMode	EQU	76
*UCM_GetPassword		EQU	77
*UCM_AutomaticSave	EQU	78
*UCM_SetSaveDir		EQU	79
*UCM_SSDir		EQU	80
*UCM_SampleMode		EQU	81
*UCM_SaveAsProT		EQU	82
*UCM_SetFileReqMode	EQU	83
*UCM_SetSpeed		EQU	84
*UCM_Volume		EQU	85
*UCM_Balance		EQU	86
*UCM_Voice		EQU	87	;in Arg 1 Number,Arg 2 State 0=on 1=off
*UCM_VolVoice		EQU	88	;in Arg 1 Number,Arg 2 Volume
*UCM_FileREQUest		EQU	89
*UCM_QuitSurface		EQU	90
*UCM_PlayMem		EQU	91	;in Arg 1 Address,in Arg 2 Size, ...
*UCM_GetModuleData	EQU	92	;in Arg 1 Address,in Arg 2 Size, ...
*UCM_GetEPBase		EQU	93
*UCM_ClearList		EQU	94
*UCM_AddList		EQU	95
*UCM_AddListEntry	EQU	96		;not implemented
*UCM_HideAll		EQU	97
*UCM_UseEPDir		EQU	98
*UCM_SaveT		EQU	99
*UCM_LoadBefore		EQU	100
*UCM_QuitEagle		EQU	101
*UCM_SmallModule		EQU	102
*UCM_Protect		EQU	103
*UCM_PBits		EQU	104
*UCM_ChangeXPKName	EQU	105
*UCM_AutoPassword	EQU	106
*UCM_BigModule		EQU	107
*UCM_ModuleFault		EQU	108
*UCM_SafeSave		EQU	109
*UCM_Hide		EQU	110
*UCM_AddUserDir		EQU	111
*UCM_RescanDir		EQU	112
*UCM_Notify		EQU	113
*UCM_ScanAlways		EQU	114
*UCM_UsePrefix		EQU	115
*UCM_Dirmemsize		EQU	116
*UCM_PrintText		EQU	117
*UCM_EntriesPerMenu	EQU	118
*UCM_EjectPlayer		EQU	119
*UCM_EjectEngine		EQU	120
*UCM_EPBatch		EQU	121
*UCM_UPrgBatch		EQU	122
*UCM_LoadEPBatch		EQU	123
*UCM_LoadUPrgBatch	EQU	124
*UCM_GetSampleInfo	EQU	125
*UCM_FreeSampleInfo	EQU	126
*UCM_LoadFast		EQU	127
*UCM_FileListNr		EQU	128
*UCM_AudioChannels	EQU	129
*UCM_SoftInt		EQU	130
*UCM_SetTimingMode	EQU	131
*UCM_SetWaitingMode	EQU	132
*UCM_DBFloops		EQU	133
*UCM_RasterLines		EQU	134
*UCM_TimeMode		EQU	135
*UCM_CalcDuration	EQU	136
*UCM_MinTimeOut		EQU	137
*UCM_LHAExtractor	EQU	138
*UCM_RandomStart		EQU	139
*
*UCM_XFDDecrunch		EQU	140		;new at V1.60
*UCM_XFDLoadSeg		EQU	141
*UCM_QuitEngine		EQU	142
*
*UCM_MaxId		EQU	142

		*----------- for current Surface ----------*
* ist jetzt überflüssig !!!!
;UCM_AddAbortGadget	EQU	142
;UCM_RemoveAbortGadget	EQU	143
;UCM_PrintText2		EQU	144		;for surface unlimited Text
;UCM_ScrollText2		EQU	145		;for surface unlimited Text
;UCM_AllocEPSurface	EQU	146
;UCM_FreeEPSurface	EQU	147
;UCM_AddYesNoGadget	EQU	148
;UCM_RemoveYesNoGadget	EQU	149
;UCM_AddRetryCancelGadget	EQU	150
;UCM_RemoveRetryCancelGadget	EQU	151
;UCM_InitHook		EQU	152		;für CrunchAnzeige
;UCM_EndHook		EQU	153
;UCM_SurfaceInfo		EQU	154		;with Playerwindow ? ...
;UCM_TextREQUest		EQU	155

;UCM_MaxId		EQU	155


*----------------------------- Nummer der Aktionen ---------------------------*
		RS_RESET
		RS_BYTE	EPNr_Dummy,1		;unused

	*-------------------- Eagleplayer-Windows -------------------*
		RS_BYTE	EPNr_MainWindow,1

	*--------------------- General-Funktionen -------------------*
		RS_BYTE	EPNr_LoadConfig,1
		RS_BYTE	EPNr_SaveConfig,1
		RS_BYTE	EPNr_Help,1
		RS_BYTE	EPNr_DeleteFile,1
		RS_BYTE	EPNr_AboutEP,1
		RS_BYTE	EPNr_Iconify,1
		RS_BYTE	EPNr_ToggleFilter,1
		RS_BYTE	EPNr_Hide,1
		RS_BYTE	EPNr_Quit,1
		RS_BYTE	EPNr_Voices,1		;d0=Voice d1=state
		RS_BYTE	EPNr_Module,1		;\
		RS_BYTE	EPNr_Player,1		; > d0=EPNr d1=Mod/Pl/UPrgNr
		RS_BYTE	EPNr_Engine,1		;/
		RS_BYTE	EPNr_PlayMem,1
		RS_BYTE	EPNr_PublicScreen,1
		RS_BYTE	EPNr_Abort,1
		RS_BYTE	EPNr_Clock,1
		RS_BYTE	EPNr_HotKey,1
		RS_BYTE	EPNr_CXPriority,1
		RS_BYTE	EPNr_Status,1
		RS_BYTE	EPNr_Status2,1		; for Engines

	*---------------------- Module Funktionen -------------------*
		RS_BYTE	EPNr_LoadModule,1	;d0=0->select in FilerEQUester
		RS_BYTE	EPNr_SaveModule,1
		RS_BYTE	EPNr_SaveModulePrefs,1
		RS_BYTE	EPNr_PrevModule,1
		RS_BYTE	EPNr_PrevSong,1
		RS_BYTE	EPNr_PrevPattern,1
		RS_BYTE	EPNr_ReplaySong,1
		RS_BYTE	EPNr_Play,1
		RS_BYTE	EPNr_PlayFaster,1	;nur Pause
		RS_BYTE	EPNr_NextPattern,1
		RS_BYTE	EPNr_NextSong,1
		RS_BYTE	EPNr_NextModule,1
		RS_BYTE	EPNr_StopPlay,1
		RS_BYTE	EPNr_EjectModule,1
		RS_BYTE	EPNr_AboutModule,1
		RS_BYTE	EPNr_SubSong,1
		RS_BYTE	EPNr_GetSampleInfo,1
		RS_BYTE	EPNr_FreeSampleInfo,1

	*---------------------- Player Funktionen -------------------*
		RS_BYTE	EPNr_LoadPlayer,1
		RS_BYTE	EPNr_LoadPlayerDir,1
		RS_BYTE	EPNr_DeletePlayer,1
		RS_BYTE	EPNr_DeleteAllPlayer,1
		RS_BYTE	EPNr_PlayerState,1		;oder disable
		RS_BYTE	EPNr_PlayerConfig,1
		RS_BYTE	EPNr_LoadPlayerConfig,1
		RS_BYTE	EPNr_SavePlayerConfig,1

	*--------------------- Engine-Funktionen -------------------*
		RS_BYTE	EPNr_LoadEngine,1
		RS_BYTE	EPNr_LoadEngineDir,1
		RS_BYTE	EPNr_DeleteEngine,1
		RS_BYTE	EPNr_DeleteAllEngines,1
		RS_BYTE	EPNr_LoadEngineConfig,1
		RS_BYTE	EPNr_SaveEngineConfig,1


	*---------------------- Listen-Funktionen -------------------*
		RS_BYTE	EPNr_InsertModulesList,1

	*---------------------- Font-Funktionen ---------------------*
		RS_BYTE	EPNr_MenuFont,1
		RS_BYTE	EPNr_ModulesFont,1
		RS_BYTE	EPNr_ScrollFont,1
		RS_BYTE	EPNr_FontReserved,1
		RS_BYTE	EPNr_FontReserved2,1


	*---------------------- Einstellungssachen ------------------*

			*------------ Allgemein --------------*
		RS_BYTE	EPNr_QuickStart,1
		RS_BYTE	EPNr_ScrollInfos,1
		RS_BYTE	EPNr_SongName,1
		RS_BYTE	EPNr_Filter,1
		RS_BYTE	EPNr_MasterVolume,1
		RS_BYTE	EPNr_SaveT,1
		RS_BYTE	EPNr_FadeIn,1
		RS_BYTE	EPNr_Fadeout,1
		RS_BYTE	EPNr_Quiteagle,1
		RS_BYTE	EPNr_EjectPlayers,1
		RS_BYTE	EPNr_EjectEngines,1
		RS_BYTE	EPNr_PlayerBatch,1
		RS_BYTE	EPNr_Prefix,1
		RS_BYTE	EPNr_LoadFast,1
			*----------- Dir/ModulesPrefs ---------*
		RS_BYTE	EPNr_LoadDir,1
		RS_BYTE	EPNr_EPDir,1
		RS_BYTE	EPNr_HideAll,1
		RS_BYTE	EPNr_RescanDir,1
		RS_BYTE	EPNr_Notify,1
		RS_BYTE	EPNr_ScanAlways,1
		RS_BYTE	EPNr_Reserved,1
		RS_BYTE	EPNr_LoadParentDir,1
			*------------ Programm ---------------*
		RS_BYTE	EPNr_ProgramMode,1
		RS_BYTE	EPNr_LoadBefore,1
		RS_BYTE	EPNr_RandomSong,1
		RS_BYTE	EPNr_AutoSubSong,1
		RS_BYTE	EPNr_LoadAlways,1
		RS_BYTE	EPNr_SongEnd,1
		RS_BYTE	EPNr_CalcDuration,1
		RS_BYTE	EPNr_DirJump,1
		RS_BYTE	EPNr_RandomStart,1
		RS_BYTE	EPNr_TimeMode,1
		RS_BYTE	EPNr_PlayTime,1
		RS_BYTE	EPNr_MinPlayTime,1
			*--------------- SaveMode ------------*
		RS_BYTE	EPNr_CrunchMode,1
		RS_BYTE	EPNr_SaveDir,1
		RS_BYTE	EPNr_SSDir,1
		RS_BYTE	EPNr_SafeSave,1
		RS_BYTE	EPNr_SampleMode,1
		RS_BYTE	EPNr_SaveAsProTracker,1
		RS_BYTE	EPNr_Protect,1
		RS_BYTE	EPNr_ProtectionBits,1
		RS_BYTE	EPNr_XPKCrunchMode,1
		RS_BYTE	EPNr_Overwrite,1
		RS_BYTE	EPNr_AutomaticSave,1
		RS_BYTE	EPNr_Password,1
			*-------------- Timing ---------------*
		RS_BYTE	EPNr_TimingMode,1
		RS_BYTE	EPNr_WaitingMode,1
		RS_BYTE	EPNr_RasterLines,1
		RS_BYTE	EPNr_DBFLoops,1
		RS_BYTE	EPNr_AllocChannels,1
		RS_BYTE	EPNr_SoftInt,1

		RS_BYTE	EPNr_AutoPassword,1
		RS_BYTE	EPNr_XFDDecrunch,1
		RS_BYTE	EPNr_XFDLoadSeg,1

		RS_BYTE	EPNr_Volume,1
		RS_BYTE	EPNr_VolumeDefault,1
		RS_BYTE	EPNr_VolumeLower,1
		RS_BYTE	EPNr_VolumeHigher,1

		RS_BYTE	EPNr_Balance,1
		RS_BYTE	EPNr_BalanceDefault,1
		RS_BYTE	EPNr_BalanceLeft,1
		RS_BYTE	EPNr_BalanceRight,1

		RS_BYTE	EPNr_Speed,1
		RS_BYTE	EPNr_SpeedDefault,1
		RS_BYTE	EPNr_SpeedFaster,1
		RS_BYTE	EPNr_SpeedSlower,1

		RS_BYTE	EPNr_SmallModule,1
		RS_BYTE	EPNr_BigModule,1
		RS_BYTE	EPNr_ModuleFault,1

		RS_BYTE	EPNr_FileREQUesterMode,1
		RS_BYTE	EPNr_IconifyMode,1

		RS_BYTE	EPNr_ChipRamAmplifier,1
		RS_BYTE	EPNr_FastRamAmplifier,1
		RS_BYTE	EPNr_PrintText,1
		RS_BYTE	EPNr_ScrollText,1
		RS_BYTE	EPNr_LockModule,1

		RS_BYTE	EPNr_EngineShowState,1	* Konfiguration Show/Hide
		RS_BYTE	EPNr_EngineItemCommand,1		* Item aus dem Menu
		RS_BYTE	EPNr_EngineConfig,1
		RS_BYTE	EPNr_Documentation,1
		RS_BYTE	EPNr_NoGui,1
		RS_BYTE	EPNr_ExtractFirstModule,1
		RS_BYTE	EPNr_HelpLink,1
		RS_BYTE	EPNr_Instrumentsdir,1
		RS_BYTE	EPNr_NoEngine,1
		RS_BYTE	EPNr_AscEngine,1
		RS_BYTE	EPNr_LoadList,1
		RS_BYTE	EPNr_SaveList,1
		RS_BYTE	EPNr_SetAuthor,1
		RS_BYTE	EPNr_InsertPysionList,1
		RS_BYTE	EPNr_FormatLoad,1
		RS_BYTE	EPNr_PlaySample,1
		RS_BYTE	EPNr_Config,1
		RS_BYTE	EPNr_AddEntry,1		; Rev 12
		RS_BYTE	EPNr_FreeDirPuffer,1	; Rev 12
		RS_BYTE	EPNr_ScreenMode,1	; Rev 13
		RS_BYTE	EPNr_SetPosition,1	; Rev 13
		RS_BYTE	EPNr_LastInternal,0

EPNr_FirstExternal	EQU	$6000
EPNr_LastExternal	EQU	$6fff
EPNr_FirstEngine	EQU	$7000
EPNr_LastEngine		EQU	$77ff
EPNr_FirstPlayer	EQU	$7800
EPNr_LastPlayer		EQU	$7fff
EPNr_FirstModule	EQU	$8000
EPNr_LastModule		EQU	$FFFF



EPNrI_AppIcon		EQU	0		* IconifyModi
EPNrI_AppItem		EQU	1
EPNrI_MaxId		EQU	1

EPNrF_Req		EQU	0
EPNrF_ReqTools		EQU	1
EPNrF_ASL		EQU	2
EPNrF_MaxId		EQU	2

EPNrC_Uncrunched	EQU	0		* CrunchModi
EPNrC_PPCrunched	EQU	1
EPNrC_LHCrunched	EQU	2
EPNrC_XPKCrunched	EQU	3
EPNrC_CrMCrunched	EQU	4
EPNrC_MaxId		EQU	4


EPNrT_CiaTiming		EQU	0
EPNrT_VBlankTiming	EQU	1
EPNrT_TimerDevice	EQU	2
EPNrT_MaxId		EQU	2


EPNrW_Automaticwait	EQU	0
EPNrW_RasterWait	EQU	1
EPNrW_DBFWait		EQU	2
EPNrW_MaxId		EQU	2

EPNrP_NoNextModule	EQU	0
EPNrP_PrevModule	EQU	1
EPNrP_NextModule	EQU	2
EPNrP_RandomModule	EQU	3
EPNrP_MaxId		EQU	3



EP_MaxVolume		EQU	255		;v2.0
EP_DefVolume		EQU	255		;v2.0
EP_MaxVoices		EQU	32		;v2.0
EP_MaxSpeed		EQU	25		;v2.0 -25 to 25


EPFF_Proportional	EQU	1		;proportinal-font
EPFB_Proportional	EQU	1<<EPFF_Proportional

EPRMF_DownGadget	EQU	1				*z.B Volumegadget gedrückt
EPRMF_NoGuiInfo		EQU	2
EPRMB_DownGadget	EQU	1<<EPRMF_DownGadget
EPRMB_NoGuiInfo		EQU	1<<EPRMF_NoGuiInfo


*-----------------------------------------------------------------------------*
* Nummern für Moduleseigenschaften (Pysion)

		RS_RESET
		RS_BYTE	EPMENr_Dummy,1
		RS_BYTE	EPMENr_Songname,1
		RS_BYTE	EPMENr_Specialinfo,1
		RS_BYTE	EPMENr_Group,1

*--------------------------------------------------------------------------*
LNr_FirstInternal	EQU	-1
LNr_FirstExternal	EQU	1
LNr_FirstEngine		EQU	$3000
LNr_FirstPlayer		EQU	$6000

		RS_RESET
		RS_BYTE	LNr_Dummy,LNr_FirstInternal
		RS_BYTE	LNr_StringError,-1
		RS_BYTE	LNr_EPVersion,-1
		RS_BYTE	LNr_Dosname,-1
		RS_BYTE	LNr_Intuiname,-1
		RS_BYTE	LNr_IconName,-1
		RS_BYTE	LNr_Graphname,-1
		RS_BYTE	LNr_GadToolsName,-1
		RS_BYTE	LNr_LocaleName,-1
		RS_BYTE	LNr_Reqname,-1
		RS_BYTE	LNr_ASLName,-1
		RS_BYTE	LNr_ReqToolsName,-1
		RS_BYTE	LNr_LhName,-1
		RS_BYTE	LNr_PPName,-1
		RS_BYTE	LNr_WBName,-1
		RS_BYTE	LNr_DiskFontName,-1
		RS_BYTE	LNr_ScreenNotifyName,-1
		RS_BYTE	LNr_AmigaGuideName,-1
		RS_BYTE	LNr_CommodityName,-1
		RS_BYTE	LNr_XPKName,-1
		RS_BYTE	LNr_XFDMasterName,-1
		RS_BYTE	LNr_CrMName,-1
		RS_BYTE	LNr_Audioname,-1
		RS_BYTE	LNr_CiaBname,-1
		RS_BYTE	LNr_TopazName,-1
		RS_BYTE	LNr_RexxSysName,-1
		RS_BYTE	LNr_Prozessname,-1
		RS_BYTE	LNr_EPEnginePort,-1
		RS_BYTE	LNr_EPReplayEnginePort,-1
		RS_BYTE	LNr_EPAppPort,-1
		RS_BYTE	LNr_EPArexxPort,-1
		RS_BYTE	LNr_EPScreenNotifyPort,-1
		RS_BYTE	LNr_InterruptName,-1
		RS_BYTE	LNr_TimerName,-1
		RS_BYTE	LNr_SoftIntName,-1
		RS_BYTE	LNr_EPAudioName,-1
		RS_BYTE	LNr_EPNotifyName,-1
		RS_BYTE	LNr_EPCommodityName,-1
		RS_BYTE	LNr_CommoTitle,-1
		RS_BYTE	LNr_CommoText,-1
		RS_BYTE	LNr_SaveKopf,-1
		RS_BYTE	LNr_EngineDir,-1
		RS_BYTE	LNr_EaglePlayerDir,-1
		RS_BYTE	LNr_MainGuideName,-1
		RS_BYTE	LNr_EngineGuideName,-1
		RS_BYTE	LNr_PlayerGuideName,-1
		RS_BYTE	LNr_GuideALink,-1
		RS_BYTE	LNr_MainLink,-1
		RS_BYTE	LNr_ShowLink,-1
		RS_BYTE	LNr_InstrumentsDir,-1
		RS_BYTE	LNr_TName,-1
		RS_BYTE	LNr_EPTName,-1
		RS_BYTE	LNr_EnvName,-1
		RS_BYTE	LNr_EnvarcName,-1
		RS_BYTE	LNr_SName,-1
		RS_BYTE	LNr_DevsName,-1
		RS_BYTE	LNr_LocaleAssign,-1
		RS_BYTE	LNr_EPAssign,-1
		RS_BYTE	LNr_ConfigDir,-1
		RS_BYTE	LNr_EPDirName,-1
		RS_BYTE	LNr_ConfigDirName,-1
		RS_BYTE	LNr_ConfigName,-1
		RS_BYTE	LNr_KeyName,-1
		RS_BYTE	LNr_PlayerBatchName,-1
		RS_BYTE	LNr_MainCatalog,-1
		RS_BYTE	LNr_EngineCatalog,-1
		RS_BYTE	LNr_PlayerCatalog,-1
		RS_BYTE	LNr_DefaultGui,-1
		RS_BYTE	LNr_WindowTitleRegistered,-1
		RS_BYTE	LNr_WindowTitleGeneric,-1
		RS_BYTE	LNr_EPTitle,-1
		RS_BYTE	LNr_CommentText,-1
		RS_BYTE	LNr_Yes,-1
		RS_BYTE	LNr_On,-1
		RS_BYTE	LNr_No,-1
		RS_BYTE	LNr_Off,-1
		RS_BYTE	LNr_1,-1
		RS_BYTE	LNr_0,-1
		RS_BYTE	LNr_Toggle,-1
		RS_BYTE	LNr_Toggle2,-1
		RS_BYTE	LNr_DefaultHotkey,-1
		RS_BYTE	LNr_DefPubScreen,-1
		RS_BYTE	LNr_Str_Status,-1
		RS_BYTE	LNr_Str_Popup,-1
		RS_BYTE	LNr_Str_PlayFaster,-1
		RS_BYTE	LNr_Str_HotKey,-1
		RS_BYTE	LNr_Str_Priority,-1
		RS_BYTE	LNr_Str_ChipRamAmplifier,-1
		RS_BYTE	LNr_Str_FastRamAmplifier,-1
		RS_BYTE	LNr_Str_PrintText,-1
		RS_BYTE	LNr_Str_ScrollText,-1
		RS_BYTE	LNr_Str_MenuFont,-1
		RS_BYTE	LNr_Str_ModulesFont,-1
		RS_BYTE	LNr_Str_ScrollFont,-1
		RS_BYTE	LNr_Str_Help,-1
		RS_BYTE	LNr_Str_DeleteFile,-1
		RS_BYTE	LNr_Str_Iconify,-1
		RS_BYTE	LNr_Str_LoadConfig,-1
		RS_BYTE	LNr_Str_SaveConfig,-1
		RS_BYTE	LNr_Str_Config,-1
		RS_BYTE	LNr_Str_AboutEP,-1
		RS_BYTE	LNr_Str_Hide,-1
		RS_BYTE	LNr_Str_Quit,-1
		RS_BYTE	LNr_Str_LoadGui,-1
		RS_BYTE	LNr_Str_DeleteGui,-1
		RS_BYTE	LNr_Str_LoadPlayer,-1
		RS_BYTE	LNr_Str_LoadPlayerDir,-1
		RS_BYTE	LNr_Str_DeletePlayer,-1
		RS_BYTE	LNr_Str_DeleteAllPlayer,-1
		RS_BYTE	LNr_Str_Enable,-1
		RS_BYTE	LNr_Str_LoadPlayerConfig,-1
		RS_BYTE	LNr_Str_SavePlayerConfig,-1
		RS_BYTE	LNr_Str_PlayerConfig,-1
		RS_BYTE	LNr_Str_LoadEngine,-1
		RS_BYTE	LNr_Str_LoadEngineDir,-1
		RS_BYTE	LNr_Str_DeleteEngine,-1
		RS_BYTE	LNr_Str_DeleteAllEngines,-1
		RS_BYTE	LNr_Str_XPKPackmethod,-1
		RS_BYTE	LNr_Str_Password,-1
		RS_BYTE	LNr_Str_InstrumentsDir,-1
		RS_BYTE	LNr_Str_Module,-1
		RS_BYTE	LNr_Str_LoadModule,-1
		RS_BYTE	LNr_Str_SaveModule,-1
		RS_BYTE	LNr_Str_SaveModulePrefs,-1
		RS_BYTE	LNr_Str_SaveDir,-1
		RS_BYTE	LNr_Str_Eject,-1
		RS_BYTE	LNr_Str_PrevSong,-1
		RS_BYTE	LNr_Str_NextSong,-1
		RS_BYTE	LNr_Str_SubSong,-1
		RS_BYTE	LNr_Str_PrevPattern,-1
		RS_BYTE	LNr_Str_NextPattern,-1
		RS_BYTE	LNr_Str_PrevModule,-1
		RS_BYTE	LNr_Str_NextModule,-1
		RS_BYTE	LNr_Str_Pause,-1
		RS_BYTE	LNr_Str_Play,-1
		RS_BYTE	LNr_Str_Stop,-1
		RS_BYTE	LNr_Str_ReplaySong,-1
		RS_BYTE	LNr_Str_AboutModule,-1
		RS_BYTE	LNr_Str_PubScreen,-1
		RS_BYTE	LNr_Str_Filter,-1
		RS_BYTE	LNr_Str_ToggleFilter,-1
		RS_BYTE	LNr_Str_FadeIn,-1
		RS_BYTE	LNr_Str_FadeOut,-1
		RS_BYTE	LNr_Str_QuitEagle,-1
		RS_BYTE	LNr_Str_SaveT,-1
		RS_BYTE	LNr_Str_ScrollInfos,-1
		RS_BYTE	LNr_Str_LoadFast,-1
		RS_BYTE	LNr_Str_SongName,-1
		RS_BYTE	LNr_Str_Prefix,-1
		RS_BYTE	LNr_Str_EjectPlayer,-1
		RS_BYTE	LNr_Str_EjectEngine,-1
		RS_BYTE	LNr_Str_PlayerBatch,-1
		RS_BYTE	LNr_Str_AddListEntry,-1
		RS_BYTE	LNr_Str_AddList,-1
		RS_BYTE	LNr_Str_RescanDir,-1
		RS_BYTE	LNr_Str_LoadDir,-1
		RS_BYTE	LNr_Str_ParentDir,-1
		RS_BYTE	LNr_Str_EPDir,-1
		RS_BYTE	LNr_Str_HideAll,-1
		RS_BYTE	LNr_Str_Notify,-1
		RS_BYTE	LNr_Str_ScanAlways,-1
		RS_BYTE	LNr_Str_Protect,-1
		RS_BYTE	LNr_Str_ProtectionBits,-1
		RS_BYTE	LNr_Str_SampleMode,-1
		RS_BYTE	LNr_Str_SaveAsProTracker,-1
		RS_BYTE	LNr_Str_SSDir,-1
		RS_BYTE	LNr_Str_SafeSave,-1
		RS_BYTE	LNr_Str_AutomaticSave,-1
		RS_BYTE	LNr_Str_MasterVolume,-1
		RS_BYTE	LNr_Str_Overwrite,-1
		RS_BYTE	LNr_Str_AutoPassword,-1
		RS_BYTE	LNr_Str_XFDDecrunch,-1
		RS_BYTE	LNr_Str_XFDLoadSeg,-1
		RS_BYTE	LNr_Str_SmallModule,-1
		RS_BYTE	LNr_Str_BigModule,-1
		RS_BYTE	LNr_Str_ModuleFault,-1
		RS_BYTE	LNr_Str_ProgramMode,-1
		RS_BYTE	LNr_Str_SongEnd,-1
		RS_BYTE	LNr_Str_QuickStart,-1
		RS_BYTE	LNr_Str_RandomSong,-1
		RS_BYTE	LNr_Str_LoadBefore,-1
		RS_BYTE	LNr_Str_LoadAlways,-1
		RS_BYTE	LNr_Str_DirJump,-1
		RS_BYTE	LNr_Str_TimeMode,-1
		RS_BYTE	LNr_Str_CalcDuration,-1
		RS_BYTE	LNr_Str_Randomstart,-1
		RS_BYTE	LNr_Str_AutoSubSong,-1
		RS_BYTE	LNr_Str_TimeOut,-1
		RS_BYTE	LNr_Str_MinTimeOut,-1
		RS_BYTE	LNr_Str_TimingMode,-1
		RS_BYTE	LNr_Str_WaitingMode,-1
		RS_BYTE	LNr_Str_RasterLines,-1
		RS_BYTE	LNr_Str_DBFLoops,-1
		RS_BYTE	LNr_Str_AllocChannels,-1
		RS_BYTE	LNr_Str_SoftInt,-1
		RS_BYTE	LNr_Str_IconifyMode,-1
		RS_BYTE	LNr_Str_CrunchMode,-1
		RS_BYTE	LNr_Str_FileREQUesterMode,-1
		RS_BYTE	LNr_Str_Volume,-1
		RS_BYTE	LNr_Str_Balance,-1
		RS_BYTE	LNr_Str_Voice,-1
		RS_BYTE	LNr_Str_Speed,-1
		RS_BYTE	LNr_Str_DefaultSpeed,-1
		RS_BYTE	LNr_Str_SlowerSpeed,-1
		RS_BYTE	LNr_Str_FasterSpeed,-1
		RS_BYTE	LNr_Str_LockModule,-1
		RS_BYTE	LNr_Str_NoGui,-1
		RS_BYTE	LNr_Str_ExtractFirstModule,-1
		RS_BYTE	LNr_Str_Helplink,-1
		RS_BYTE	LNr_Err_Exists,-1
		RS_BYTE	LNr_Err_Signals,-1
		RS_BYTE	LNr_Err_Dos,-1
		RS_BYTE	LNr_Err_GFX,-1
		RS_BYTE	LNr_Err_Intuition,-1
		RS_BYTE	LNr_ErrorConWin,-1
		RS_BYTE	LNr_OldSoundTracker,-1
		RS_BYTE	LNr_SoundTracker,-1
		RS_BYTE	LNr_NoiseTracker,-1
		RS_BYTE	LNr_ProTracker,-1
		RS_BYTE	LNr_Registration,-1
		RS_BYTE	LNr_Str_NoEngine,-1
		RS_BYTE	LNr_Str_ASCEngine,-1
		RS_BYTE	LNr_Str_LoadList,-1
		RS_BYTE	LNr_Str_SaveList,-1
		RS_BYTE	LNr_Str_SetAuthor,-1
		RS_BYTE	LNr_EPTName2,-1
		RS_BYTE	LNr_DracoResource,-1
		RS_BYTE	LNr_Str_FreeDirPuffer,-1	; Rev ,12
		RS_BYTE	LNr_Str_ScreenMode,-1		; Rev 13
		RS_BYTE	LNr_AppIconName,-1		; Rev 13
		RS_BYTE	LNr_Str_SetPosition,-1		; Rev 13
		RS_BYTE	LNr_LastInternal,0

	*---------- Nun die externen,localisierbaren Strings ----------*
		RS_RESET
		RS_BYTE	LNr_FirstExtDummy,LNr_FirstExternal
		RS_BYTE	LNr_Ok,1
		RS_BYTE	LNr_Yes_Extern,1
		RS_BYTE	LNr_No_Extern,1
		RS_BYTE	LNr_On_Extern,1
		RS_BYTE	LNr_Off_Extern,1
		RS_BYTE	LNr_Continue,1
		RS_BYTE	LNr_Abort,1
		RS_BYTE	LNr_Fr_LoadModule,1
		RS_BYTE	LNr_Fr_SaveModule,1
		RS_BYTE	LNr_SW_SaveModule,1
		RS_BYTE	LNr_Fr_SaveDir,1
		RS_BYTE	LNr_SW_SaveDir,1
		RS_BYTE	LNr_Fr_AddEngine,1
		RS_BYTE	LNr_SW_AddEngine,1
;		RS_BYTE	LNr_Fr_AddEngineDir,1
		RS_BYTE	LNr_Fr_DeleteFile,1
		RS_BYTE	LNr_SW_DeleteFile,1
		RS_BYTE	LNr_Fr_AddPlayer,1
		RS_BYTE	LNr_SW_AddPlayer,1
*		RS_BYTE	LNr_Fr_AddPlayerDir,1
		RS_BYTE	LNr_Fr_LoadConfig,1
		RS_BYTE	LNr_SW_LoadConfig,1
		RS_BYTE	LNr_Fr_SaveConfig,1
		RS_BYTE	LNr_SW_SaveConfig,1
		RS_BYTE	LNr_Fr_InstrumentDir,1
		RS_BYTE	LNr_SW_InstrumentDir,1
		RS_BYTE	LNr_Fr_SelectFile,1
		RS_BYTE	LNr_SW_SelectFile,1
		RS_BYTE	LNr_Welcome,1
		RS_BYTE	LNr_PlayersAdded,1
		RS_BYTE	LNr_EnginesAdded,1
		RS_BYTE	LNr_EngineCommandDone,1
		RS_BYTE	LNr_Show,1
		RS_BYTE	LNr_Hide,1
		RS_BYTE	LNr_DelPlayer,1
		RS_BYTE	LNr_Loaded,1
		RS_BYTE	LNr_Reloaded,1
		RS_BYTE	LNr_DeleteAllPlayer,1
		RS_BYTE	LNr_DeleteFile,1
		RS_BYTE	LNr_LoadConfig,1
		RS_BYTE	LNr_SaveConfig,1
		RS_BYTE	LNr_Unknown,1
		RS_BYTE	LNr_LoadingInstruments,1
		RS_BYTE	LNr_Enabled,1
		RS_BYTE	LNr_Disabled,1
		RS_BYTE	LNr_SelectFont,1
		RS_BYTE	LNr_NewFontset,1
		RS_BYTE	LNr_Deleted,1
		RS_BYTE	LNr_OperationSuccesful,1
		RS_BYTE	LNr_Configurating,1
		RS_BYTE	LNr_NowLoaded,1
		RS_BYTE	LNr_Loading,1
		RS_BYTE	LNr_Decrunching,1
		RS_BYTE	LNr_Exploding,1
		RS_BYTE	LNr_Extracting,1
		RS_BYTE	LNr_Playing,1
		RS_BYTE	LNr_Pause,1
		RS_BYTE	LNr_Song,1
		RS_BYTE	LNr_CurrentPosition,1
		RS_BYTE	LNr_ReplaySong,1
		RS_BYTE	LNr_StopPlay,1
		RS_BYTE	LNr_EjectModule,1
		RS_BYTE	LNr_Crunching,1
		RS_BYTE	LNr_Saving,1
		RS_BYTE	LNr_SaveOk,1
		RS_BYTE	LNr_Voicex,1
		RS_BYTE	LNr_VoiceOff,1
		RS_BYTE	LNr_ResetVoices,1
		RS_BYTE	LNr_Quit,1
		RS_BYTE	LNr_Bytes,1
		RS_BYTE	LNr_NoNewDirLoaded,1
		RS_BYTE	LNr_Active,1
		RS_BYTE	LNr_Show2,1
		RS_BYTE	LNr_SmallPlay,1
		RS_BYTE	LNr_SmallEject,1
		RS_BYTE	LNr_Author,1
		RS_BYTE	LNr_SubSongs,1
		RS_BYTE	LNr_Duration,1
		RS_BYTE	LNr_Minuts,1
		RS_BYTE	LNr_Seconds,1
		RS_BYTE	LNr_Length,1
		RS_BYTE	LNr_Steps,1
		RS_BYTE	LNr_Pattern,1
		RS_BYTE	LNr_Voices,1
		RS_BYTE	LNr_Samples,1
		RS_BYTE	LNr_SynthSamples,1
		RS_BYTE	LNr_SongSize,1
		RS_BYTE	LNr_SamplesSize,1
		RS_BYTE	LNr_ChipSize,1
		RS_BYTE	LNr_EQUalSize,1
		RS_BYTE	LNr_Creator,1
		RS_BYTE	LNr_Help,1
		RS_BYTE	LNr_Documentationloaded,1
		RS_BYTE	LNr_QuickStart,1
		RS_BYTE	LNr_ScrollInfos,1
		RS_BYTE	LNr_MasterVolume,1
		RS_BYTE	LNr_Filter,1
		RS_BYTE	LNr_FadeIn,1
		RS_BYTE	LNr_FadeOut,1
		RS_BYTE	LNr_QuitEagle,1
		RS_BYTE	LNr_SaveT,1
		RS_BYTE	LNr_Songname,1
		RS_BYTE	LNr_Prefix,1
		RS_BYTE	LNr_LoadFast,1
		RS_BYTE	LNr_EjectPlayers,1
		RS_BYTE	LNr_EjectEngines,1
		RS_BYTE	LNr_PlayerBatch,1
		RS_BYTE	LNr_LoadDir,1
		RS_BYTE	LNr_NewDirLoaded,1
		RS_BYTE	LNr_EPDir,1
		RS_BYTE	LNr_HideAll,1
		RS_BYTE	LNr_RescanDir,1
		RS_BYTE	LNr_Notify,1
		RS_BYTE	LNr_ScanAlways,1
		RS_BYTE	LNr_UnCrunched,1
		RS_BYTE	LNr_PPCrunched,1
		RS_BYTE	LNr_LHCrunched,1
		RS_BYTE	LNr_XPKCrunched,1
		RS_BYTE	LNr_CrMCrunched,1
		RS_BYTE	LNr_SetSaveDir,1
		RS_BYTE	LNr_SaveDirOk,1
		RS_BYTE	LNr_AutomaticSave,1
		RS_BYTE	LNr_SSDir,1
		RS_BYTE	LNr_SafeSave,1
		RS_BYTE	LNr_OverWrite,1
		RS_BYTE	LNr_SampleMode,1
		RS_BYTE	LNr_SaveAsProTracker,1
		RS_BYTE	LNr_Protect,1
		RS_BYTE	LNr_PBits,1
		RS_BYTE	LNr_NoNewModule,1
		RS_BYTE	LNr_LoadPrev,1
		RS_BYTE	LNr_LoadNext,1
		RS_BYTE	LNr_RandomModule,1
		RS_BYTE	LNr_RandomSong,1
		RS_BYTE	LNr_AutoSubSong,1
		RS_BYTE	LNr_LoadAlways,1
		RS_BYTE	LNr_Dirjump,1
		RS_BYTE	LNr_Songend,1
		RS_BYTE	LNr_LoadBefore,1
		RS_BYTE	LNr_CalcDuration,1
		RS_BYTE	LNr_RandomStart,1
		RS_BYTE	LNr_TimeMode,1
		RS_BYTE	LNr_CiaTiming,1
		RS_BYTE	LNr_VBlankTiming,1
		RS_BYTE	LNr_TimerDevice,1
		RS_BYTE	LNr_AutomaticWait,1
		RS_BYTE	LNr_RasterWait,1
		RS_BYTE	LNr_DBFWait,1
		RS_BYTE	LNr_AllocChannels,1
		RS_BYTE	LNr_SoftInt,1
		RS_BYTE	LNr_AutoPassword,1
		RS_BYTE	LNr_XFDDecrunch,1
		RS_BYTE	LNr_XFDLoadSeg,1
		RS_BYTE	LNr_SmallModule,1
		RS_BYTE	LNr_BigModule,1
		RS_BYTE	LNr_ModuleFault,1
		RS_BYTE	LNr_ReqLib,1
		RS_BYTE	LNr_ReqToolsLib,1
		RS_BYTE	LNr_AslLib,1
		RS_BYTE	LNr_AppIcon,1
		RS_BYTE	LNr_Appitem,1
		RS_BYTE	LNr_Volume,1
		RS_BYTE	LNr_Balance,1
		RS_BYTE	LNr_Speed,1
		RS_BYTE	LNr_ModuleLocked,1
		RS_BYTE	LNr_ModuleUnlocked,1
		RS_BYTE	LNr_NoGui,1
		RS_BYTE	LNr_ExtractFirstModule,1
		RS_BYTE	LNr_PleaseEnter,1
		RS_BYTE	LNr_SG_Publicscreen,1
		RS_BYTE	LNr_Publicscreen,1
		RS_BYTE	LNr_SG_PlayTime,1
		RS_BYTE	LNr_PlaytimeNow,1
		RS_BYTE	LNr_SG_MinTimeOut,1
		RS_BYTE	LNr_MinTimeOut,1
		RS_BYTE	LNr_SG_RasterLines,1
		RS_BYTE	LNr_RasterLines,1
		RS_BYTE	LNr_SG_DBFLoops,1
		RS_BYTE	LNr_DBFLoops,1
		RS_BYTE	LNr_SG_Password,1
		RS_BYTE	LNr_Password,1
		RS_BYTE	LNr_SG_XPKCrunchMode,1
		RS_BYTE	LNr_XPKCrunchMode,1

		RS_BYTE	LNr_PM_ProjectTitle,1
		RS_BYTE	LNr_PM_LoadModule,1
		RS_BYTE	LNr_PM_SaveModule,1
		RS_BYTE	LNr_PM_SaveModulePrefs,1
		RS_BYTE	LNr_PM_AboutModule,1
		RS_BYTE	LNr_PM_LoadPlayer,1
		RS_BYTE	LNr_PM_LoadConfig,1
		RS_BYTE	LNr_PM_SaveConfig,1
		RS_BYTE	LNr_PM_DeleteFile,1
		RS_BYTE	LNr_PM_Help,1
		RS_BYTE	LNr_PM_Documentation,1
		RS_BYTE	LNr_PM_Iconify,1
		RS_BYTE	LNr_PM_About,1
		RS_BYTE	LNr_PM_Hide,1
		RS_BYTE	LNr_PM_Quit,1

		RS_BYTE	LNr_PM_CommandTitle,1
		RS_BYTE	LNr_PM_PrevModule,1
		RS_BYTE	LNr_PM_PrevSong,1
		RS_BYTE	LNr_PM_PrevPattern,1
		RS_BYTE	LNr_PM_ReplaySong,1
		RS_BYTE	LNr_PM_Play,1
		RS_BYTE	LNr_PM_FastPlay,1
		RS_BYTE	LNr_PM_NextPattern,1
		RS_BYTE	LNr_PM_NextSong,1
		RS_BYTE	LNr_PM_NextModule,1
		RS_BYTE	LNr_PM_StopPlay,1
		RS_BYTE	LNr_PM_Eject,1

		RS_BYTE	LNr_PM_EngineTitle,1
		RS_BYTE	LNr_PM_LoadEngine,1
		RS_BYTE	LNr_PM_SpecialTitle,1
		RS_BYTE	LNr_PM_Preferences,1
		RS_BYTE	LNr_PM_ModulesMenu,1
		RS_BYTE	LNr_PM_SaveMode,1
		RS_BYTE	LNr_PM_Program,1
		RS_BYTE	LNr_PM_Timing,1
		RS_BYTE	LNr_PM_Warning,1
		RS_BYTE	LNr_PM_FileREQUester,1
		RS_BYTE	LNr_PM_IconifyMode,1
		RS_BYTE	LNr_PM_Layout,1
		RS_BYTE	LNr_PM_Cruncher,1
		RS_BYTE	LNr_PM_Voices,1
		RS_BYTE	LNr_PM_Volume,1
		RS_BYTE	LNr_PM_Balance,1
		RS_BYTE	LNr_PM_Speed,1
		RS_BYTE	LNr_PM_ToggleFilter,1
		RS_BYTE	LNr_PM_Publicscreen,1

		RS_BYTE	LNr_PM_SmallModule,1
		RS_BYTE	LNr_PM_BigModule,1
		RS_BYTE	LNr_PM_ModuleFault,1
		RS_BYTE	LNr_PM_ScrollInfos,1
		RS_BYTE	LNr_PM_FadeIn,1
		RS_BYTE	LNr_PM_FadeOut,1
		RS_BYTE	LNr_PM_Filter,1
		RS_BYTE	LNr_PM_MasterVolume,1
		RS_BYTE	LNr_PM_SongName,1
		RS_BYTE	LNr_PM_Prefix,1
		RS_BYTE	LNr_PM_SaveT,1
		RS_BYTE	LNr_PM_LoadFast,1
		RS_BYTE	LNr_PM_EjectPlayers,1
		RS_BYTE	LNr_PM_EjectEngines,1
		RS_BYTE	LNr_PM_PlayerBatch,1

		RS_BYTE	LNr_PM_RescanDir,1
		RS_BYTE	LNr_PM_LoadDir,1
		RS_BYTE	LNr_PM_EPDir,1
		RS_BYTE	LNr_PM_Notify,1
		RS_BYTE	LNr_PM_ScanAlways,1
		RS_BYTE	LNr_PM_HideAll,1

		RS_BYTE	LNr_PM_Uncrunched,1
		RS_BYTE	LNr_PM_PPCrunched,1
		RS_BYTE	LNr_PM_LHCrunched,1
		RS_BYTE	LNr_PM_XPKCrunched,1
		RS_BYTE	LNr_PM_CrMCrunched,1
		RS_BYTE	LNr_PM_AutomaticSave,1
		RS_BYTE	LNr_PM_Overwrite,1
		RS_BYTE	LNr_PM_SafeSave,1
		RS_BYTE	LNr_PM_SSDir,1
		RS_BYTE	LNr_PM_SampleMode,1
		RS_BYTE	LNr_PM_SaveAsProTracker,1
		RS_BYTE	LNr_PM_Protect,1
		RS_BYTE	LNr_PM_ProtectionBits,1
		RS_BYTE	LNr_PM_SaveDir,1
		RS_BYTE	LNr_PM_XPKCrunchmode,1
		RS_BYTE	LNr_PM_Password,1

		RS_BYTE	LNr_PM_NoNewModule,1
		RS_BYTE	LNr_PM_LoadPrev,1
		RS_BYTE	LNr_PM_LoadNext,1
		RS_BYTE	LNr_PM_RandomModule,1
		RS_BYTE	LNr_PM_Songend,1
		RS_BYTE	LNr_PM_LoadBefore,1
		RS_BYTE	LNr_PM_RandomSong,1
		RS_BYTE	LNr_PM_AutoSubSong,1
		RS_BYTE	LNr_PM_LoadAlways,1
		RS_BYTE	LNr_PM_Dirjump,1
		RS_BYTE	LNr_PM_TimeMode,1
		RS_BYTE	LNr_PM_CalcDuration,1
		RS_BYTE	LNr_PM_RandomStart,1
		RS_BYTE	LNr_PM_Quickstart,1
		RS_BYTE	LNr_PM_PlayTime,1
		RS_BYTE	LNr_PM_MinPlayTime,1


		RS_BYTE	LNr_PM_AutoPassword,1
		RS_BYTE	LNr_PM_ExtractFirstModule,1
		RS_BYTE	LNr_PM_XFDDecrunch,1
		RS_BYTE	LNr_PM_XFDLoadSeg,1
		RS_BYTE	LNr_PM_Req,1
		RS_BYTE	LNr_PM_ReqTools,1
		RS_BYTE	LNr_PM_Asl,1
		RS_BYTE	LNr_PM_AppIcon,1
		RS_BYTE	LNr_PM_AppItem,1
		RS_BYTE	LNr_PM_Menufont,1
		RS_BYTE	LNr_PM_Modulesfont,1
		RS_BYTE	LNr_PM_ScrollFont,1
		RS_BYTE	LNr_PM_CiaTiming,1
		RS_BYTE	LNr_PM_VBlankTiming,1
		RS_BYTE	LNr_PM_TimerDevice,1
		RS_BYTE	LNr_PM_AutomaticWait,1
		RS_BYTE	LNr_PM_RasterWait,1
		RS_BYTE	LNr_PM_DBFWait,1
		RS_BYTE	LNr_PM_RasterLines,1
		RS_BYTE	LNr_PM_DBFLoops,1
		RS_BYTE	LNr_PM_AllocChannels,1
		RS_BYTE	LNr_PM_SoftInt,1

		RS_BYTE	LNr_PM_Voice1,1
		RS_BYTE	LNr_PM_Voice2,1
		RS_BYTE	LNr_PM_Voice3,1
		RS_BYTE	LNr_PM_Voice4,1
		RS_BYTE	LNr_PM_ResetVoices,1
		RS_BYTE	LNr_PM_VolumeDefault,1
		RS_BYTE	LNr_PM_VolumeHigher,1
		RS_BYTE	LNr_PM_VolumeLower,1
		RS_BYTE	LNr_PM_BalanceDefault,1
		RS_BYTE	LNr_PM_BalanceLeft,1
		RS_BYTE	LNr_PM_BalanceRight,1
		RS_BYTE	LNr_PM_SpeedDefault,1
		RS_BYTE	LNr_PM_SpeedFaster,1
		RS_BYTE	LNr_PM_SpeedSlower,1
		RS_BYTE	LNr_PM_LastMenuItem,0

		RS_BYTE	LNr_ModuleWarning,1
		RS_BYTE	LNr_ModuleShorter,1
		RS_BYTE	LNr_ModuleHey,1
		RS_BYTE	LNr_ModuleLonger,1
		RS_BYTE	LNr_ModuleAs,1
		RS_BYTE	LNr_ThisIsA,1
		RS_BYTE	LNr_OriginalSize,1
		RS_BYTE	LNr_PackedSize,1
		RS_BYTE	LNr_BytesWon,1
		RS_BYTE	LNr_Module,1
		RS_BYTE	LNr_SongIsOver,1
		RS_BYTE	LNr_TimeIsOver,1
		RS_BYTE	LNr_LoadingList,1
		RS_BYTE	LNr_Scanning,1
		RS_BYTE	LNr_Parent,1
		RS_BYTE	LNr_Entries,1
		RS_BYTE	LNr_Entry,1
		RS_BYTE	LNr_ModulesTitle,1
		RS_BYTE	LNr_FT1,1
		RS_BYTE	LNr_FT2,1
		RS_BYTE	LNr_FT3,1
		RS_BYTE	LNr_FT4,1
		RS_BYTE	LNr_FT5,1
		RS_BYTE	LNr_FT6,1
		RS_BYTE	LNr_FT7,1
		RS_BYTE	LNr_FT8,1
		RS_BYTE	LNr_FT9,1
		RS_BYTE	LNr_FT10,1
		RS_BYTE	LNr_FT11,1
		RS_BYTE	LNr_FT12,1
		RS_BYTE	LNr_FT13,1
		RS_BYTE	LNr_FT14,1
		RS_BYTE	LNr_FT15,1
		RS_BYTE	LNr_FT16,1
		RS_BYTE	LNr_FT17,1
		RS_BYTE	LNr_FT18,1
		RS_BYTE	LNr_FT19,1
		RS_BYTE	LNr_FT20,1
		RS_BYTE	LNr_FT21,1
		RS_BYTE	LNr_FT22,1
		RS_BYTE	LNr_FT23,1
		RS_BYTE	LNr_FT24,1
		RS_BYTE	LNr_FT25,1
		RS_BYTE	LNr_FT26,1
		RS_BYTE	LNr_FT27,1
		RS_BYTE	LNr_FT28,1
		RS_BYTE	LNr_FT29,1
		RS_BYTE	LNr_FT30,1
		RS_BYTE	LNr_FT31,1
		RS_BYTE	LNr_FT32,1
		RS_BYTE	LNr_FT33,1
		RS_BYTE	LNr_FT34,1
		RS_BYTE	LNr_FT35,1
		RS_BYTE	LNr_FT36,1
		RS_BYTE	LNr_FT37,1
		RS_BYTE	LNr_FT38,1
		RS_BYTE	LNr_FT39,1
		RS_BYTE	LNr_FT40,1
		RS_BYTE	LNr_FT41,1
		RS_BYTE	LNr_FT42,1
		RS_BYTE	LNr_FT43,1
		RS_BYTE	LNr_FT44,1
		RS_BYTE	LNr_FT45,1
		RS_BYTE	LNr_FT46,1
		RS_BYTE	LNr_FT47,1
		RS_BYTE	LNr_FT48,1
		RS_BYTE	LNr_FT49,1
		RS_BYTE	LNr_FT50,1
		RS_BYTE	LNr_FT51,1
		RS_BYTE	LNr_FT52,1
		RS_BYTE	LNr_FT53,1
		RS_BYTE	LNr_FT54,1
		RS_BYTE	LNr_FT55,1
		RS_BYTE	LNr_FT56,1
		RS_BYTE	LNr_FT57,1
		RS_BYTE	LNr_FT58,1
		RS_BYTE	LNr_FT59,1
		RS_BYTE	LNr_FT60,1
		RS_BYTE	LNr_FT61,1
		RS_BYTE	LNr_FT62,1
		RS_BYTE	LNr_FT63,1
		RS_BYTE	LNr_FT64,1
		RS_BYTE	LNr_FT65,1
		RS_BYTE	LNr_FT66,1
		RS_BYTE	LNr_FT67,1
		RS_BYTE	LNr_FT68,1
		RS_BYTE	LNr_FT69,1
		RS_BYTE	LNr_FT70,1
		RS_BYTE	LNr_CreditsTitle,1
		RS_BYTE	LNr_DistributionTitle,1
		RS_BYTE	LNr_LookingTitle,1
		RS_BYTE	LNr_GlobalInfosTitle,1
		RS_BYTE	LNr_RegisterTitle,1
		RS_BYTE	LNr_AboutEagleplayer,1
		RS_BYTE	LNr_EagleplayerREQUest,1
		RS_BYTE	LNr_AboutGlobal,1
		RS_BYTE	LNr_AboutCredits,1
		RS_BYTE	LNr_AboutDistribution,1
		RS_BYTE	LNr_AboutLooking,1
		RS_BYTE	LNr_AboutRegister,1
		RS_BYTE	LNr_AboutRRegister,1
		RS_BYTE	LNr_TR_KeyHasAFault,1
		RS_BYTE	LNr_TR_KeyIsDisabled,1
		RS_BYTE	LNr_TR_Config,1
		RS_BYTE	LNr_TR_ToolTypes,1
		RS_BYTE	LNr_TR_Overwrite,1
		RS_BYTE	LNr_TR_SaveAsPT,1
		RS_BYTE	LNr_TR_Protection,1
		RS_BYTE	LNr_TR_SmallModule,1
		RS_BYTE	LNr_TR_ModuleFault,1
		RS_BYTE	LNr_TR_AudioAlloc,1
		RS_BYTE	LNr_TR_RippError,1
		RS_BYTE	LNr_TR_EmulatorError,1
		RS_BYTE	LNr_TR_IllegalFunction,1
		RS_BYTE	LNr_TR_ErrorLoadingInstruments,1
		RS_BYTE	LNr_EPBy,1
		RS_BYTE	LNr_GA_Ok,1
		RS_BYTE	LNr_GA_Yes,1
		RS_BYTE	LNr_GA_Config,1
		RS_BYTE	LNr_GA_RetrySkipCancel,1
		RS_BYTE	LNr_GA_SmallModule,1
		RS_BYTE	LNr_GA_Global,1
		RS_BYTE	LNr_GA_Credits,1
		RS_BYTE	LNr_GA_Distribution,1
		RS_BYTE	LNr_GA_Looking,1
		RS_BYTE	LNr_GA_Register,1
		RS_BYTE	LNr_AboutKey,1
		RS_BYTE	LNr_HideKey,1

		RS_BYTE	LNr_TR_RegisteredNormal,1
		RS_BYTE	LNr_TR_RegisteredSaveModule,1
		RS_BYTE	LNr_GA_Registered,1
		RS_BYTE	LNr_NervREQUest,1
		RS_BYTE	LNr_TR_RegisteredStart,1

		RS_BYTE	LNr_Fr_LoadList,1
		RS_BYTE	LNr_SW_LoadList,1
		RS_BYTE	LNr_Fr_SaveList,1
		RS_BYTE	LNr_SW_SaveList,1
		RS_BYTE	LNr_PM_LoadList,1
		RS_BYTE	LNr_PM_SaveList,1
		RS_BYTE	LNr_NewListloaded,1
		RS_BYTE	LNr_By,1
		RS_BYTE	LNr_ModulImFastMem,1
		RS_BYTE	LNr_ChipRAMAmplifier,1
		RS_BYTE	LNr_FastRAMAmplifier,1

		RS_BYTE	LNr_LastExternal,0


		*--- Nun die externen Enginestrings ---*

		RS_RESET
		RS_BYTE	LNr_E_Dummy,LNr_FirstEngine
		RS_BYTE	LNrE_SelectExoticPath,1
		RS_BYTE	LNrE_SelectingExoticPath,1
		RS_BYTE	LNrE_SelectExoticPathTitle,1
		RS_BYTE	LNrE_CantRunSecondExoticGui,1
		RS_BYTE	LNrE_CantQuitEagleexotic,1
		RS_BYTE	LNrE_CantQuitExoticripper,1
		RS_BYTE	LNrE_CantLoadExoticripper,1
		RS_BYTE	LNrE_ExoticKeinKey,1

		RS_BYTE	LNrE_SpecialInfo_8BitAmplifier,1
		RS_BYTE	LNrE_SpecialInfo_14BitAmplifier,1
		RS_BYTE	LNrE_SpecialInfo_AmplifierManager,1
		RS_BYTE	LNrE_SpecialInfo_BifatGUI,1
		RS_BYTE	LNrE_SpecialInfo_DirListViewer,1
		RS_BYTE	LNrE_SpecialInfo_Eagleexotic,1
		RS_BYTE	LNrE_SpecialInfo_Eagleplayer1GUI,1
		RS_BYTE	LNrE_SpecialInfo_Extractor,1
		RS_BYTE	LNrE_SpecialInfo_FFTAnalyzer,1
		RS_BYTE	LNrE_SpecialInfo_Levelgraph,1
		RS_BYTE	LNrE_SpecialInfo_Levelmeter,1
		RS_BYTE	LNrE_SpecialInfo_Manager,1
		RS_BYTE	LNrE_SpecialInfo_Messagewindow,1
		RS_BYTE	LNrE_SpecialInfo_Moduleinfo,1
		RS_BYTE	LNrE_SpecialInfo_Patternscroll,1
		RS_BYTE	LNrE_SpecialInfo_Playerloader,1
		RS_BYTE	LNrE_SpecialInfo_PublicscreenSelector,1
		RS_BYTE	LNrE_SpecialInfo_Quadrascope,1
		RS_BYTE	LNrE_SpecialInfo_SpaceScope,1
		RS_BYTE	LNrE_SpecialInfo_Stereoscope,1
		RS_BYTE	LNrE_SpecialInfo_Time,1


		RS_BYTE	LNrE_RefreshList,1		; früher RefreshList
		RS_BYTE	LNrE_RefreshListKey,1		; früher RefreshList
		RS_BYTE	LNrE_DLV_EasyTitle,1
		RS_BYTE	LNrE_DLV_About,1

		RS_BYTE	LNrE_PSS_EasyTitle,1
		RS_BYTE	LNrE_PSS_About,1
		RS_BYTE	LNrE_ChooseBackPic,1
		RS_BYTE	LNrE_BackPic,1

		RS_BYTE	LNrE_SpecialInfo_Pysion,1
		RS_BYTE	LNrE_SpecialInfo_SampleSaver,1
		RS_BYTE	LNrE_SpecialInfo_Noiseconverter,1
		RS_BYTE	LNrE_SpecialInfo_Formatloader,1

		RS_BYTE	LNrE_Exoticerror,1

		RS_BYTE	LNrE_SpecialInfo_Eagleripper,1
		RS_BYTE	LNrE_Pass1,1
		RS_BYTE	LNrE_Pass2,1
		RS_BYTE	LNrE_Pass3,1
		RS_BYTE	LNrE_SecurityMode,1
		RS_BYTE	LNrE_Ripping,1

		RS_BYTE	LNrE_Security1,1
		RS_BYTE	LNrE_Security2,1
		RS_BYTE	LNrE_Unknown,1
		RS_BYTE	LNrE_ER_SecurityGadgets,1

*LNrE_DefectSoftworks		RS_BYTE	1
*LNrE_RescanDirKey		RS_BYTE	1
*LNrE_AboutKey			RS_BYTE	1
*LNrE_HideKey			RS_BYTE	1
*LNrE_QuitKey			RS_BYTE	1


		RS_BYTE	LNrE_LastEngine,0

		*--- Nun die externen Playerstrings ---*
		RS_RESET
		RS_BYTE	LNrP_Dummy,LNr_FirstPlayer
		RS_BYTE	LNrP_AdaptedByDefect,1
		RS_BYTE LNrP_PleaseSelectYourInstrumentsDir,1
		RS_BYTE	LNrP_LastPlayer,0


*-----------------------------------------------------------------------------*

	RS_RESET
	RS_LONG	UPrgS_NextUPrg,1	;Next Userprogramm

	*------------ Userprogramm-Item-Struktur ------------*
	RS_LONG	UPrgS_NextItem,1	;Next
	RS_WORD	UPrgS_Left,1   ;SpecialItemBreite-30	;Left Edge
	RS_WORD	UPrgS_Verti,1  ;PrEngineVerti1	;Verti Pos.
	RS_WORD	UPrgS_Width,1  ;UserItemBreite	;Width-Size (Negationbreite)
	RS_WORD	UPrgS_Height,1 ;10			;Hight-Size
	RS_WORD	UPrgS_Flags,1  ;$4b
	RS_LONG	UPrgS_Dummy1,1	;Mutalexclude
	RS_LONG	UPrgS_IAdr,1	;ItemFill
	RS_LONG	UPrgS_Dummy2,1	;SelectFill
	RS_WORD	UPrgS_Dummy3,1	;Command/KludgeFill
	RS_LONG	UPrgS_SubItem,1	;SubItem
	RS_WORD	UPrgS_Dummy4,1	;NextSelect
	RS_LONG	UPrgS_Dummy5,1	;ab Kick2.0+
	RS_LONG	UPrgS_IText,2	;dc.w	3,0,20,1
	RS_LONG	UPrgS_Font,1
	RS_LONG	UPrgS_Name,1	;Menuname des Engines
	RS_LONG	UPrgS_Wackel,1
	RS_WORD	UPrgS_EPNr,1	;AktivationsNr =EPNr_Engine

	RS_BYTE	UPrgS_Node,LN_SIZE

	*----------- Zusätzliche Vereinbarungen -------------*
	RS_BYTE	UPrgS_Message,UM_SizeOf		;Message-Struktur+Sicherheit
	RS_LONG	UPrgS_EUSAdr,1			;EUS_Adresse
	RS_LONG	UPrgS_EngineNr,1		;EngineNr
	RS_WORD	UPrgS_WinX,1			;WinX
	RS_WORD	UPrgS_WinY,1			;WinY
	RS_WORD	UPrgS_EUSFlags,1		;Flags
	RS_LONG	UPrgS_Special1,1		;Special1
	RS_LONG	UPrgS_Special2,1		;Special2
	RS_LONG	UPrgS_Special3,1		;Special3
	RS_WORD	UPrgS_CFlags,1			;Welche Zellen sind belegt
*UPrgS_BatchName	RS_LONG	1		;NamenAdresse in der BatchDatei
*UPrgS_NameAdr	RS_LONG	1			;PfadAdresse in der BatchDatei
	RS_LONG	UPrgS_UFlags,1			;Flags (1=Loaded)
	RS_LONG	UPrgS_Size,1
*UPrgS_SizeOf
 	RS_BYTE	UPrgS_NamePuffer,0		;additional


UPrgF_Engine		EQU	0
UPrgF_BatchName		EQU	1
UPrgF_Debugger		EQU	2
UPrgF_TagUser		EQU	3
UPrgF_Active		EQU	4
UPrgF_Disabled		EQU	5
UPrgF_Gui		EQU	6
UPrgF_Show		EQU	7

UPrgB_Engine		EQU	1<<UPrgF_Engine
UPrgB_BatchName		EQU	1<<UPrgF_BatchName
UPrgB_Debugger		EQU	1<<UPrgF_Debugger
UPrgB_TagUser		EQU	1<<UPrgF_TagUser
UPrgB_Active		EQU	1<<UPrgF_Active
UPrgB_Disabled		EQU	1<<UPrgF_Disabled
UPrgB_Gui		EQU	1<<UPrgF_Gui
UPrgB_Show		EQU	1<<UPrgF_Show


;Namenlange		EQU	32			; obsolete !!!
MaxFilenameSize		EQU	108
MaxDirectorynameSize	EQU	500


*--------- Struktur für einen Eintrag im Eagleplayer-FilePuffer --------------*
* wird vom Eagleplayer, Dirlistviewer, Pysion benutzt.
*-----------------------------------------------------------------------------*
	RS_RESET
	RS_LONG	EFP_NextAdr,1			;MenuItem
	RS_LONG	EFP_MI_NextItem,1
	RS_WORD	EFP_MI_LeftEdge,1
	RS_WORD	EFP_MI_TopEdge,1
	RS_WORD	EFP_MI_Width,1
	RS_WORD	EFP_MI_Height,1
	RS_WORD	EFP_MI_Flags,1
	RS_LONG	EFP_MI_MutualExclude,1
	RS_LONG	EFP_MI_ItemFill,1
	RS_LONG	EFP_MI_SelectFill,1
	RS_BYTE	EFP_MI_Command,1
	RS_BYTE	EFP_MI_KludgeFill00,1
	RS_LONG	EFP_MI_SubItem,1
	RS_WORD	EFP_MI_NextSelect,1
	RS_LONG	EFP_Dummy1,1				;ab Kick2.0+
	RS_BYTE	EFP_IT_FrontPen,1			;I-TextStrukture
	RS_BYTE	EFP_IT_BackPen,1
	RS_BYTE	EFP_IT_DrawMode,1
	RS_BYTE	EFP_IT_KludgeFill00,1
	RS_WORD	EFP_IT_LeftEdge,1
	RS_WORD	EFP_IT_TopEdge,1
	RS_LONG	EFP_IT_ITextFont,1
	RS_LONG	EFP_IT_IText,1
	RS_LONG	EFP_IT_NextText,1
	RS_WORD	EFP_ModuleNr,1
	RS_BYTE	EFP_Node,LN_SIZE
	RS_LONG	EFP_StructSize,1
	RS_LONG	EFP_PathArrayPtr,1		;kompletter Pfad (für Listen)
	RS_LONG	EFP_DirArrayPtr,1
	RS_LONG	EFP_FileArrayPtr,1
	RS_LONG	EFP_NameArrayPtr,1		;without Präfix
	RS_LONG	EFP_NameDirLock,1
	RS_WORD	EFP_ArchiveEngineNr,1		;Nummer des Extractors
	RS_BYTE	EFP_Flags,1			;Achtung,wegen LHA kann
	RS_BYTE	EFP_NameRKennung,1		;EFP_NamePuffer länger sein !!!
	RS_BYTE	EFP_NamePuffer,MaxFilenameSize+2	;komplette Size in EFP_StructSize
	RS_BYTE	EFP_NamePufferend,0

EFPF_FileName		EQU	0
EFPF_DirName		EQU	1
EFPF_Parent		EQU	2
EFPF_ArchiveDir		EQU	3
EFPF_ArchiveFile	EQU	4
EFPF_ParentList		EQU	5
EFPF_All		EQU	-1

EFPB_FileName		EQU	1<<EFPF_FileName
EFPB_DirName		EQU	1<<EFPF_DirName
EFPB_Parent		EQU	1<<EFPF_Parent
EFPB_ArchiveDir		EQU	1<<EFPF_ArchiveDir
EFPB_ArchiveFile	EQU	1<<EFPF_ArchiveFile
EFPB_ParentList		EQU	1<<EFPF_ParentList



*-------------------------------Audio Struct---------------------------------
	RS_RESET
	RS_LONG	AS_CurrentAdr,1	;wenn AS_CurrentAdr gesetzt wird, muß
	RS_LONG	AS_CurrentPos,1	;AS_Currentpos auf 0 zurückgesetzt werden !!!
				;(Currentpos = aktuelles Offset von Currentadr aus gesehen,
				;wird vom Amplifier erhöht)
	RS_LONG	AS_CurrentFPos,1 ;genauso AS_CurrentFPos (Fraction Position),
				;muss auch 0 gesetzt werden bei neuem Sample vom Replay,
				;Inhalt undefiniert.
					
	RS_LONG	AS_SampleSize,1	;Länge in Bytes
	RS_WORD	AS_Period,1	;Periodenwert, kompatibel zu Amiga-Standardwerten
	RS_LONG	AS_RepeatAdr,1	;Repeatadresse (wenn kein Repeat, AS_Loopflag=0 setzen !!)
	RS_LONG	AS_RepeatSize,1	;Repeatlänge in Bytes
	RS_WORD	AS_LeftVolume,1	;Lautstärke 0..64   -> wenn kein Panning, BEIDE !
	RS_WORD	AS_RightVolume,1 ;Lautstärke 0..64      gleich setzen !!!!!!!!!!
	RS_WORD	AS_DMABit,1	; 0 -> Kanal aus, sonst an
	RS_WORD	AS_LeftRight,1	; 0 = gor nix auf diesem Kanal
				; 1 = Kanal nur links
				;-1 = Kanal nur rechts
				; 2 = Rechts & Links (Panning über AS_LeftVolume & -RightVolume)
	RS_BYTE	AS_LoopFlag,1	;see Definititions below
	RS_BYTE	AS_NoLoop,1	;rücksetzen auf 0 durch Player, wenn neue  Note gespielt wird!!
	RS_LONG	AS_Changeflags,1 ;Flags für geänderte Variablen
	RS_LONG	AS_SampleFlags,1 ;Typ des Samples usw. (unsupported yet)

	RS_LONG	AS_Int,1	;Softint Struktur, wird aufgerufen, wenn auf
				;diesem Kanal ein neues Sample angespielt wird,
				;genau wie beim "Paula"-Chip

	RS_LONG	AS_AmplifierPrivate1,1	;können beliebig durch den Amplifier genutzt
	RS_LONG	AS_AmplifierPrivate2,1	;werden, dürfen nach der Übergabe der Struktur an den
	RS_LONG	AS_AmplifierPrivate3,1	;Amplifier keinesfalls durch das Replay verändert werden,
	RS_LONG	AS_AmplifierPrivate4,1	;am besten mit 0 initialisieren, dann an den Amplifier
	RS_LONG	AS_AmplifierPrivate5,1	;übergeben und nicht weiter drum kümmern
	RS_LONG	AS_AmplifierPrivate6,1	;
	RS_LONG	AS_AmplifierPrivate7,1	;
	RS_LONG	AS_AmplifierPrivate8,1	;

	RS_LONG	AS_Reserved1,1
	RS_LONG	AS_Reserved2,1
	RS_LONG	AS_Reserved3,1
	RS_LONG	AS_Reserved4,1
	RS_LONG	AS_Reserved5,1
	RS_WORD	AS_Sizeof,0

;----------------------------- Flags for Looping ------------------------------
ASLoop_None		EQU	0		;no Loop (whole Byte = 0)
ASLoopF_Forwards	EQU	0		;1 = Loop nur vorwärts
ASLoopF_Backwards	EQU	1		;2 = Loop nur rückwärts (not supported yet)
ASLoopF_PingPong	EQU	2		;3 = Ping-Pong Loop

ASLoopB_Forwards	EQU	1<<ASLoopF_Forwards
ASLoopB_Backwards	EQU	1<<ASLoopF_Backwards
ASLoopB_PingPong	EQU	1<<ASLoopF_PingPong

;----------------------------- ChangeFlags -----------------------------------

ASChF_Adr		EQU	0
ASChF_Len		EQU	1
ASChF_RepAdr		EQU	2
ASChF_RepLen		EQU	3
ASChF_Per		EQU	4
ASChF_Vol		EQU	5
ASChF_DMA		EQU	6
ASChF_SamFlags		EQU	7

ASChB_Adr		EQU	1<<ASChF_Adr
ASChB_Len		EQU	1<<ASChF_Len
ASChB_RepAdr		EQU	1<<ASChF_RepAdr
ASChB_RepLen		EQU	1<<ASChF_RepLen
ASChB_Per		EQU	1<<ASChF_Per
ASChB_Vol		EQU	1<<ASChF_Vol
ASChB_DMA		EQU	1<<ASChF_DMA
ASChB_SamFlags		EQU	1<<ASChF_SamFlags

;---------------------- Tagliste für Amplifierstrukturen/Infos ---------------

EPAMT_TagBase	EQU TAG_USER+16717		;TAG_USER+"AM"

	ENUM	EPAMT_TagBase			; TagBase

	EITEM	EPAMT_NumStructs		;Number of supplied Audio
						;structures
	EITEM	EPAMT_AudioStructs		;Address of the first Audio
						;structure
	EITEM	EPAMT_MinEPVersion		;Minimum EP Version needed
	
	EITEM	EPAMT_Flags			;Flags (see below)

;--------------------------- Flags für Amplifier ----------------------------

EPAMF_Direct		EQU	0 ;for Chipram/Fastram Amplifier, Replayer supports "ENPP_PokeXXX"
EPAMF_8Bit		EQU	1 ;8 Bit SampleData, signed
EPAMF_8BitUnsigned	EQU	2 ;8 Bit SampleData, unsigned
EPAMF_16Bit		EQU	3 ;16 Bit SampleData, signed
EPAMF_ChipRam		EQU	4 ;Samples located in ChipRam
EPAMF_PingPongLoops	EQU	5 ;Replayer supports Ping-Pong-Loops (e.g. FT-II)
EPAMF_WaitForStruct	EQU	6 ;Amplifier has to wait for a complete AS-Structure
EPAMF_AudioInts		EQU	7 ;Replayer needs Audio Interrupt support

EPAMB_Direct		EQU	1<<EPAMF_Direct
EPAMB_8Bit		EQU	1<<EPAMF_8Bit
EPAMB_8BitUnsigned	EQU	1<<EPAMF_8BitUnsigned
EPAMB_16Bit		EQU	1<<EPAMF_16Bit
EPAMB_ChipRam		EQU	1<<EPAMF_ChipRam
EPAMB_PingPongLoops	EQU	1<<EPAMF_PingPongLoops
EPAMB_WaitForStruct	EQU	1<<EPAMF_WaitForStruct
EPAMB_AudioInts		EQU	1<<EPAMF_AudioInts

*---------------------- Playerstruct (for Playerloader) ----------------------*
	RS_RESET
	RS_LONG	EPPl_Next,1
	RS_BYTE	EPPl_Node,LN_SIZE
	RS_LONG	EPPl_Size,1			*StructSize
	RS_LONG	EPPl_Segment,1
	RS_LONG	EPPl_TagList,1
	RS_LONG	EPPl_ExtraMem,1
	RS_LONG	EPPl_ExtraMemSize,1
	RS_LONG	EPPl_Taskadr,1
	RS_LONG	EPPl_MsgPort,1
	RS_WORD	EPPl_Flags,1
	RS_BYTE	EPPl_PlayerName,0
	RS_BYTE	EPPl_Sizeof,0

EPPLF_Enabled		EQU	0
EPPLB_Enabled		EQU	1<<EPPLF_Enabled

	ENDC	; EAGLEPLAYERENGINE_I

*******************************************************************************
* Buggs:
*
* Jan:
*	Menus
*	Userprograms
*
* Amplifiersystem:
* -----------------
*
* Konfiguration der Amplifiers
*	EPConfig		= Win...,[Amplifier_Priorität.w,Flags.w] (EUS_Special.l)
*	eigene Config	= ???
*
*EUS_AMPriority	EQU	EUS_Special	;Priorität des Amplifiers
*					;(zum Einsortieren)
*EUS_AMFlags	EQU	EUS_Special+2	;Flags des Amplifers (NoWin)
*
*--------------------------------- EUS-Flags ------------------------------*
*EUSF_Disable	EQU	0	;Amplifier Disabled ?
*				;Bit set = yes
*EUSB_Disable	EQU	1<<EUSF_Disable
*
*
* 2 interne User ChipRAMPlayer,FastRAMPlayer (4 Voices), nur in Liste der
* Amplifier, werden in Userprogrammliste nicht mit aufgenommen, da hier
* keine Einstellungen wie Mixrate usw. vorgenommen werden müssen, einziges
* Problem sind die Priorität und Enable/Disable, welche ja mit in der Konfig
* gespeichert werden sollen, mal sehen was sich da machen läßt
*
*
* Beim Entfernen der Engines Amplifierliste überarbeiten und gegebenenfalls
* das aktuelle Modul rauswerfen, falls der aktuelle Amplifier davon betroffen
* ist, selbiges gilt für das Laden/Einfügen von Userprogrammen
*
*	EUT_AmplifierManager <- An diesen bei jeder Änderung der Amplifierliste
*				eine Message "USClass_NewAmplifierlist" schicken
*
*
*
* jeder Amplifier wird höchstens einmal geladen
*		(EUS_AMIDNr		; wird von uns bestimmt)
* eigene Liste in
*
*
*			EPG_AmplifierList
*			EPG_ActiveAmplifier ;EUS_StartAdr
*			EPG_AudioStruct	;aktuelle AudioStruct (EP privat)
*			EPG_AmplifierTagList
*
*			LONG	EPAMT_NumStructs
*			APTR	EPAMT_AudioStructs
*			LONG	EPAMT_MinEPVersion
*			LONG	EPAMT_Flags
*				EPAMF_Direct
*				EPAMF_8Bit
*				EPAMF_8BitUnsigned
*				EPAMF_16Bit
*				EPAMF_ChipRam		;Samples im Chip
*
*
*
*	EUS_Type = EUT_Amplifier		;Userprogramtype (0=unknown)
*
* ENPP_PokeAdr
* ENPP_PokeLen
* ENPP_PokePer
* ENPP_PokeVol
* ENPP_DMABit
*
*	APTR	EUS_SpecialJumpTab	;private Jumptab (e.g Amplifier)
*			AMJ_Init	;Übergabe der Struktur/test&Init
*			AMJ_StartInt	;Alloc Audio & Start Int
*			AMJ_StopInt	;Free Audio & Stop Int
*			AMJ_End		;Mem freigeben, etc.
*
*			AMJ_PokeAdr	;nur für Amplifier initialisieren,
*			AMJ_PokeLen	;die auf die Hardware poken, wie
*			AMJ_PokePer	;z.B. der Chipram Player, sonst
*			AMJ_PokeVol	;auf 0 setzen, diese Jumps werden mit
*			AMJ_DMABit	;den selben Parametern aufgerufen, wie
*					;die ENPP_Poke...
*
* AudioStructure: siehe oben
*
*
*
*
*EUF_NoPDMenu	\ können die nicht eigentlich raus ?
*EUF_PDDisabled	/
*
*
*-> UCM_KillEngine -> oder gibt`s das schon ?
*-> UCM_Engine	-> bei "Hide" darf das UPS auch bei "Eject Engine"
*			nicht rausgeworfen werden, "Eject" ständig ein/aus-
*			zuschalten würde wegen des Menüs zu lange dauern
