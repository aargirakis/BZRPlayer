#ifndef AMIGA_H
#define AMIGA_H
#include <vector>
#include "AmigaFilter.h"
class AmigaPlayer;
class AmigaFilter;
class AmigaChannel;
class Sample;
class Amiga
{
    friend class AmigaPlayer;
public:
    Amiga();
    enum
    {
            MODEL_A500 = 0,
            MODEL_A1200 = 1
    };
    ~Amiga();
    AmigaPlayer* player;
    AmigaFilter* filter;

    std::vector<signed char> memory;
    std::vector<AmigaChannel*> channels;
    int samplesTick;
    int loopPtr;
    int loopLen;
    void setComplete(int value);
    void setVolume(int value);
    int store(void* data, unsigned long int len, unsigned int& position, unsigned long int datalength, int ptr=-1);
    void mixer(void *_stream, unsigned long int length);
    int isCompleted();
    void setModel(int);
    void setFilter(int filterType);

private:
    double clock;
    double master;
    std::vector<Sample*> m_buffer;
    int m_complete;
    int samplesLeft;
    int remains;
    void initialize();
    void reset();
    bool memoryFixed;
    void setup();

};

#endif // AMIGA_H
