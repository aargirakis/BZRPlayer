#include <algorithm>
#include "v2mplayer.h"
#include "v2mconv.h"
#include "sounddef.h"
#include "fmod_errors.h"
#include "info.h"
#include "plugins.h"

using namespace std;

static FMOD_RESULT F_CALL open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec);

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read);

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_v2m_player_NAME, // name.
    0x00010000, // version 0xAAAABBBB   A = major, B = minor.
    1, // whether or not force everything using this codec to be a stream
    // the time formats we would like to accept into setposition/getposition
    FMOD_TIMEUNIT_MS,
    &open, // open callback
    &close, // close callback.
    &read, // read callback
    // getlength callback (If not specified FMOD returns the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure)
    nullptr,
    &setPosition, // setposition callback
    // getposition callback (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES)
    nullptr,
    nullptr, // sound create callback (don't need it)
    nullptr // getwaveformat
};

class pluginV2mPlayer {
    FMOD_CODEC_STATE *_codec;

public:
    pluginV2mPlayer(FMOD_CODEC_STATE *codec) {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginV2mPlayer() {
        // delete some stuff

        player->Close();
        delete player;
        delete[] convertedSong;
    }

    V2MPlayer *player;
    uint8_t *convertedSong;
    FMOD_CODEC_WAVEFORMAT waveformat;
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
    auto *info = static_cast<Info *>(userexinfo->userdata);

    string filename_lowercase = info->filename;

    transform(filename_lowercase.begin(), filename_lowercase.end(), filename_lowercase.begin(), ::tolower);
    if (const string ext = filename_lowercase.substr(filename_lowercase.find_last_of('.') + 1);
        ext != "v2m" && ext != "v2") {
        return FMOD_ERR_FORMAT;
    }

    unsigned int filesize;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);
    // allocate space for buffer
    auto *myBuffer = new uint8_t[filesize];

    // rewind file pointer
    FMOD_RESULT result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);

    // read whole file to memory
    unsigned int bytesread;
    result = FMOD_CODEC_FILE_READ(codec, myBuffer, filesize, &bytesread);

    sdInit();
    if (ssbase base{}; CheckV2MVersion(myBuffer, filesize, base) < 0) {
        delete [] myBuffer;
        return FMOD_ERR_FORMAT;
    }

    auto *plugin = new pluginV2mPlayer(codec);

    int converted_length;
    ConvertV2M(myBuffer, filesize, &plugin->convertedSong, &converted_length);

    delete [] myBuffer;

    plugin->player = new V2MPlayer();
    plugin->player->Init();
    if (!plugin->player->Open(plugin->convertedSong)) {
        return FMOD_ERR_FORMAT;
    }

    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCMFLOAT;
    plugin->waveformat.channels = 2;
    plugin->waveformat.frequency = 44100;
    plugin->waveformat.pcmblocksize = 4096;
    plugin->waveformat.lengthpcm = -1;

    codec->waveformat = &plugin->waveformat;
    codec->numsubsounds = 0;
    // number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds
    codec->plugindata = plugin; // user data value

    info->fileFormat = "Farbrausch V2M";
    info->setSeekable(true);
    info->plugin = PLUGIN_v2m_player;
    info->pluginName = PLUGIN_v2m_player_NAME;

    sS32 *p;
    const uint32_t pos = plugin->player->CalcPositions(&p);

    unsigned int length;
    if (pos % 2 == 0) {
        length = p[pos];
    } else {
        length = p[pos - 1];
    }

    delete[] p;

    // add one extra second for reverb
    plugin->waveformat.lengthpcm = static_cast<unsigned int>(
        (1000 + length) * 2 / 1000.0 * plugin->waveformat.frequency);

    return FMOD_OK;
}

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec) {
    const auto *plugin = static_cast<pluginV2mPlayer *>(codec->plugindata);

    if (plugin) {
        plugin->player->Stop();
        plugin->player->Close();
    }

    delete plugin;

    return FMOD_OK;
}

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read) {
    const auto *plugin = static_cast<pluginV2mPlayer *>(codec->plugindata);

    plugin->player->Render(static_cast<float *>(buffer), plugin->waveformat.pcmblocksize);

    if (size < plugin->waveformat.pcmblocksize) {
        *read = size;
    } else {
        *read = plugin->waveformat.pcmblocksize;
    }

    return FMOD_OK;
}

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype) {
    const auto *plugin = static_cast<pluginV2mPlayer *>(codec->plugindata);

    if (postype == FMOD_TIMEUNIT_MS) {
        plugin->player->Play(position);
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}
