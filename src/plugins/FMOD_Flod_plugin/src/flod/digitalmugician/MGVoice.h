#ifndef MGVOICE_H
#define MGVOICE_H

class AmigaChannel;
class MGSample;
class BaseStep;
class MGVoice
{
friend class MGPlayer;
public:
    MGVoice(int index);
    void initialize();
private:
    int index;
    MGVoice* next;
    AmigaChannel* channel;
    MGSample* sample ;
    BaseStep* step  ;
    int note         ;
    int period       ;
    int val1         ;
    int val2         ;
    int fperiod  ;
    int arpStep ;
    int fxCtr    ;
    int pitch        ;
    int pitchCtr     ;
    int pitchStep    ;
    int portamento   ;
    int volume       ;
    int volCtr    ;
    int volStep   ;
    int mixMute      ;
    int mixPtr       ;
    int mixEnd       ;
    int mixSpeed     ;
    int mixStep      ;
    int mixVolume    ;
};

#endif // MGVOICE_H
