#ifndef BPVOICE_H
#define BPVOICE_H

class AmigaChannel;
class BPVoice
{
friend class BPPlayer;
public:
    BPVoice(int index);
    void initialize();
private:
    int index  ;
    BPVoice* next;
    AmigaChannel* channel;
    int enabled;
    int restart;
    int note   ;
    int period ;
    int sample ;
    int samplePtr;
    int sampleLen;
    int synth  ;
    int synthPtr ;
    int arpeggio ;
    int autoArpeggio;
    int autoSlide;
    int vibrato;
    int volume ;
    int volumeDef;
    int adsrControl ;
    int adsrPtr;
    int adsrCtr;
    int lfoControl  ;
    int lfoPtr ;
    int lfoCtr ;
    int egControl;
    int egPtr  ;
    int egCtr  ;
    int egValue;
    int fxControl;
    int fxCtr  ;
    int modControl  ;
    int modPtr ;
    int modCtr;
};

#endif // BPVOICE_H
