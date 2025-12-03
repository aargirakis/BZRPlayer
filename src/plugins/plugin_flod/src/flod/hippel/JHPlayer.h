#ifndef JHPLAYER_H
#define JHPLAYER_H
#include <vector>
#include "AmigaPlayer.h"
#include "Amiga.h"

using namespace std;

class JHSong;
class JHVoice;

class JHPlayer : public AmigaPlayer {
public:
    JHPlayer(Amiga *amiga);

    ~JHPlayer();

    int load(void *data, unsigned long int _length);

private:
    unsigned char *stream;
    unsigned int position;
    vector<JHSong *> songs;
    vector<BaseSample *> samples;
    int base;
    int patterns;
    int patternLen;
    int periods;
    int freqs;
    int vols;
    int sampleData;
    JHSong *song;
    vector<JHVoice *> voices;
    int coso;

    static const int PERIODS[84];

    void process();

    void initialize();

    unsigned char getSubsongsCount();

    void selectSong(unsigned char);

    vector<BaseSample *> getSamples();

    void printData();
};

#endif // JHPLAYER_H