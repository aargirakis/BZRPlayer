#ifndef JHVOICE_H
#define JHVOICE_H

class AmigaChannel;
class JHVoice
{
friend class JHPlayer;
public:
    JHVoice(int index);
    void initialize();
private:
	int index;
	JHVoice*  next;
	AmigaChannel*  channel;
	int  enabled;
    int  cosoCtr;
	int  cosoSpeed;
	int  trackPtr;
	int  trackPos;
        signed char  trackTrans;
	int  patternPtr;
	int  patternPos;
    int  freqsPtr;
    int  freqsPos;
    int  volsPtr;
    int  volsPos;
	int  sample;
	int  loopPtr;
	int  repeat;
	int  tick;
	int  note;
	int  transpose;
	int  info;
	int  infoPrev;
	int  volume;
    int  volCtr;
	int  volSpeed;
	int  volSustain;
        signed char  volTrans;
	int  volFade;
	int  portaDelta;
	int  vibrato;
	int  vibDelay;
	int  vibDelta;
	int  vibDepth;
	int  vibSpeed;
	int  slide;
	int  sldActive;
	int  sldDone;
    int  sldCtr;
	int  sldSpeed;
	int  sldDelta;
	int  sldPointer;
	int  sldLen;
	int  sldEnd;
	int  sldLoopPtr;
};

#endif // JHVOICE_H
