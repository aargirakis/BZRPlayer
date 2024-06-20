#ifndef FEVoice_H
#define FEVoice_H

class AmigaChannel;
class FESample;
class FEVoice
{
friend class FEPlayer;
public:
    FEVoice(int index);
    void initialize();
private:
    int index;
    FEVoice* next;
      AmigaChannel* channel;
      FESample* sample;
      int trackPos ;
      int patternPos;
      int tick ;
      int busy ;
      int synth;
      int note ;
      int period   ;
      int volume   ;
      int envelopePos   ;
      int sustainTime   ;
      int arpeggioPos   ;
      int arpeggioSpeed ;
      int portamento;
      int portaCounter  ;
      int portaDelay;
      int portaFlag;
      int portaLimit;
      int portaNote;
      int portaPeriod   ;
      int portaSpeed;
      int vibrato  ;
      int vibratoDelay  ;
      int vibratoDepth  ;
      int vibratoFlag   ;
      int vibratoSpeed  ;
      int pulseCounter  ;
      int pulseDelay;
      int pulseDir ;
      int pulsePos ;
      int pulseSpeed;
      int blendCounter  ;
      int blendDelay;
      int blendDir ;
      int blendPos ;
};

#endif // FEVoice_H
