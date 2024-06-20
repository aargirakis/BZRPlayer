#ifndef D1VOICE_H
#define D1VOICE_H

class BaseRow;
class AmigaChannel;
class BaseStep;
class D1Sample;
class D1Voice
{
    friend class D1Player;
public:
    D1Voice(int index);
    void initialize();
private:
    int index ;
    D1Voice* next;
    AmigaChannel* channel;
    D1Sample* sample;
    int trackPos;
    int patternPos;
    int status;
    int speed;
    BaseStep* step;
    BaseRow* row;
    int note;
    int period;
    int arpeggioPos;
    int pitchBend ;
    int tableCtr;
    int tablePos;
    int vibratoCtr;
    int vibratoDir;
    int vibratoPos;
    int vibratoPeriod;
    int volume;
    int attackCtr ;
    int decayCtr;
    int releaseCtr;
    int sustain;
};

#endif // D1VOICE_H
