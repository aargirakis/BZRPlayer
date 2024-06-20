#include "test.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#undef TEST
#include "../src/player.h"
#include "../src/mixer.h"
#include "../src/virtual.h"

void compare_mixer_data(char *mod, char *data)
{
	xmp_context opaque;
	struct context_data *ctx;
	struct module_data *m;
        struct player_data *p;
        struct mixer_voice *vi;
	struct xmp_frame_info fi;
	int time, row, frame, chan, period, note, ins, vol, pan, pos0, cutoff;
	char line[200];
	FILE *f;
	int i, voc;

	f = fopen(data, "r");

	opaque = xmp_create_context();
	xmp_load_module(opaque, mod);

	ctx = (struct context_data *)opaque;
	m = &ctx->m;
	p = &ctx->p;

	xmp_start_player(opaque, 44100, 0);
	xmp_set_player(opaque, XMP_PLAYER_MIX, 100);

	while (1) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &fi);
		if (fi.loop_count > 0)
			break;

		for (i = 0; i < m->mod.chn; i++) {
			struct xmp_channel_info *ci = &fi.channel_info[i];
			struct channel_data *xc = &p->xc_data[i];
			int num;

			voc = map_channel(p, i);
			if (voc < 0 || TEST_NOTE(NOTE_SAMPLE_END))
				continue;

			vi = &p->virt.voice_array[voc];

			fgets(line, 200, f);
			num = sscanf(line, "%d %d %d %d %d %d %d %d %d %d %d",
				&time, &row, &frame, &chan, &period,
				&note, &ins, &vol, &pan, &pos0, &cutoff);

			fail_unless(fi.time    == time,   "time mismatch");
			fail_unless(fi.row     == row,    "row mismatch");
			fail_unless(fi.frame   == frame,  "frame mismatch");
			fail_unless(ci->period == period, "period mismatch");
			fail_unless(vi->note   == note,   "note mismatch");
			fail_unless(vi->ins    == ins,    "instrument");
			fail_unless(vi->vol    == vol,    "volume mismatch");
			fail_unless(vi->pan    == pan,    "pan mismatch");
			fail_unless(vi->pos0   == pos0,   "position mismatch");
			if (num >= 11) {
				fail_unless(vi->filter.cutoff == cutoff,
							  "cutoff mismatch");
			}
		}
		
	}

	fgets(line, 200, f);
	fail_unless(feof(f), "not end of data file");

	xmp_end_player(opaque);
	xmp_release_module(opaque);
	xmp_free_context(opaque);
}
