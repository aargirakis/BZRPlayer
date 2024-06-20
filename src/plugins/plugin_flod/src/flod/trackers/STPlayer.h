#ifndef STPLAYER_H
#define STPLAYER_H
#include <vector>
#include <list>
#include "AmigaPlayer.h"
#include "Amiga.h"

class BaseRow;
class AmigaSample;
class STVoice;
class STPlayer : public AmigaPlayer
{
public:
    enum
    {
        ULTIMATE_SOUNDTRACKER = 1,
        DOC_SOUNDTRACKER_9 = 2,
        MASTER_SOUNDTRACKER = 3,
        DOC_SOUNDTRACKER_20 = 4
    };
    STPlayer(Amiga* amiga);
    ~STPlayer();
    int load(void* data, unsigned long int _length);
    void setForce(int);
    void setNTSC(int value);
private:
    std::list<int> trackPosBuffer;
    std::list<int> patternPosBuffer;
    std::vector<int> track;
    std::vector<AmigaRow*> patterns;
    std::vector<AmigaSample*> samples;
    int length;
    std::vector<STVoice*> voices;
    int trackPos;
    int patternPos;
    int jumpFlag;

    static const int PERIODS[39];
    static const char* NOTES[38];

    int isLegal(std::string text);
    void arpeggio(STVoice* voice);
    void process();
    void initialize();
    void printData();
    std::vector<AmigaSample*> getSamples();
    bool getTitle(std::string& title);
    unsigned int getCurrentRow();
    unsigned int getCurrentPattern();
    void getModRows(std::vector<BaseRow*>&);
};

#endif // STPLAYER_H
