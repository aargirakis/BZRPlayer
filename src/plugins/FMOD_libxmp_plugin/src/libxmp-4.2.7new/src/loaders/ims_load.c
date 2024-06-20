/* Extended Module Player
 * Copyright (C) 1996-2014 Claudio Matsuoka and Hipolito Carraro Jr
 *
 * This file is part of the Extended Module Player and is distributed
 * under the terms of the GNU Lesser General Public License. See COPYING.LIB
 * for more information.
 */

/* Loader for Images Music System modules based on the EP replayer.
 *
 * Date: Thu, 19 Apr 2001 19:13:06 +0200
 * From: Michael Doering <mldoering@gmx.net>
 *
 * I just "stumbled" upon something about the Unic.3C format when I was
 * testing replayers for the upcoming UADE 0.21 that might be also
 * interesting to you for xmp. The "Beastbusters" tune is not a UNIC file :)
 * It's actually a different Format, although obviously related, called
 * "Images Music System".
 *
 * I was testing the replayer from the Wanted Team with one of their test
 * tunes, among them also the beastbuster music. When I first listened to
 * it, I knew I have heard it somewhere, a bit different but it was alike.
 * This one had more/richer percussions and there was no strange beep in
 * the bg. ;) After some searching on my HD I found it among the xmp test
 * tunes as a UNIC file.
 */

#include <ctype.h>
#include <sys/types.h>

#include "loader.h"
#include "period.h"

struct ims_instrument {
    uint8 name[20];
    int16 finetune;		/* Causes squeaks in beast-busters1! */
    uint16 size;
    uint8 unknown;
    uint8 volume;
    uint16 loop_start;
    uint16 loop_size;
};

struct ims_header {
    uint8 title[20];
    struct ims_instrument ins[31];
    uint8 len;
    uint8 zero;
    uint8 orders[128];
    uint8 magic[4];
};


static int ims_test (HIO_HANDLE *, char *, const int);
static int ims_load (struct module_data *, HIO_HANDLE *, const int);

const struct format_loader ims_loader = {
    "Images Music System",
    ims_test,
    ims_load
};

static int ims_test(HIO_HANDLE *f, char *t, const int start)
{
    int i;
    int smp_size, pat;
    struct ims_header ih;

    smp_size = 0;

    hio_read(&ih.title, 20, 1, f);

    for (i = 0; i < 31; i++) {
	if (hio_read(&ih.ins[i].name, 1, 20, f) < 20)
	    return -1;

	ih.ins[i].finetune = (int16)hio_read16b(f);
	ih.ins[i].size = hio_read16b(f);
	ih.ins[i].unknown = hio_read8(f);
	ih.ins[i].volume = hio_read8(f);
	ih.ins[i].loop_start = hio_read16b(f);
	ih.ins[i].loop_size = hio_read16b(f);

	smp_size += ih.ins[i].size * 2;

	if (test_name(ih.ins[i].name, 20) < 0)
	    return -1;

	if (ih.ins[i].volume > 0x40)
	    return -1;

	if (ih.ins[i].size > 0x8000)
	    return -1;

	if (ih.ins[i].loop_start > ih.ins[i].size)
	    return -1;

        if (ih.ins[i].size && ih.ins[i].loop_size > 2 * ih.ins[i].size)
	    return -1;
    }

    if (smp_size < 8)
	return -1;

    ih.len = hio_read8(f);
    ih.zero = hio_read8(f);
    hio_read (&ih.orders, 128, 1, f);
    hio_read (&ih.magic, 4, 1, f);
  
    if (ih.zero > 1)		/* not sure what this is */
	return -1;

    if (ih.magic[3] != 0x3c)
	return -1;

    if (ih.len > 0x7f)
	return -1;

    for (pat = i = 0; i < ih.len; i++)
	if (ih.orders[i] > pat)
	    pat = ih.orders[i];
    pat++;

    if (pat > 0x7f || ih.len == 0 || ih.len > 0x7f)
	return -1;
   
    hio_seek(f, start + 0, SEEK_SET);
    read_title(f, t, 20);

    return 0;
}


static int ims_load(struct module_data *m, HIO_HANDLE *f, const int start)
{
    struct xmp_module *mod = &m->mod;
    int i, j;
    int smp_size;
    struct xmp_event *event;
    struct ims_header ih;
    uint8 ims_event[3];
    int xpo = 21;		/* Tuned against UADE */

    LOAD_INIT();

    mod->ins = 31;
    mod->smp = mod->ins;
    smp_size = 0;

    hio_read (&ih.title, 20, 1, f);

    for (i = 0; i < 31; i++) {
	hio_read (&ih.ins[i].name, 20, 1, f);
	ih.ins[i].finetune = (int16)hio_read16b(f);
	ih.ins[i].size = hio_read16b(f);
	ih.ins[i].unknown = hio_read8(f);
	ih.ins[i].volume = hio_read8(f);
	ih.ins[i].loop_start = hio_read16b(f);
	ih.ins[i].loop_size = hio_read16b(f);

	smp_size += ih.ins[i].size * 2;
    }

    ih.len = hio_read8(f);
    ih.zero = hio_read8(f);
    hio_read (&ih.orders, 128, 1, f);
    hio_read (&ih.magic, 4, 1, f);
  
    mod->len = ih.len;
    memcpy (mod->xxo, ih.orders, mod->len);

    for (i = 0; i < mod->len; i++)
	if (mod->xxo[i] > mod->pat)
	    mod->pat = mod->xxo[i];

    mod->pat++;
    mod->trk = mod->chn * mod->pat;

    strncpy(mod->name, (char *)ih.title, 20);
    set_type(m, "Images Music System");

    MODULE_INFO();

    if (instrument_init(mod) < 0)
	return -1;

    for (i = 0; i < mod->ins; i++) {
	if (subinstrument_alloc(mod, i, 1) < 0)
	    return -1;

	mod->xxs[i].len = 2 * ih.ins[i].size;
	mod->xxs[i].lpe = mod->xxs[i].lps + 2 * ih.ins[i].loop_size;
	mod->xxs[i].flg = ih.ins[i].loop_size > 1 ? XMP_SAMPLE_LOOP : 0;
	mod->xxi[i].sub[0].fin = 0; /* ih.ins[i].finetune; */
	mod->xxi[i].sub[0].vol = ih.ins[i].volume;
	mod->xxi[i].sub[0].pan = 0x80;
	mod->xxi[i].sub[0].sid = i;
	mod->xxi[i].rls = 0xfff;

	if (mod->xxs[i].len > 0)
		mod->xxi[i].nsm = 1;

	instrument_name(mod, i, ih.ins[i].name, 20);

	D_(D_INFO "[%2X] %-20.20s %04x %04x %04x %c V%02x %+d",
		i, mod->xxi[i].name, mod->xxs[i].len, mod->xxs[i].lps,
		mod->xxs[i].lpe, ih.ins[i].loop_size > 1 ? 'L' : ' ',
		mod->xxi[i].sub[0].vol, mod->xxi[i].sub[0].fin >> 4);
    }

    if (pattern_init(mod) < 0)
	return -1;

    /* Load and convert patterns */
    D_(D_INFO "Stored patterns: %d", mod->pat);

    for (i = 0; i < mod->pat; i++) {
	if (pattern_tracks_alloc(mod, i, 64) < 0)
	    return -1;

	for (j = 0; j < 0x100; j++) {
	    event = &EVENT (i, j & 0x3, j >> 2);
	    hio_read (ims_event, 1, 3, f);

	    /* Event format:
	     *
	     * 0000 0000  0000 0000  0000 0000
	     *  |\     /  \  / \  /  \       /
	     *  | note    ins   fx   parameter
	     * ins
	     *
	     * 0x3f is a blank note.
	     */
	    event->note = ims_event[0] & 0x3f;
	    if (event->note != 0x00 && event->note != 0x3f)
		event->note += xpo + 12;
	    else
		event->note = 0;
	    event->ins = ((ims_event[0] & 0x40) >> 2) | MSN(ims_event[1]);
	    event->fxt = LSN(ims_event[1]);
	    event->fxp = ims_event[2];

	    disable_continue_fx (event);

	    /* According to Asle:
	     * ``Just note that pattern break effect command (D**) uses
	     * HEX value in UNIC format (while it is DEC values in PTK).
	     * Thus, it has to be converted!''
	     *
	     * Is this valid for IMS as well? --claudio
	     */
	    if (event->fxt == 0x0d)
		 event->fxp = (event->fxp / 10) << 4 | (event->fxp % 10);
	}
    }

    m->quirk |= QUIRK_MODRNG;

    /* Load samples */

    D_(D_INFO "Stored samples: %d", mod->smp);

    for (i = 0; i < mod->smp; i++) {
	if (!mod->xxs[i].len)
	    continue;
	if (load_sample(m, f, 0, &mod->xxs[i], NULL) < 0)
	    return -1;
    }

    return 0;
}
