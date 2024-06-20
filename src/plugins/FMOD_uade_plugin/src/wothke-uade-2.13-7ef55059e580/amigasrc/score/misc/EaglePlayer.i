**
**  $Filename: misc/EaglePlayer.i
**  $Release: 2.00 $
**  $Revision: 13$
**  $Date: 22/02/97$
**
** Definitions and Macros for creating EaglePlayer Playermodules
**
**	(C) Copyright 1993-97 by DEFECT
**	    All Rights Reserved
**
**

	IFND	EAGLEPLAYER_I
EAGLEPLAYER_I	SET	1


	IFND	DeliTracker_Player_i
		Include	"misc/DeliPlayer.i"
	ENDC


EAGLEVERSION	EQU	11			;Current Version of EaglePlayer
;9=V1.53/54
;10=1.55 Alpha
;11=2.00

*--------------------------- New EaglePlayer Tags ----------------------------*
EP_TagBase	EQU	TAG_USER+"EP"


	ENUM	EP_TagBase		;EaglePlayer-TagBase

	EITEM	EP_Get_ModuleInfo	;get Pointer to Moduleinfo Taglist
	EITEM	EP_Free_ModuleInfo	;free Taglist (e.g. Free Mem)
	EITEM	EP_Voices		;Set the Voices
	EITEM	EP_SampleInit		;create Sampleinfostructure
	EITEM	EP_SampleEnd		;ejected !!!
	EITEM	EP_Save
	EITEM	EP_ModuleChange		;Change Module
	EITEM	EP_ModuleRestore	;Restore Module
	EITEM	EP_StructInit		;Return Pointer to Userprg.Struct (UPS_)
	EITEM	EP_StructEnd		;Free Struct (e.g. Free allocated Mem)
	EITEM	EP_LoadPlConfig		;Load Config of Player
	EITEM	EP_SavePlConfig		;Save Config of Player
	EITEM	EP_GetPositionNr	;return Patternnumber (in D0.l)
	EITEM	EP_SetSpeed		;Set Speed (for Players with own Timer)
					;Value submitted in D0
	EITEM	EP_Flags		;see below
	EITEM	EP_KickVersion		;min Kickstartversion
	EITEM	EP_PlayerVersion	;min EP-Version
	EITEM	EP_CheckModule		;Checks the module (d0=Error or zero)
	EITEM	EP_EjectPlayer
	EITEM	EP_Date			;Creator-Date (e.g 11.05.1993
					;11.b, 05.b, 1993.w )
	EITEM	EP_Check3
	EITEM	EP_SaveAsPT		;Save Module as Protrackermodule

	EITEM	EP_NewModuleInfo	;TI_Data -> Pointer to Moduleinfo Taglist
	EITEM	EP_FreeExtLoad
	EITEM	EP_PlaySample		;Play Sample d0=SampleNr
	EITEM	EP_PatternInit		;Init Patterninfostruct
	EITEM	EP_PatternEnd		;Free Patterninfostruct - optional
	EITEM	EP_Check4
	EITEM	EP_Check5
	EITEM	EP_Check6

	EITEM	EP_CreatorLNr
	EITEM	EP_PlayerNameLNr

	EITEM	EP_PlayerInfo		;TI_Data -> APTR to TagList
	EITEM	EP_PlaySampleInit
	EITEM	EP_PlaySampleEnd

	EITEM	EP_InitAmplifier	;Input:  None
					;Output: d0=NUll oder Error
					;  Tagliste in EPGL_AmplifierTagList

	EITEM	EP_CheckSegment
	EITEM	EP_Show
	EITEM	EP_Hide
	EITEM	EP_LocaleTable
	EITEM	EP_Helpnodename
	EITEM	EP_AttnFlags
	EITEM	EP_EagleBase
	EITEM	EP_Check7		;for Formatloader (DTP_Check1) a0=Formattags
	EITEM	EP_Check8		;for Formatloader (DTP_Check2) a0=Formattags
	EITEM	EP_SetPlayFrequency
	EITEM	EP_SamplePlayer

*********** end of player interface enumeration for EaglePlayer ***********



*------------------------- EaglePlayer EP_Flags --------------------------*
*    if you use EP_Flags, please set all functions your player offers     *

EPF_Songend	EQU	1			;Songend
EPF_Restart	EQU	2			;Player restartable
EPF_Disable	EQU	3			;Player disabled (1=yes)
EPF_NextSong	EQU	4
EPF_PrevSong	EQU	5
EPF_NextPatt	EQU	6
EPF_PrevPatt	EQU	7
EPF_Volume	EQU	8
EPF_Balance	EQU	9
EPF_Voices	EQU	10
EPF_Save	EQU	11
EPF_Analyzer	EQU	12
EPF_ModuleInfo	EQU	13
EPF_SampleInfo	EQU	14
EPF_Packable	EQU	15
EPF_VolVoices	EQU	16
EPF_InternalUPSStructure EQU	17
EPF_RestartSong	EQU	18
EPF_LoadFast	EQU	19
EPF_EPAudioAlloc EQU	20
EPF_VolBalVoi	EQU	21
EPF_CalcDuration EQU	22


EPB_Songend	EQU	1<<EPF_Songend
EPB_Restart	EQU	1<<EPF_Restart
EPB_Disable	EQU	1<<EPF_Disable
EPB_NextSong	EQU	1<<EPF_NextSong
EPB_PrevSong	EQU	1<<EPF_PrevSong
EPB_NextPatt	EQU	1<<EPF_NextPatt
EPB_PrevPatt	EQU	1<<EPF_PrevPatt
EPB_Volume	EQU	1<<EPF_Volume
EPB_Balance	EQU	1<<EPF_Balance	
EPB_Voices	EQU	1<<EPF_Voices
EPB_Save	EQU	1<<EPF_Save
EPB_Analyzer	EQU	1<<EPF_Analyzer
EPB_ModuleInfo	EQU	1<<EPF_ModuleInfo
EPB_SampleInfo	EQU	1<<EPF_SampleInfo
EPB_Packable	EQU	1<<EPF_Packable
EPB_VolVoices	EQU	1<<EPF_VolVoices
EPB_InternalUPSStructure EQU 1<<EPF_InternalUPSStructure
EPB_RestartSong	EQU	1<<EPF_RestartSong
EPB_LoadFast	EQU	1<<EPF_LoadFast
EPB_EPAudioAlloc EQU	1<<EPF_EPAudioAlloc
EPB_VolBalVoi	EQU	1<<EPF_VolBalVoi
EPB_CalcDuration EQU	1<<EPF_CalcDuration

*---------------------------- Module-Info Tags -------------------------------*
MI_TagBase	EQU TAG_USER+"MI"

	ENUM	MI_TagBase			; TagBase

	EITEM	MI_SongName			;Pointer to Songname
	EITEM	MI_AuthorName			;Pointer to Authorname
	EITEM	MI_SubSongs			;Subsongs
	EITEM	MI_Pattern			;highest PatternNr
	EITEM	MI_MaxPattern			;Max Pattern (SoundTr. 64)
	EITEM	MI_Length			;Length of Module
	EITEM	MI_MaxLength			;Max Length (SoundTr. 128)
	EITEM	MI_Steps			;Steps (SoundMon)
	EITEM	MI_MaxSteps			;Max Steps
	EITEM	MI_Samples			;Number of used Samples
	EITEM	MI_MaxSamples			;Max Samples (SoundTr 31)
	EITEM	MI_SynthSamples			;Number of used SynthSamples
	EITEM	MI_MaxSynthSamples		;Max SynthSamples
	EITEM	MI_Songsize			;Songsize in Bytes
	EITEM	MI_SamplesSize			;Samplessize in Bytes
	EITEM	MI_ChipSize			;needed Chip-Size in Bytes
	EITEM	MI_OtherSize			;FastRam (MDAT-Files) in Bytes
	EITEM	MI_Calcsize			;calculated length of module in Bytes
	EITEM	MI_SpecialInfo			;Pointer to general InfoText 
						;Null Terminated String, may
						;countain linefeeds
	EITEM	MI_LoadSize			;Loadsize (TFMX,Startrekker) in Bytes
	EITEM	MI_Unpacked			;Unpacked Size (e.g. Noise-
						;Packer) in Bytes
	EITEM	MI_UnPackedSystem		;MIUS (see below) or APTR
						;to Soundsysname
	EITEM	MI_Prefix			;Save-Prefix (e.g. "MOD." or
						;"Mdat.")
	EITEM	MI_About			;TI_Data points on a string
						;telling something about this
						;Soundsystem, 
						;Null Terminated String, may
						;countain linefeeds
	EITEM	MI_MaxSubSongs			;Max SubSongs
	EITEM	MI_Voices			;used Voices
	EITEM	MI_MaxVoices			;Max Voices
	EITEM	MI_UnPackedSongSize		;for converted modules
	EITEM	MI_Duration			;Duration
	EITEM	MI_Soundsystem			;Soundsystem Name
						;(for multiple players)
	EITEM	MI_PlayFrequency		;Mixingfrequency
	EITEM	MI_Volumeboost			;Volume Boost
	EITEM	MI_Playmode			;Playing Mode
						;(mono,stereo,14 bit etc.
						;as STRING !!)
	EITEM	MI_ExtraInfo			;One More Field for Special
						;informations,TI_Data points
						;onto a string telling 
						;something more or less useful
						;Null Terminated String, may
						;countain linefeeds
	EITEM	MI_InfoFlags			;Flags

*--------- Unpacked Soundsystems (intern), more follow --------------------*
MIUS_OldSoundTracker	EQU	1
MIUS_SoundTracker	EQU	2
MIUS_NoiseTracker	EQU	3
MIUS_ProTracker		EQU	4
MIUS_SizeOF		EQU	4


*------------------------------ MI_Flags ----------------------------------*
MIF_ReplayinModule	EQU	1

MIB_ReplayinModule	EQU	1<<MIF_ReplayinModule



*------------------------------ Sample-Info Tags --------------------------*

 STRUCTURE EP_Sampletable,0

	APTR	EPS_NextSample			; Next SampleTag-Structure
	APTR	EPS_SampleName			; Name of Sample
	ULONG	EPS_Adr				; Adr of sample
	ULONG	EPS_Length			; Length of sample
	ULONG	EPS_Repeat			; Repeat
	ULONG	EPS_Replen			; Replen
	ULONG	EPS_Period			; default Sampleperiod
	ULONG	EPS_Volume			; default Volume
	UWORD	EPS_Finetune			; Finetune
	UWORD	EPS_Type			; SampleType
	UWORD	EPS_Flags			; Flags for this Samplestructure
	UWORD	EPS_MaxNameLen			; max Namelength

	LABEL	EPS_SizeOF			; to be extended !!!!!!



*------------------------------- Sample types ------------------------------*
USITY_RAW		EQU	0
USITY_IFF		EQU	1
USITY_FMSynth		EQU	2
USITY_AMSynth		EQU	3
USITY_Unknown		EQU	4

*--------------- Sample-Flags for one SampleInfo-TagStructure --------------*
USIF_Playable		EQU	0
USIF_Saveable		EQU	1
USIF_8Bit		EQU	2
USIF_16Bit		EQU	3
USIF_Interleaved	EQU	4
USIF_Intel		EQU	5
USIF_Unsigned		EQU	6

USIB_Playable		EQU	1<<USIF_Playable
USIB_Saveable		EQU	1<<USIF_Saveable
USIB_8BIT		EQU	1<<USIF_8Bit
USIB_16BIT		EQU	1<<USIF_16Bit
USIB_Interleaved	EQU	1<<USIF_Interleaved
USIB_Intel		EQU	1<<USIF_Intel
USIB_Unsigned		EQU	1<<USIF_Unsigned




*-------------------------- Eagleplayer-PatternInfo ------------------------*

 STRUCTURE EP_Patterninfo,0
	UWORD	PI_NumPatts		;Overall Number of Patterns
	UWORD	PI_Pattern		;Current Pattern (from 0)
	UWORD	PI_Pattpos		;Current Position in Pattern (from 0)
	UWORD	PI_Songpos		;Position in Song (from 0)
	UWORD	PI_MaxSongPos		;Songlengh
	UWORD	PI_BPM			;Beats per Minute
	UWORD	PI_Speed		;Speed
	UWORD	PI_Pattlength		;Length of Actual Pattern in Rows
	UWORD	PI_Voices		;Number of Voices (Patternstripes)
	ULONG	PI_Modulo		;Range from one note to the next
	APTR	PI_Convert		;Converts Note (a0)
					;to Period (D0),Samplenumber (D1),
					;Commandstring (D2) and Argument (D3)
	LABEL	PI_Stripes		;Address of first Patternstripe,
					;followed by the next one etc. of
					;current pattern


*--------- User-Program-Structure (every player should be use this) ---------*
*--------------- no Taglist used for higher access speed --------------------*
* how to use:                                                                *
* the player sets everytime it accesses the audio-hardware these following   *
* values ; the UPS_VoiceXPer is most important, is it 0 means it that no new *
* note was played, is it different from 0 , all current Userprograms use this* 
* as a new note ; the minimum to see something in the analyzers is period in *
* connection with Volume (should be easy), additionally Address and Length   *
* are recommended for the Quadra/Stereo/MonoScope ; don`t forget to set the  *
* flags, see below                                                           *
*----------------------------------------------------------------------------*
* Attention: Please, dont't use this table in the future !!!                 *
*----------------------------------------------------------------------------*

 STRUCTURE UPS_USER,0
						;for Voice 1
	APTR	UPS_Voice1Adr			;Adr of Samplestart
	UWORD	UPS_Voice1Len			;Size of Sample
	UWORD	UPS_Voice1Per			;Periode of Sample
	UWORD	UPS_Voice1Vol			;Volume of Sample
	UWORD	UPS_Voice1Note			;Note (unused)
	UWORD	UPS_Voice1SampleNr		;SampleNr
	UWORD	UPS_Voice1SampleType		;SampleType
	UWORD	UPS_Voice1Repeat		;Repeat on

	LABEL	UPS_Modulo			;MemSize for one Voice

						;for Voice2
	APTR	UPS_Voice2Adr			;Adr of Samplestart
	UWORD	UPS_Voice2Len			;Length of Sample
	UWORD	UPS_Voice2Per			;Periode of Sample
	UWORD	UPS_Voice2Vol			;Volume of Sample
	UWORD	UPS_Voice2Note			;Note (unused)
	UWORD	UPS_Voice2SampleNr		;SampleNr
	UWORD	UPS_Voice2SampleType		;SampleType
	UWORD	UPS_Voice2Repeat		;Repeat on

						;for Voice3
	APTR	UPS_Voice3Adr			;Adr of Samplestart
	UWORD	UPS_Voice3Len			;Length of Sample
	UWORD	UPS_Voice3Per			;Periode of Sample
	UWORD	UPS_Voice3Vol			;Volume of Sample
	UWORD	UPS_Voice3Note			;Note (unused)
	UWORD	UPS_Voice3SampleNr		;SampleNr
	UWORD	UPS_Voice3SampleType		;SampleType
	UWORD	UPS_Voice3Repeat		;Repeat on

						;for Voice4
	APTR	UPS_Voice4Adr			;Adr of Samplestart
	UWORD	UPS_Voice4Len			;Length of Sample
	UWORD	UPS_Voice4Per			;Periode of Sample
	UWORD	UPS_Voice4Vol			;Volume of Sample
	UWORD	UPS_Voice4Note			;Note (unused)
	UWORD	UPS_Voice4SampleNr		;SampleNr
	UWORD	UPS_Voice4SampleType		;SampleType
	UWORD	UPS_Voice4Repeat		;Repeat on

	UWORD	UPS_DMACon			;Voices on/off 15=all on
						;Name irritating, better was
						;VoicesStaus, Use like the
						;real Dmacon, Bit x set,
						;Voice x on

	UWORD	UPS_Flags			;Flags , see below
	UWORD	UPS_Enabled			;Zero = Read enabled
	UWORD	UPS_Reserved			;unused

	LABEL	UPS_SizeOF

*--------------------------- Flags to UPS_Flags ---------------------------*
* these Flags tell, which analyzerfunctions the actual player is able to do*

UPSFL_Adr	EQU	0		;submits Sampleaddress
UPSFL_Len	EQU	1		;submits Samplesize
UPSFL_Per	EQU	2		;submits Period (important !)
UPSFL_Vol	EQU	3		;submits Volume
UPSFL_Note	EQU	4		;corrently not used
UPSFL_SNr	EQU	5		;corrently not used
UPSFL_STy	EQU	6		;corrently not used
UPSFL_DMACon	EQU	7		;submits which Voices are on/off
UPSFL_Res	EQU	8

UPSB_Adr	EQU	1<<UPSFL_Adr
UPSB_Len	EQU	1<<UPSFL_Len
UPSB_Per	EQU	1<<UPSFL_Per
UPSB_Vol	EQU	1<<UPSFL_Vol
UPSB_Note	EQU	1<<UPSFL_Note
UPSB_SNr	EQU	1<<UPSFL_SNr
UPSB_STy	EQU	1<<UPSFL_STy
UPSB_DMACon	EQU	1<<UPSFL_DMACon
UPSB_Res	EQU	1<<UPSFL_Res




*-------- Errornumbers for Tags with Error-Returns, for UserPrograms -------*
*-------- & for Subprograms (e.g. EPG_NewLoadFile)		     -------*
EPR_UnknownFormat	EQU	1
EPR_FileNotFound	EQU	2
EPR_ErrorInFile		EQU	3
EPR_NotEnoughMem	EQU	4
EPR_CorruptModule	EQU	5
EPR_ErrorInstallPlayer	EQU	6
EPR_EagleRunning	EQU	7
EPR_CantAllocCia	EQU	8
EPR_CantAllocAudio	EQU	9
EPR_CantFindReq		EQU	10
EPR_NoModuleLoaded	EQU	11
EPR_ErrorInString	EQU	12
EPR_CantAllocSerial	EQU	13
EPR_ErrorDecrunching	EQU	14
EPR_ErrorExtLoad	EQU	15
EPR_ErrorAddPlayer	EQU	16
EPR_SaveError		EQU	17
EPR_LoadError		EQU	18
EPR_CantOpenWin		EQU	19
EPR_PlayerExists	EQU	20
EPR_WriteError		EQU	21
EPR_XPKError		EQU	22
EPR_XPKMasterNotFound	EQU	23
EPR_PPNotFound		EQU	24
EPR_LHNotFound		EQU	25
EPR_ErrorAddUserPrg	EQU	26
EPR_NoMoreUserPrgs	EQU	27
EPR_ModuleTooShort	EQU	28
EPR_CantDeletingPlayer	EQU	29
EPR_ErrorLoadingInstruments EQU	30
EPR_NoMoreEntries	EQU	31
EPR_ErrorLoadingDir	EQU	32
EPR_DirIsEmpty		EQU	33
EPR_BufferFull		EQU	34
EPR_UnknownError	EQU	35
EPR_FunctionNotEnabled	EQU	36
EPR_Passwordfailed	EQU	37
EPR_CommandError	EQU	38
EPR_ErrorInArguments	EQU	39
EPR_NotImplemented	EQU	40
EPR_FileIsNotExecutable	EQU	41
EPR_NotEnoughArguments	EQU	42
EPR_Functionaborted	EQU	43
EPR_InvalidNr		EQU	44
EPR_FileReqCancelled	EQU	45
EPR_ErrorDeletingFile	EQU	46
EPR_ErrorLoadingFont	EQU	47
EPR_NeedHigherKickstart	EQU	48
EPR_CrunchAborted	EQU	49
EPR_DirNotChanged	EQU	50
EPR_NoHelpFile		EQU	51
EPR_CrmNotFound		EQU	52
EPR_CantFindReqTools	EQU	53
EPR_CantFindAsl		EQU	54
EPR_ExtractorNotFound	EQU	55
EPR_UserprogramNotFound	EQU	56
EPR_ErrorInArchive	EQU	57
EPR_UnknownArchive	EQU	58
EPR_Archivescanned	EQU	59
EPR_NoAmplifier		EQU	60
EPR_AmplifierInitError	EQU	61
EPR_CantAddGadget	EQU	62
EPR_FatalError		EQU	63
EPR_LibraryNotFound	EQU	64
EPR_NeedhigherCPUFPU	EQU	65
EPR_NeednewerEP		EQU	66
EPR_EngineNotLoaded	EQU	67

EPR_LastError		EQU	67

EPR_ErrorAddEngine	EQU	EPR_ErrorAddUserPrg
EPR_NoMoreEngines	EQU	EPR_NoMoreUserPrgs
EPR_ErrorLoadEngine	EQU	EPR_ErrorAddUserPrg
EPR_EngineNotFound	EQU	EPR_UserprogramNotFound



*------------------------ Check-Module-Errornumbers ------------------------*
EPRS_LengthTooLong	EQU	1
EPRS_ReplayerCorrupt	EQU	2
EPRS_PeriodTableDestroyed EQU	3
EPRS_SamplesTooSmall	EQU	4
EPRS_ModDatasTooSmall	EQU	5
EPRS_CorruptSamples	EQU	6
EPRS_InkorrectSpeed	EQU	7
EPRS_InkorrectTiming	EQU	8
EPRS_TooManySubSongs	EQU	9
EPRS_NotImplmentedCommand	EQU	10
EPRS_FilesNotFound	EQU	11
EPRS_AdressError	EQU	12
EPRS_WrongArgs		EQU	13

*------------------------------ Global Variables ----------------------------*
 STRUCTURE EaglePlayerGlobals,0

	STRUCT	OLDDeliTrackerGlobals,dtg_NotePlayer
	FPTR	EPG_FTPRReserved1
	FPTR	EPG_FTPRReserved2
	FPTR	EPG_FTPRReserved3
	FPTR	EPG_SaveMem		;Save Mem to Disk
	FPTR	EPG_FileRequest		;FileRequester
	FPTR	EPG_TextRequest		;Own Textrequester
	FPTR	EPG_LoadExecutable	;Load % Decrunch
	FPTR	EPG_NewLoadFile		;new DTG_LoadFile with Parameters
	FPTR	EPG_ScrollText		;Scroll Text
	FPTR	EPG_LoadPlConfig	;Loads a Config from Env:Eagleplayer/..
	FPTR	EPG_SavePlConfig	;Saves a Config to EnvArc:Eagleplayer/..
	FPTR	EPG_FindTag		;Finds a Tag in Tagliste
	FPTR	EPG_FindAuthor		;Find Authorname for Soundtracker-
					;compatibles
	FPTR	EPG_Hexdez		;Convert Dual to Dezimal (Ascii)
	FPTR	EPG_TypeText		;Print Text into the Mainwindow
	FPTR	EPG_ModuleChange	;Change Playroutine in Module
	FPTR	EPG_ModuleRestore	;Restore Playroutine in Module

	FPTR	EPG_FTPRReserved8	;dont't use !!!

	APTR	EPG_XPKBase		;librarybase (don`t close) or zero
	APTR	EPG_LHBase		;librarybase (don`t close) or zero
	APTR	EPG_PPBase		;librarybase (don`t close) or zero
	APTR	EPG_DiskFontBase	;librarybase (don`t close) or zero
	APTR	EPG_ReqToolsBase	;librarybase (don`t close) or zero
	APTR	EPG_ReqBase		;librarybase (don`t close) or zero
	APTR	EPG_XFDMasterBase	;librarybase (don`t close) or zero
	APTR	EPG_WorkBenchBase	;librarybase (don`t close) or zero
	APTR	EPG_RexxSysBase		;librarybase (don`t close) or zero
	APTR	EPG_CommoditiesBase	;librarybase (don`t close) or zero
	APTR	EPG_IconBase		;librarybase (don`t close) or zero
	APTR	EPG_LocaleBase		;librarybase (don`t close) or zero

	APTR	EPG_WinHandle		;MainWindow or zero (don`t close)
	APTR	EPG_TitlePuffer		;Puffer for Screentitlename
	APTR	EPG_SoundSystemname	;Ptr to Soundsystemname
	APTR	EPG_Songname		;songname or filename
	APTR	EPG_Reserved2		;zero (don`t use)
	APTR	EPG_Reserved3		;zero (don`t use)

	APTR	EPG_PubScreen		;Pointer to PubScreenname or zero
	APTR	EPG_CiaBBase		;Ciab.Resource-Base or zero
	APTR	EPG_UPS_Structure	;Private UPS_Structure, don't change
	APTR	EPG_ModuleInfoTagList	;Pointer to ModuleInfo-TagList
	APTR	EPG_Author

	LONG	EPG_Identifier		;Longword="EPGL" --> Eagleplayer used
	LONG	EPG_EagleVersion
	WORD	EPG_Speed

	LONG	EPG_ARGN			;Anz. of Parameters

	LONG	EPG_ARG1		;\
	LONG	EPG_ARG2		; \
	LONG	EPG_ARG3		;  \	Parameter-Buffer for SubProggys
	LONG	EPG_ARG4		;   \_\ You must use this. You must set
	LONG	EPG_ARG5		;   / /	ArgN (max. used Parameters)
	LONG	EPG_ARG6		;  /	--> New SubProggys can use
	LONG	EPG_ARG7		; /	more Parameters !!!!
	LONG	EPG_ARG8		;/ don`t use in interrupts !!!!

	UWORD	EPG_Voices

	UWORD	EPG_Voice1Vol		;0-64
	UWORD	EPG_Voice2Vol
	UWORD	EPG_Voice3Vol
	UWORD	EPG_Voice4Vol

	APTR	EPG_VoiceTable		;Voicetable (only for Amplifier) 0-255
	UWORD	EPG_VoiceTableEntries
	UWORD	EPG_Unused1

	ULONG	EPG_SomePrefs		;Bits you find above the Globals
	ULONG	EPG_Timeout		;Timeout in Secs ; 0 = No Timeout

	WORD	EPG_FirstSnd	;first Subsong Nr. (-1 for unknown)
	WORD	EPG_SubSongs	;Subsong Range (-1 for unknown)
	ULONG	EPG_MODNr	;Actual Module in List (Nr) ; 0 = No Mod.
	ULONG	EPG_MODS	;Number of Modules in List (Nr); 0 = No List

	APTR	EPG_PlayerTagList	;Pointer to actual PlayerTaglist
	APTR	EPG_TextFont		;Pointer to actual Font-Structure

	UWORD	EPG_Volume		;Volumerange 0-255
	UWORD	EPG_Balance		;Balance
	UWORD	EPG_LeftBalance		;Balancerange 0-255
	UWORD	EPG_RightBalance	;Balancerange 0-255
	UWORD	EPG_UnUsed6
	UWORD	EPG_UnUsed7
	UWORD	EPG_UnUsed8
	UWORD	EPG_UnUsed9

	WORD	EPG_DefTimer
	WORD	EPG_CurrentPosition
	WORD	EPG_WORDReserved3
	WORD	EPG_WORDReserved4
	WORD	EPG_WORDReserved5
	WORD	EPG_WORDReserved6
	WORD	EPG_WORDReserved7
	WORD	EPG_WORDReserved8

	LONG	EPG_Dirs
	LONG	EPG_LoadedFiles		;loaded files by DTG/EPG_LoadFile
	APTR	EPG_AppPort
	APTR	EPG_SampleInfoStructure	;call first EPNr_Sampleinit !!!!

	LONG	EPG_MinTimeOut
	LONG	EPG_CurrentTime		;Current Playtime
	LONG	EPG_Duration		;Duration
	LONG	EPG_FirstUserStruct
	LONG	EPG_FirstFileStruct
	LONG	EPG_Entries
	LONG	EPG_Modulesize
	APTR	EPG_Playerlist
	APTR	EPG_Enginelist
	APTR	EPG_Moduleslist


	APTR	EPG_AmplifierList	;Wie Enginelist
	APTR	EPG_ActiveAmplifier	;EUS_StartAdr
	APTR	EPG_AudioStruct		;aktuelle AudioStruct (EP privat)
	APTR	EPG_AmplifierTagList
	APTR	EPG_ConfigDirArrayPtr
	APTR	EPG_PlayerDirArrayPtr	;Future use
	APTR	EPG_EngineDirArrayPtr
	APTR	EPG_FirstPlayerStruct
	APTR	EPG_ChkSegment
	APTR	EPG_EagleplayerDirArrayPtr


	LABEL	EPG_SizeOf		;to be extended


	*------------- Eagleplayer Globals Preferences-Flags ------------*
EGPRF_Unused		EQU	0			;FadeIn V1.0-V1.54ß
EGPRF_Fadeout		EQU	1
EGPRF_Songend		EQU	2
EGPRF_Loadnext		EQU	3
EGPRF_Randommodule	EQU	4
EGPRF_Mastervolume	EQU	5
EGPRF_NowPlay		EQU	6
EGPRF_Surfacequit	EQU	7
EGPRF_LoadPrev		EQU	8
EGPRF_PausePlay		EQU	9
EGPRF_LoadFast		EQU	10
EGPRF_CalcDuration	EQU	11
EGPRF_AllocChannels	EQU	12
EGPRF_SoftInt		EQU	13
EGPRF_Iconify		EQU	14
EGPRF_Help		EQU	15
EGPRF_Fadein		EQU	16

EGPRB_UnUsed		EQU	1<<EGPRF_Unused		;FadeIn V1.0-V1.54ß
EGPRB_Fadeout		EQU	1<<EGPRF_Fadeout
EGPRB_Songend		EQU	1<<EGPRF_Songend
EGPRB_Loadnext		EQU	1<<EGPRF_Loadnext
EGPRB_Randommodule	EQU	1<<EGPRF_Randommodule
EGPRB_Mastervolume	EQU	1<<EGPRF_Mastervolume
EGPRB_NowPlay		EQU	1<<EGPRF_NowPlay
EGPRB_Surfacequit	EQU	1<<EGPRF_Surfacequit
EGPRB_LoadPrev		EQU	1<<EGPRF_LoadPrev
EGPRB_PausePlay		EQU	1<<EGPRF_PausePlay
EGPRB_LoadFast		EQU	1<<EGPRF_LoadFast
EGPRB_CalcDuration	EQU	1<<EGPRF_CalcDuration
EGPRB_AllocChannels	EQU	1<<EGPRF_AllocChannels
EGPRB_SoftInt		EQU	1<<EGPRF_SoftInt
EGPRB_Iconify		EQU	1<<EGPRF_Iconify
EGPRB_Help		EQU	1<<EGPRF_Help
EGPRB_Fadein		EQU	1<<EGPRF_Fadein



*----------------------- Extended GlobalJumps -----------------------------*
*-- This structure is the negative buffer relative to a5 (Basisregister) --*
*--------------------------------------------------------------------------*

 STRUCTURE NEWEP_GLOBALS,-6
		STRUCT	ENPP_AllocSampleStruct,-6
		STRUCT	ENPP_NewLoadFile2,-6	;You must Freemem it !!!
		STRUCT	ENPP_MakeDirCorrect,-6
		STRUCT	ENPP_TestAufHide,-6
		STRUCT	ENPP_ClearCache,-6
		STRUCT	ENPP_CopyMemQuick,-6
		STRUCT	ENPP_GetPassword,-6
		STRUCT	ENPP_StringCopy2,-6
		STRUCT	ENPP_ScreenToFront,-6
		STRUCT	ENPP_WindowToFront,-6

	*--------- old DeliTracker-Globals ---------*
		STRUCT	ENPP_GetListData,-6
		STRUCT	ENPP_LoadFile,-6
		STRUCT	ENPP_CopyDir,-6		;\  new! not 100% kompatibel
		STRUCT	ENPP_CopyFile,-6	; > to dtg_copyxxx
		STRUCT	ENPP_CopyString,-6	;/
		STRUCT	ENPP_AllocAudio,-6
		STRUCT	ENPP_FreeAudio,-6
		STRUCT	ENPP_StartInterrupt,-6
		STRUCT	ENPP_StopInterrupt,-6
		STRUCT	ENPP_SongEnd,-6
		STRUCT	ENPP_CutSuffix,-6
		STRUCT	ENPP_SetTimer,-6
		STRUCT	ENPP_WaitAudioDMA,-6

	*--------- old EaglePlayer-Globals --------*
		STRUCT	ENPP_SaveMem,-6
		STRUCT	ENPP_FileReq,-6
		STRUCT	ENPP_TextRequest,-6
		STRUCT	ENPP_LoadExecutable,-6
		STRUCT	ENPP_NewLoadFile,-6
		STRUCT	ENPP_ScrollText,-6
		STRUCT	ENPP_LoadPlConfig,-6
		STRUCT	ENPP_SavePlConfig,-6
		STRUCT	ENPP_FindTag,-6
		STRUCT	ENPP_FindAuthor,-6

		STRUCT	ENPP_Hexdez,-6
		STRUCT	ENPP_TypeText,-6
		STRUCT	ENPP_ModuleChange,-6
		STRUCT	ENPP_ModuleRestore,-6

		STRUCT	ENPP_StringCopy,-6
		STRUCT	ENPP_CalcStringSize,-6
		STRUCT	ENPP_StringCMP,-6


		STRUCT	ENPP_DMAMask,-6
		STRUCT	ENPP_PokeAdr,-6
		STRUCT	ENPP_PokeLen,-6
		STRUCT	ENPP_PokePer,-6
		STRUCT	ENPP_PokeVol,-6
		STRUCT	ENPP_PokeCommand,-6
		STRUCT	ENPP_Amplifier,-6

		STRUCT	ENPP_TestAbortGadget,-6
		STRUCT	ENPP_GetEPNrfromMessage,-6	;In:a1=Message

		STRUCT	ENPP_InitDisplay,-6		;a0=Text a1=Args
		STRUCT	ENPP_FillDisplay,-6		;-++- d0=per thousand
		STRUCT	ENPP_RemoveDisplay,-6

		STRUCT	ENPP_GetLocaleString,-6

		STRUCT	ENPP_SetWaitPointer,-6
		STRUCT	ENPP_ClearWaitPointer,-6

		STRUCT	ENPP_OpenCatalog,-6
		STRUCT	ENPP_CloseCatalog,-6

		STRUCT	ENPP_AllocAmigaAudio,-6		*allocate original Amigaaudio (for Amplifiers)
		STRUCT	ENPP_FreeAmigaAudio,-6

		STRUCT	ENPP_RawToFormat,-6
		STRUCT	ENPP_FindAmplifier,-6
		STRUCT	ENPP_UserCallup5,-6
		STRUCT	ENPP_GetLoadListData,-6	;(for ExtLoad)
		STRUCT	ENPP_SetListData,-6	;only for rippers !!!
		LABEL	ENPP_SizeOf		;to be extended !!!


EPNPC_Filter	EQU	1			;d0=Command d1=0-> bclr
						;	    d1=1-> bset




*-------------- Eagleplayer GadgetStrukture for EPP_Textrequest --------------*
* STRUCTURE EPGG_Gadget,0
*		APTR	EPGG_NextGadget
*		APTR	EPGG_GadgetName
*		UBYTE	EPGG_Keys1
*		UBYTE	EPGG_Keys2
*		UBYTE	EPGG_Keys3
*		UBYTE	EPGG_Keys4

*------------ Eagleplayer new GadgetStrukture for EPP_Textrequest ------------*
* STRUCTURE EPNG_Gadget,0			;neu für Locale
*		APTR	EPNG_NextGadget
*		UWORD	EPNG_LocaleNr
*		UBYTE	EPNG_Keys1		;\ should be a Startcase
*		UBYTE	EPNG_Keys2		;/
*		UBYTE	EPNG_Keys3		;\ should be Esc or ENTER
*		UBYTE	EPNG_Keys4		;/
*	;if in the Localestring exists a _ then the EPGN_Key1 and 2
*	;will set to lower an higher case of this
*	;Example	L_oad  	EPNG_Keys1=L EPNG_Keys2=l
*	

*----------------------------- Player header --------------------------------*

EPPHEADER	MACRO
	moveq	#-1,d0			; prevent crashes if someone
	rts				; tries to call the player from shell
	dc.b	'EPPLAYER'		; identifier as "Eagleplayer Player"
	dc.l	\1			; Pointer to TagItem Array
	ENDM


*----------------------------- Ron Klaren Header -----------------------------*
KLARENHEADER MACRO
	moveq	#-1,d0				; prevent crashes if someone
	rts					; tries to call the module from shell
						; 
	dc.l	\1				; Size of Ron Klaren Module,im-
						; portant !!!
	dc.b	"RON_KLAREN_SOUNDMODULE!",0	; identifier
	ENDM


	ENDC	; EAGLEPLAYER_I

