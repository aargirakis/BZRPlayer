#ifndef MGPLAYER_H
#define MGPLAYER_H
#include <vector>
#include "AmigaPlayer.h"
#include "Amiga.h"

class AmigaRow;
class MGSample;
class MGVoice;
class MGSong;

class MGPlayer : public AmigaPlayer
{
public:
    MGPlayer(Amiga* amiga);
    ~MGPlayer();
    int load(void* data, unsigned long int _length);
    std::vector<BaseSample*> getSamples();
    bool getTitle(std::string& title);

private:
    void tables();


    std::vector<MGSong*> songs;
    std::vector<BaseRow*> patterns;
    std::vector<MGSample*> samples;
    std::vector<MGVoice*> voices;
    std::vector<unsigned char> subSongsList;
    int buffer1;
    int buffer2;
    MGSong* song1;
    MGSong* song2;
    int trackPos;
    int patternPos;
    int patternLen;
    int patternEnd;
    int stepEnd;
    int chans;
    std::vector<int> arpeggios;
    std::vector<int> averages;
    std::vector<int> volumes;

    int mixPeriod;

    enum
    {
        MUGICIAN_V1 = 1,
        MUGICIAN_V2 = 2
    };

    static const int PERIODS[1017];
};

#endif // MGPLAYER_H
