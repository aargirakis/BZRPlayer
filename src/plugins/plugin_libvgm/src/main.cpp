#include <algorithm>
#include <cstring>
#include <format>
#include <fstream>
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

static FMOD_RESULT F_CALL getLength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype);

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_libvgm_NAME, // name.
    0x00012300, // version 0xAAAABBBB   A = major, B = minor.
    1, // whether or not force everything using this codec to be a stream
    // the time formats we would like to accept into setposition/getposition
    FMOD_TIMEUNIT_MS | FMOD_TIMEUNIT_MUTE_VOICE,
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

class pluginLibvgm {
    FMOD_CODEC_STATE *_codec;

public:
    pluginLibvgm(FMOD_CODEC_STATE *codec) {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginLibvgm() {
        DataLoader_Deinit(loader);
    }

    FMOD_CODEC_WAVEFORMAT waveformat;
    Info *info;
    DATA_LOADER *loader;
    PlayerA *mainPlr;
    PlayerBase *player;
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
    auto *plugin = new pluginLibvgm(codec);
    plugin->info = static_cast<Info *>(userexinfo->userdata);

    plugin->loader = MemoryLoader_Init(plugin->info->fileBuffer, static_cast<UINT32>(plugin->info->filesize));

    if (plugin->loader == nullptr) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    if (DataLoader_Load(plugin->loader)) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    plugin->mainPlr = new PlayerA();
    plugin->mainPlr->RegisterPlayerEngine(new DROPlayer);
    plugin->mainPlr->RegisterPlayerEngine(new GYMPlayer);
    plugin->mainPlr->RegisterPlayerEngine(new S98Player);
    plugin->mainPlr->RegisterPlayerEngine(new VGMPlayer);

    plugin->mainPlr->SetFileReqCallback([](void *, PlayerBase *, const char *filename) {
        DATA_LOADER *loader = nullptr;

        if (strcmp(filename, "yrw801.rom") == 0) {
            loader = MemoryLoader_Init(yrw801_rom, sizeof(yrw801_rom));
        }

        if (DataLoader_Load(loader)) {
            DataLoader_Deinit(loader);
        }

        return loader;
    }, nullptr);

    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.channels = 2;
    plugin->waveformat.frequency = 44100;
    plugin->waveformat.pcmblocksize = plugin->waveformat.format * plugin->waveformat.channels;
    plugin->waveformat.lengthpcm = -1;

    codec->waveformat = &plugin->waveformat;
    codec->numsubsounds = 0;
    // number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds
    codec->plugindata = plugin; // user data value

    if (plugin->mainPlr->SetOutputSettings(plugin->waveformat.frequency,
                                           static_cast<UINT8>(plugin->waveformat.channels),
                                           static_cast<UINT8>(8 * plugin->waveformat.format),
                                           plugin->waveformat.frequency / 100)) {
        return FMOD_ERR_FORMAT;
    }

    string filename = plugin->info->userPath + PLUGINS_CONFIG_DIR + "/libvgm.cfg";
    ifstream ifs(filename.c_str());
    bool useDefaults = false;

    if (ifs.fail()) {
        // the file could not be opened
        useDefaults = true;
    }

    // defaults
    plugin->info->isContinuousPlaybackActive = false;

    if (!useDefaults) {
        string line;
        while (getline(ifs, line)) {
            if (int i = line.find_first_of("="); i != -1) {
                string word = line.substr(0, i);
                string value = line.substr(i + 1);
                if (word == "continuousPlayback") {
                    plugin->info->isContinuousPlaybackActive =
                            plugin->info->isPlayModeRepeatSongEnabled && value == "true";
                }
            }
        }
        ifs.close();
    }

    PlayerA::Config pCfg = plugin->mainPlr->GetConfiguration();
    pCfg.masterVol = 0x10000;
    pCfg.loopCount = plugin->info->isContinuousPlaybackActive ? 0 : 1;
    pCfg.fadeSmpls = 0;
    pCfg.endSilenceSmpls = 0;
    pCfg.pbSpeed = 1.0;
    plugin->mainPlr->SetConfiguration(pCfg);

    if (plugin->mainPlr->LoadFile(plugin->loader)) {
        return FMOD_ERR_FORMAT;
    }

    plugin->mainPlr->Start();

    plugin->player = plugin->mainPlr->GetPlayer();

    vector<PLR_DEV_INFO> deviceInfoList;
    plugin->player->GetSongDeviceInfo(deviceInfoList);

    unsigned int channels = 0;

    for (const auto &deviceInfo: deviceInfoList) {
        channels += deviceInfo.devDecl->channelCount(deviceInfo.devCfg);
    }

    plugin->info->numChannels = channels;

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
            plugin->info->chips += ", ";
        }

        if (instancesPerDevices[i].first > 1) {
            plugin->info->chips += format("{}x ", instancesPerDevices[i].first);
        }

        plugin->info->chips += instancesPerDevices[i].second;
    }

    PLR_SONG_INFO songInfo{};
    plugin->player->GetSongInfo(songInfo);

    if (plugin->player->GetPlayerType() == FCC_DRO) {
        const auto *drohdr = dynamic_cast<DROPlayer *>(plugin->player)->GetFileHeader();

        string hwType;

        if (drohdr->hwType == 0)
            hwType = "OPL2";
        else if (drohdr->hwType == 1)
            hwType = "DualOPL2";
        else if (drohdr->hwType == 2)
            hwType = "OPL3";

        if (!hwType.empty()) {
            if (plugin->info->chips.empty()) {
                plugin->info->chips = hwType;
            } else {
                plugin->info->chips += " [" + hwType + "]";
            }
        }

        plugin->info->fileFormat = "DOSBox Raw OPL v";

        if (songInfo.fileVerMin == 0) {
            plugin->info->fileFormat += format("{}", songInfo.fileVerMaj);
        } else {
            plugin->info->fileFormat += format("{}.{}", songInfo.fileVerMaj, songInfo.fileVerMin);
        }

        plugin->info->allowedFields = &allowedFieldsDro;
    } else if (plugin->player->GetPlayerType() == FCC_GYM) {
        plugin->info->fileFormat = "Genesis YM2612";
        plugin->info->allowedFields = &allowedFieldsGym;
    } else if (plugin->player->GetPlayerType() == FCC_S98) {
        plugin->info->fileFormat = format("S98 v{}", songInfo.fileVerMaj);
        plugin->info->allowedFields = &allowedFieldsS98;
    } else if (plugin->player->GetPlayerType() == FCC_VGM) {
        plugin->info->fileFormat = format("Video Game Music v{:x}.{:02x}", songInfo.fileVerMaj, songInfo.fileVerMin);
        plugin->info->allowedFields = &allowedFieldsVgm;
    }

    const char *const *tags = plugin->player->GetTags();
    for (const char *const *t = tags; *t; t += 2) {
        if (!strcmp(t[0], "ARTIST"))
            plugin->info->artist = t[1];
        else if (!strcmp(t[0], "COMMENT"))
            plugin->info->comments = t[1];
        else if (!strcmp(t[0], "COPYRIGHT") || !strcmp(t[0], "PUBLISHER"))
            plugin->info->copyright = t[1];
        else if (!strcmp(t[0], "DATE"))
            plugin->info->date = t[1];
        else if (!strcmp(t[0], "EMULATOR"))
            plugin->info->emulator = t[1];
        else if (!strcmp(t[0], "ENCODED_BY"))
            plugin->info->ripper = t[1];
        else if (!strcmp(t[0], "GAME"))
            plugin->info->game = t[1];
        else if (!strcmp(t[0], "GENRE"))
            plugin->info->genre = t[1];
        else if (!strcmp(t[0], "SYSTEM"))
            plugin->info->system = t[1];
        else if (!strcmp(t[0], "TITLE"))
            plugin->info->title = t[1];
    }

    plugin->info->plugin = PLUGIN_libvgm;
    plugin->info->pluginName = PLUGIN_libvgm_NAME;
    plugin->info->setSeekable(true);

    return FMOD_OK;
}

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec) {
    delete static_cast<pluginLibvgm *>(codec->plugindata);
    return FMOD_OK;
}

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read) {
    const auto *plugin = static_cast<pluginLibvgm *>(codec->plugindata);

    if (plugin->mainPlr->GetState() & PLAYSTATE_END) {
        if (!plugin->info->isContinuousPlaybackActive) {
            return FMOD_ERR_FILE_EOF;
        }

        plugin->mainPlr->Seek(PLAYPOS_SAMPLE, 0);
    }

    const UINT32 renderedBytes = plugin->mainPlr->Render(size, buffer);
    *read = renderedBytes / plugin->waveformat.pcmblocksize;
    return FMOD_OK;
}

static FMOD_RESULT F_CALL getLength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype) {
    const auto *plugin = static_cast<pluginLibvgm *>(codec->plugindata);

    if (lengthtype == FMOD_TIMEUNIT_MS_REAL) {
        *length = static_cast<unsigned int>(plugin->mainPlr->GetTotalTime(PLAYTIME_LOOP_EXCL) * 1000.0);
        return FMOD_OK;
    }
    if (lengthtype == FMOD_TIMEUNIT_MUTE_VOICE) {
        *length = -1; // ignored
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype) {
    const auto *plugin = static_cast<pluginLibvgm *>(codec->plugindata);

    if (postype == FMOD_TIMEUNIT_MS) {
        plugin->mainPlr->Seek(PLAYPOS_SAMPLE, position * plugin->waveformat.frequency / 1000);
        return FMOD_OK;
    }

    if (postype == FMOD_TIMEUNIT_MUTE_VOICE) {
        auto overallMutedChannelsMask = plugin->info->mutedChannelsMask;

        for (char &bit: overallMutedChannelsMask) {
            bit = bit == '0' ? '1' : '0';
        }

        vector<PLR_DEV_INFO> deviceInfoList;
        plugin->player->GetSongDeviceInfo(deviceInfoList);

        for (const auto &deviceInfo: deviceInfoList) {
            const unsigned int deviceChannelsCount = deviceInfo.devDecl->channelCount(deviceInfo.devCfg);

            string deviceMutedChannelsMask = overallMutedChannelsMask.substr(0, deviceChannelsCount);

            ranges::reverse(deviceMutedChannelsMask);

            const auto deviceChannelsMask = static_cast<UINT32>(stoul(deviceMutedChannelsMask, nullptr, 2));

            PLR_MUTE_OPTS muteOpts = {};
            plugin->player->GetDeviceMuting(deviceInfo.id, muteOpts);

            muteOpts.chnMute[deviceInfo.parentIdx == -1 ? 0 : 1] = deviceChannelsMask;

            plugin->player->SetDeviceMuting(deviceInfo.id, muteOpts);

            overallMutedChannelsMask.erase(0, deviceChannelsCount);
        }

        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}
