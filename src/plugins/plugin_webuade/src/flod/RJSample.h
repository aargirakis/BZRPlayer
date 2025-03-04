#ifndef RJSAMPLE_H
#define RJSAMPLE_H
#include "BaseSample.h"

class RJSample : public BaseSample
{
    friend class RJPlayer;

public:
    RJSample();

private:
    int offset;
    int envelopePos;
    int periodPtr;
    int periodStart;
    int periodLen;
    int volumePtr;
    int volumeStart;
    int volumeLen;
    int volumeScale;
};

#endif // RJSAMPLE_H
