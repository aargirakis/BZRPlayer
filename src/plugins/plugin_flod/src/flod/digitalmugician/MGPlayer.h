#ifndef MGPLAYER_H
#define MGPLAYER_H
#include <vector>
#include "AmigaPlayer.h"
#include "Amiga.h"

using namespace std;

class AmigaRow;
class MGSample;
class MGVoice;
class MGSong;

class MGPlayer : public AmigaPlayer {
public:
    MGPlayer(Amiga *amiga);

    ~MGPlayer();

    int load(void *data, unsigned long int _length);

private:
    void tables();


    vector<MGSong *> songs;
    vector<BaseRow *> patterns;
    vector<MGSample *> samples;
    vector<MGVoice *> voices;
    vector<unsigned char> subsongsList;
    int buffer1;
    int buffer2;
    MGSong *song1;
    MGSong *song2;
    int trackPos;
    int patternPos;
    int patternLen;
    int patternEnd;
    int stepEnd;
    int chans;
    vector<int> arpeggios;
    vector<int> averages;
    vector<int> volumes;
    AmigaChannel *mixChannel;
    int mixPeriod;

    enum {
        MUGICIAN_V1 = 1,
        MUGICIAN_V2 = 2
    };

    static const int PERIODS[1017];

    void process();

    void initialize();

    vector<BaseSample *> getSamples();

    bool getTitle(string &title);

    unsigned char getSubsongsCount();

    void selectSong(unsigned char);

    void printData();
};

#endif // MGPLAYER_H