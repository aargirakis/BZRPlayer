#include <filesystem>
#include <regex>
#include "attributes.h"
#include "binary/container_factories.h"
#include "core/data_location.h"
#include "core/service.h"
#include "error.h"
#include "module/track_information.h"
#include "module/track_state.h"
#include "module/players/pipeline.h"
#include "fmod_errors.h"
#include "info.h"
#include "plugins.h"

using namespace std;

static FMOD_RESULT F_CALL open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec);

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read);

static FMOD_RESULT F_CALL getLength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype);

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_zxtune_NAME, // Name.
    0x00009000, // Version 0xAAAABBBB   A = major, B = minor.
    1, // Force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS, // The time format we would like to accept into setposition/getposition.
    &open, // Open callback.
    &close, // Close callback.
    &read, // Read callback.
    &getLength,
    // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &setPosition, // Setposition callback.
    nullptr,
    // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    nullptr // Sound create callback (don't need it)
};

class pluginZxtune {
    FMOD_CODEC_STATE *_codec;

public:
    pluginZxtune(FMOD_CODEC_STATE *codec) {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginZxtune() {
        //delete some stuff
    }

    class ModulesDetector : public Module::DetectCallback {
        string filename;
        int numModules = 0;
        int currentModuleNum = 0;
        Module::Holder::Ptr currentModule;
        string containerFilenames;
        bool isContainer = false;

        Parameters::Container::Ptr CreateInitialProperties(const StringView subpath) const override {
            return Parameters::Container::Create();
        }

        void ProcessModule(const ZXTune::DataLocation &location, const ZXTune::Plugin &decoder,
                           Module::Holder::Ptr holder) override {
            if (numModules++ != currentModuleNum) {
                return;
            }

            containerFilenames = filesystem::path(filename).filename().string();

            if (const auto path = location.GetPath(); !path->Empty()) {
                isContainer = true;
                for (const auto &element: path->Elements()) {
                    if (!element.starts_with("+")) {
                        if (!element.empty()) {
                            containerFilenames += " > ";
                        }

                        containerFilenames += element;
                    }
                }
            }

            currentModule = holder;
        }

        Log::ProgressCallback *GetProgress() const override { return nullptr; }

    public:
        void setFilename(string_view const &_filename) {
            this->filename = _filename;
        }

        void setCurrentModuleNum(const int _currentModuleNum) {
            this->currentModuleNum = _currentModuleNum;
        }

        auto getCurrentModule() const {
            return currentModule;
        }

        auto getContainerFilenames() const {
            return containerFilenames;
        }

        auto getNumModules() const {
            return numModules;
        }

        auto isInContainer() const {
            return isContainer;
        }
    };

    static const ZXTune::Service &GetService() {
        static const auto service = ZXTune::Service::Create(Parameters::Container::Create());
        return *service;
    }

    FMOD_CODEC_WAVEFORMAT waveformat;
    ModulesDetector modulesDetector;
    Module::Renderer::Ptr renderer;
    Module::Information::Ptr moduleInfo;
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

F_EXPORT FMOD_CODEC_DESCRIPTION * F_CALL FMODGetCodecDescription() {
    return &codecDescription;
}

#ifdef __cplusplus
}
#endif

static FMOD_RESULT F_CALL open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo) {
    try {
        unsigned int filesize;
        unsigned int bytesread;
        FMOD_CODEC_FILE_SIZE(codec, &filesize);

        if (filesize == 4294967295) //stream
        {
            return FMOD_ERR_FORMAT;
        }

        //rewind file pointer
        FMOD_RESULT result = FMOD_CODEC_FILE_SEEK(codec, 0, 0);

        if (result != FMOD_OK) {
            return FMOD_ERR_FORMAT;
        }

        /* Allocate space for buffer. */
        auto myBuffer = make_unique<Binary::Dump>(filesize);

        //read whole file to memory
        result = FMOD_CODEC_FILE_READ(codec, myBuffer->data(), filesize, &bytesread);

        if (result != FMOD_OK) {
            return FMOD_ERR_FORMAT;
        }

        myBuffer->resize(bytesread);

        const auto fileDataContainer = Binary::CreateContainer(move(myBuffer));

        if (!fileDataContainer) {
            return FMOD_ERR_FORMAT;
        }

        auto *plugin = new pluginZxtune(codec);

        auto info = static_cast<Info *>(userexinfo->userdata);

        plugin->modulesDetector.setCurrentModuleNum(info->currentSubsong);
        plugin->modulesDetector.setFilename(info->filename);
        pluginZxtune::GetService().DetectModules(fileDataContainer, plugin->modulesDetector);

        if (plugin->modulesDetector.getNumModules() == 0) {
            delete plugin;
            return FMOD_ERR_FORMAT;
        }

        // handle plugin preferences here
        shared_ptr<Parameters::Container> soundParams = Parameters::Container::Create();

        plugin->renderer = CreatePipelinedRenderer(*plugin->modulesDetector.getCurrentModule(), soundParams);

        plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
        plugin->waveformat.channels = 2;
        plugin->waveformat.frequency = 44100;
        plugin->waveformat.pcmblocksize = plugin->waveformat.format * plugin->waveformat.channels;
        plugin->waveformat.lengthpcm = -1;

        codec->waveformat = &plugin->waveformat;
        codec->numsubsounds = 0;
        /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
        codec->plugindata = plugin; /* user data value */

        info->numSubsongs = plugin->modulesDetector.getNumModules();
        plugin->moduleInfo = plugin->modulesDetector.getCurrentModule()->GetModuleInformation();

        if (plugin->modulesDetector.isInContainer()) {
            info->containerFilenames = plugin->modulesDetector.getContainerFilenames();
        }

        if (const auto *trackInfo = dynamic_cast<const Module::TrackInformation *>(plugin->moduleInfo.get())) {
            info->numChannels = trackInfo->ChannelsCount();
            info->loopPosition = trackInfo->LoopPosition();
        }

        const Parameters::Accessor::Ptr moduleProperties = plugin->modulesDetector.getCurrentModule()->
                GetModuleProperties();

        string containerFormats = moduleProperties->FindString(Module::ATTR_CONTAINER).value_or("");
        containerFormats = regex_replace(containerFormats, regex(">"), " > ");
        info->containerFileformats = containerFormats;

        info->author = moduleProperties->FindString(Module::ATTR_AUTHOR).value_or("");
        info->comments = moduleProperties->FindString(Module::ATTR_COMMENT).value_or("");
        info->date = moduleProperties->FindString(Module::ATTR_DATE).value_or("");
        info->title = moduleProperties->FindString(Module::ATTR_TITLE).value_or(info->containerFilenames);

        if (const auto type = moduleProperties->FindString(Module::ATTR_TYPE).value_or("");
            type == "MTC") {
            info->fileformat = "Multitrack Container";
        } else if (type == "PSG") {
            info->fileformat = "Programmable Sound Generator";
        } else if (type == "TFD") {
            info->fileformat = "TurboFM Dumped";
        } else {
            if (const auto program = moduleProperties->FindString(Module::ATTR_PROGRAM).value_or("");
                !program.empty()) {
                info->fileformat = program;
            } else {
                info->fileformat = type;
            }
        }

        info->system = moduleProperties->FindString(Module::ATTR_PLATFORM).value_or("");

        if (info->system.empty()) {
            info->system = moduleProperties->FindString(Module::ATTR_COMPUTER).value_or("");
        }

        if (const auto samples = moduleProperties->FindString(Module::ATTR_STRINGS).value_or("");
            !samples.empty()) {
            vector<string> samplesVector;
            stringstream ss(samples);
            string sampleName;

            while (getline(ss, sampleName)) {
                samplesVector.push_back(sampleName);
            }

            const auto samplesCount = static_cast<int>(samplesVector.size());

            info->numSamples = samplesCount;
            info->samples = new string[samplesCount];

            for (int i = 0; i < samplesCount; ++i) {
                info->samples[i] = samplesVector[i];
            }
        }

        if (const auto trackState = dynamic_pointer_cast<const Module::TrackState>(plugin->renderer->GetState())) {
            // TODO gather at-the-moment-state data for visualizer and tracker view in the read callback
            // TODO along with: ATTR_CURRENT_POSITION, ATTR_CURRENT_PATTERN, ATTR_CURRENT_LINE
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

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec) {
    delete static_cast<pluginZxtune *>(codec->plugindata);
    return FMOD_OK;
}

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read) {
    auto *plugin = static_cast<pluginZxtune *>(codec->plugindata);

    if (plugin->chunkSamplesBuffered == plugin->chunk.size()) {
        plugin->chunkSamplesBuffered = 0;
        plugin->chunk = plugin->renderer->Render();

        if (plugin->chunk.empty()) {
            return FMOD_ERR_FILE_EOF;
        }
    }

    const auto chunkSamplesLeft = static_cast<unsigned int>(plugin->chunk.size() - plugin->chunkSamplesBuffered);
    const auto chunkSamplesToBuffer = min(chunkSamplesLeft, size);

    memcpy(buffer, plugin->chunk.data() + plugin->chunkSamplesBuffered,
           chunkSamplesToBuffer * sizeof(Sound::Sample));

    plugin->chunkSamplesBuffered += chunkSamplesToBuffer;
    *read = chunkSamplesToBuffer;
    return FMOD_OK;
}

static FMOD_RESULT F_CALL getLength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype) {
    const auto *plugin = static_cast<pluginZxtune *>(codec->plugindata);

    if (lengthtype == FMOD_TIMEUNIT_MS_REAL) {
        *length = plugin->moduleInfo->Duration().Get();
        return FMOD_OK;
    }

    return FMOD_ERR_FORMAT;
}

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype) {
    const auto plugin = static_cast<pluginZxtune *>(codec->plugindata);

    if (postype == FMOD_TIMEUNIT_MS) {
        plugin->renderer->SetPosition(Time::Instant<Time::Millisecond>(position));
        return FMOD_OK;
    }

    return FMOD_ERR_FORMAT;
}
