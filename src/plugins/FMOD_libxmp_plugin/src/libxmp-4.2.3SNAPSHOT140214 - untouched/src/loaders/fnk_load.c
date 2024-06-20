/* Extended Module Player
 * Copyright (C) 1996-2014 Claudio Matsuoka and Hipolito Carraro Jr
 *
 * This file is part of the Extended Module Player and is distributed
 * under the terms of the GNU Lesser General Public License. See COPYING.LIB
 * for more information.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include "loader.h"

#define MAGIC_Funk	MAGIC4('F','u','n','k')


static int fnk_test (HIO_HANDLE *, char *, const int);
static int fnk_load (struct module_data *, HIO_HANDLE *, const int);

const struct format_loader fnk_loader = {
    "Funktracker",
    fnk_test,
    fnk_load
};

static int fnk_test(HIO_HANDLE *f, char *t, const int start)
{
    uint8 a, b;
    int size;
    struct stat st;

    if (hio_read32b(f) != MAGIC_Funk)
	return -1;

    hio_read8(f); 
    a = hio_read8(f);
    b = hio_read8(f); 
    hio_read8(f); 

    if ((a >> 1) < 10)			/* creation year (-1980) */
	return -1;

    if (MSN(b) > 7 || LSN(b) > 9)	/* CPU and card */
	return -1;

    size = hio_read32l(f);
    if (size < 1024)
	return -1;

    hio_stat(f, &st);
    if (size != st.st_size)
        return -1;

    read_title(f, t, 0);

    return 0;
}


struct fnk_instrument {
    uint8 name[19];		/* ASCIIZ instrument name */
    uint32 loop_start;		/* Instrument loop start */
    uint32 length;		/* Instrument length */
    uint8 volume;		/* Volume (0-255) */
    uint8 pan;			/* Pan (0-255) */
    uint8 shifter;		/* Portamento and offset shift */
    uint8 waveform;		/* Vibrato and tremolo waveforms */
    uint8 retrig;		/* Retrig and arpeggio speed */
};

struct fnk_header {
    uint8 marker[4];		/* 'Funk' */
    uint8 info[4];		/* */
    uint32 filesize;		/* File size */
    uint8 fmt[4];		/* F2xx, Fkxx or Fvxx */
    uint8 loop;			/* Loop order number */
    uint8 order[256];		/* Order list */
    uint8 pbrk[128];		/* Break list for patterns */
    struct fnk_instrument fih[64];	/* Instruments */
};


static int fnk_load(struct module_data *m, HIO_HANDLE *f, const int start)
{
    struct xmp_module *mod = &m->mod;
    int i, j;
    int day, month, year;
    struct xmp_event *event;
    struct fnk_header ffh;
    uint8 ev[3];

    LOAD_INIT();

    hio_read(&ffh.marker, 4, 1, f);
    hio_read(&ffh.info, 4, 1, f);
    ffh.filesize = hio_read32l(f);
    hio_read(&ffh.fmt, 4, 1, f);
    ffh.loop = hio_read8(f);
    hio_read(&ffh.order, 256, 1, f);
    hio_read(&ffh.pbrk, 128, 1, f);

    for (i = 0; i < 64; i++) {
	hio_read(&ffh.fih[i].name, 19, 1, f);
	ffh.fih[i].loop_start = hio_read32l(f);
	ffh.fih[i].length = hio_read32l(f);
	ffh.fih[i].volume = hio_read8(f);
	ffh.fih[i].pan = hio_read8(f);
	ffh.fih[i].shifter = hio_read8(f);
	ffh.fih[i].waveform = hio_read8(f);
	ffh.fih[i].retrig = hio_read8(f);
    }

    day = ffh.info[0] & 0x1f;
    month = ((ffh.info[1] & 0x01) << 3) | ((ffh.info[0] & 0xe0) >> 5);
    year = 1980 + ((ffh.info[1] & 0xfe) >> 1);

    mod->smp = mod->ins = 64;

    for (i = 0; i < 256 && ffh.order[i] != 0xff; i++) {
	if (ffh.order[i] > mod->pat)
	    mod->pat = ffh.order[i];
    }
    mod->pat++;

    mod->len = i;
    memcpy (mod->xxo, ffh.order, mod->len);

    mod->spd = 4;
    mod->bpm = 125;
    mod->chn = 0;

    /*
     * If an R1 fmt (funktype = Fk** or Fv**), then ignore byte 3. It's
     * unreliable. It used to store the (GUS) sample memory requirement.
     */
    if (ffh.fmt[0] == 'F' && ffh.fmt[1] == '2') {
	if (((int8)ffh.info[3] >> 1) & 0x40)
	    mod->bpm -= (ffh.info[3] >> 1) & 0x3f;
	else
	    mod->bpm += (ffh.info[3] >> 1) & 0x3f;

	set_type(m, "FunktrackerGOLD");
    } else if (ffh.fmt[0] == 'F' && (ffh.fmt[1] == 'v' || ffh.fmt[1] == 'k')) {
	set_type(m, "Funktracker");
    } else {
	mod->chn = 8;
	set_type(m, "Funktracker DOS32");
    }

    if (mod->chn == 0) {
	mod->chn = (ffh.fmt[2] < '0') || (ffh.fmt[2] > '9') ||
		(ffh.fmt[3] < '0') || (ffh.fmt[3] > '9') ? 8 :
		(ffh.fmt[2] - '0') * 10 + ffh.fmt[3] - '0';
    }

    mod->bpm = 4 * mod->bpm / 5;
    mod->trk = mod->chn * mod->pat;

    /* FNK allows mode per instrument but we don't, so use linear like 669 */
    m->quirk |= QUIRK_LINEAR;

    MODULE_INFO();
    D_(D_INFO "Creation date: %02d/%02d/%04d", day, month, year);

    if (instrument_init(mod) < 0)
	return -1;

    /* Convert instruments */
    for (i = 0; i < mod->ins; i++) {
	if (subinstrument_alloc(mod, i, 1) < 0)
	    return -1;

	mod->xxs[i].len = ffh.fih[i].length;
	mod->xxs[i].lps = ffh.fih[i].loop_start;
	if (mod->xxs[i].lps == -1)
	    mod->xxs[i].lps = 0;
	mod->xxs[i].lpe = ffh.fih[i].length;
	mod->xxs[i].flg = ffh.fih[i].loop_start != -1 ? XMP_SAMPLE_LOOP : 0;
	mod->xxi[i].sub[0].vol = ffh.fih[i].volume;
	mod->xxi[i].sub[0].pan = ffh.fih[i].pan;
	mod->xxi[i].sub[0].sid = i;

	if (mod->xxs[i].len > 0)
	     mod->xxi[i].nsm = 1;

	instrument_name(mod, i, ffh.fih[i].name, 19);

	D_(D_INFO "[%2X] %-20.20s %04x %04x %04x %c V%02x P%02x", i,
		mod->xxi[i].name,
		mod->xxs[i].len, mod->xxs[i].lps, mod->xxs[i].lpe,
		mod->xxs[i].flg & XMP_SAMPLE_LOOP ? 'L' : ' ',
		mod->xxi[i].sub[0].vol, mod->xxi[i].sub[0].pan);
    }

    if (pattern_init(mod) < 0)
	return -1;

    /* Read and convert patterns */
    D_(D_INFO "Stored patterns: %d", mod->pat);

    for (i = 0; i < mod->pat; i++) {
	if (pattern_tracks_alloc(mod, i, 64) < 0)
	    return -1;

	EVENT(i, 1, ffh.pbrk[i]).f2t = FX_BREAK;

	for (j = 0; j < 64 * mod->chn; j++) {
	    event = &EVENT(i, j % mod->chn, j / mod->chn);
	    hio_read(&ev, 1, 3, f);

	    switch (ev[0] >> 2) {
	    case 0x3f:
	    case 0x3e:
	    case 0x3d:
		break;
	    default:
		event->note = 37 + (ev[0] >> 2);
		event->ins = 1 + MSN(ev[1]) + ((ev[0] & 0x03) << 4);
		event->vol = ffh.fih[event->ins - 1].volume;
	    }

	    switch (LSN(ev[1])) {
	    case 0x00:
		event->fxt = FX_PER_PORTA_UP;
		event->fxp = ev[2];
		break;
	    case 0x01:
		event->fxt = FX_PER_PORTA_DN;
		event->fxp = ev[2];
		break;
	    case 0x02:
		event->fxt = FX_PER_TPORTA;
		event->fxp = ev[2];
		break;
	    case 0x03:
		event->fxt = FX_PER_VIBRATO;
		event->fxp = ev[2];
		break;
	    case 0x06:
		event->fxt = FX_PER_VSLD_UP;
		event->fxp = ev[2] << 1;
		break;
	    case 0x07:
		event->fxt = FX_PER_VSLD_DN;
		event->fxp = ev[2] << 1;
		break;
	    case 0x0b:
		event->fxt = FX_ARPEGGIO;
		event->fxp = ev[2];
		break;
	    case 0x0d:
		event->fxt = FX_VOLSET;
		event->fxp = ev[2];
		break;
	    case 0x0e:
		if (ev[2] == 0x0a || ev[2] == 0x0b || ev[2] == 0x0c) {
		    event->fxt = FX_PER_CANCEL;
		    break;
		}

		switch (MSN(ev[2])) {
		case 0x1:
		    event->fxt = FX_EXTENDED;
		    event->fxp = (EX_CUT << 4) | LSN(ev[2]);
		    break;
		case 0x2:
		    event->fxt = FX_EXTENDED;
		    event->fxp = (EX_DELAY << 4) | LSN(ev[2]);
		    break;
		case 0xd:
		    event->fxt = FX_EXTENDED;
		    event->fxp = (EX_RETRIG << 4) | LSN(ev[2]);
		    break;
		case 0xe:
		    event->fxt = FX_SETPAN;
		    event->fxp = 8 + (LSN(ev[2]) << 4);	
		    break;
		case 0xf:
		    event->fxt = FX_SPEED;
		    event->fxp = LSN(ev[2]);	
		    break;
		}
	    }
	}
    }

    /* Read samples */
    D_(D_INFO "Stored samples: %d", mod->smp);

    for (i = 0; i < mod->ins; i++) {
	if (mod->xxs[i].len <= 2)
	    continue;

	if (load_sample(m, f, 0, &mod->xxs[i], NULL) < 0)
	    return -1;
    }

    for (i = 0; i < mod->chn; i++)
	mod->xxc[i].pan = 0x80;

    m->volbase = 0xff;
    m->quirk = QUIRK_VSALL;

    return 0;
}
