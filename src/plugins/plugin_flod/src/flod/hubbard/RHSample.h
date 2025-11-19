#ifndef RHSAMPLE_H
#define RHSAMPLE_H
#include "BaseSample.h"
#include <vector>

using namespace std;

class RHSample : public BaseSample
{
    friend class RHPlayer;

public:
    RHSample();

private:
    int divider;
    int vibrato;
    int hiPos;
    int loPos;
    vector<signed char> wave;
};

#endif // RHSAMPLE_H
