////////////////////////////////////////////////////////////////////////////
//                           **** WAVPACK ****                            //
//                  Hybrid Lossless Wavefile Compressor                   //
//              Copyright (c) 1998 - 2007 Conifer Software.               //
//                          All Rights Reserved.                          //
//      Distributed under the BSD Software License (see license.txt)      //
////////////////////////////////////////////////////////////////////////////

// pack.c

// This module actually handles the compression of the audio data, except for
// the entropy coding which is handled by the words? modules. For efficiency,
// the conversion is isolated to tight loops that handle an entire buffer.

#include "include/wplocal.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

//////////////////////////////// local tables ///////////////////////////////

// These two tables specify the characteristics of the decorrelation filters.
// Each term represents one layer of the sequential filter, where positive
// values indicate the relative sample involved from the same channel (1=prev),
// 17 & 18 are special functions using the previous 2 samples, and negative
// values indicate cross channel decorrelation (in stereo only).

const signed char very_high_terms [] = { 18,18,2,3,-2,18,2,4,7,5,3,6,8,-1,18,2,0 };
const signed char high_terms [] =      { 18,18,18,-2,2,3,5,-1,17,4,0 };
const signed char default_terms [] =   { 18,18,2,17,3,0 };
const signed char fast_terms [] =      { 18,17,0 };

///////////////////////////// executable code ////////////////////////////////

// This function initializes everything required to pack WavPack bitstreams
// and must be called BEFORE any other function in this module.

void pack_init (WavpackContext *wpc)
{
    WavpackStream *wps = &wpc->stream;
    uint32_t flags = wps->wphdr.flags;
    const signed char *term_string;
    struct decorr_pass *dpp;
    int ti;

    wps->sample_index = 0;
    CLEAR (wps->decorr_passes);
    CLEAR (wps->dc);

    if (wpc->config.flags & CONFIG_VERY_HIGH_FLAG)
        term_string = very_high_terms;
    else if (wpc->config.flags & CONFIG_HIGH_FLAG)
        term_string = high_terms;
    else if (wpc->config.flags & CONFIG_FAST_FLAG)
        term_string = fast_terms;
    else
        term_string = default_terms;

    for (dpp = wps->decorr_passes, ti = 0; ti < (int) strlen((char *) term_string); ti++)
    {
        if (term_string [ti] >= 0)
        {
            dpp->term = term_string [ti];
            dpp++->delta = 2;
        }
    }
    wps->num_terms = dpp - wps->decorr_passes;
    init_words (wps);
}

// Allocate room for and copy the decorrelation terms from the decorr_passes
// array into the specified metadata structure. Both the actual term id and
// the delta are packed into single characters.

void write_decorr_terms (WavpackStream *wps, WavpackMetadata *wpmd)
{
    int tcount = wps->num_terms;
    struct decorr_pass *dpp;
    char *byteptr;

    byteptr = (char *) wpmd->temp_data; 
    wpmd->data = wpmd->temp_data; 
    wpmd->id = ID_DECORR_TERMS;

    for (dpp = wps->decorr_passes; tcount--; ++dpp)
        *byteptr++ = ((dpp->term + 5) & 0x1f) | ((dpp->delta << 5) & 0xe0);

    wpmd->byte_length = byteptr - (char *) wpmd->data;
}

// Allocate room for and copy the decorrelation term weights from the
// decorr_passes array into the specified metadata structure. The weights
// range +/-1024, but are rounded and truncated to fit in signed chars for
// metadata storage. Weights are separate for the two channels

void write_decorr_weights (WavpackStream *wps, WavpackMetadata *wpmd)
{
    struct decorr_pass *dpp = wps->decorr_passes;
    int tcount, i;
    char *byteptr;

    byteptr = (char *) wpmd->temp_data;
    wpmd->data = wpmd->temp_data;
    wpmd->id = ID_DECORR_WEIGHTS;

    for (i = wps->num_terms - 1; i >= 0; --i)
        if (store_weight (dpp [i].weight_A) 
            //||
           // (!(wps->wphdr.flags & MONO_DATA) && 
            //store_weight (dpp [i].weight_B))
           )
                break;

    tcount = i + 1;

    for (i = 0; i < wps->num_terms; ++i) {
        if (i < tcount) {
            dpp [i].weight_A = restore_weight (*byteptr++ = store_weight (dpp [i].weight_A));
        }
        else
            dpp [i].weight_A = dpp [i].weight_B = 0;
    }

    wpmd->byte_length = byteptr - (char *) wpmd->data;
}

// Allocate room for and copy the decorrelation samples from the decorr_passes
// array into the specified metadata structure. The samples are signed 32-bit
// values, but are converted to signed log2 values for storage in metadata.
// Values are stored for both channels and are specified from the first term
// with unspecified samples set to zero. The number of samples stored varies
// with the actual term value, so those must obviously be specified before
// these in the metadata list. Any number of terms can have their samples
// specified from no terms to all the terms, however I have found that
// sending more than the first term's samples is a waste. The "wcount"
// variable can be set to the number of terms to have their samples stored.

void write_decorr_samples (WavpackStream *wps, WavpackMetadata *wpmd)
{
    int tcount = wps->num_terms, wcount = 1, temp;
    struct decorr_pass *dpp;
    uchar *byteptr;

    byteptr = (uchar *) wpmd->temp_data;
    wpmd->data = wpmd->temp_data;
    wpmd->id = ID_DECORR_SAMPLES;

    for (dpp = wps->decorr_passes; tcount--; ++dpp)
        if (wcount) {
            if (dpp->term > MAX_TERM) {
                dpp->samples_A [0] = exp2s (temp = log2s (dpp->samples_A [0]));
                *byteptr++ = temp;
                *byteptr++ = temp >> 8;
                dpp->samples_A [1] = exp2s (temp = log2s (dpp->samples_A [1]));
                *byteptr++ = temp;
                *byteptr++ = temp >> 8;
            }
            else if (dpp->term < 0) {
                dpp->samples_A [0] = exp2s (temp = log2s (dpp->samples_A [0]));
                *byteptr++ = temp;
                *byteptr++ = temp >> 8;
                dpp->samples_B [0] = exp2s (temp = log2s (dpp->samples_B [0]));
                *byteptr++ = temp;
                *byteptr++ = temp >> 8;
            }
            else {
                int m = 0, cnt = dpp->term;

                while (cnt--) {
                    dpp->samples_A [m] = exp2s (temp = log2s (dpp->samples_A [m]));
                    *byteptr++ = temp;
                    *byteptr++ = temp >> 8;
                    m++;
                }
            }

            wcount--;
        }
        else {
            CLEAR (dpp->samples_A);
            CLEAR (dpp->samples_B);
        }

    wpmd->byte_length = byteptr - (uchar *) wpmd->data;
}

// Allocate room for and copy the configuration information into the specified
// metadata structure. Currently, we just store the upper 3 bytes of
// config.flags and only in the first block of audio data. Note that this is
// for informational purposes not required for playback or decoding (like
// whether high or fast mode was specified).

void write_config_info (WavpackContext *wpc, WavpackMetadata *wpmd)
{
    char *byteptr;

    byteptr = (char *) wpmd->temp_data;
    wpmd->data = wpmd->temp_data;
    wpmd->id = ID_CONFIG_BLOCK;
    *byteptr++ = (char) (wpc->config.flags >> 8);
    *byteptr++ = (char) (wpc->config.flags >> 16);
    *byteptr++ = (char) (wpc->config.flags >> 24);
    wpmd->byte_length = byteptr - (char *) wpmd->data;
}

// Prepare a WavPack block for writing. The block will be written at
// "wps->blockbuff" and "wps->blockend" points to the end of the available
// space. If a wvc file is being written, then block2buff and block2end are
// also used. This also sets up the bitstreams so that pack_samples() can be
// called next with actual sample data. To find out how much data was written
// the caller must look at the ckSize field of the written WavpackHeader, NOT
// the one in the WavpackStream. A return value of FALSE indicates an error.

static int copy_metadata (WavpackMetadata *wpmd, uchar *buffer_start, uchar *buffer_end);

int pack_start_block (WavpackContext *wpc)
{
    WavpackStream *wps = &wpc->stream;
    uint32_t flags = wps->wphdr.flags;
    WavpackMetadata wpmd;

    wps->lossy_block = FALSE;
    wps->wphdr.block_samples = 0;
    wps->wphdr.ckSize = sizeof (WavpackHeader) - 8;
    memcpy (wps->blockbuff, &wps->wphdr, sizeof (WavpackHeader));

    write_decorr_terms (wps, &wpmd);
    copy_metadata (&wpmd, wps->blockbuff, wps->blockend);

    write_decorr_weights (wps, &wpmd);
    copy_metadata (&wpmd, wps->blockbuff, wps->blockend);

    write_decorr_samples (wps, &wpmd);
    copy_metadata (&wpmd, wps->blockbuff, wps->blockend);

    write_entropy_vars (wps, &wpmd);
    copy_metadata (&wpmd, wps->blockbuff, wps->blockend);

    write_hybrid_profile (wps, &wpmd);
    copy_metadata (&wpmd, wps->blockbuff, wps->blockend);

    if ((flags & INITIAL_BLOCK) && !wps->sample_index) {
        write_config_info (wpc, &wpmd);
        copy_metadata (&wpmd, wps->blockbuff, wps->blockend);
    }

    bs_open_write (&wps->wvbits, wps->blockbuff + ((WavpackHeader *) wps->blockbuff)->ckSize + 12, wps->blockend);
    return TRUE;
}

// Pack the given samples into the block currently being assembled. This function
// checks the available space each sample so that it can return prematurely to
// indicate that the blocks must be terminated. The return value is the number
// of actual samples packed and will be the same as the provided sample_count
// in no error occurs.

uint32_t pack_samples (WavpackContext *wpc, int32_t *buffer, uint32_t sample_count)
{
    WavpackStream *wps = &wpc->stream;
    uint32_t flags = wps->wphdr.flags;
    int tcount, lossy = 0, m;
    struct decorr_pass *dpp;
    uint32_t crc, i;
    int32_t *bptr;

    if (!sample_count)
        return 0;

    m = ((WavpackHeader *) wps->blockbuff)->block_samples & (MAX_TERM - 1);
    crc = wps->crc;

    for (bptr = buffer, i = 0; i < sample_count; ++i) {
        int32_t code;

        if (bs_remain_write (&wps->wvbits) < 64 ||
            (wpc->wvc_flag && bs_remain_write (&wps->wvcbits) < 64))
                break;

        crc = crc * 3 + (code = *bptr++);

        for (tcount = wps->num_terms, dpp = wps->decorr_passes; tcount-- ; dpp++)
            if (dpp->term > MAX_TERM) {
                if (dpp->term & 1)
                    dpp->samples_A [2] = 2 * dpp->samples_A [0] - dpp->samples_A [1];
                else
                    dpp->samples_A [2] = (3 * dpp->samples_A [0] - dpp->samples_A [1]) >> 1;

                code -= (dpp->aweight_A = apply_weight (dpp->weight_A, dpp->samples_A [2]));
            }
            else
                code -= (dpp->aweight_A = apply_weight (dpp->weight_A, dpp->samples_A [m]));

        code = send_word (wps, code, 0);

        while (--dpp >= wps->decorr_passes) {
            if (dpp->term > MAX_TERM) {
                update_weight (dpp->weight_A, dpp->delta, dpp->samples_A [2], code);
                dpp->samples_A [1] = dpp->samples_A [0];
                dpp->samples_A [0] = (code += dpp->aweight_A);
            }
            else {
                int32_t sam = dpp->samples_A [m];

                update_weight (dpp->weight_A, dpp->delta, sam, code);
                dpp->samples_A [(m + dpp->term) & (MAX_TERM - 1)] = (code += dpp->aweight_A);
            }
        }

        wps->dc.error [0] += code;
        m = (m + 1) & (MAX_TERM - 1);

        if ((crc = crc * 3 + code) != crc)
            lossy = TRUE;
    }

    ((WavpackHeader *) wps->blockbuff)->block_samples += i;
    wps->crc = crc;

    if (lossy)
    {
        wps->lossy_block = TRUE;
    }
    wps->sample_index += i;
    return i;
}

// Once all the desired samples have been packed into the WavPack block being
// built, this function is called to prepare it for writing. Basically, this
// means just closing the bitstreams because the block_samples and crc fields
// of the WavpackHeader are updated during packing.

int pack_finish_block (WavpackContext *wpc)
{
    WavpackStream *wps = &wpc->stream;
    int lossy = wps->lossy_block, tcount, m;
    struct decorr_pass *dpp;
    uint32_t data_count;

    m = ((WavpackHeader *) wps->blockbuff)->block_samples & (MAX_TERM - 1);

    if (m) {
        for (tcount = wps->num_terms, dpp = wps->decorr_passes; tcount--; dpp++)
            if (dpp->term > 0 && dpp->term <= MAX_TERM) {
                int32_t temp_A [MAX_TERM], temp_B [MAX_TERM];
                int k;

                memcpy (temp_A, dpp->samples_A, sizeof (dpp->samples_A));
                memcpy (temp_B, dpp->samples_B, sizeof (dpp->samples_B));

                for (k = 0; k < MAX_TERM; k++) {
                    dpp->samples_A [k] = temp_A [m];
                    dpp->samples_B [k] = temp_B [m];
                    m = (m + 1) & (MAX_TERM - 1);
                }
            }
    }

    flush_word (wps);
    data_count = bs_close_write (&wps->wvbits);

    if (data_count)
    {
        if (data_count != (uint32_t) -1)
        {
            uchar *cptr = wps->blockbuff + ((WavpackHeader *) wps->blockbuff)->ckSize + 8;

            *cptr++ = ID_WV_BITSTREAM | ID_LARGE;
            *cptr++ = (unsigned char) (data_count >> 1);
            *cptr++ = (unsigned char) (data_count >> 9);
            *cptr = (unsigned char) (data_count >> 17);
            ((WavpackHeader *) wps->blockbuff)->ckSize += data_count + 4;
        }
        else
            return FALSE;
    }
    if (lossy)
        wpc->lossy_blocks = TRUE;

    return TRUE;
}

// Copy the specified metadata item to the WavPack block being contructed. This
// function tests for writing past the end of the available space, however the
// rest of the code is designed so that can't occur.

static int copy_metadata (WavpackMetadata *wpmd, uchar *buffer_start, uchar *buffer_end)
{
    uint32_t mdsize = wpmd->byte_length + (wpmd->byte_length & 1);
    WavpackHeader *wphdr = (WavpackHeader *) buffer_start;

    if (wpmd->byte_length & 1)
        ((char *) wpmd->data) [wpmd->byte_length] = 0;

    mdsize += (wpmd->byte_length > 510) ? 4 : 2;
    buffer_start += wphdr->ckSize + 8;

    if (buffer_start + mdsize >= buffer_end)
        return FALSE;

    buffer_start [0] = wpmd->id | (wpmd->byte_length & 1 ? ID_ODD_SIZE : 0);
    buffer_start [1] = (wpmd->byte_length + 1) >> 1;

    if (wpmd->byte_length > 510) {
        buffer_start [0] |= ID_LARGE;
        buffer_start [2] = (wpmd->byte_length + 1) >> 9;
        buffer_start [3] = (wpmd->byte_length + 1) >> 17;
    }

    if (wpmd->data && wpmd->byte_length) {
        if (wpmd->byte_length > 510) {
            buffer_start [0] |= ID_LARGE;
            buffer_start [2] = (wpmd->byte_length + 1) >> 9;
            buffer_start [3] = (wpmd->byte_length + 1) >> 17;
            memcpy (buffer_start + 4, wpmd->data, mdsize - 4);
        }
        else
            memcpy (buffer_start + 2, wpmd->data, mdsize - 2);
    }

    wphdr->ckSize += mdsize;
    return TRUE;
}
