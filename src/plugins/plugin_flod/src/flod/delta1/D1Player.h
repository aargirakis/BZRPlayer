#ifndef D1PLAYER_H
#define D1PLAYER_H
#include <vector>
#include "AmigaPlayer.h"
#include "Amiga.h"

class BaseStep;
class BaseRow;
class D1Sample;
class D1Voice;

class D1Player : public AmigaPlayer
{
public:
    D1Player(Amiga* amiga);
    ~D1Player();
    int load(void* data, unsigned long int length);

private:
    std::vector<int> pointers;
    std::vector<BaseStep*> tracks;
    std::vector<BaseRow*> patterns;
    std::vector<D1Sample*> samples;
    std::vector<D1Voice*> voices;

    static const int PERIODS[84];
    void process();
    void initialize();
    void printData();
    std::vector<BaseSample*> getSamples();
};

#endif // D1PLAYER_H
