#ifndef DWVoice_H
#define DWVoice_H

class AmigaChannel;
class BaseSample;
class DWVoice
{
friend class DWPlayer;
public:
    DWVoice(int index, int bitflag);
    void initialize();
	
private:
	DWVoice* next;
	AmigaChannel* channel;
    BaseSample* sample;
	int index    ;
	int bitFlag  ;
	int trackPtr ;
	int trackPos  ;
	int patternPos  ;
    int freqsPtr ;
    int freqsPos  ;
    int volsPtr   ;
    int volsPos  ;
    int volSpeed  ;
    int volCtr ;
	int halve   ;
	int speed   ;
	int tick  ;
	int busy  ;
	int flags ;
	int note  ;
	int period  ;
	int transpose ;
	int portaDelay;
	int portaDelta ;
	int portaSpeed  ;
	int vibrato ;
    int vibDelta;
    int vibSpeed ;
    int vibDepth ;
};

#endif // DWVoice_H
