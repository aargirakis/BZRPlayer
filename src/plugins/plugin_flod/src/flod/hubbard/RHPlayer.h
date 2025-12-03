#ifndef RHPLAYER_H
#define RHPLAYER_H
#include <vector>
#include "AmigaPlayer.h"
#include "Amiga.h"

using namespace std;

class RHSong;
class RHVoice;
class RHSample;

class RHPlayer : public AmigaPlayer {
public:
    RHPlayer(Amiga *amiga);

    ~RHPlayer();

    int load(void *data, unsigned long int _length);

private:
    unsigned char *stream;
    vector<RHSong *> songs;
    vector<RHSample *> samples;
    RHSong *song;
    int periods;
    int vibrato;
    vector<RHVoice *> voices;
    void *data;
    int complete;
    int variant;
    unsigned int position;

    void process();

    void initialize();

    unsigned char getSubsongsCount();

    void selectSong(unsigned char);

    vector<BaseSample *> getSamples();

    void printData();
};

#endif // RHPLAYER_H