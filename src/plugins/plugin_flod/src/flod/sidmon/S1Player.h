#ifndef S1PLAYER_H
#define S1PLAYER_H
#include <vector>
#include "AmigaPlayer.h"
#include "Amiga.h"

using namespace std;

class BaseRow;
class S1Sample;
class S1Voice;
class BaseStep;

class S1Player : public AmigaPlayer
{
public:
    S1Player(Amiga* amiga);
    ~S1Player();
    int load(void* data, unsigned long int _length);

private:
    vector<int> tracksPtr;
    vector<BaseStep*> tracks;
    vector<int> patternsPtr;
    vector<BaseRow*> patterns;
    vector<S1Sample*> samples;
    vector<int> waveLists;
    int speedDef;
    int trackLen;
    int patternDef;
    int mix1Speed;
    int mix2Speed;
    int mix1Dest;
    int mix2Dest;
    int mix1Source1;
    int mix1Source2;
    int mix2Source1;
    int mix2Source2;
    int doFilter;
    int doReset;
    vector<S1Voice*> voices;
    int trackPos;
    int trackEnd;
    int patternPos;
    int patternEnd;
    int patternLen;
    int mix1Ctr;
    int mix2Ctr;
    int mix1Pos;
    int mix2Pos;
    int audioPtr;
    int audioLen;
    int audioPer;
    int audioVol;

    enum
    {
        SIDMON_0FFA = 0x0ffa,
        SIDMON_1170 = 0x1170,
        SIDMON_11C6 = 0x11c6,
        SIDMON_11DC = 0x11dc,
        SIDMON_11E0 = 0x11e0,
        SIDMON_125A = 0x125a,
        SIDMON_1444 = 0x1444
    };

    static const int EMBEDDED[3];
    static const int PERIODS[540];

    void process();
    void initialize();
    vector<BaseSample*> getSamples();
    void printData();
};

#endif // S1PLAYER_H
