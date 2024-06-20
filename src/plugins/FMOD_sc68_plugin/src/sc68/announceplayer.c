#include "tn_sc68.h"
#include "TN_SC68.tnplug_rev.h"

struct audio_player *TNPlug_AnnouncePlayer (struct TNPlugIFace *Self, uint32 version) {
	static struct audio_player my_player = {
		"SC68",
		"Fredrik Wikstrom",
		"SC68 Music Player",
		".sc68,.sndh",
		"#?.(sc68|sndh)",
		NULL,
		NULL,
		FALSE,
		PLM_File,
		aPlayer_SEEK,
		PLUGIN_API_VERSION,
		VERSION*10000+REVISION*100
	};
	struct TNPlugBase *tnpb = (struct TNPlugBase *)Self->Data.LibBase;
	return (version >= 7600) ? &my_player : NULL;
}
