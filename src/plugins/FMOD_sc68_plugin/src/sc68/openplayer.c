#include "tn_sc68.h"
#include <libraries/iffparse.h>
#include <string.h>

#define SC68_MAGIC MAKE_ID('S','C','6','8')
#define SNDH_MAGIC MAKE_ID('S','N','D','H')
#define ICE_MAGIC MAKE_ID('I','C','E','!')

extern struct Interface *INewlib;

int32 TNPlug_TestPlayer (struct TNPlugIFace *Self, uint8 *testme, uint8 *testbuffer,
	uint32 totalsize, uint32 testsize)
{
	int32 res = 0;

	if (totalsize >= 4) {
		uint32 magic = *(uint32 *)testbuffer;
		if (magic == SC68_MAGIC || magic == ICE_MAGIC) {
			res = PLM_File;
		} else
		if (totalsize >= 16) {
			magic = *(uint32 *)&testbuffer[12];
			if (magic == SNDH_MAGIC) {
				res = PLM_File;
			}
		}
	}
	if (!res) {
		char *ext;
		ext = strrchr((char *)testme, '.');
		if (ext) {
			ext++;
			if (!stricmp(ext, "sc68") || !stricmp(ext, "sndh")) {
				res = PLM_File;
			}
		}
	}

	return res;
}

BOOL TNPlug_OpenPlayer (struct TNPlugIFace *Self, uint8 *openme, struct TuneNet *inTuneNet) {
	BOOL res = FALSE;

	if (inTuneNet->playmode == PLM_File) {
		struct TNDecHandle *handle;

		handle = IExec->AllocVec(sizeof(*handle), MEMF_SHARED|MEMF_CLEAR);
		inTuneNet->handle = handle;
		if (!handle) goto out;

		inTuneNet->pcm[0] = IExec->AllocVec(AUDIO_BUFFER_SIZE, MEMF_SHARED);
		if (!inTuneNet->pcm[0]) goto out;
		inTuneNet->pcm[1] = inTuneNet->pcm[0] + 1;

		handle->sc68bin = IDOS->LoadSeg("PROGDIR:plugins/data/TN_SC68.bin");
		if (!handle->sc68bin) goto out;

		handle->isc68 = (sc68iface_t *)IDOS->RunCommand(handle->sc68bin, 4096, "", 0);
		if (!handle->isc68) goto out;

		handle->newlib = INewlib;
		res = handle->isc68->open(handle, (char *)openme);
out:
		if (res) {
			dbug(("success\n"));
			inTuneNet->DirectRender = ARender_InternalBuffer;

			inTuneNet->current_subsong = 0; //handle->info.track-1;
			inTuneNet->max_subsongs = 0; //handle->info.tracks;
			
			inTuneNet->dec_frequency = 44100;
			inTuneNet->dec_channels = 2;
			inTuneNet->bitrate = 0;
			inTuneNet->mix_lr = TN_STEREO_50PER;

			if (handle->info.title) {
				dbug(("title: %s\n", handle->info.title));
				strncpy(inTuneNet->Tune, handle->info.title, sizeof(inTuneNet->Tune));
			} else {
				inTuneNet->Tune[0] = 0;
			}

			inTuneNet->ms_duration = handle->info.start_ms + handle->info.time_ms;

			inTuneNet->ST_Name[0] = 0;
			inTuneNet->ST_Url[0] = 0;
			inTuneNet->ST_Note[0] = 0;

			inTuneNet->songEOF = TRUE;
		} else {
			TNPlug_ClosePlayer(Self, inTuneNet);
		}
	}

	return res;
}

void TNPlug_ClosePlayer (struct TNPlugIFace *Self, struct TuneNet *inTuneNet) {
	struct TNDecHandle *handle = inTuneNet->handle;
	if (handle) {
		if (handle->isc68) handle->isc68->close(handle);
		IDOS->UnLoadSeg(handle->sc68bin);
		IExec->FreeVec(handle);
		inTuneNet->handle = NULL;
	}
	IExec->FreeVec(inTuneNet->pcm[0]);
	inTuneNet->pcm[0] =
	inTuneNet->pcm[1] = NULL;
}
