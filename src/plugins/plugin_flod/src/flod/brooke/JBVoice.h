#ifndef JBVOICE_H
#define JBVOICE_H

class AmigaChannel;
class JBVoice
{
friend class JBPlayer;
public:
    JBVoice(int index);
    void initialize();
private:
    int index;
    JBVoice* next;
	JBVoice* prev;
    AmigaChannel* channel;

    int track;
	int trackLen;
	int trackLoop;
	int trackPos;
	int patternPos;
	int loopCounter;
    int loopPos;
    int flags;
    int state;
    int delay;
    int counter;
    int note;
    int sample1;
    int sample2;
    int volume;
	int volumeMod;
	int volCounter;
	int volPointer;
    int volPos;
	int periodMod;
	int slidePointer;
	int slidePos;
	int slideStep;
	int slideLimit;
	int slideValue;
	int portaCounter;
	int portaStep;
	int portaPeriod;
};

#endif // JBVOICE_H
