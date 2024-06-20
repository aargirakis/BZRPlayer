#ifndef lib_TNPlugCommon
#define lib_TNPlugCommon

/* TuneNet API Version 1.02 */
#define	PLUGIN_API_VERSION	10200						// Version 1.02

/* Defines for PlayModes */
#define	PLM_File				1								// ** Standard File **
#define	PLM_FileLoad		2								// ** Load in all at once **
#define	PLM_Stream			4								// ** Char buffer to a http stream **
#define	PLM_ContainerFile	8								// ** Information stored in a container file ** (PLS)
#define	PLM_Container		16								// ** Information stored in a buffer **

/* Player bit flags */
#define aPlayer_SAVE_stream	1
#define aPlayer_SAVE_convert	2
#define aPlayer_SEEK				4

/* Render Modes */
#define	ARender_Direct				1						// ** DO NOT USE! **
#define	ARender_InternalBuffer	2						// ** PCM allocated by the player **

/* Notify Events */
#define TNDN_SubSong		500								// ** Sets a new sub song to play **
#define TNDN_PlaySpeed	501

/* Left / Right Mix (Please stick to just these values, future compatibility cannot be gaurenteed otherwise) */
#define	TN_STEREO						0
#define	TN_STEREO_25PER				12
#define	TN_STEREO_50PER				25
#define	TN_STEREO_75PER				38
#define	TN_MONO							50
#define	TN_REVERSE_STEREO_25PER		62
#define	TN_REVERSE_STEREO_50PER		75
#define	TN_REVERSE_STEREO_75PER		88
#define	TN_REVERSE_STEREO				100


/* The TuneNet structure is allocated by TuneNet and passed to the Player.

	Please take notes of the values you are allowed to change.
*/

struct TuneNet  	//MP3StreamData
{
	/* Error messages reported from Player in Real Time: *WRITE* by Plugin Player Only (TuneNet will Read) */
	volatile	int	lastmessage;					// ** Last Error reported by Stream Read **
	volatile	int	exterror;						// ** Error II (eg: ICY errors) **

	/* Preload Space (READ ONLY FROM PLUGIN PLAYERS) */
	BYTE		* in_file;								// ** File In (If preload) **
	ULONG		in_file_size;							// ** Size of File **
	ULONG		in_file_offset;						// ** Offset **
	
	int		playmode;								// ** READ ONLY - Contains Current Playmode ** 
	
	/* Tune Strings (WRITE ONLY FROM PLUGIN PLAYERS) */	
	char		ST_Name[255];							// Station Name - Copy Null termianted 255 byte string here 
	char		ST_Url[255];							// URL - Copy Null terminated 255 byte string here
	char		ST_Genre[255];							// GENRE - Copy Null terminated 255 byte string here
	char		Tune[255];								// Tune Name - Copy Null terminated 255 byte string here
	char		ST_Note[255];							// Notes, also contains any customer error messages **

	/* Audio Details - Set by Players only */
	APTR		handle;									// ** Internal player handle */
	ULONG		DirectRender;							// ** Must be set to Direct Render 2 mode at the moment **
	ULONG		dec_frequency;							// eg: 44100
	ULONG		dec_channels;							// eg: 2
	ULONG		dec_quality;							// ** Depreciated **
	ULONG		ms_duration;							// 0 = no duration, otherwise time given in milliseconds
	ULONG		bitrate;									// kbps rate

	ULONG		versionmajor;							// eg: MP3
	ULONG		versionminor;							// eg: 2 (mp3.2)

	WORD	*	pcm[2];									// ** 16 Bit Audio (2 channels) - MUST BE ALLOCATED BY THE PLUGIN PLAYER **
	
	ULONG		max_subsongs;							// ** Max Songs 0 or 1 = single file, 2+ = multiple sub songs (default = 0)
	ULONG		current_subsong;						// ** Current song starting from 0 (default = 0)
	int		mix_lr;									// ** Suggested Initial Mix LR (can be overridden by user) (See Left and Right Mix defines above) **
	BOOL		songEOF;									// ** Set to TRUE if priority given to EOF over Timer (1) - doesn't work with X-FADE **	
};

/* Audio Players */
struct audio_player
{
	char	name[127];											// ** Name of player **
	char	author[127];
	char	description[127];
	char	ext[16];												// ** Extentions (.mp3) comma seperated (mp3,mp2) **
	char	*	patternmatch;									// ** Pointer to pattern match string **
	struct Library * 	ap_library;							// ** This Library Pointer   - Leave as NULL
	struct Interface * ap_interface;						// ** This Interface Pointer - Leave as NULL
	BOOL	extANDpmatch;										// ** If not set then one or the other **
	uint8	playmodes;											// ** Modes of play (Or'ed together) **
	uint8 playerflags;										// ** Abilities - SAVE etc..... **
	LONG  plugin_api_version;								// ** V01.02 = 010200
	LONG	plugin_version;									// ** Your Revision in MMmmss (Major, Minor and Sub Format) **
};

#endif
