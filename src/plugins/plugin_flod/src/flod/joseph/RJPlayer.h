#ifndef RJPLAYER_H
#define RJPLAYER_H
#include <vector>
#include "AmigaPlayer.h"
#include "Amiga.h"

using namespace std;

class BaseSample;
class RJVoice;
class RJSample;

class RJPlayer : public AmigaPlayer {
public:
    RJPlayer(Amiga *amiga);

    ~RJPlayer();

    int load(void *data, unsigned long int length, const char *filename);

private:
    vector<int> songs;
    vector<int> tracks;
    vector<int> patterns;
    vector<int> envelope;
    vector<RJVoice *> voices;
    vector<RJSample *> samples;

    int complete;


    static const int PERIODS[36];

    void process();

    void initialize();

    void printData();

    vector<BaseSample *> getSamples();

    unsigned char getSubsongsCount();

    void selectSong(unsigned char);
};

#endif // RJPLAYER_H