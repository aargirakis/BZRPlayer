#include "tn_sc68.h"

int32 TNPlug_DecodeFramePlayer (struct TNPlugIFace *Self, struct TuneNet *inTuneNet,
	int16 *outbuffer)
{
	struct TNDecHandle *handle = inTuneNet->handle;
	return handle->isc68->process(handle, inTuneNet->pcm[0], AUDIO_BUFFER_SIZE >> 2);
}

int32 TNPlug_SeekPlayer (struct TNPlugIFace *Self, struct TuneNet *inTuneNet, uint32 seconds) {
	struct TNDecHandle *handle = inTuneNet->handle;
	handle->isc68->seek(handle, seconds);
	return seconds;
}
