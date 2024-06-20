#ifndef BDVOICE_H
#define BDVOICE_H
class BDSample;
class AmigaChannel;
class BDVoice
{
friend class BDPlayer;
public:
    BDVoice(int index);
    void initialize();
private:
      int index;
      BDVoice* next;
      AmigaChannel* channel;
      BDSample* sample;
      BDSample* sample2;
      int bank;
      int trackPos;
      int patternPtr;
      int patternPos;
      int s1byte18;
      int s1byte19;
      int s1byte20;
      int s1byte21;
      int s1byte22;
      int s1byte23;
      int s1byte24;
      int s1byte30;
      int s1byte31;
      int s1byte32;
      int s1byte33;
      int s1byte34;
      int s1byte35;
      int s1byte36;
      int s1byte37;
      int s1long38;
      int s1byte42;
      int s1byte43;
      int s1byte44;
      int s1byte45;
      int s1word46;
      int s1byte48;
      int s1byte49;
      int s1byte50;
      int s1byte51;
      int s1word52;
      int s1word54;
      int s1byte56;
      int s1byte64;
      int s1word66;
      int s1word68;
      int s1word70;
      int state;
      int period  ;
      int s2word10;
      int volume  ;
      int s2word14;
      int s2word16;
      int s2word18;
      int s2word20;
      int s2word22;
      int s2word24;
      int type;
      int v1word14;
      int v1word16;
      int v1word18;
};

#endif // BDVOICE_H
