#ifndef D2VOICE_H
#define D2VOICE_H

class BaseStep;
class BaseRow;
class D2Sample;
class AmigaChannel;

class D2Voice
{
    friend class D2Player;

public:
    D2Voice(int index);
    void initialize();

private:
    int index;
    D2Voice* next;
    AmigaChannel* channel;
    D2Sample* sample;
    int trackPtr;
    int trackPos;
    int trackLen;
    int patternPos;
    int restart;
    BaseStep* step;
    BaseRow* row;
    int note;
    int period;
    int finalPeriod;
    int arpeggioPtr;
    int arpeggioPos;
    int pitchBend;
    int portamento;
    int tableCtr;
    int tablePos;
    int vibratoCtr;
    int vibratoDir;
    int vibratoPos;
    int vibratoPeriod;
    int vibratoSustain;
    int volume;
    int volumeMax;
    int volumePos;
    int volumeSustain;
};

#endif // D2VOICE_H
