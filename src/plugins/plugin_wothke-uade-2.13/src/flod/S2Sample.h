#ifndef S2SAMPLE_H
#define S2SAMPLE_H
#include "BaseSample.h"

class S2Sample : public BaseSample
{
    friend class S2Player;
public:
    S2Sample();
private:
    int negStart;
    int negLen;
    int negSpeed;
    int negDir;
    int negOffset;
    int negPos;
    int negCtr;
    int negToggle;
};

#endif // S2SAMPLE_H
