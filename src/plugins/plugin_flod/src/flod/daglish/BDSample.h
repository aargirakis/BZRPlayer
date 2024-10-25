#ifndef BDSAMPLE_H
#define BDSAMPLE_H
#include "BaseSample.h"

class BDSample : public BaseSample
{
    friend class BDPlayer;

public:
    BDSample();

private:
    int word14;
    int word16;
    int word18;
    int word20;
    int word22;
    int word24;
    int word26;
};

#endif // BDSAMPLE_H
