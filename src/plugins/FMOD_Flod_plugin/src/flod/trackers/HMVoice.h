#ifndef HMVOICE_H
#define HMVOICE_H

class AmigaRow;
class HMSample;
class AmigaChannel;
class HMVoice
{
friend class HMPlayer;
public:
    HMVoice(int index);
    void initialize();
private:
    int index  ;
    HMVoice* next;
    AmigaChannel* channel;
    HMSample* sample;
	int enabled;
	int period;
	int effect;
	int param;
	int volume1;
	int volume2;
	int handler;
	int portaDir;
	int portaPeriod;
	int portaSpeed;
	int vibratoPos;
	int vibratoSpeed;
	int wavePos;
};

#endif // HMVOICE_H
