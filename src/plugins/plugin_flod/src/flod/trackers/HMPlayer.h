#ifndef HMPLAYER_H
#define HMPLAYER_H
#include <list>
#include <vector>
#include "AmigaPlayer.h"
#include "Amiga.h"

using namespace std;

class AmigaRow;
class HMSample;
class HMVoice;

class HMPlayer : public AmigaPlayer {
public:
    HMPlayer(Amiga *amiga);

    ~HMPlayer();

    int load(void *data, unsigned long int _length);

private:
    list<int> trackPosBuffer;
    list<int> patternPosBuffer;
    vector<int> track;
    vector<AmigaRow *> patterns;
    vector<HMSample *> samples;
    int length;
    int restart;
    vector<HMVoice *> voices;
    int trackPos;
    int patternPos;
    int jumpFlag;

    static const int MEGARPEGGIO[256];
    static const int PERIODS[37];
    static const int VIBRATO[32];
    static const char *NOTES[38];

    void handler(HMVoice *voice);

    void effects(HMVoice *voice);

    void process();

    void initialize();

    void printData();

    vector<AmigaSample *> getSamples();

    bool getTitle(string &title);

    unsigned int getCurrentRow();

    unsigned int getCurrentPattern();

    void getModRows(vector<BaseRow *> &);
};

#endif // HMPLAYER_H