#include "mdxmini.h"
#include <string.h>
#include "fmod_errors.h"
#include "info.h"
#include "plugins.h"

FMOD_RESULT F_CALLBACK fcopen(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo);
FMOD_RESULT F_CALLBACK fcclose(FMOD_CODEC_STATE* codec);
FMOD_RESULT F_CALLBACK fcread(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read);
FMOD_RESULT F_CALLBACK fcsetposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position,
                                     FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION fccodec =
{
    FMOD_CODEC_PLUGIN_VERSION,
    "MDX plugin", // Name.
    0x00010000, // Version 0xAAAABBBB   A = major, B = minor.
    0, // Don't force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS, // The time format we would like to accept into setposition/getposition.
    &fcopen, // Open callback.
    &fcclose, // Close callback.
    &fcread, // Read callback.
    0,
    // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &fcsetposition, // Setposition callback.
    0,
    // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    0 // Sound create callback (don't need it)
};

class fcplugin
{
    FMOD_CODEC_STATE* _codec;

public:
    fcplugin(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        memset(&fcwaveformat, 0, sizeof(fcwaveformat));
    }

    ~fcplugin()
    {
        mdx_close(&data);
        //delete some stuff
    }

    t_mdxmini data;
    Info* info;

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
    return &fccodec;
}

#ifdef __cplusplus
}
#endif
FMOD_RESULT F_CALLBACK fcopen(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo)
{
    FMOD_RESULT result;

    fcplugin* fc = new fcplugin(codec);
    fc->info = (Info*)userexinfo->userdata;

    unsigned int filesize;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);

    if (filesize == 4294967295) //stream
    {
        return FMOD_ERR_FORMAT;
    }
    //I don't know if there are some kind of magic bytes to be sure it is an mdx file
    //And since it sometimes tries to play m4a-files, I return false if the size seems too big
    //Biggest mdx file on modland is 85kb
    if (filesize > 102400) //100kb
    {
        return FMOD_ERR_FORMAT;
    }

    int found = fc->info->filename.find_last_of("/\\");

    int success = mdx_open(&fc->data, const_cast<char*>(fc->info->filename.c_str()),
                           const_cast<char*>(fc->info->filename.substr(0, found).c_str()));

    if (success < 0)
    {
        return FMOD_ERR_FORMAT;
    }


    fc->fcwaveformat.format = FMOD_SOUND_FORMAT_PCM16;
    fc->fcwaveformat.channels = 2;
    fc->fcwaveformat.frequency = 44100;
    fc->fcwaveformat.pcmblocksize = (16 >> 3) * fc->fcwaveformat.channels;
    fc->fcwaveformat.lengthpcm = mdx_get_length(&fc->data) * fc->fcwaveformat.frequency;

    codec->waveformat = &(fc->fcwaveformat);
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = fc; /* user data value */

    fc->info->numChannels = mdx_get_tracks(&fc->data);
    mdx_set_rate(fc->fcwaveformat.frequency);
    char title[MDX_MAX_TITLE_LENGTH];

    mdx_get_title(&fc->data, title);
    fc->info->title = title;
    fc->info->fileformat = "MDX";
    fc->info->plugin = PLUGIN_mdxmini;
    fc->info->pluginName = PLUGIN_mdxmini_NAME;


    fc->info->setSeekable(false);

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fcclose(FMOD_CODEC_STATE* codec)
{
    delete (fcplugin*)codec->plugindata;
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fcread(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    fcplugin* fc = (fcplugin*)codec->plugindata;
    mdx_calc_sample(&fc->data, (short*)buffer, size);
    *read = size;

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fcsetposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position,
                                     FMOD_TIMEUNIT postype)
{
    fcplugin* fc = (fcplugin*)codec->plugindata;

    int success = mdx_open(&fc->data, const_cast<char*>(fc->info->filename.c_str()), 0);
    return FMOD_OK;
}
