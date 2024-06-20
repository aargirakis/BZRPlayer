#ifndef TN_SC68_H
#define TN_SC68_H

#include <exec/exec.h>
#include <dos/dos.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>

#include <libraries/tnplug.h>
#include <proto/tnplug.h>

#define AUDIO_BUFFER_SIZE (4 << 10)

struct TNPluginBase;
struct TNDecHandle;

#include "api68/api68.h"
#include "debug.h"

struct TNPlugBase {
	struct Library tnpb_LibNode;
	BPTR tnpb_SegList;
};

typedef struct {
	int32 (*open)(struct TNDecHandle *handle, char *openme);
	void (*close)(struct TNDecHandle *handle);
	int32 (*process)(struct TNDecHandle *handle, void *buf, uint32 len);
	void (*seek)(struct TNDecHandle *handle, uint32 seconds);
} sc68iface_t;

struct TNDecHandle {
	sc68iface_t *isc68;
	BPTR sc68bin;
	struct Interface *newlib;

	api68_t *sc68;
	api68_music_info_t info;
	int code;
};

/* announceplayer.c */
struct audio_player *TNPlug_AnnouncePlayer (struct TNPlugIFace *Self, uint32 version);

/* initplayer.c */
BOOL TNPlug_InitPlayer (struct TNPlugIFace *Self, struct TuneNet *inTuneNet, uint32 playmode);
void TNPlug_ExitPlayer (struct TNPlugIFace *Self, struct audio_player *aplayer);

/* openplayer.c */
int32 TNPlug_TestPlayer (struct TNPlugIFace *Self, uint8 *testme, uint8 *testbuffer,
	uint32 totalsize, uint32 testsize);
BOOL TNPlug_OpenPlayer (struct TNPlugIFace *Self, uint8 *openme, struct TuneNet *inTuneNet);
void TNPlug_ClosePlayer (struct TNPlugIFace *Self, struct TuneNet *inTuneNet);

/* decodeframeplayer.c */
int32 TNPlug_DecodeFramePlayer (struct TNPlugIFace *Self, struct TuneNet *inTuneNet,
	int16 *outbuffer);
int32 TNPlug_SeekPlayer (struct TNPlugIFace *Self, struct TuneNet *inTuneNet, uint32 seconds);

/* donotify.c */
uint32 TNPlug_DoNotify (struct TNPlugIFace *Self, struct TuneNet *inTuneNet, uint32 item,
	uint32 value);

#endif
