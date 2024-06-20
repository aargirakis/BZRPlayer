/* Extended Module Player
 * Copyright (C) 1996-2013 Claudio Matsuoka and Hipolito Carraro Jr
 *
 * This file is part of the Extended Module Player and is distributed
 * under the terms of the GNU Lesser General Public License. See COPYING.LIB
 * for more information.
 */

#include "loader.h"
#include "period.h"


static int gtk_test(FILE *, char *, const int);
static int gtk_load (struct module_data *, FILE *, const int);

const struct format_loader gtk_loader = {
	"Graoumf Tracker (GTK)",
	gtk_test,
	gtk_load
};

static int gtk_test(FILE * f, char *t, const int start)
{
	char buf[4];

	if (fread(buf, 1, 4, f) < 4)
		return -1;

	if (memcmp(buf, "GTK", 3) || buf[3] > 4)
		return -1;

	read_title(f, t, 32);

	return 0;
}

static void translate_effects(struct xmp_event *event)
{
	/* Ignore extended effects */
	if (event->fxt == 0x0e || event->fxt == 0x0c) {
		event->fxt = 0;
		event->fxp = 0;
	}

	/* handle high-numbered effects */
	if (event->fxt > 0x0f) {
		switch (event->fxt) {
		case 0x10:	/* arpeggio */
			event->fxt = FX_ARPEGGIO;
			break;
		case 0x15:	/* linear volume slide down */
			event->fxt = FX_VOLSLIDE_DN;
			break;
		case 0x16:	/* linear volume slide up */
			event->fxt = FX_VOLSLIDE_UP;
			break;
		case 0x20:	/* set volume */
			event->fxt = FX_VOLSET;
			break;
		case 0x21:	/* set volume to 0x100 */
			event->fxt = FX_VOLSET;
			event->fxp = 0xff;
			break;
		case 0xa4:	/* fine volume slide up */
			event->fxt = FX_F_VSLIDE;
			if (event->fxp > 0x0f)
				event->fxp = 0x0f;
			event->fxp <<= 4;
			break;
		case 0xa5:	/* fine volume slide down */
			event->fxt = FX_F_VSLIDE;
			if (event->fxp > 0x0f)
				event->fxp = 0x0f;
			break;
		case 0xa8:	/* set number of frames */
			event->fxt = FX_S3M_SPEED;
			break;
		default:
			event->fxt = event->fxp = 0;
		}
	
	}
}

static int gtk_load(struct module_data *m, FILE *f, const int start)
{
	struct xmp_module *mod = &m->mod;
	struct xmp_event *event;
	int i, j, k;
	uint8 buffer[40];
	int rows, bits, c2spd, size;
	int ver, patmax;

	LOAD_INIT();

	fread(buffer, 4, 1, f);
	ver = buffer[3];
	fread(mod->name, 32, 1, f);
	set_type(m, "Graoumf Tracker GTK v%d", ver);
	fseek(f, 160, SEEK_CUR);	/* skip comments */

	mod->ins = read16b(f);
	mod->smp = mod->ins;
	rows = read16b(f);
	mod->chn = read16b(f);
	mod->len = read16b(f);
	mod->rst = read16b(f);
	m->volbase = 0x100;

	MODULE_INFO();

	D_(D_INFO "Instruments    : %d ", mod->ins);

	INSTRUMENT_INIT();
	for (i = 0; i < mod->ins; i++) {
		mod->xxi[i].sub = calloc(sizeof (struct xmp_subinstrument), 1);
		fread(buffer, 28, 1, f);
		copy_adjust(mod->xxi[i].name, buffer, 28);

		if (ver == 1) {
			read32b(f);
			mod->xxs[i].len = read32b(f);
			mod->xxs[i].lps = read32b(f);
			size = read32b(f);
			mod->xxs[i].lpe = mod->xxs[i].lps + size - 1;
			read16b(f);
			read16b(f);
			mod->xxi[i].sub[0].vol = 0xff;
			mod->xxi[i].sub[0].pan = 0x80;
			bits = 1;
			c2spd = 8363;
		} else {
			fseek(f, 14, SEEK_CUR);
			read16b(f);		/* autobal */
			bits = read16b(f);	/* 1 = 8 bits, 2 = 16 bits */
			c2spd = read16b(f);
			c2spd_to_note(c2spd, &mod->xxi[i].sub[0].xpo, &mod->xxi[i].sub[0].fin);
			mod->xxs[i].len = read32b(f);
			mod->xxs[i].lps = read32b(f);
			size = read32b(f);
			mod->xxs[i].lpe = mod->xxs[i].lps + size - 1;
			mod->xxi[i].sub[0].vol = read16b(f);
			read8(f);
			mod->xxi[i].sub[0].fin = read8s(f);
		}

		mod->xxi[i].nsm = !!mod->xxs[i].len;
		mod->xxi[i].sub[0].sid = i;
		mod->xxs[i].flg = size > 2 ? XMP_SAMPLE_LOOP : 0;

		if (bits > 1) {
			mod->xxs[i].flg |= XMP_SAMPLE_16BIT;
			mod->xxs[i].len >>= 1;
			mod->xxs[i].lps >>= 1;
			mod->xxs[i].lpe >>= 1;
		}

		D_(D_INFO "[%2X] %-28.28s  %05x%c%05x %05x %c "
						"V%02x F%+03d %5d", i,
			 	mod->xxi[i].name,
				mod->xxs[i].len,
				bits > 1 ? '+' : ' ',
				mod->xxs[i].lps,
				size,
				mod->xxs[i].flg & XMP_SAMPLE_LOOP ? 'L' : ' ',
				mod->xxi[i].sub[0].vol, mod->xxi[i].sub[0].fin,
				c2spd);
	}

	for (i = 0; i < 256; i++)
		mod->xxo[i] = read16b(f);

	for (patmax = i = 0; i < mod->len; i++) {
		if (mod->xxo[i] > patmax)
			patmax = mod->xxo[i];
	}

	mod->pat = patmax + 1;
	mod->trk = mod->pat * mod->chn;

	PATTERN_INIT();

	/* Read and convert patterns */
	D_(D_INFO "Stored patterns: %d", mod->pat);

	for (i = 0; i < mod->pat; i++) {
		PATTERN_ALLOC(i);
		mod->xxp[i]->rows = rows;
		TRACK_ALLOC(i);

		for (j = 0; j < mod->xxp[i]->rows; j++) {
			for (k = 0; k < mod->chn; k++) {
				event = &EVENT (i, k, j);

				event->note = read8(f);
				if (event->note) {
					event->note += 13;
				}
				event->ins = read8(f);

				event->fxt = read8(f);
				event->fxp = read8(f);
				if (ver >= 4) {
					event->vol = read8(f);
				}

				translate_effects(event);

			}
		}
	}

	/* Read samples */
	D_(D_INFO "Stored samples: %d", mod->smp);

	for (i = 0; i < mod->ins; i++) {
		if (mod->xxs[i].len == 0)
			continue;
		load_sample(m, f, 0, &mod->xxs[mod->xxi[i].sub[0].sid], NULL);
	}

	return 0;
}
