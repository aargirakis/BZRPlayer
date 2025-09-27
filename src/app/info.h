#ifndef BZRINFO_H
#define BZRINFO_H

using namespace std;

#include "BaseRow.h"
#include <vector>
#include <iostream>
#include <cstdint>

class Info
{
public:
    Info()
    {
        clear();
    }

    ~Info()
    {
    }

    bool getSeekable() const
    {
        if (plugin == 0) return true;
        return seekable;
    }

    void setSeekable(bool seekable)
    {
        this->seekable = seekable;
    }

    void clear()
    {
        tempPath = "";
        dataPath = "";
        libPath = "";
        userPath = "";
        mutedChannelsMask = "";
        filename = "";
        artist = "";
        title = "";
        hasTitle = false;
        copyright = "";
        comments = "";
        date = "";
        fade = "";
        chips = "";
        loopInfo = "";
        version = 0;
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
        system = "";
        game = "";
        dumper = "";
        songPlayer = "";
        songType = "";
        author = "";
        composer = "";
        converter = "";
        ripper = "";
        replay = "";
        hwname = "";
        fileformat = "";
        fileformatSpecific = "";
        md5New = "";
        md5Old = "";
        rate = 0;
        address = 0;
        turboSound = false;
        clockSpeed = 0;
        sidChip = "";
        numPatterns = 0;
        numTracksteps = 0;
        numMacros = 0;
        numUsedPatterns = 0;
        numSndModSeqs = 0;
        numVolModSeqs = 0;
        numOrders = 0;
        numChannels = 0;
        numChannelsStream = 0;
        startSubSong = 0;
        numSubsongs = 0;
        currentSubsong = 0;
        modPatternRestart = 0;
        modPatternRows = 0;
        modVUMeters = nullptr;
        instrumentsFilterType = nullptr;
        instrumentsFlags = nullptr;
        instrumentsCydFlags = nullptr;
        instrumentsRingMod = nullptr;
        instrumentsSyncSource = nullptr;
        playerFrequency = 0;
        gain = 0;
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
        songSpeed = 0;
        compatibility = 0;

        isContinuousPlaybackActive = false;
        isFmodSeamlessLoopEnabled = false;
        isSeamlessLoopActive = false;
    }

    void clearMemory()
    {
        if (modVUMeters) delete[] modVUMeters;
        if (samplesSize) delete[] samplesSize;
        if (samplesLoopStart) delete[] samplesLoopStart;
        if (samplesLoopEnd) delete[] samplesLoopEnd;
        if (samplesLoopOffset) delete[] samplesLoopOffset;
        if (samplesLoopLength) delete[] samplesLoopLength;
        if (samplesFineTune) delete[] samplesFineTune;
        if (samples) delete[] samples;
        if (samplesData)
        {
            //TODO
            //Not sure of this is a memory leak or how to free the memory
            //            for( int i = 0 ; i < numSamples ; i++ )
            //            {
            //                delete[] samplesData[i];
            //            }
            //            delete[] samplesData;
        }

        if (instrumentsBaseNote) delete[] instrumentsBaseNote;
        if (instrumentsVolume) delete[] instrumentsVolume;
        if (instrumentsCutoff) delete[] instrumentsCutoff;
        if (instrumentsResonance) delete[] instrumentsResonance;
        if (instrumentsPulseWidth) delete[] instrumentsPulseWidth;
        if (instrumentsEnvAttack) delete[] instrumentsEnvAttack;
        if (instrumentsEnvDelay) delete[] instrumentsEnvDelay;
        if (instrumentsEnvSustain) delete[] instrumentsEnvSustain;
        if (instrumentsEnvRelease) delete[] instrumentsEnvRelease;
        if (instrumentsFilterType) delete[] instrumentsFilterType;
        if (instrumentsFlags) delete[] instrumentsFlags;
        if (instrumentsCydFlags) delete[] instrumentsCydFlags;
        if (instrumentsRingMod) delete[] instrumentsRingMod;
        if (instrumentsSyncSource) delete[]instrumentsSyncSource;

        if (samples16Bit) delete[]samples16Bit;
        if (samplesStereo) delete[]samplesStereo;
        if (instruments) delete[]instruments;
        if (instrumentsBaseNote) delete[]instrumentsBaseNote;
        if (instrumentsVolume) delete[]instrumentsVolume;
        if (instrumentsCutoff) delete[]instrumentsCutoff;
        if (instrumentsResonance) delete[]instrumentsResonance;
        if (instrumentsPulseWidth) delete[]instrumentsPulseWidth;
        if (instrumentsEnvAttack) delete[]instrumentsEnvAttack;
        if (instrumentsEnvDelay) delete[]instrumentsEnvDelay;
        if (instrumentsEnvSustain) delete[]instrumentsEnvSustain;
        if (instrumentsEnvRelease) delete[]instrumentsEnvRelease;
        if (instrumentsFilterType) delete[]instrumentsFilterType;
        if (instrumentsFlags) delete[]instrumentsFlags;
        if (instrumentsCydFlags) delete[]instrumentsCydFlags;
        if (instrumentsRingMod) delete[]instrumentsRingMod;
        if (instrumentsSyncSource) delete[]instrumentsSyncSource;
        if (instrumentsQuantize) delete[] instrumentsQuantize;
        if (instrumentsVolume1) delete[] instrumentsVolume1;
        if (instrumentsVolume2) delete[] instrumentsVolume2;
        if (instrumentsNumber) delete[] instrumentsNumber;
        if (instrumentsFilterLowerLimit) delete[] instrumentsFilterLowerLimit;
        if (instrumentsFilterUpperLimit) delete[] instrumentsFilterUpperLimit;
        if (instrumentsFilterSpeed) delete[] instrumentsFilterSpeed;
        if (instrumentsWavelen) delete[] instrumentsWavelen;

        modTrackPositions.clear();
        modRows.clear();

        for (vector<vector<BaseRow*>>::iterator itr = patterns.begin(); itr != patterns.end(); ++itr)
        {
            for (vector<BaseRow*>::iterator itr2 = (*itr).begin(); itr2 != (*itr).end(); ++itr2)
            {
                delete (*itr2);
            }
            (*itr).clear();
        }
        patterns.clear();
    }

    string mutedChannelsMask;
    string tempPath;
    string dataPath;
    string libPath;
    string userPath;
    string filename;
    string artist;
    string title;
    string converter;
    string ripper;
    bool hasTitle;
    string copyright;
    string comments;
    string date;
    string fade;
    string chips;
    string loopInfo;
    unsigned int version;
    string* samples;
    unsigned char** samplesData;
    unsigned int* samplesSize;
    unsigned int* samplesLoopStart;
    unsigned int* samplesLoopEnd;
    unsigned int* samplesLoopOffset;
    unsigned int* samplesLoopLength;
    int* samplesFineTune;
    unsigned short* samplesVolume;
    bool* samples16Bit;
    bool* samplesStereo;
    int numInstruments;
    string* instruments;
    unsigned char* instrumentsBaseNote;
    unsigned char* instrumentsVolume;
    unsigned short* instrumentsCutoff;
    unsigned char* instrumentsResonance;
    unsigned short* instrumentsPulseWidth;
    unsigned char* instrumentsEnvAttack;
    unsigned char* instrumentsEnvDelay;
    unsigned char* instrumentsEnvSustain;
    unsigned char* instrumentsEnvRelease;
    unsigned char* instrumentsFilterType;
    unsigned long* instrumentsFlags;
    unsigned long* instrumentsCydFlags;
    unsigned char* instrumentsRingMod;
    unsigned char* instrumentsSyncSource;
    unsigned char* instrumentsFilterLowerLimit;
    unsigned char* instrumentsFilterUpperLimit;
    unsigned char* instrumentsFilterSpeed;
    unsigned char* instrumentsWavelen;
    char* instrumentsQuantize;
    unsigned char* instrumentsVolume1;
    unsigned char* instrumentsVolume2;
    char* instrumentsNumber;
    float playerFrequency;
    float gain;
    float chipFrequency;
    int stereoType;
    int bitRate;
    char bitsPerSample;
    string channelConfig;
    signed long long int originalSize;
    string method;
    unsigned int samplerate;
    string sampleType;
    uint32_t* waveformDisplay;

    float volumeAmplification;

    int numSamples;
    unsigned char plugin;
    string pluginName;
    string system;
    string game;
    string dumper;
    string songPlayer;
    string songType;
    string author;
    string composer;
    string replay;
    string hwname;
    string fileformat;
    string fileformatSpecific;
    string md5New;
    string md5Old;
    string volumeAmplificationStr;
    int rate;
    int address;
    bool turboSound;

    int clockSpeed;
    string sidChip;
    int numPatterns;
    int numTracksteps;
    int numMacros;
    int numUsedPatterns;
    int numSndModSeqs;
    int numVolModSeqs;
    int numOrders;
    unsigned int numChannels;
    unsigned int numChannelsStream;
    int startSubSong;
    int numSubsongs;
    int currentSubsong;

    int loopPosition;
    int loopFrame;
    int numFrames;
    int initialTempo;
    int restart;

    unsigned short initAddr;
    unsigned short loadAddr;
    unsigned short playAddr;
    int songSpeed;
    unsigned char compatibility;

    vector<unsigned char> modTrackPositions;
    vector<BaseRow*> modRows;
    vector<vector<BaseRow*>> patterns;

    unsigned int modPatternRows; //how many rows modPattern has
    unsigned int modPatternRestart;
    unsigned char* modVUMeters;

    bool isPlayModeRepeatSongEnabled;
    bool isContinuousPlaybackActive;
    bool isFmodSeamlessLoopEnabled;
    bool isSeamlessLoopActive;

private:
    bool seekable;
};
#endif // BZRINFO_H
