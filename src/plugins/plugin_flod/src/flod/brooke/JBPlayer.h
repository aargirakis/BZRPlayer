#ifndef JBPLAYER_H
#define JBPLAYER_H
#include <vector>
#include "AmigaPlayer.h"
#include "Amiga.h"

using namespace std;

class JBSong;
class BaseSample;
class JBVoice;

class JBPlayer : public AmigaPlayer {
public:
    JBPlayer(Amiga *amiga);

    ~JBPlayer();

    int load(void *data, unsigned long int _length);

    int oldLoader(void *data, unsigned long int _length);

    unsigned char getSubsongsCount();

    void selectSong(unsigned char subsong);

private:
    vector<JBSong *> songs;
    vector<BaseSample *> samples;
    vector<JBVoice *> voices;
    int command;
    int periods;
    int ptrack;
    int pblock;
    int vtrack;
    int vblock;
    int transpose;
    int waveDir;
    int wavePos;
    int waveLower;
    int waveUpper;
    int complete;
    unsigned int position;
    unsigned char *stream;
    bool oldProcess;

    int fx(JBVoice *, int);

    void process();

    void initialize();

    void printData();

    vector<BaseSample *> getSamples();
};

#endif // JBPLAYER_H