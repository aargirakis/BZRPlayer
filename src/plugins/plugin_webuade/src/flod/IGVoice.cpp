#include "IGVoice.h"
#include "IGBlock.h"

IGVoice::IGVoice(int index)
{
    this->index = index;
    next = 0;
    track = std::vector<int>();
}

void IGVoice::initialize()
{
    channel = 0;
    sample = 0;
    state = 0;
    trackPos = 0;
    speed = 1;
    tick = 1;
    position = 0;
    period = 0;
    transpose = 0;
    perBlock = new IGBlock();
    volBlock = new IGBlock();
}
