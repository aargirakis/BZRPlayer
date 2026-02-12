#include <algorithm>
#include <filesystem>
#include <format>
#include <fstream>

extern "C" {
#include "api_internal.h"
#include "coding.h"
}

#include "libcue.h"
#include "fmod_errors.h"
#include "../app/info.h"
#include "../app/plugins.h"

static FMOD_RESULT F_CALL open(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec);

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read);

static FMOD_RESULT F_CALL getLength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype);

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_vgmstream_NAME, // Name.
    0x00010000, // Version 0xAAAABBBB   A = major, B = minor.
    0, // Don't force everything using this codec to be a stream
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

class pluginVgmstream {
    FMOD_CODEC_STATE *_codec;

public:
    pluginVgmstream(FMOD_CODEC_STATE *codec) {
        _codec = codec;
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginVgmstream() {
        //delete some stuff
        libvgmstream_free(libvgmstream);
        libvgmstream = nullptr;
    }

    FMOD_CODEC_WAVEFORMAT waveformat;
    Info *info;
    libvgmstream_t *libvgmstream = nullptr;
    long trackStartFromCueSheet;
    long trackLengthFromCueSheet;
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
    auto *smallBuffer = new uint8_t[16];
    FMOD_CODEC_FILE_READ(codec, smallBuffer, 16, nullptr);

    /* skip gm.dls:
     * plugin_vgmstream has higher prio than FMOD_SOUND_TYPE_MIDI,
     * thus when plugin_vgmstream will fail on midi loading
     * FMOD will try to invoke plugin_vgmstream again for loading gm.dls too
    */
    if (memcmp(smallBuffer, "RIFF\x0c\x80" "4\0DLS colh", 16) == 0) {
        delete[] smallBuffer;
        return FMOD_ERR_FORMAT;
    }

    delete[] smallBuffer;

    unsigned int filesize;

    FMOD_CODEC_FILE_SIZE(codec, &filesize);

    auto *plugin = new pluginVgmstream(codec);
    plugin->info = static_cast<Info *>(userexinfo->userdata);

    pair<string, string> metadataCueCdTextAlbum;
    pair<string, string> metadataCueCdTextAlbumArtist;
    pair<string, string> metadataCueCdTextComposer;
    pair<string, string> metadataCueCdTextGenre;
    pair<string, string> metadataCueTrackTextTitle;
    pair<string, string> metadataCueTrackTextArtist;
    pair<string, string> metadataCueTrackTextComposer;
    pair<string, string> metadataCueTrackTextGenre;
    pair<string, string> metadataCueCdRemDate;
    pair<string, string> metadataCueCdRemDisc;
    pair<string, string> metadataCueCdRemComment;
    pair<string, string> metadataCueCdRemReplayGainAlbumGain;
    pair<string, string> metadataCueCdRemReplayGainAlbumPeak;
    pair<string, string> metadataCueTrackRemDate;
    pair<string, string> metadataCueTrackRemComment;
    pair<string, string> metadataCueTrackRemReplayGainTrackGain;
    pair<string, string> metadataCueTrackRemReplayGainTrackPeak;

    if (filesize <= 1024 * 500) {
        auto *myBuffer = new char[filesize];

        FMOD_CODEC_FILE_SEEK(codec, 0, 0);
        FMOD_CODEC_FILE_READ(codec, myBuffer, filesize, nullptr);

        if (const auto cd = cue_parse_string(myBuffer); cd) {
            if (const auto tracksCount = cd_get_ntrack(cd); tracksCount >= 1) {
                if (const auto currentTrack = cd_get_track(cd, plugin->info->currentSubsong + 1); currentTrack) {
                    if (const auto trackFilename = track_get_filename(currentTrack)) {
                        plugin->info->isTrackFromCueSheet = true;
                        plugin->info->cueSheetTrackFilename = trackFilename;
                        plugin->info->filename = filesystem::path(plugin->info->filename)
                                .replace_filename(trackFilename).string();
                        plugin->info->numSubsongs = tracksCount;
                        plugin->trackStartFromCueSheet = track_get_start(currentTrack);
                        plugin->trackLengthFromCueSheet = track_get_length(currentTrack);

                        if (Cdtext const *cdText = cd_get_cdtext(cd)) {
                            if (const char *s; (s = cdtext_get(PTI_TITLE, cdText))) {
                                metadataCueCdTextAlbum = pair("Album", s);
                            }
                            if (const char *s; (s = cdtext_get(PTI_PERFORMER, cdText))) {
                                metadataCueCdTextAlbumArtist = pair("Album Artist", s);
                            }
                            if (const char *s; (s = cdtext_get(PTI_COMPOSER, cdText))) {
                                metadataCueCdTextComposer = pair("Composer", s);
                            }
                            if (const char *s; (s = cdtext_get(PTI_GENRE, cdText))) {
                                metadataCueCdTextGenre = pair("Genre", s);
                            }
                        }

                        if (Cdtext const *trackText = track_get_cdtext(currentTrack)) {
                            const char *s;
                            if ((s = cdtext_get(PTI_TITLE, trackText))) {
                                metadataCueTrackTextTitle = pair("Title", s);
                            }
                            if ((s = cdtext_get(PTI_PERFORMER, trackText))) {
                                metadataCueTrackTextArtist = pair("Artist", s);
                            }
                            if ((s = cdtext_get(PTI_COMPOSER, trackText))) {
                                metadataCueTrackTextComposer = pair("Composer", s);
                            }
                            if ((s = cdtext_get(PTI_GENRE, trackText))) {
                                metadataCueTrackTextGenre = pair("Genre", s);
                            }
                        }

                        if (Rem *cdRem = cd_get_rem(cd)) {
                            if (const char *s; (s = rem_get(REM_DATE, cdRem))) {
                                metadataCueCdRemDate = pair("Date", s);
                            }
                            if (const char *s1; (s1 = rem_get(REM_DISCNUMBER, cdRem))) {
                                if (const char *s2; (s2 = rem_get(REM_TOTALDISCS, cdRem))) {
                                    metadataCueCdRemDisc = pair("Disc", format("{} / {}", s1, s2));
                                } else {
                                    metadataCueCdRemDisc = pair("Disc", s1);
                                }
                            }
                            if (const char *s; (s = rem_get(REM_COMMENT, cdRem))) {
                                metadataCueCdRemComment = pair("Comment", s);
                            }
                            if (const char *s; (s = rem_get(REM_REPLAYGAIN_ALBUM_GAIN, cdRem))) {
                                metadataCueCdRemReplayGainAlbumGain = pair("REPLAYGAIN_ALBUM_GAIN", format("{} dB", s));
                            }
                            if (const char *s; (s = rem_get(REM_REPLAYGAIN_ALBUM_PEAK, cdRem))) {
                                metadataCueCdRemReplayGainAlbumPeak = pair("REPLAYGAIN_ALBUM_PEAK", s);
                            }
                        }

                        if (Rem *trackRem = track_get_rem(currentTrack)) {
                            if (const char *s; (s = rem_get(REM_DATE, trackRem))) {
                                metadataCueTrackRemDate = pair("Date", s);
                            }
                            if (const char *s; (s = rem_get(REM_COMMENT, trackRem))) {
                                metadataCueTrackRemComment = pair("Comment", s);
                            }
                            if (const char *s; (s = rem_get(REM_REPLAYGAIN_TRACK_GAIN, trackRem))) {
                                metadataCueTrackRemReplayGainTrackGain = pair(
                                    "REPLAYGAIN_TRACK_GAIN", format("{} dB", s));
                            }
                            if (const char *s; (s = rem_get(REM_REPLAYGAIN_TRACK_PEAK, trackRem))) {
                                metadataCueTrackRemReplayGainTrackPeak = pair("REPLAYGAIN_TRACK_PEAK", s);
                            }
                        }
                    }
                }
            }
        }

        delete[] myBuffer;
    }

    plugin->libvgmstream = libvgmstream_init();

    if (!plugin->libvgmstream) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    string filename = plugin->info->userPath + PLUGINS_CONFIG_DIR + "/vgmstream.cfg";
    ifstream ifs(filename.c_str());
    bool useDefaults = false;

    if (ifs.fail()) {
        //The file could not be opened
        useDefaults = true;
    }

    //defaults
    plugin->info->isContinuousPlaybackActive = false;

    if (!useDefaults) {
        string line;
        while (getline(ifs, line)) {
            if (int i = line.find_first_of("="); i != -1) {
                string word = line.substr(0, i);
                string value = line.substr(i + 1);
                if (word == "continuous_playback") {
                    plugin->info->isContinuousPlaybackActive =
                            plugin->info->isPlayModeRepeatSongEnabled && value == "true";
                }
            }
        }
        ifs.close();
    }

    libvgmstream_config_t config = {};

    if (plugin->info->isContinuousPlaybackActive) {
        config.allow_play_forever = true;
        config.play_forever = true;
        config.force_loop = true;
    }

    libvgmstream_setup(plugin->libvgmstream, &config);

    auto libsf = libstreamfile_open_from_stdio(plugin->info->filename.c_str());
    if (!libsf) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    auto retval = libvgmstream_open_stream(plugin->libvgmstream, libsf,
                                           plugin->info->isTrackFromCueSheet ? 1 : plugin->info->currentSubsong + 1);

    libstreamfile_close(libsf);

    if (retval < 0) {
        delete plugin;
        return FMOD_ERR_FORMAT;
    }

    switch (plugin->libvgmstream->format->sample_format) {
        default:
        case LIBVGMSTREAM_SFMT_PCM16:
            plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
            break;
        case LIBVGMSTREAM_SFMT_PCM24:
            plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM24;
            break;
        case LIBVGMSTREAM_SFMT_PCM32:
            plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM32;
            break;
        case LIBVGMSTREAM_SFMT_FLOAT:
            plugin->waveformat.format = FMOD_SOUND_FORMAT_PCMFLOAT;
            break;
    }

    plugin->waveformat.channels = plugin->libvgmstream->format->channels;
    plugin->waveformat.frequency = plugin->libvgmstream->format->sample_rate;
    plugin->waveformat.lengthpcm = -1;

    codec->waveformat = &plugin->waveformat;
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = plugin; /* user data value */

    plugin->info->samplerate = plugin->waveformat.frequency;
    plugin->info->bitRate = plugin->libvgmstream->format->stream_bitrate;
    plugin->info->numChannels = plugin->waveformat.channels;

    if (!plugin->info->isTrackFromCueSheet) {
        plugin->info->numSubsongs = plugin->libvgmstream->format->subsong_count;
    }

    if (plugin->libvgmstream->format->stream_name[0] != '\0') {
        plugin->info->title = plugin->libvgmstream->format->stream_name;
    }

    vector<pair<string, string> > metadataFfmpegTrack;

    if (const auto priv = static_cast<libvgmstream_priv_t *>(plugin->libvgmstream->priv);
        priv->vgmstream->coding_type != coding_FFmpeg) {
        plugin->info->fileformat = plugin->libvgmstream->format->meta_name;
    } else {
        if (priv->vgmstream->meta_type == meta_FFMPEG || priv->vgmstream->meta_type == meta_FFMPEG_faulty) {
            plugin->info->fileformat = ffmpeg_get_format_name(
                static_cast<ffmpeg_codec_data *>(priv->vgmstream->codec_data));
        } else {
            plugin->info->fileformat = plugin->libvgmstream->format->meta_name;
        }

        const AVDictionaryEntry *tags = ffmpeg_get_all_metadata(
            static_cast<ffmpeg_codec_data *>(priv->vgmstream->codec_data));

        int i = 0;
        while (tags[i].key != nullptr &&
               ranges::all_of(string(tags[i].value), [](const char c) { return isspace(c); })) {
            metadataFfmpegTrack.emplace_back(tags[i].key, tags[i].value);
            i++;
        }
    }

    // metadata priority (higher to lower) by insertion order
    plugin->info->metadata.emplace_back(metadataCueTrackTextTitle);
    plugin->info->metadata.emplace_back(metadataCueTrackTextArtist);
    plugin->info->metadata.emplace_back(metadataCueTrackTextComposer);
    plugin->info->metadata.emplace_back(metadataCueTrackTextGenre);
    plugin->info->metadata.emplace_back(metadataCueTrackRemDate);
    plugin->info->metadata.emplace_back(metadataCueTrackRemComment);
    plugin->info->metadata.emplace_back(metadataCueTrackRemReplayGainTrackGain);
    plugin->info->metadata.emplace_back(metadataCueTrackRemReplayGainTrackPeak);
    plugin->info->metadata.emplace_back(metadataCueCdTextAlbum);
    plugin->info->metadata.emplace_back(metadataCueCdTextAlbumArtist);
    plugin->info->metadata.emplace_back(metadataCueCdRemDisc);
    plugin->info->metadata.emplace_back(metadataCueCdRemReplayGainAlbumGain);
    plugin->info->metadata.emplace_back(metadataCueCdRemReplayGainAlbumPeak);
    ranges::move(metadataFfmpegTrack, back_inserter(plugin->info->metadata));
    plugin->info->metadata.emplace_back(metadataCueCdTextComposer);
    plugin->info->metadata.emplace_back(metadataCueCdTextGenre);
    plugin->info->metadata.emplace_back(metadataCueCdRemDate);
    plugin->info->metadata.emplace_back(metadataCueCdRemComment);

    plugin->info->fileformatSpecific = plugin->libvgmstream->format->codec_name;
    plugin->info->plugin = PLUGIN_vgmstream;
    plugin->info->pluginName = PLUGIN_vgmstream_NAME;
    plugin->info->setSeekable(true);

    return FMOD_OK;
}

static FMOD_RESULT F_CALL close(FMOD_CODEC_STATE *codec) {
    delete static_cast<pluginVgmstream *>(codec->plugindata);
    return FMOD_OK;
}

static FMOD_RESULT F_CALL read(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read) {
    const auto *plugin = static_cast<pluginVgmstream *>(codec->plugindata);

    if (libvgmstream_fill(plugin->libvgmstream, buffer, static_cast<int>(size)) < 0) {
        return FMOD_ERR_FORMAT;
    }

    *read = plugin->libvgmstream->decoder->buf_samples;

    if (plugin->libvgmstream->decoder->done) {
        return FMOD_ERR_FILE_EOF;
    }

    return FMOD_OK;
}

static FMOD_RESULT F_CALL getLength(FMOD_CODEC_STATE *codec, unsigned int *length, FMOD_TIMEUNIT lengthtype) {
    const auto *plugin = static_cast<pluginVgmstream *>(codec->plugindata);

    if (lengthtype == FMOD_TIMEUNIT_MS_REAL) {
        if (plugin->info->isTrackFromCueSheet) {
            if (plugin->trackLengthFromCueSheet != -1) {
                *length = static_cast<unsigned int>(plugin->trackLengthFromCueSheet * 1000 / CUE_FPS);
                return FMOD_OK;
            }
            if (plugin->info->currentSubsong + 1 == plugin->info->numSubsongs) {
                *length = static_cast<unsigned int>(plugin->libvgmstream->format->stream_samples * 1000
                                                    / plugin->waveformat.frequency
                                                    - plugin->trackStartFromCueSheet * 1000 / CUE_FPS);
                return FMOD_OK;
            }
        }

        *length = static_cast<unsigned int>(plugin->libvgmstream->format->stream_samples * 1000
                                            / plugin->waveformat.frequency);
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}

static FMOD_RESULT F_CALL setPosition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position,
                                      FMOD_TIMEUNIT postype) {
    const auto *plugin = static_cast<pluginVgmstream *>(codec->plugindata);

    if (postype == FMOD_TIMEUNIT_MS) {
        int64_t seek_sample;

        if (plugin->info->isTrackFromCueSheet) {
            seek_sample = (static_cast<int64_t>(position * 0.001)
                           + plugin->trackStartFromCueSheet / CUE_FPS)
                          * plugin->libvgmstream->format->sample_rate;
        } else {
            seek_sample = static_cast<int64_t>(position * 0.001 * plugin->libvgmstream->format->sample_rate);
        }

        libvgmstream_seek(plugin->libvgmstream, seek_sample);
        return FMOD_OK;
    }

    return FMOD_ERR_UNSUPPORTED;
}
