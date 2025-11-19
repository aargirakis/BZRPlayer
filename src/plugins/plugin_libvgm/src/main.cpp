#include <cstring>
#include <format>
#include <emu/SoundDevs.h>
#include <emu/SoundEmu.h>
#include <player/droplayer.hpp>
#include <player/gymplayer.hpp>
#include <player/s98player.hpp>
#include <player/vgmplayer.hpp>
#include <player/playera.hpp>
#include <utils/MemoryLoader.h>
#include "yrw801.h"
#include "fmod_errors.h"
#include "info.h"
#include "plugins.h"

using namespace std;

const vector<string> allowedFieldsDro = {"CHIPS"};
const vector<string> allowedFieldsGym = {"TITLE", "GAME", "CHIPS", "EMULATOR", "PUBLISHER", "ENCODED_BY", "COMMENT"};
const vector<string> allowedFieldsS98 = {
    "TITLE", "ARTIST", "GAME", "SYSTEM", "CHIPS", "GENRE", "DATE", "COPYRIGHT", "ENCODED_BY", "COMMENT",
};
const vector<string> allowedFieldsVgm = {"TITLE", "ARTIST", "GAME", "SYSTEM", "CHIPS", "DATE", "ENCODED_BY", "COMMENT"};

static FMOD_RESULT F_CALL open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);
static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec);
static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read);
static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_libvgm_NAME, // Name.
    0x00012300, // Version 0xAAAABBBB   A = major, B = minor.
    1, // Force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS, // The time format we would like to accept into setposition/getposition.
    &open, // Open callback.
    &close, // Close callback.
    &read, // Read callback.
    nullptr,
    // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &setPosition, // Setposition callback.
    nullptr,
    // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    nullptr // Sound create callback (don't need it)
};

class pluginVgmplayLegacy
{
    FMOD_CODEC_STATE *_codec;

public:
    pluginVgmplayLegacy(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginVgmplayLegacy()
    {
        DataLoader_Deinit(loader);
        delete [] myBuffer;
    }

    FMOD_CODEC_WAVEFORMAT waveformat;
    uint8_t *myBuffer;
    DATA_LOADER *loader;
    PlayerA *mainPlr;
};

/*
    FMODGetCodecDescription is mandatory for every fmod plugin.  This is the symbol the registerplugin function searches for.
    Must be declared with F_API to make it export as stdcall.
    MUST BE EXTERN'ED AS C!  C++ functions will be mangled incorrectly and not load in fmod.
*/
#ifdef __cplusplus
extern "C" {
#endif

F_EXPORT FMOD_CODEC_DESCRIPTION* F_CALL FMODGetCodecDescription()
{
    return &codecDescription;
}

#ifdef __cplusplus
}
#endif

static FMOD_RESULT F_CALL open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo)
{
    unsigned int filesize;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);

    auto *plugin = new pluginVgmplayLegacy(codec);
    auto *info = static_cast<Info *>(userexinfo->userdata);
    plugin->myBuffer = new uint8_t[filesize];

    auto result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);
    result = FMOD_CODEC_FILE_READ(codec, plugin->myBuffer, filesize, nullptr);

    plugin->loader = MemoryLoader_Init(plugin->myBuffer, filesize);

    if (plugin->loader == nullptr) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    if (DataLoader_Load(plugin->loader)) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.channels = 2;
    plugin->waveformat.frequency = 44100;
    plugin->waveformat.pcmblocksize = plugin->waveformat.format * plugin->waveformat.channels;

    codec->waveformat = &plugin->waveformat;
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = plugin; /* user data value */

    plugin->mainPlr = new PlayerA();
    plugin->mainPlr->RegisterPlayerEngine(new DROPlayer);
    plugin->mainPlr->RegisterPlayerEngine(new GYMPlayer);
    plugin->mainPlr->RegisterPlayerEngine(new S98Player);
    plugin->mainPlr->RegisterPlayerEngine(new VGMPlayer);

    plugin->mainPlr->SetFileReqCallback([](void *, PlayerBase *, const char *fileName) {
        DATA_LOADER *loader = nullptr;

        if (strcmp(fileName, "yrw801.rom") == 0) {
            loader = MemoryLoader_Init(yrw801_rom, sizeof(yrw801_rom));
        }

        if (DataLoader_Load(loader)) {
            DataLoader_Deinit(loader);
        }

        return loader;
    }, nullptr);

    if (plugin->mainPlr->SetOutputSettings(plugin->waveformat.frequency,
                                           static_cast<UINT8>(plugin->waveformat.channels),
                                           static_cast<UINT8>(8 * plugin->waveformat.format),
                                           plugin->waveformat.frequency / 100)) {
        return FMOD_ERR_FORMAT;
    }

    PlayerA::Config pCfg = plugin->mainPlr->GetConfiguration();
    pCfg.masterVol = 0x10000;
    pCfg.loopCount = 1;
    pCfg.fadeSmpls = 0;
    pCfg.endSilenceSmpls = 0;
    pCfg.pbSpeed = 1.0;
    plugin->mainPlr->SetConfiguration(pCfg);

    if (plugin->mainPlr->LoadFile(plugin->loader)) {
        return FMOD_ERR_FORMAT;
    }

    if (const auto length = plugin->mainPlr->GetTotalTime(PLAYTIME_LOOP_INCL | PLAYTIME_TIME_FILE); length != 0) {
        plugin->waveformat.lengthpcm = static_cast<unsigned int>(length * plugin->waveformat.frequency);
    } else {
        plugin->waveformat.lengthpcm = -1;
    }

    plugin->mainPlr->Start();

    PlayerBase *player = plugin->mainPlr->GetPlayer();

    vector<PLR_DEV_INFO> deviceInfoList;
    player->GetSongDeviceInfo(deviceInfoList);

    vector<string> devices;

    for (const auto &deviceInfo: deviceInfoList) {
        if (deviceInfo.type == DEVID_SN76496 &&
            deviceInfo.devCfg->flags & 0x01 &&
            deviceInfo.instance & 0x01) {
            continue; // the T6W28 consists of two "half" chips in VGMs
        }

        string device = SndEmu_GetDevName(deviceInfo.type, 0x01, deviceInfo.devCfg);

        if (deviceInfo.core != 0) {
            string core(5, '\0');
            core[0] = static_cast<char>(deviceInfo.core >> 24 & 0xFF);
            core[1] = static_cast<char>(deviceInfo.core >> 16 & 0xFF);
            core[2] = static_cast<char>(deviceInfo.core >> 8 & 0xFF);
            core[3] = static_cast<char>(deviceInfo.core >> 0 & 0xFF);
            core.resize(core.find('\0'));
            device += " (" + core + ")";
        }

        devices.push_back(device);
    }

    vector<pair<int, string> > instancesPerDevices;

    for (const auto &currentDevice: devices) {
        bool found = false;

        for (auto &[instances, device]: instancesPerDevices) {
            if (device == currentDevice) {
                ++instances;
                found = true;
                break;
            }
        }

        if (!found) {
            instancesPerDevices.emplace_back(1, currentDevice);
        }
    }

    for (int i = 0; i < instancesPerDevices.size(); i++) {
        if (i != 0) {
            info->chips += ", ";
        }

        if (instancesPerDevices[i].first > 1) {
            info->chips += format("{}x ", instancesPerDevices[i].first);
        }

        info->chips += instancesPerDevices[i].second;
    }

    PLR_SONG_INFO songInfo{};
    player->GetSongInfo(songInfo);

    if (player->GetPlayerType() == FCC_DRO) {
        const auto *drohdr = dynamic_cast<DROPlayer *>(player)->GetFileHeader();

        string hwType;

        if (drohdr->hwType == 0)
            hwType = "OPL2";
        else if (drohdr->hwType == 1)
            hwType = "DualOPL2";
        else if (drohdr->hwType == 2)
            hwType = "OPL3";

        if (!hwType.empty()) {
            if (info->chips.empty()) {
                info->chips = hwType;
            } else {
                info->chips += " [" + hwType + "]";
            }
        }

        info->fileformat = "DOSBox Raw OPL v";

        if (songInfo.fileVerMin == 0) {
            info->fileformat += format("{}", songInfo.fileVerMaj);
        } else {
            info->fileformat += format("{}.{}", songInfo.fileVerMaj, songInfo.fileVerMin);
        }

        info->allowedFields = &allowedFieldsDro;
    } else if (player->GetPlayerType() == FCC_GYM) {
        info->fileformat = "Genesis YM2612";
        info->allowedFields = &allowedFieldsGym;
    } else if (player->GetPlayerType() == FCC_S98) {
        info->fileformat = format("S98 v{}", songInfo.fileVerMaj);
        info->allowedFields = &allowedFieldsS98;
    } else if (player->GetPlayerType() == FCC_VGM) {
        info->fileformat = format("Video Game Music v{:x}.{:02x}", songInfo.fileVerMaj, songInfo.fileVerMin);
        info->allowedFields = &allowedFieldsVgm;
    }

    const char *const *tags = player->GetTags();
    for (const char *const *t = tags; *t; t += 2) {
        if (!strcmp(t[0], "ARTIST"))
            info->artist = t[1];
        else if (!strcmp(t[0], "COMMENT"))
            info->comments = t[1];
        else if (!strcmp(t[0], "COPYRIGHT") || !strcmp(t[0], "PUBLISHER"))
            info->copyright = t[1];
        else if (!strcmp(t[0], "DATE"))
            info->date = t[1];
        else if (!strcmp(t[0], "EMULATOR"))
            info->emulator = t[1];
        else if (!strcmp(t[0], "ENCODED_BY"))
            info->ripper = t[1];
        else if (!strcmp(t[0], "GAME"))
            info->game = t[1];
        else if (!strcmp(t[0], "GENRE"))
            info->genre = t[1];
        else if (!strcmp(t[0], "SYSTEM"))
            info->system = t[1];
        else if (!strcmp(t[0], "TITLE"))
            info->title = t[1];
    }

    info->plugin = PLUGIN_libvgm;
    info->pluginName = PLUGIN_libvgm_NAME;
    info->setSeekable(true);

    return FMOD_OK;
}

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE* codec)
{
    delete static_cast<pluginVgmplayLegacy *>(codec->plugindata);
    return FMOD_OK;
}

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read) {
    const auto *plugin = static_cast<pluginVgmplayLegacy *>(codec->plugindata);

    if (plugin->waveformat.lengthpcm == -1 && plugin->mainPlr->GetState() & PLAYSTATE_END) {
        return FMOD_ERR_FILE_EOF;
    }

    const UINT32 renderedBytes = plugin->mainPlr->Render(size, buffer);
    *read = renderedBytes / plugin->waveformat.pcmblocksize;
    return FMOD_OK;
}

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    const auto *plugin = static_cast<pluginVgmplayLegacy *>(codec->plugindata);

    if (postype == FMOD_TIMEUNIT_MS) {
        plugin->mainPlr->Seek(PLAYPOS_SAMPLE, position * plugin->waveformat.frequency / 1000);
    } else {
        return FMOD_ERR_UNSUPPORTED;
    }
    return FMOD_OK;
}
