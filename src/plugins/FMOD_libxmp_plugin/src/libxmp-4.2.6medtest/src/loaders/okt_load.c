/* Extended Module Player format loaders
 * Copyright (C) 1996-2014 Claudio Matsuoka and Hipolito Carraro Jr
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/* Based on the format description written by Harald Zappe.
 * Additional information about Oktalyzer modules from Bernardo
 * Innocenti's XModule 3.4 sources.
 */

#include "loader.h"
#include "iff.h"

static int okt_test(HIO_HANDLE *, char *, const int);
static int okt_load(struct module_data *, HIO_HANDLE *, const int);

const struct format_loader okt_loader = {
	"Oktalyzer",
	okt_test,
	okt_load
};

static int okt_test(HIO_HANDLE *f, char *t, const int start)
{
	char magic[8];

	if (hio_read(magic, 1, 8, f) < 8)
		return -1;

	if (strncmp(magic, "OKTASONG", 8))
		return -1;

	read_title(f, t, 0);

	return 0;
}

#define OKT_MODE8 0x00		/* 7 bit samples */
#define OKT_MODE4 0x01		/* 8 bit samples */
#define OKT_MODEB 0x02		/* Both */

#define NONE 0xff

struct local_data {
	int mode[36];
	int idx[36];
	int pattern;
	int sample;
};

static const int fx[] = {
	NONE,
	FX_PORTA_UP,		/*  1 */
	FX_PORTA_DN,		/*  2 */
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	NONE,
	FX_OKT_ARP3,		/* 10 */
	FX_OKT_ARP4,		/* 11 */
	FX_OKT_ARP5,		/* 12 */
	FX_NSLIDE_DN,		/* 13 */
	NONE,
	NONE,			/* 15 - filter */
	NONE,
	FX_NSLIDE_UP,		/* 17 */
	NONE,
	NONE,
	NONE,
	FX_F_NSLIDE_DN,		/* 21 */
	NONE,
	NONE,
	NONE,
	FX_JUMP,		/* 25 */
	NONE,
	NONE,			/* 27 - release */
	FX_SPEED,		/* 28 */
	NONE,
	FX_F_NSLIDE_UP,		/* 30 */
	FX_VOLSET		/* 31 */
};

static int get_cmod(struct module_data *m, int size, HIO_HANDLE *f, void *parm)
{
	struct xmp_module *mod = &m->mod;
	int i, j, k;

	mod->chn = 0;
	for (i = 0; i < 4; i++) {
		j = hio_read16b(f);
		for (k = ! !j; k >= 0; k--) {
			mod->xxc[mod->chn].pan = (((i + 1) / 2) % 2) * 0xff;
			mod->chn++;
		}
	}

	return 0;
}

static int get_samp(struct module_data *m, int size, HIO_HANDLE *f, void *parm)
{
	struct xmp_module *mod = &m->mod;
	struct local_data *data = (struct local_data *)parm;
	int i, j;
	int looplen;

	/* Should be always 36 */
	mod->ins = size / 32;	/* sizeof(struct okt_instrument_header); */
	mod->smp = mod->ins;

	if (instrument_init(mod) < 0)
		return -1;

	for (j = i = 0; i < mod->ins; i++) {
		struct xmp_instrument *xxi = &mod->xxi[i];
		struct xmp_sample *xxs = &mod->xxs[j];
		struct xmp_subinstrument *sub;

		if (subinstrument_alloc(mod, i, 1) < 0)
			return -1;

		sub = &xxi->sub[0];

		hio_read(xxi->name, 1, 20, f);
		adjust_string((char *)xxi->name);

		/* Sample size is always rounded down */
		xxs->len = hio_read32b(f) & ~1;
		xxs->lps = hio_read16b(f);
		looplen = hio_read16b(f);
		xxs->lpe = xxs->lps + looplen;
		xxs->flg = looplen > 2 ? XMP_SAMPLE_LOOP : 0;

		sub->vol = hio_read16b(f);
		data->mode[i] = hio_read16b(f);

		sub->pan = 0x80;
		sub->sid = j;

		data->idx[j] = i;

		if (xxs->len > 0) {
			xxi->nsm = 1;
			j++;
		}
	}

	return 0;
}

static int get_spee(struct module_data *m, int size, HIO_HANDLE *f, void *parm)
{
	struct xmp_module *mod = &m->mod;

	mod->spd = hio_read16b(f);
	mod->bpm = 125;

	return 0;
}

static int get_slen(struct module_data *m, int size, HIO_HANDLE *f, void *parm)
{
	struct xmp_module *mod = &m->mod;

	mod->pat = hio_read16b(f);
	mod->trk = mod->pat * mod->chn;

	return 0;
}

static int get_plen(struct module_data *m, int size, HIO_HANDLE *f, void *parm)
{
	struct xmp_module *mod = &m->mod;

	mod->len = hio_read16b(f);
	D_(D_INFO "Module length: %d", mod->len);

	return 0;
}

static int get_patt(struct module_data *m, int size, HIO_HANDLE *f, void *parm)
{
	struct xmp_module *mod = &m->mod;

	hio_read(mod->xxo, 1, mod->len, f);

	return 0;
}

static int get_pbod(struct module_data *m, int size, HIO_HANDLE *f, void *parm)
{
	struct xmp_module *mod = &m->mod;
	struct local_data *data = (struct local_data *)parm;
	struct xmp_event *e;
	uint16 rows;
	int j;

	if (data->pattern >= mod->pat)
		return 0;

	if (!data->pattern) {
		if (pattern_init(mod) < 0)
			return -1;
		D_(D_INFO "Stored patterns: %d", mod->pat);
	}

	rows = hio_read16b(f);

	if (pattern_tracks_alloc(mod, data->pattern, rows) < 0)
		return -1;

	for (j = 0; j < rows * mod->chn; j++) {
		uint8 note, ins;

		e = &EVENT(data->pattern, j % mod->chn, j / mod->chn);
		memset(e, 0, sizeof(struct xmp_event));

		note = hio_read8(f);
		ins = hio_read8(f);

		if (note) {
			e->note = 48 + note;
			e->ins = 1 + ins;
		}

		e->fxt = fx[hio_read8(f)];
		e->fxp = hio_read8(f);

		if ((e->fxt == FX_VOLSET) && (e->fxp > 0x40)) {
			if (e->fxp <= 0x50) {
				e->fxt = FX_VOLSLIDE;
				e->fxp -= 0x40;
			} else if (e->fxp <= 0x60) {
				e->fxt = FX_VOLSLIDE;
				e->fxp = (e->fxp - 0x50) << 4;
			} else if (e->fxp <= 0x70) {
				e->fxt = FX_F_VSLIDE_DN;
				e->fxp = e->fxp - 0x60;
			} else if (e->fxp <= 0x80) {
				e->fxt = FX_F_VSLIDE_UP;
				e->fxp = e->fxp - 0x70;
			}
		}
		if (e->fxt == FX_ARPEGGIO)	/* Arpeggio fixup */
			e->fxp = (((24 - MSN(e->fxp)) % 12) << 4) | LSN(e->fxp);
		if (e->fxt == NONE)
			e->fxt = e->fxp = 0;
	}
	data->pattern++;

	return 0;
}

static int get_sbod(struct module_data *m, int size, HIO_HANDLE *f, void *parm)
{
	struct xmp_module *mod = &m->mod;
	struct local_data *data = (struct local_data *)parm;
	int flags = 0;
	int i, sid;

	if (data->sample >= mod->ins)
		return 0;

	D_(D_INFO "Stored samples: %d", mod->smp);

	i = data->idx[data->sample];
	if (data->mode[i] == OKT_MODE8 || data->mode[i] == OKT_MODEB)
		flags = SAMPLE_FLAG_7BIT;

	sid = mod->xxi[i].sub[0].sid;
	if (load_sample(m, f, flags, &mod->xxs[sid], NULL) < 0)
		return -1;

	data->sample++;

	return 0;
}

static int okt_load(struct module_data *m, HIO_HANDLE * f, const int start)
{
	iff_handle handle;
	struct local_data data;
	int ret;

	LOAD_INIT();

	hio_seek(f, 8, SEEK_CUR);	/* OKTASONG */

	handle = iff_new();
	if (handle == NULL)
		return -1;

	memset(&data, 0, sizeof(struct local_data));

	/* IFF chunk IDs */
	ret = iff_register(handle, "CMOD", get_cmod);
	ret |= iff_register(handle, "SAMP", get_samp);
	ret |= iff_register(handle, "SPEE", get_spee);
	ret |= iff_register(handle, "SLEN", get_slen);
	ret |= iff_register(handle, "PLEN", get_plen);
	ret |= iff_register(handle, "PATT", get_patt);
	ret |= iff_register(handle, "PBOD", get_pbod);
	ret |= iff_register(handle, "SBOD", get_sbod);

	if (ret != 0)
		return -1;

	set_type(m, "Oktalyzer");

	MODULE_INFO();

	/* Load IFF chunks */
	if (iff_load(handle, m, f, &data) < 0) {
		iff_release(handle);
		return -1;
	}

	iff_release(handle);

	return 0;
}
