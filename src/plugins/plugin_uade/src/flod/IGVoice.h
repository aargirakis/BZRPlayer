#ifndef IGVOICE_H
#define IGVOICE_H
#include <vector>

class BaseSample;
class AmigaChannel;
class IGBlock;

class IGVoice
{
    friend class IGPlayer;

public:
    IGVoice(int index);
    void initialize();

private:
    int index;
    IGVoice* next;
    AmigaChannel* channel;
    BaseSample* sample;
    int state;
    std::vector<int> track;
    int trackPos;
    int speed;
    int tick;
    int position;
    int period;
    int transpose;
    IGBlock* perBlock;
    IGBlock* volBlock;
};

#endif // IGVOICE_H
