#ifndef BZRINFO_H
#define BZRINFO_H

#include <cstdint>
#include <vector>
#include "BaseRow.h"

using namespace std;

class Info {
public:
    Info() {
        clear();
    }

    ~Info() {
    }

    bool getSeekable() const {
        if (plugin == 0) return true;
        return seekable;
    }

    void setSeekable(const bool seekable) {
        this->seekable = seekable;
    }

    void clear() {
        tempPath = "";
        dataPath = "";
        libPath = "";
        userPath = "";
        mutedChannelsMask = "";
        isLocalFilePath = false;
        filePath = "";
        filename = "";
        fileDir = "";
        fileLastModified = "";
        fileCreatedAt = "";
        filesize = 0;
        fileBuffer = nullptr;
        containerFilenames = "";
        containerLastFilename = "";
        isTrackFromCueSheet = false;
        cueSheetTrackFilename = "";
        artist = "";
        title = "";
        hasTitle = false;
        copyright = "";
        comments = "";
        date = "";
        fade = "";
        samples = nullptr;
        samplesSize = nullptr;
        samplesLoopStart = nullptr;
        samplesLoopEnd = nullptr;
        samplesLoopOffset = nullptr;
        samplesLoopLength = nullptr;
        samplesFineTune = nullptr;
        samplesVolume = nullptr;
        samples16Bit = nullptr;
        samplesStereo = nullptr;
        numInstruments = 0;
        instruments = nullptr;
        instrumentsBaseNote = nullptr;
        instrumentsVolume = nullptr;
        instrumentsCutoff = nullptr;
        instrumentsResonance = nullptr;
        instrumentsPulseWidth = nullptr;
        instrumentsEnvAttack = nullptr;
        instrumentsEnvDelay = nullptr;
        instrumentsEnvSustain = nullptr;
        instrumentsEnvRelease = nullptr;
        instrumentsQuantize = nullptr;
        instrumentsVolume1 = nullptr;
        instrumentsVolume2 = nullptr;
        instrumentsNumber = nullptr;
        instrumentsFilterLowerLimit = nullptr;
        instrumentsFilterUpperLimit = nullptr;
        instrumentsFilterSpeed = nullptr;
        instrumentsWavelen = nullptr;
        numSamples = 0;
        plugin = 0;
        pluginName = "";
        pluginsDir = "";
        allowedFields = nullptr;
        isSid = false;
        isAhx = false;
        system = "";
        game = "";
        album = "";
        genre = "";
        emulator = "";
        songPlayer = "";
        composer = "";
        converter = "";
        ripper = "";
        replay = "";
        hardware = "";
        fileFormat = "";
        fileFormatSpecific = "";
        containerFileformats = "";
        md5 = "";
        rate = 0;
        address = 0;
        turboSound = false;
        clockSpeed = 0;
        clockSpeedStr = "";
        chips = "";
        numPatterns = 0;
        numMacros = 0;
        numUsedPatterns = 0;
        numSndModSeqs = 0;
        numVolModSeqs = 0;
        numOrders = 0;
        numChannels = 0;
        numChannelsStream = 0;
        defaultLengthMs = 0;
        defaultSubsong = 0;
        numSubsongs = 0;
        currentSubsong = 0;
        modPatternRestart = 0;
        modPatternRows = 0;
        modVuMeters = nullptr;
        instrumentsFilterType = nullptr;
        instrumentsFlags = nullptr;
        instrumentsCydFlags = nullptr;
        instrumentsRingMod = nullptr;
        instrumentsSyncSource = nullptr;
        playerFrequency = 0;
        chipFrequency = 0;
        stereoType = 0;
        bitRate = 0;
        bitsPerSample = 0;
        channelConfig = "";
        originalSize = 0;
        method = "";
        samplerate = 0;
        sampleType = "";
        seekable = false;
        loopPosition = 0;
        loopFrame = 0;
        numFrames = 0;
        initialTempo = 0;
        restart = 0;
        volumeAmplification = 1.0;
        volumeAmplificationStr = "";

        initAddr = 0;
        loadAddr = 0;
        playAddr = 0;
        compatibility = "";

        isContinuousPlaybackActive = false;
    }

    void clearMemory() {
        delete[] modVuMeters;
        delete[] samplesSize;
        delete[] samplesLoopStart;
        delete[] samplesLoopEnd;
        delete[] samplesLoopOffset;
        delete[] samplesLoopLength;
        delete[] samplesFineTune;
        delete[] samples;

        if (samplesData) {
            //TODO
            // not sure of this is a memory leak or how to free the memory
            //            for( int i = 0 ; i < numSamples ; i++ )
            //            {
            //                delete[] samplesData[i];
            //            }
            //            delete[] samplesData;
        }

        delete[] instrumentsBaseNote;
        delete[] instrumentsVolume;
        delete[] instrumentsCutoff;
        delete[] instrumentsResonance;
        delete[] instrumentsPulseWidth;
        delete[] instrumentsEnvAttack;
        delete[] instrumentsEnvDelay;
        delete[] instrumentsEnvSustain;
        delete[] instrumentsEnvRelease;
        delete[] instrumentsFilterType;
        delete[] instrumentsFlags;
        delete[] instrumentsCydFlags;
        delete[] instrumentsRingMod;
        delete[] instrumentsSyncSource;

        delete[] samples16Bit;
        delete[] samplesStereo;
        delete[] instruments;
        delete[] instrumentsBaseNote;
        delete[] instrumentsVolume;
        delete[] instrumentsCutoff;
        delete[] instrumentsResonance;
        delete[] instrumentsPulseWidth;
        delete[] instrumentsEnvAttack;
        delete[] instrumentsEnvDelay;
        delete[] instrumentsEnvSustain;
        delete[] instrumentsEnvRelease;
        delete[] instrumentsFilterType;
        delete[] instrumentsFlags;
        delete[] instrumentsCydFlags;
        delete[] instrumentsRingMod;
        delete[] instrumentsSyncSource;
        delete[] instrumentsQuantize;
        delete[] instrumentsVolume1;
        delete[] instrumentsVolume2;
        delete[] instrumentsNumber;
        delete[] instrumentsFilterLowerLimit;
        delete[] instrumentsFilterUpperLimit;
        delete[] instrumentsFilterSpeed;
        delete[] instrumentsWavelen;

        modTrackPositions.clear();
        modRows.clear();

        for (auto itr = patterns.begin(); itr != patterns.end(); ++itr) {
            for (auto itr2 = itr->begin(); itr2 != itr->end(); ++itr2) {
                delete *itr2;
            }

            itr->clear();
        }

        patterns.clear();
        metadata.clear();
    }

    string mutedChannelsMask;
    string tempPath;
    string dataPath;
    string libPath;
    string userPath;
    bool isLocalFilePath;
    string filePath;
    string filename;
    string fileDir;
    string fileLastModified;
    string fileCreatedAt;
    size_t filesize;
    uint8_t *fileBuffer;
    string containerFilenames;
    string containerLastFilename;
    bool isTrackFromCueSheet;
    string cueSheetTrackFilename;
    string artist;
    string title;
    string converter;
    string ripper;
    bool hasTitle;
    string copyright;
    string comments;
    string date;
    string fade;
    string *samples;
    unsigned char **samplesData;
    unsigned int *samplesSize;
    unsigned int *samplesLoopStart;
    unsigned int *samplesLoopEnd;
    unsigned int *samplesLoopOffset;
    unsigned int *samplesLoopLength;
    int *samplesFineTune;
    unsigned short *samplesVolume;
    bool *samples16Bit;
    bool *samplesStereo;
    int numInstruments;
    string *instruments;
    unsigned char *instrumentsBaseNote;
    unsigned char *instrumentsVolume;
    unsigned short *instrumentsCutoff;
    unsigned char *instrumentsResonance;
    unsigned short *instrumentsPulseWidth;
    unsigned char *instrumentsEnvAttack;
    unsigned char *instrumentsEnvDelay;
    unsigned char *instrumentsEnvSustain;
    unsigned char *instrumentsEnvRelease;
    unsigned char *instrumentsFilterType;
    unsigned long *instrumentsFlags;
    unsigned long *instrumentsCydFlags;
    unsigned char *instrumentsRingMod;
    unsigned char *instrumentsSyncSource;
    unsigned char *instrumentsFilterLowerLimit;
    unsigned char *instrumentsFilterUpperLimit;
    unsigned char *instrumentsFilterSpeed;
    unsigned char *instrumentsWavelen;
    char *instrumentsQuantize;
    unsigned char *instrumentsVolume1;
    unsigned char *instrumentsVolume2;
    char *instrumentsNumber;
    float playerFrequency;
    float chipFrequency;
    int stereoType;
    int bitRate;
    char bitsPerSample;
    string channelConfig;
    signed long long int originalSize;
    string method;
    unsigned int samplerate;
    string sampleType;
    uint32_t *waveformDisplay;

    float volumeAmplification;

    int numSamples;
    unsigned char plugin;
    string pluginName;
    string pluginsDir;
    const vector<string> *allowedFields;
    vector<pair<string, string> > metadata;
    bool isSid;
    bool isAhx;
    string system;
    string game;
    string album;
    string genre;
    string emulator;
    string songPlayer;
    string composer;
    string replay;
    string hardware;
    string fileFormat;
    string fileFormatSpecific;
    string containerFileformats;
    string md5;
    string volumeAmplificationStr;
    int rate;
    int address;
    bool turboSound;

    int clockSpeed;
    string clockSpeedStr;
    string chips;
    int numPatterns;
    int numMacros;
    int numUsedPatterns;
    int numSndModSeqs;
    int numVolModSeqs;
    int numOrders;
    unsigned int numChannels;
    unsigned int numChannelsStream;
    int defaultLengthMs;
    int defaultSubsong;
    int numSubsongs;
    int currentSubsong;

    unsigned int loopPosition;
    int loopFrame;
    int numFrames;
    int initialTempo;
    int restart;

    unsigned short initAddr;
    unsigned short loadAddr;
    unsigned short playAddr;
    string compatibility;

    vector<unsigned char> modTrackPositions;
    vector<BaseRow *> modRows;
    vector<vector<BaseRow *> > patterns;

    unsigned int modPatternRows; // how many rows modPattern has
    unsigned int modPatternRestart;
    unsigned char *modVuMeters;

    bool isPlayModeRepeatSongEnabled;
    bool isContinuousPlaybackActive;

private:
    bool seekable;
};
#endif // BZRINFO_H
