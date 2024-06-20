#ifndef FXVOICE_H
#define FXVOICE_H
#include <vector>
#include "AmigaPlayer.h"
#include "Amiga.h"
class BaseSample;
class FXVoice
{
    friend class FXPlayer;
public:
    FXVoice(int index);
private:
    void initialize();

    int index;
    FXVoice* next;
    AmigaChannel* channel;
    BaseSample* sample;
    int period;
    int effect;
    int param;
    int volume;
    int last;
    int slideCtr;
    int slideDir;
    int slideParam;
    int slidePeriod;
    int slideSpeed;
    int stepPeriod;
    int stepSpeed;
    int stepWanted;
};

#endif // FXVOICE_H
