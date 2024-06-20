/* Extended Module Player
 * Copyright (C) 1996-2014 Claudio Matsuoka and Hipolito Carraro Jr
 *
 * This file is part of the Extended Module Player and is distributed
 * under the terms of the GNU Lesser General Public License. See COPYING.LIB
 * for more information.
 */

#include "loader.h"
#include "period.h"

#define MAGIC_PSM_	MAGIC4('P','S','M',0xfe)


static int psm_test (HIO_HANDLE *, char *, const int);
static int psm_load (struct module_data *, HIO_HANDLE *, const int);

const struct format_loader psm_loader = {
	"Protracker Studio",
	psm_test,
	psm_load
};

static int psm_test(HIO_HANDLE *f, char *t, const int start)
{
	if (hio_read32b(f) != MAGIC_PSM_)
		return -1;

	read_title(f, t, 60);

	return 0;
}


/* FIXME: effects translation */

static int psm_load(struct module_data *m, HIO_HANDLE *f, const int start)
{
	struct xmp_module *mod = &m->mod;
	int c, r, i;
	struct xmp_event *event;
	uint8 buf[1024];
	uint32 p_ord, p_chn, p_pat, p_ins;
	uint32 p_smp[64];
	int type, ver, mode;
 
	LOAD_INIT();

	hio_read32b(f);

	hio_read(buf, 1, 60, f);
	strncpy(mod->name, (char *)buf, XMP_NAME_SIZE);

	type = hio_read8(f);	/* song type */
	ver = hio_read8(f);		/* song version */
	mode = hio_read8(f);	/* pattern version */

	if (type & 0x01)	/* song mode not supported */
		return -1;

	set_type(m, "Protracker Studio PSM %d.%02d", MSN(ver), LSN(ver));

	mod->spd = hio_read8(f);
	mod->bpm = hio_read8(f);
	hio_read8(f);		/* master volume */
	hio_read16l(f);		/* song length */
	mod->len = hio_read16l(f);
	mod->pat = hio_read16l(f);
	mod->ins = hio_read16l(f);
	hio_read16l(f);               /* ignore channels to play */
	mod->chn = hio_read16l(f);    /* use channels to proceed */
	mod->smp = mod->ins;
	mod->trk = mod->pat * mod->chn;

	p_ord = hio_read32l(f);
	p_chn = hio_read32l(f);
	p_pat = hio_read32l(f);
	p_ins = hio_read32l(f);

	/* should be this way but fails with Silverball song 6 */
	//mod->flg |= ~type & 0x02 ? XXM_FLG_MODRNG : 0;

	MODULE_INFO();

	hio_seek(f, start + p_ord, SEEK_SET);
	hio_read(mod->xxo, 1, mod->len, f);

	hio_seek(f, start + p_chn, SEEK_SET);
	hio_read(buf, 1, 16, f);

	if (instrument_init(mod) < 0)
		return -1;

	hio_seek(f, start + p_ins, SEEK_SET);
	for (i = 0; i < mod->ins; i++) {
		uint16 flags, c2spd;
		int finetune;

		if (subinstrument_alloc(mod, i, 1) < 0)
			return -1;

		hio_read(buf, 1, 13, f);	/* sample filename */
		hio_read(buf, 1, 24, f);	/* sample description */
		strncpy((char *)mod->xxi[i].name, (char *)buf, 24);
		str_adj((char *)mod->xxi[i].name);
		p_smp[i] = hio_read32l(f);
		hio_read32l(f);			/* memory location */
		hio_read16l(f);			/* sample number */
		flags = hio_read8(f);		/* sample type */
		mod->xxs[i].len = hio_read32l(f); 
		mod->xxs[i].lps = hio_read32l(f);
		mod->xxs[i].lpe = hio_read32l(f);
		finetune = (int8)(hio_read8(f) << 4);
		mod->xxi[i].sub[0].vol = hio_read8(f);
		c2spd = 8363 * hio_read16l(f) / 8448;
		mod->xxi[i].sub[0].pan = 0x80;
		mod->xxi[i].sub[0].sid = i;
		mod->xxs[i].flg = flags & 0x80 ? XMP_SAMPLE_LOOP : 0;
		mod->xxs[i].flg |= flags & 0x20 ? XMP_SAMPLE_LOOP_BIDIR : 0;
		c2spd_to_note(c2spd, &mod->xxi[i].sub[0].xpo,
						&mod->xxi[i].sub[0].fin);
		mod->xxi[i].sub[0].fin += finetune;

		if (mod->xxs[i].len > 0)
			mod->xxi[i].nsm = 1;

		D_(D_INFO "[%2X] %-22.22s %04x %04x %04x %c V%02x %5d",
			i, mod->xxi[i].name, mod->xxs[i].len, mod->xxs[i].lps,
			mod->xxs[i].lpe, mod->xxs[i].flg & XMP_SAMPLE_LOOP ?
			'L' : ' ', mod->xxi[i].sub[0].vol, c2spd);
	}
	
	if (pattern_init(mod) < 0)
		return -1;

	D_(D_INFO "Stored patterns: %d", mod->pat);

	hio_seek(f, start + p_pat, SEEK_SET);
	for (i = 0; i < mod->pat; i++) {
		int len;
		uint8 b, rows, chan;

		len = hio_read16l(f) - 4;
		rows = hio_read8(f);
		chan = hio_read8(f);

		if (pattern_tracks_alloc(mod, i, rows) < 0)
			return -1;

		for (r = 0; r < rows; r++) {
			while (len > 0) {
				b = hio_read8(f);
				len--;

				if (b == 0)
					break;
	
				c = b & 0x0f;
				if (c >= mod->chn)
					return -1;
				event = &EVENT(i, c, r);
	
				if (b & 0x80) {
					event->note = hio_read8(f) + 36 + 1;
					event->ins = hio_read8(f);
					len -= 2;
				}
	
				if (b & 0x40) {
					event->vol = hio_read8(f) + 1;
					len--;
				}
	
				if (b & 0x20) {
					event->fxt = hio_read8(f);
					event->fxp = hio_read8(f);
					len -= 2;
				}
			}
		}

		if (len > 0)
			hio_seek(f, len, SEEK_CUR);
	}

	/* Read samples */

	D_(D_INFO "Stored samples: %d", mod->smp);

	for (i = 0; i < mod->ins; i++) {
		hio_seek(f, start + p_smp[i], SEEK_SET);
		if (load_sample(m, f, SAMPLE_FLAG_DIFF,
				&mod->xxs[mod->xxi[i].sub[0].sid], NULL) < 0)
			return -1;
	}

	return 0;
}
