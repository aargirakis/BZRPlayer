#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <attributes.h>
#include <binary/container_factories.h>
#include <core/data_location.h>
#include <core/service.h>
#include <error.h>
#include <parameters/container.h>
#include <module/holder.h>
#include <module/track_information.h>
#include <module/track_state.h>
#include <module/players/pipeline.h>
#include "info.h"
#include "fmod_errors.h"
#include "plugins.h"

using namespace std;

Binary::Container::Ptr CreateData(FMOD_CODEC_STATE *codec) {
    unsigned int filesize;
    unsigned int bytesread;
    FMOD_CODEC_FILE_SIZE(codec, &filesize);
    if (filesize == 4294967295) //stream
    {
        return nullptr;
    }

    //rewind file pointer
    FMOD_RESULT result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);

    if (result != FMOD_OK) {
        return nullptr;
    }

    /* Allocate space for buffer. */
    auto myBuffer = std::make_unique<Binary::Dump>(filesize);

    //read whole file to memory
    result = FMOD_CODEC_FILE_READ(codec, myBuffer->data(), filesize, &bytesread);
    if (result != FMOD_OK) {
        return nullptr;
    }
    myBuffer->resize(bytesread);
    return Binary::CreateContainer(std::move(myBuffer));
}

const ZXTune::Service &GetService() {
    static const auto service = ZXTune::Service::Create(Parameters::Container::Create());
    return *service;
}

static FMOD_RESULT F_CALL open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);
static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec);
static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read);
static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_zxtune_NAME, // Name.
    0x00009000, // Version 0xAAAABBBB   A = major, B = minor.
    1, // Force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS | FMOD_TIMEUNIT_MS_REAL, // The time format we would like to accept into setposition/getposition.
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

class pluginZxtune
{
    FMOD_CODEC_STATE* _codec;

public:
    pluginZxtune(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginZxtune()
    {
        //delete some stuff
    }

    Module::Renderer::Ptr renderer;
    FMOD_CODEC_WAVEFORMAT waveformat;
    Sound::Chunk chunk;
    unsigned int chunkSamplesBuffered = 0;
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
    try {
        const auto dataContainer = CreateData(codec);

        if (!dataContainer) {
            return FMOD_ERR_FORMAT;
        }

        const auto openedModule = GetService().OpenModule(dataContainer, "", Parameters::Container::Create());

        auto *plugin = new pluginZxtune(codec);

        plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
        plugin->waveformat.channels = 2;
        plugin->waveformat.frequency = 44100;
        plugin->waveformat.pcmblocksize = plugin->waveformat.format * plugin->waveformat.channels;

        const auto moduleInfo = openedModule->GetModuleInformation();

        const auto durationMs = moduleInfo->Duration().Get();
        plugin->waveformat.lengthpcm = durationMs / 1000 * plugin->waveformat.frequency;

        codec->waveformat = &(plugin->waveformat);
        codec->numsubsounds = 0;
        /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
        codec->plugindata = plugin; /* user data value */

        // handle plugin preferences here
        const auto params = Parameters::Container::Create();

        plugin->renderer = CreatePipelinedRenderer(*openedModule, params);

        const Parameters::Accessor::Ptr moduleProperties = openedModule->GetModuleProperties();

        auto *info = static_cast<Info *>(userexinfo->userdata);

        if (const auto trackState = std::dynamic_pointer_cast<const Module::TrackState>(plugin->renderer->GetState())) {
            // TODO gather at-the-moment-state data for visualizer and tracker view in the read callback
            // TODO along with: ATTR_CURRENT_POSITION, ATTR_CURRENT_PATTERN, ATTR_CURRENT_LINE
        }

        if (const auto *trackInfo = dynamic_cast<const Module::TrackInformation *>(moduleInfo.get())) {
            info->numChannels = trackInfo->ChannelsCount();
            info->loopPosition = trackInfo->LoopPosition();
        }

        Parameters::FindValue(*moduleProperties, Module::ATTR_AUTHOR, info->author);
        Parameters::FindValue(*moduleProperties, Module::ATTR_COMMENT, info->comments);
        Parameters::FindValue(*moduleProperties, Module::ATTR_DATE, info->date);
        Parameters::FindValue(*moduleProperties, Module::ATTR_TITLE, info->title);

        std::string type;
        Parameters::FindValue(*moduleProperties, Module::ATTR_TYPE, type);

        if (type == "MTC") {
            info->fileformat = "Multitrack Container";
        } else if (type == "PSG") {
            info->fileformat = "Programmable Sound Generator";
        } else if (type == "TFD") {
            info->fileformat = "TurboFM Dumped";
        } else {
            Parameters::FindValue(*moduleProperties, Module::ATTR_PROGRAM, info->fileformat);
            if (info->fileformat.empty()) {
                info->fileformat = type;
            }
        }

        Parameters::FindValue(*moduleProperties, Module::ATTR_PLATFORM, info->system);

        if (info->system.empty()) {
            Parameters::FindValue(*moduleProperties, Module::ATTR_COMPUTER, info->system);
        }

        std::string samples;
        Parameters::FindValue(*moduleProperties, Module::ATTR_STRINGS, samples);

        if (!samples.empty()) {

            std::vector<std::string> samplesVector;
            std::stringstream ss(samples);
            std::string sampleName;

            while (std::getline(ss, sampleName)) {
                samplesVector.push_back(sampleName);
            }

            const auto samplesCount = static_cast<int>(samplesVector.size());

            info->numSamples = samplesCount;
            info->samples = new std::string[samplesCount]; // Alloca array statico

            for (int i = 0; i < samplesCount; ++i) {
                info->samples[i] = samplesVector[i];
            }
        }

        info->plugin = PLUGIN_zxtune;
        info->pluginName = PLUGIN_zxtune_NAME;
        info->setSeekable(true);

        return FMOD_OK;
    } catch (const Error &error) {
        cout << "ZXTune: " << error.ToString() << endl;
        return FMOD_ERR_FORMAT;
    }
}

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE* codec)
{
    delete static_cast<pluginZxtune *>(codec->plugindata);

    return FMOD_OK;
}

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read)
{
    auto *plugin = static_cast<pluginZxtune *>(codec->plugindata);

    if (plugin->chunkSamplesBuffered == plugin->chunk.size()) {
        plugin->chunkSamplesBuffered = 0;
        plugin->chunk = plugin->renderer->Render();

        if (plugin->chunk.empty()) {
            return FMOD_ERR_FILE_EOF;
        }
    }

    const auto chunkSamplesLeft = static_cast<unsigned int>(plugin->chunk.size()) - plugin->chunkSamplesBuffered;
    const auto chunkSamplesToBuffer = std::min(chunkSamplesLeft, size);

    std::memcpy(buffer, plugin->chunk.data() + plugin->chunkSamplesBuffered,
                chunkSamplesToBuffer * sizeof(Sound::Sample));

    plugin->chunkSamplesBuffered += chunkSamplesToBuffer;
    *read = chunkSamplesToBuffer;
    return FMOD_OK;
}

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    const auto plugin = static_cast<pluginZxtune *>(codec->plugindata);
    plugin->renderer->SetPosition(Time::Instant<Time::Millisecond>(position));

    return FMOD_OK;
}
