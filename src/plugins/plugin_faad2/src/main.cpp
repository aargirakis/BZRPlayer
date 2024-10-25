#include "neaacdec.h"
#include <string.h>
#include <algorithm>
#include <iostream>
#include <stdio.h>
#include "fmod_errors.h"
#include "info.h"
#include "plugins.h"

#ifdef WIN32
#include <malloc.h>
#else
typedef __int64_t __int64;
#endif

#ifndef MAKEFOURCC
#ifdef _BIG_ENDIAN
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
    ((DWORD)(BYTE)(ch3) | ((DWORD)(BYTE)(ch2) << 8) |   \
    ((DWORD)(BYTE)(ch1) << 16) | ((DWORD)(BYTE)(ch0) << 24 ))
#else
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
    ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |   \
    ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))
#endif
#endif

#define MAX_CHANNELS 2
#define BUFFER_SIZE FAAD_MIN_STREAMSIZE*MAX_CHANNELS

typedef unsigned long DWORD;
typedef int BOOL;
typedef unsigned char BYTE;
typedef unsigned short WORD;

typedef struct
{
    unsigned int initbytes;
    DWORD sr;
    BYTE nch;
    NeAACDecHandle neaac;
    unsigned char fbuf[BUFFER_SIZE];
    DWORD fbuflen;
} aacinfo;


FMOD_RESULT F_CALLBACK aacopen(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo);
FMOD_RESULT F_CALLBACK aacclose(FMOD_CODEC_STATE* codec);
FMOD_RESULT F_CALLBACK aacread(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read);
FMOD_RESULT F_CALLBACK aacsetposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION aaccodec =
{
    FMOD_CODEC_PLUGIN_VERSION,
    "VGM Stream plugin", // Name.
    0x00010000, // Version 0xAAAABBBB   A = major, B = minor.
    0, // Don't force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS, // The time format we would like to accept into setposition/getposition.
    &aacopen, // Open callback.
    &aacclose, // Close callback.
    &aacread, // Read callback.
    0,
    // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &aacsetposition, // Setposition callback.
    0,
    // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    0 // Sound create callback (don't need it)
};


static int get_AAC_format(aacinfo* x)
{
    unsigned int a = 0;
    do
    {
#if 0
        if (*(DWORD*)(x->fbuf+a)==MAKEFOURCC('A','D','I','F')) { // "ADIF" signature
            x->initbytes+=a;
            return -1; //Not supported
        }
#endif
        if (x->fbuf[a] == 0xff && (x->fbuf[a + 1] & 0xf6) == 0xf0 && ((x->fbuf[a + 2] & 0x3C) >> 2) < 12)
        {
            // ADTS header syncword
            x->initbytes += a;
            return 0;
        }
    }
    while (++a < x->fbuflen - 4);
    return -1;
}


class aacplugin
{
    FMOD_CODEC_STATE* _codec;

public:
    aacplugin(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        memset(&fcwaveformat, 0, sizeof(fcwaveformat));
    }

    ~aacplugin()
    {
        //delete some stuff
    }

    //VGMSTREAM* vgmstream;
    Info* info;
    aacinfo* x;
    int seek;

    FMOD_CODEC_WAVEFORMAT fcwaveformat;
};

/*
    FMODGetCodecDescription is mandatory for every fmod plugin.  This is the symbol the registerplugin function searches for.
    Must be declared with F_API to make it export as stdcall.
    MUST BE EXTERN'ED AS C!  C++ functions will be mangled incorrectly and not load in fmod.
*/
#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) FMOD_CODEC_DESCRIPTION* __stdcall _FMODGetCodecDescription()
{
    return &aaccodec;
}


#ifdef __cplusplus
}
#endif


FMOD_RESULT F_CALLBACK aacopen(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo)
{
    aacplugin* fc = new aacplugin(codec);

    fc->info = (Info*)userexinfo->userdata;

    fc->fcwaveformat.format = FMOD_SOUND_FORMAT_PCM16;
    fc->fcwaveformat.channels = 2;


    fc->fcwaveformat.lengthpcm = 0xffffffff;
    // codec->filesize;// / aacwaveformat.blockalign;   /* bytes converted to PCM samples */;

    codec->waveformat = &(fc->fcwaveformat);
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */

    unsigned int readBytes = 0;
    FMOD_RESULT r;

    std::string extension = fc->info->filename.substr(fc->info->filename.length() - 3, fc->info->filename.length());
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    if (extension.compare("aac"))
    {
        return FMOD_ERR_FORMAT;
    }
    aacinfo* x = new aacinfo;
    if (!x) return FMOD_ERR_INTERNAL;
    memset(x, 0, sizeof(aacinfo));

    codec->plugindata = fc; /* user data value */

    //rewind file pointer
    r = FMOD_CODEC_FILE_SEEK(codec, 0, 0);

    if (r != FMOD_OK)
        return FMOD_ERR_FILE_EOF;


    r = FMOD_CODEC_FILE_READ(codec, x->fbuf, BUFFER_SIZE, &readBytes);


    if (r != FMOD_OK || readBytes == 0)
        return FMOD_ERR_FILE_EOF;

    x->fbuflen += readBytes;
    x->initbytes = 0;

    if (x->fbuf[4] == 'f' &&
        x->fbuf[5] == 't' &&
        x->fbuf[6] == 'y' &&
        x->fbuf[7] == 'p')
    {
        printf("it's an mp4!\n");
        return FMOD_ERR_FORMAT;
    }
    else
    {
        if (get_AAC_format(x) == -1)
        {
            return FMOD_ERR_FORMAT;
        }


        if (!(x->neaac = NeAACDecOpen()))
            return FMOD_ERR_FORMAT;

        if (x->initbytes < 0 || x->initbytes > BUFFER_SIZE)
            return FMOD_ERR_FORMAT;


        memmove(x->fbuf, x->fbuf + x->initbytes, BUFFER_SIZE - x->initbytes);
        x->fbuflen -= x->initbytes;

        r = FMOD_CODEC_FILE_READ(codec, x->fbuf + x->fbuflen, BUFFER_SIZE - x->fbuflen, &readBytes);
        if (r != FMOD_OK)
            return FMOD_ERR_FILE_EOF;

        x->fbuflen += readBytes;

        long byt = NeAACDecInit(x->neaac, x->fbuf, x->fbuflen, &x->sr, &x->nch);
        fc->fcwaveformat.frequency = x->sr;
        fc->fcwaveformat.channels = x->nch;
        fc->fcwaveformat.pcmblocksize = 512 * fc->fcwaveformat.channels; /* 2 = 16bit pcm */
        fc->fcwaveformat.lengthpcm = 0xffffffff;
        // codec->filesize;// / aacwaveformat.blockalign;   /* bytes converted to PCM samples */;

        if (byt < 0)
            return FMOD_ERR_INTERNAL;
        if (byt > 0)
        {
            memmove(x->fbuf, x->fbuf + byt, BUFFER_SIZE - byt);
            x->fbuflen -= byt;
        }

        NeAACDecConfigurationPtr config = NeAACDecGetCurrentConfiguration(x->neaac);
        config->outputFormat = FAAD_FMT_16BIT;
        config->defSampleRate = 44100;
        NeAACDecSetConfiguration(x->neaac, config);
        fc->x = x;
        fc->seek = -1;


        fc->info->plugin = PLUGIN_faad2;
        fc->info->pluginName = PLUGIN_faad2_NAME;
        fc->info->fileformat = "aac";
        fc->info->setSeekable(false);
    }
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK aacclose(FMOD_CODEC_STATE* codec)
{
    aacplugin* fc = (aacplugin*)codec->plugindata;
    //    NeAACDecClose(fc->x->neaac);
    //    delete (aacplugin*)codec->plugindata;
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK aacread(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    aacplugin* fc = (aacplugin*)codec->plugindata;
    memset(buffer, 0, size);

    if (size < fc->fcwaveformat.pcmblocksize)
    {
        *read = size;
        return FMOD_OK;
    }

    aacinfo* x = fc->x;
    if (!x || !read)
        return FMOD_ERR_INTERNAL;

    void* buf = NULL;
    unsigned int buflen = 0;
    unsigned int r;

    NeAACDecFrameInfo info;

    bool eof = false;
    while (buflen < size || eof)
    {
        do
        {
            r = 0;
            FMOD_RESULT res;
            res = FMOD_CODEC_FILE_READ(codec, x->fbuf + x->fbuflen, BUFFER_SIZE - x->fbuflen, &r);
            if (res == FMOD_ERR_FILE_EOF)
                eof = true;
            else if (res != FMOD_OK)
                return FMOD_ERR_INTERNAL;

            x->fbuflen += r;
            buf = NeAACDecDecode(x->neaac, &info, x->fbuf, x->fbuflen);
            if (info.error != 0)
            {
                *read = 0;
                return FMOD_ERR_FILE_BAD;
            }
            if (info.bytesconsumed > x->fbuflen)
            {
                x->fbuflen = 0;
            }
            else
            {
                x->fbuflen -= info.bytesconsumed;
                memmove(x->fbuf, x->fbuf + info.bytesconsumed, x->fbuflen); // shift remaining data to start of buffer
            }
        }
        while (!info.samples || eof);
        if (info.samples != 0)
        {
            if (!buf)
                return FMOD_ERR_INTERNAL;
            memcpy((unsigned char*)buffer + buflen, buf, info.samples * 2);
            buflen += info.samples * 2;
        }
    }
    *read = buflen;
    if (eof) return FMOD_ERR_FILE_EOF;
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK aacsetposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype)
{
    aacplugin* fc = (aacplugin*)codec->plugindata;
    fc->seek = 0;
    return FMOD_OK;
}
