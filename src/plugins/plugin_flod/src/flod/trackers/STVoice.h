#ifndef STVOICE_H
#define STVOICE_H

class AmigaRow;
class AmigaSample;
class AmigaChannel;
class STVoice
{
friend class STPlayer;
public:
    STVoice(int index);
    void initialize();
private:
    int index  ;
    STVoice* next;
    AmigaChannel* channel;
    AmigaSample* sample;
	int enabled;
	int period;
	int last;
	int effect;
	int param;
};

#endif // STVOICE_H
