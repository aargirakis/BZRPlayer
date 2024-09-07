#ifndef FXPLAYER_H
#define FXPLAYER_H
#include <vector>
#include <list>
#include "AmigaPlayer.h"
#include "Amiga.h"
class FXVoice;
class BaseRow;
class BaseSample;

class FXPlayer : public AmigaPlayer
{
public:
    enum
    {
        SOUNDFX_10 = 1,
        SOUNDFX_18 = 2,
        SOUNDFX_19 = 3,
        SOUNDFX_20 = 4

    };
    FXPlayer(Amiga* amiga);
    ~FXPlayer();
    void setNTSC(int value);
    int load(void* data, unsigned long int length);
    void setVersion(int value);
private:

    int magicTempoNumber;
    std::list<int> trackPosBuffer;
    std::list<int> patternPosBuffer;
    std::vector<int> track;
    std::vector<BaseRow*> patterns;
    std::vector<BaseSample*> samples;
    std::vector<FXVoice*> voices;
    unsigned int length;
    unsigned int trackPos;
    unsigned int patternPos;
    int jumpFlag;
    int delphine;
    static const int PERIODS[67];
    static const char* NOTES[67];

    void process();
    void initialize();
    void printData();
    std::vector<BaseSample*> getSamples();
    unsigned int getCurrentRow();
    unsigned int getCurrentPattern();
    void getModRows(std::vector<BaseRow*>&);
};

#endif // FXPLAYER_H
