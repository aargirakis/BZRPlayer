#include <cstring>
#include "libsc68/sc68/sc68.h"
#include "file68/sc68/rsc68.h"
#include "fmod_errors.h"
#include "info.h"
#include "plugins.h"

static FMOD_RESULT F_CALL open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec);

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read);

static FMOD_RESULT F_CALL getLength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype);

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_sc68_NAME, // name.
    0x00012000, // version 0xAAAABBBB   A = major, B = minor.
    0, // whether or not force everything using this codec to be a stream
    // the time formats we would like to accept into setposition/getposition
    FMOD_TIMEUNIT_MS,
    &open, // open callback
    &close, // close callback.
    &read, // read callback
    // getlength callback (If not specified FMOD returns the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure)
    &getLength,
    &setPosition, // setposition callback
    // getposition callback (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES)
    nullptr,
    nullptr, // sound create callback (don't need it)
    nullptr // getwaveformat
};

class pluginSc68 {
    FMOD_CODEC_STATE *_codec;

public:
    pluginSc68(FMOD_CODEC_STATE *codec) {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginSc68() {
        // delete some stuff

        if (sc68) {
            sc68_destroy(sc68);
        }

        sc68_shutdown();
    }

    FMOD_CODEC_WAVEFORMAT waveformat;
    sc68_t *sc68 = nullptr;
    sc68_init_t init68 = {};
    sc68_create_t create68 = {};
    sc68_music_info_t info;
};

/*
    FMODGetCodecDescription is mandatory for every fmod plugin. This is the symbol the registerplugin function searches for.
    Must be declared with F_API to make it export as stdcall.
    MUST BE EXTERN'ED AS C! C++ functions will be mangled incorrectly and not load in fmod.
*/
#ifdef __cplusplus
extern "C" {
#endif

F_EXPORT FMOD_CODEC_DESCRIPTION * F_CALL FMODGetCodecDescription() {
    return &codecDescription;
}

#ifdef __cplusplus
}
#endif

static FMOD_RESULT F_CALL open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo) {
    unsigned int bytesread;
    auto *s = new uint8_t[16];

    FMOD_RESULT result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    result = FMOD_CODEC_FILE_READ(codec, s, 16, &bytesread);

    const auto plugin = new pluginSc68(codec);

    bool isSC68 = false;
    bool isValidFile = false;
    if (memcmp(s, "SC68", 4) == 0) {
        isSC68 = true;
        isValidFile = true;
    } else if (memcmp(s, "ICE!", 4) == 0 || memcmp(s, "SNDH", 4) == 0) {
        isSC68 = false;
        isValidFile = true;
    }

    delete[] s;

    if (!isValidFile) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    unsigned int filesize;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);

    // allocate space for buffer
    auto *myBuffer = new uint8_t[filesize];

    // rewind file pointer
    result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);

    // read whole file to memory
    result = FMOD_CODEC_FILE_READ(codec, myBuffer, filesize, &bytesread);

    sc68_init(&plugin->init68);

    const auto info = static_cast<Info *>(userexinfo->userdata);

    const string path = info->dataPath + SC68_DATA_DIR;
    rsc68_set_share(path.c_str());

    plugin->sc68 = sc68_create(&plugin->create68);

    if (sc68_load_mem(plugin->sc68, myBuffer, filesize) < 0) {
        delete[] myBuffer;
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    delete[] myBuffer;

    // setting loop = 1 in api68_seek function will make it work
    // but seeking is so slow (like slowly winding it up, audible so it's pretty useless
    //int* seek = 0;
    //api68_seek(plugin->sc68, position,seek);
    if (sc68_play(plugin->sc68, info->currentSubsong + 1, 0) < 0) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    // needed for grabbing the disk name: use plugin->info.album instead for newer sc68 versions
    if (sc68_music_info(plugin->sc68, &plugin->info, 0, nullptr) < 0) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    info->disk = plugin->info.title;

    if (sc68_music_info(plugin->sc68, &plugin->info, info->currentSubsong + 1, nullptr) < 0) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.channels = 2;
    plugin->waveformat.frequency = 44100;
    plugin->waveformat.pcmblocksize = plugin->waveformat.format * plugin->waveformat.channels;
    plugin->waveformat.lengthpcm = -1;

    codec->waveformat = &plugin->waveformat;
    codec->numsubsounds = 0;
    // number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds
    codec->plugindata = plugin; // user data value

    info->plugin = PLUGIN_sc68;
    info->pluginName = PLUGIN_sc68_NAME;

    if (isSC68) {
        info->fileFormat = "SC68";
    } else {
        info->fileFormat = "SNDH";
    }

    info->author = plugin->info.author;
    info->composer = plugin->info.composer;
    info->replay = plugin->info.replay;
    info->hardware = plugin->info.hwname;
    info->title = plugin->info.title;
    info->rate = static_cast<int>(plugin->info.rate);
    info->address = static_cast<int>(plugin->info.addr);
    info->converter = plugin->info.converter ? plugin->info.converter : "";
    info->ripper = plugin->info.ripper ? plugin->info.ripper : "";
    info->numSubsongs = plugin->info.tracks;
    info->setSeekable(false);

    return FMOD_OK;
}

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec) {
    delete static_cast<pluginSc68 *>(codec->plugindata);
    return FMOD_OK;
}

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read) {
    const auto *plugin = static_cast<pluginSc68 *>(codec->plugindata);

    if (sc68_process(plugin->sc68, buffer, static_cast<int>(plugin->waveformat.pcmblocksize)) == SC68_MIX_ERROR) {
        //cout << "FMOD_ERR_FORMAT play" << endl;
        //return FMOD_ERR_FORMAT;
    }

    *read = plugin->waveformat.pcmblocksize;
    return FMOD_OK;
}

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype) {
    if (postype == FMOD_TIMEUNIT_MS) {
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}

static FMOD_RESULT F_CALL getLength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype) {
    const auto *plugin = static_cast<pluginSc68 *>(codec->plugindata);

    if (lengthtype == FMOD_TIMEUNIT_MS_REAL) {
        if (plugin->info.time_ms > 0xfffff || plugin->info.time_ms == 0)
        // if length > 4.6 hours (or 0) then set it to unlimited, some songs report a ridiculous large time
        {
            *length = -1;
        } else {
            *length = plugin->info.time_ms;
        }

        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}
