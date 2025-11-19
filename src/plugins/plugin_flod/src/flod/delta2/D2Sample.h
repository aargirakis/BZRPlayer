#ifndef D2SAMPLE_H
#define D2SAMPLE_H
#include "BaseSample.h"
#include <vector>

using namespace std;

class D2Sample : public BaseSample
{
    friend class D2Player;

public:
    D2Sample();

private:
    signed char synth;
    int index;
    int pitchBend;

    vector<unsigned char> table;
    vector<unsigned char> vibratos;
    vector<unsigned char> volumes;
};

#endif // D2SAMPLE_H
