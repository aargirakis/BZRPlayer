#ifndef BPPLAYER_H
#define BPPLAYER_H
#include "BaseSample.h"
#include "AmigaPlayer.h"
#include <string>
#include <vector>
#include <list>

class BaseStep;
class BaseRow;
class BPSample;

class BPPlayer : public AmigaPlayer
{
public:
    enum
    {
        BPSOUNDMON_V1 = 1,
        BPSOUNDMON_V2 = 2,
        BPSOUNDMON_V3 = 3
    };
    BPPlayer(Amiga* amiga);
    ~BPPlayer();
    int load(void* data, unsigned long int _length);
    std::vector<BaseSample*> getSamples();
    bool getTitle(std::string& title);
private:
    std::vector<BPSample*> samples;

};

#endif // BPPLAYER_H
