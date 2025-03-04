#ifndef MGSAMPLE_H
#define MGSAMPLE_H
#include "BaseSample.h"

class MGSample : public BaseSample
{
    friend class MGPlayer;

public:
    MGSample();

private:
    int loop;
    int wave;
    int waveLen;
    int arpeggio;
    int pitch;
    int pitchDelay;
    int pitchLoop;
    int pitchSpeed;
    int fx;
    int fxDone;
    int fxStep;
    int fxSpeed;
    int source1;
    int source2;
    int volLoop;
    int volSpeed;
};

#endif // MGSAMPLE_H
