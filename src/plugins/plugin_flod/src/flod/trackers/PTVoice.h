#ifndef PTVOICE_H
#define PTVOICE_H

class BaseSample;
class AmigaChannel;
class PTVoice
{
friend class PTPlayer;
public:
    PTVoice(int index);
    void initialize();
private:
	int index  ;
	PTVoice* next;
	AmigaChannel* channel;
    BaseSample* sample;
	int enabled;
        int loopCtr ;
        int loopPos ;
        int step    ;
        int period  ;
        int effect  ;
        int param   ;
        int volume  ;
        int pointer ;
        int length  ;
        int loopPtr ;
        int repeat  ;
	int finetune     ;
        int offset  ;
	int portaDir     ;
	int portaPeriod  ;
	int portaSpeed   ;
	int glissando    ;
	int tremoloParam ;
	int tremoloPos   ;
	int tremoloWave  ;
	int vibratoParam ;
	int vibratoPos   ;
	int vibratoWave  ;
        int funkPos ;
	int funkSpeed    ;
	int funkWave ;
};

#endif // PTVOICE_H
