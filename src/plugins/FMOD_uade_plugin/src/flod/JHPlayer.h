#ifndef JHPLAYER_H
#define JHPLAYER_H
#include <vector>
#include "AmigaPlayer.h"

class JHSong;
class JHVoice;
using namespace std;
class JHPlayer : public AmigaPlayer
{
public:

    JHPlayer(Amiga* amiga);
    ~JHPlayer();
    int load(void* data, unsigned long int _length);
    std::vector<BaseSample*> getSamples();
private:

    unsigned char *stream;
    unsigned int position;
    std::vector<JHSong*> songs;
    std::vector<BaseSample*> samples;
    int base;
    int patterns;
    int patternLen;
    int periods;
    int freqs;
    int vols;
    int sampleData;
    JHSong* song;
    std::vector<JHVoice*>voices ;
    int coso;

    static const int PERIODS[84];

    unsigned char getSubsongsCount();
    void selectSong(unsigned char);


};

#endif // JHPLAYER_H
