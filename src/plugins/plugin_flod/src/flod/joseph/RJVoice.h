#ifndef RJVOICE_H
#define RJVOICE_H
class RJSample;
class AmigaChannel;

class RJVoice
{
    friend class RJPlayer;

public:
    RJVoice(int index);
    void initialize();

private:
    RJSample* sample;
    AmigaChannel* channel;
    RJVoice* next;

    int index;
    int active;
    int enabled;
    int trackPos;
    int patternPos;
    int speed1;
    int speed2;
    int tick1;
    int tick2;
    int note;
    int period;
    int periodMod;
    int periodPos;
    int volume;
    int volumePos;
    int volumeScale;
    int portaCounter;
    int portaPeriod;
    int portaStep;
    int envelPos;
    int envelStep;
    int envelScale;
    int envelStart;
    int envelEnd1;
    int envelEnd2;
    int envelVolume;
};

#endif // RJVOICE_H
