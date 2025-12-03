#ifndef BPPLAYER_H
#define BPPLAYER_H
#include <vector>
#include <list>
#include "AmigaPlayer.h"
#include "Amiga.h"

using namespace std;

class BaseStep;
class BaseRow;
class BPSample;
class BPVoice;

class BPPlayer : public AmigaPlayer {
public:
    enum {
        BPSOUNDMON_V1 = 1,
        BPSOUNDMON_V2 = 2,
        BPSOUNDMON_V3 = 3
    };

    BPPlayer(Amiga *amiga);

    ~BPPlayer();

    int load(void *data, unsigned long int _length);

private:
    list<int> trackPosBuffer;
    list<int> patternPosBuffer;
    vector<BaseStep *> tracks;
    vector<BaseRow *> patterns;
    vector<BPSample *> samples;
    int length;
    vector<int> buffer;
    vector<BPVoice *> voices;
    int trackPos;
    int patternPos;
    int nextPos;
    int jumpFlag;
    int repeatCtr;
    int arpeggioCtr;
    int vibratoPos;
    static const int PERIODS[84];
    static const int VIBRATO[8];

    void process();

    void initialize();

    void printData();

    vector<BaseSample *> getSamples();

    bool getTitle(string &title);

    unsigned int getCurrentRow();

    unsigned int getCurrentPattern();

    vector<BaseRow *> &getModRows();
};

#endif // BPPLAYER_H