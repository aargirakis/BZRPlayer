#ifndef RJPLAYER_H
#define RJPLAYER_H
#include <vector>
#include "AmigaPlayer.h"
#include "Amiga.h"

class BaseSample;
class RJVoice;
class RJSample;

class RJPlayer : public AmigaPlayer
{
public:
    RJPlayer(Amiga* amiga);
    ~RJPlayer();
    int load(void* data, unsigned long int length, const char* filename);

private:
    std::vector<int> songs;
    std::vector<int> tracks;
    std::vector<int> patterns;
    std::vector<int> envelope;
    std::vector<RJVoice*> voices;
    std::vector<RJSample*> samples;

    int complete;


    static const int PERIODS[36];

    void process();
    void initialize();
    void printData();
    std::vector<BaseSample*> getSamples();
    unsigned char getSubsongsCount();
    void selectSong(unsigned char);
};

#endif // RJPLAYER_H
