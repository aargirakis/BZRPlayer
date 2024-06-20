#ifndef RHVOICE_H
#define RHVOICE_H

class AmigaChannel;
class RHSample;
class AmigaStep;
class RHVoice
{
friend class RHPlayer;
public:
    RHVoice(int index);
    void initialize();
private:
      int index;
      int bitFlag;
      RHVoice* next;
      AmigaChannel* channel;
      RHSample* sample;
      int trackPtr;
      int trackPos;
      int patternPos;
      int tick;
      int busy;
      int flags;
      int note;
      int period;
      int volume;
      int portaSpeed;
      int vibratoPtr;
      int vibratoPos;
      int synthPos;
};

#endif // RHVOICE_H
