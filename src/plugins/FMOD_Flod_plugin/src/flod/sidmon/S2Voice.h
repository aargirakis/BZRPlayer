#ifndef S2VOICE_H
#define S2VOICE_H

class BaseStep;
class BaseRow;
class S2Sample;
class S2Instrument;
class AmigaChannel;
class S2Voice
{
friend class S2Player;
public:
    S2Voice(int index);
    void initialize();
private:
    int index;
    S2Voice* next;
    AmigaChannel* channel;
    BaseStep* step;
    BaseRow* row;
    S2Instrument* instr;
    S2Sample* sample;
    int enabled;
    int pattern;
    int instrument;
    int note;
    int period;
    int volume;
    int original;
    int adsrPos;
    int sustainCtr;
    int pitchBend;
    int pitchBendCtr;
    int noteSlideTo;
    int noteSlideSpeed;
    int waveCtr;
    int wavePos;
    int arpeggioCtr;
    int arpeggioPos;
    int vibratoCtr;
    int vibratoPos;
    int speed;
};

#endif // S2VOICE_H
