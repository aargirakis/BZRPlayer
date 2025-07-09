#ifndef PSPLAYER_H
#define PSPLAYER_H
#include <vector>
#include "AmigaPlayer.h"

class BaseStep;
class BaseRow;
class D1Sample;
class D1Voice;

class PSPlayer : public AmigaPlayer
{
public:
    PSPlayer(Amiga* amiga);
    ~PSPlayer();
    int load(void* data, unsigned long int length);
    std::vector<BaseSample*> getSamples();

private:
    std::vector<BaseSample*> samples;
};

#endif // PSPLAYER_H
