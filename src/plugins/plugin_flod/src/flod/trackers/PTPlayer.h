#ifndef PTPLAYER_H
#define PTPLAYER_H
#include <vector>
#include <list>
#include "AmigaPlayer.h"
#include "Amiga.h"

class BaseRow;
class BaseSample;
class PTVoice;
class PTPlayer : public AmigaPlayer
{
public:
    enum
    {
        ULTIMATE_SOUNDTRACKER  = 1,
        DOC_SOUNDTRACKER_9 = 2,
        MASTER_SOUNDTRACKER = 3,
        DOC_SOUNDTRACKER_20 = 4,
        SOUNDTRACKER_24 = 5,
        SOUNDTRACKER_25 = 6,
        NOISETRACKER_11 = 7,
        PROTRACKER_10 = 8,
        NOISETRACKER_20 = 9,
        PROTRACKER_11 = 10,
        PROTRACKER_12  = 11,
        FASTTRACKER_10 = 12
    };
    PTPlayer(Amiga* amiga);
    ~PTPlayer();
    int load(void* data, unsigned long int _length);
    void setVersion(int);
private:
    std::list<int> trackPosBuffer;
    std::list<int> patternPosBuffer;
    std::vector<int> track;
    int trackPos;
    std::vector<BaseRow*> patterns;
    int patternPos;
    int patternBreak;
    int patternDelay;
    int length;
    int breakPos;
    int jumpFlag;
    int restart;
    std::vector<PTVoice*> voices;
    int patternLen;
    int restartCopy;
    int vibratoDepth;
    std::vector<BaseSample*> samples;


    static const int FUNKREP[16];
    static const int PERIODS[592];
    static const int VIBRATO[32];
    static const char* NOTES[38];


    void setNTSC(bool value);

    void process();
    void initialize();
    void printData();
    void fx();
    void moreFx(PTVoice* voice);
    void updateFunk(PTVoice* voice);
    void extendedFx(PTVoice* voice);

    std::vector<BaseSample*> getSamples();
    bool getTitle(std::string& title);
    unsigned int getCurrentRow();
    unsigned int getCurrentPattern();
    void getModRows(std::vector<BaseRow*>&);
};

#endif // PTPLAYER_H
