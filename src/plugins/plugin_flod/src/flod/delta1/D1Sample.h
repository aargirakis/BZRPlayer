#ifndef D1SAMPLE_H
#define D1SAMPLE_H
#include "BaseSample.h"
#include <vector>

using namespace std;

class D1Sample : public BaseSample
{
    friend class D1Player;

public:
    D1Sample();

private:
    int loop;
    int synth;
    int attackStep;
    int attackDelay;
    int decayStep;
    int decayDelay;
    int releaseStep;
    int releaseDelay;
    int sustain;
    vector<unsigned char> arpeggio;
    signed char pitchBend;
    int portamento;
    vector<signed char> table;
    int tableDelay;
    int vibratoWait;
    int vibratoStep;
    int vibratoLen;
};

#endif // D1SAMPLE_H
