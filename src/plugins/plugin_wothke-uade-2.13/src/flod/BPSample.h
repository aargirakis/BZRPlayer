#ifndef BPSAMPLE_H
#define BPSAMPLE_H
#include "BaseSample.h"

class BPSample : public BaseSample
{
    friend class BPPlayer;
public:
    BPSample();
private:
    int synth;
    int table;
    int adsrControl;
    int adsrTable  ;
    int adsrLen    ;
    int adsrSpeed  ;
    int lfoControl ;
    int lfoTable   ;
    int lfoDepth   ;
    int lfoLen     ;
    int lfoDelay   ;
    int lfoSpeed   ;
    int egControl  ;
    int egTable    ;
    int egLen;
    int egDelay    ;
    int egSpeed    ;
    int fxControl  ;
    int fxDelay    ;
    int fxSpeed    ;
    int modControl ;
    int modTable   ;
    int modLen     ;
    int modDelay   ;
    int modSpeed   ;
};

#endif // BPSAMPLE_H
