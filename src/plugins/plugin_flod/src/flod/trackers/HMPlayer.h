#ifndef HMPLAYER_H
#define HMPLAYER_H
#include <list>
#include <vector>
#include "AmigaPlayer.h"
#include "Amiga.h"

class AmigaRow;
class HMSample;
class HMVoice;
class HMPlayer : public AmigaPlayer
{
public:
    HMPlayer(Amiga* amiga);
    ~HMPlayer();
    int load(void* data, unsigned long int _length);
private:
    std::list<int> trackPosBuffer;
    std::list<int> patternPosBuffer;
    std::vector<int> track;
    std::vector<AmigaRow*> patterns;
    std::vector<HMSample*> samples;
    int length;
    int restart;
    std::vector<HMVoice*> voices;
    int trackPos;
    int patternPos;
    int jumpFlag;

    static const int MEGARPEGGIO[256];
    static const int PERIODS[37];
    static const int VIBRATO[32];
    static const char* NOTES[38];

    void handler(HMVoice* voice);
    void effects(HMVoice* voice);

    void process();
    void initialize();
    void printData();
    std::vector<AmigaSample*> getSamples();
    bool getTitle(std::string& title);
    unsigned int getCurrentRow();
    unsigned int getCurrentPattern();
    void getModRows(std::vector<BaseRow*>&);
};

#endif // HMPLAYER_H
