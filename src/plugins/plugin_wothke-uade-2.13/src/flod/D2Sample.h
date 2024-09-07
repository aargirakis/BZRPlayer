#ifndef D2SAMPLE_H
#define D2SAMPLE_H
#include "BaseSample.h"
#include <vector>
class D2Sample : public BaseSample
{
    friend class D2Player;
public:
    D2Sample();
private:
    signed char synth;
    int index;
    int pitchBend;

    std::vector<unsigned char> table;
    std::vector<unsigned char> vibratos;
    std::vector<unsigned char> volumes;
};

#endif // D2SAMPLE_H
