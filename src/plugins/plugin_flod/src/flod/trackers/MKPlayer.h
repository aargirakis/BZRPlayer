#ifndef MKPLAYER_H
#define MKPLAYER_H
#include <vector>
#include <list>
#include "AmigaPlayer.h"
#include "Amiga.h"

class BaseRow;
class AmigaSample;
class MKVoice;
class MKPlayer : public AmigaPlayer
{
public:
    enum
    {
        SOUNDTRACKER_23 = 1,
        SOUNDTRACKER_24 = 2,
        NOISETRACKER_10 = 3,
        NOISETRACKER_11 = 4,
        NOISETRACKER_20 = 5
    };
    MKPlayer(Amiga* amiga);
    ~MKPlayer();
    int load(void* data, unsigned long int _length);
    void setForce(int);
private:

    std::list<int> trackPosBuffer;
    std::list<int> patternPosBuffer;
    std::vector<int> track;
    std::vector<AmigaRow*> patterns;
    std::vector<AmigaSample*> samples;
    int length;
    int restart;
    std::vector<MKVoice*> voices;
    int trackPos;
    int patternPos;
    int jumpFlag;
    int vibratoDepth;
    int restartSave;

    static const int PERIODS[37];
    static const int VIBRATO[32];
    static const char* NOTES[38];

    void process();
    void initialize();
    void printData();
    bool getTitle(std::string& title);
    std::vector<AmigaSample*> getSamples();
    unsigned int getCurrentRow();
    unsigned int getCurrentPattern();
    void getModRows(std::vector<BaseRow*>&);
};

#endif // MKPLAYER_H
