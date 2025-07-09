#ifndef S1VOICE_H
#define S1VOICE_H

class AmigaChannel;

class S1Voice
{
    friend class S1Player;

public:
    S1Voice(int index);
    void initialize();

    int index;
    S1Voice* next;
    AmigaChannel* channel;
    int step;
    int row;
    int sample;
    int samplePtr;
    int sampleLen;
    int note;
    int noteTimer;
    int period;
    int volume;
    int bendTo;
    int bendSpeed;
    int arpeggioCtr;
    int envelopeCtr;
    int pitchCtr;
    int pitchFallCtr;
    int sustainCtr;
    int phaseTimer;
    int phaseSpeed;
    int wavePos;
    int waveList;
    int waveTimer;
    int waitCtr;

private:
};

#endif // S1VOICE_H
