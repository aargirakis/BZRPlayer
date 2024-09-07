#ifndef DWPlayer_H
#define DWPlayer_H
#include <vector>
#include "AmigaPlayer.h"
#include "Amiga.h"
class DWSong;
class DWVoice;
class BaseSample;
using namespace std;
class DWPlayer : public AmigaPlayer
{
public:

    DWPlayer(Amiga* amiga);
    ~DWPlayer();
    int load(void* data, unsigned long int _length);
private:
    unsigned char *stream;
    vector<DWSong*>songs;
    vector<BaseSample*>samples;
    DWSong* song;
    int variant;
    unsigned int position;

    int songVol;
    int master ;
    int periods;
    int freqs;
    int vols;
    int transpose;
    int slower ;
    int slowerCtr;
    int delaySpeed   ;
    int delayCtr ;
    int fadeSpeed;
    int fadeCtr  ;
    BaseSample* wave;
    int waveCenter   ;
    int waveLo ;
    int waveHi ;
    int waveDir;
    int waveLen;
    int wavePos;
    int waveRateNeg  ;
    int waveRatePos  ;
    vector<DWVoice*>voices;
    int active ;
    int complete ;
    int base   ;
    int com2   ;
    int com3   ;
    int com4   ;
    unsigned char readMix;
    int readLen;

    enum
    {
        USHORT = 0,
        UINT = 1
    };
    void process();
    void initialize();
     unsigned char getSubsongsCount();
     void selectSong(unsigned char);
     std::vector<BaseSample*> getSamples();
    void printData();

    static const int PERIODS[72];


};

#endif // DWPlayer_H
