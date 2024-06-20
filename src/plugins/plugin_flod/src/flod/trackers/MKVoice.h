#ifndef MKVOICE_H
#define MKVOICE_H

class AmigaRow;
class AmigaSample;
class AmigaChannel;
class MKVoice
{
friend class MKPlayer;
public:
    MKVoice(int index);
    void initialize();
private:
    int index  ;
    MKVoice* next;
    AmigaChannel* channel;
    AmigaSample* sample;
	int enabled;
	int period;
	int effect;
	int param;
	int volume;
	int portaDir;
	int portaPeriod;
	int portaSpeed;
	int vibratoPos;
	int vibratoSpeed;
};

#endif // MKVOICE_H