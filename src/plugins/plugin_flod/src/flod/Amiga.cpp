#include "Amiga.h"
#include "AmigaPlayer.h"
#include "AmigaFilter.h"
#include "AmigaChannel.h"
#include "Sample.h"
#include <iostream>
#include <math.h>
#include <MyEndian.h>
Amiga::Amiga()
{
    
    loopLen = 4;
    master = 0.00390625;

    channels = std::vector<AmigaChannel*>();
    filter = new AmigaFilter();
    filter->setModel(MODEL_A1200);
    m_buffer = std::vector<Sample*>(8192);
    m_buffer[8192-1] = new Sample();
    m_complete = samplesLeft = remains = 0;

    for (int i=8192-2; i >= 0; --i)
    {
        m_buffer[i] = new Sample();
        m_buffer[i]->next = m_buffer[i+1];
    }

}
Amiga::~Amiga()
{
    for(unsigned int i = 0; i < m_buffer.size(); i++)
    {
        if(m_buffer[i]) delete m_buffer[i];
    }
    m_buffer.clear();
    for(unsigned int i = 0; i < channels.size(); i++)
    {
        if(channels[i]) delete channels[i];
    }
    channels.clear();
    delete filter;
}
void Amiga::setFilter(int filterVal)
{
    filter->setFilter(filterVal);
}

void Amiga::setComplete(int value)
{
    m_complete = value ^ player->loopSong;
}
void Amiga::setVolume(int value)
{
    if (value > 0) {
        if (value > 64) value = 64;
        master = (value / 64) * (0.015625 / channels.size());
    } else {
        master = 0.0;
    }
}
void Amiga::setup()
{
    int len = player->getChannels();
    loopPtr = memory.size();
    memory.resize(memory.size()+loopLen);
    if (len != channels.size())
    {
        channels = std::vector<AmigaChannel*>(len);
        channels[0] = new AmigaChannel(0);
        for (int i = 1; i < len; ++i)
        {
            channels[i] = channels[int(i - 1)]->next = new AmigaChannel(i);
        }
    }

}
int Amiga::store(void* data, unsigned long int size, unsigned int& position, unsigned long int datalength, int ptr)
{
    int add=0;
    unsigned long int total;
    int pos = position;
    unsigned long int start = memory.size();
    unsigned char *stream = static_cast<unsigned char*>(data);

    if (ptr > -1) position = ptr;
    total = position + size;

    if (total >= datalength) {
        add = total - datalength;
        size = datalength - position;
    }


    size += start;
    memory.reserve(size); //TODO If this is'nt here, infogrames songs crashed here... I HAVE NO FUCKING IDEA WHY
    for (unsigned int i = start; i < size; ++i)
    {
        memory.push_back(stream[position]);position++;
    }

    for (unsigned int i = 0 ; i < add; ++i)
    {
        memory.push_back(0);
    }
    if (ptr > -1) position = pos;

    return start;
}

void Amiga::initialize()
{
    //wave.clear();
    filter->initialize();
    master = (player->getVolume() / channels.size()) * 0.015625;
    AmigaChannel* chan = channels[0];
    do
    {
        chan->initialize();
    }
    while (chan = chan->next);


    Sample* sample;
    for (int i = 0; i < 8192; ++i) {
        sample = m_buffer[i];
        sample->l = sample->r = 0.0;
    }

}
void Amiga::reset()
{
    loopPtr = 0;
    loopLen = 4;
    //memory = std::vector<signed char>();

    std::cout << "reset mem\n";
    std::flush(std::cout);
}
void Amiga::mixer(void *_stream, unsigned long int length)
{
    int mixed = 0;
    int mixLen = 0;
    int bufferSize = length;
    int size = bufferSize;
    double value;
    double lvol;
    double rvol;
    int mixPos = 0;
    Sample* sample;

    if (m_complete) {
        if (!remains) return;
        size = remains;
    }


    while (mixed < size)
    {
        if (!samplesLeft)
        {
            player->process();
            samplesLeft = samplesTick;


            if(m_complete)
            {
                size = mixed + samplesTick;
                if (size > bufferSize) {
                    remains = size - bufferSize;
                    size = bufferSize;
                }
            }
        }


        int toMix = samplesLeft;
        if ((mixed + toMix) >= size) toMix = size - mixed;
        mixLen = mixPos + toMix;

        AmigaChannel* chan = channels[0];
        do
        {
            sample = m_buffer[mixPos];
            if (chan->audena && chan->audper)
            {
                double speed = chan->audper / clock;
                if (chan->mute)
                {
                    chan->ldata = 0.0;
                    chan->rdata = 0.0;
                }
                else {
                    double value = chan->audvol * master;
                    lvol = value * (1 - chan->level);
                    rvol = value * (1 + chan->level);
                }
                



                for (int i = mixPos; i < mixLen; ++i)
                {

                    if (chan->delay)
                    {
                        chan->delay--;
                    }
                    else if (--chan->timer < 1.0)
                    {

                        if (!chan->mute)
                        {
                            value = memory[chan->audloc] * 0.0078125;
                            chan->ldata = value * lvol;
                            chan->rdata = value * rvol;



                        }

                        chan->audloc++;
                        chan->timer += speed;


                        if (chan->audloc >= chan->audctr)
                        {
                            chan->audloc = chan->pointer;
                            chan->audctr = chan->pointer + chan->length;
                        }
                    }


                    sample->l += chan->ldata;
                    sample->r += chan->rdata;
                    sample = sample->next;

                }
            }
            else
            {

                for (int i = mixPos; i < mixLen; ++i)
                {
                    sample->l += chan->ldata;
                    sample->r += chan->rdata;
                    sample = sample->next;
                }

            }
        }
        while (chan = chan->next);

        mixPos = mixLen;
        mixed += toMix;
        samplesLeft -= toMix;
    }

    sample = m_buffer[0];
    short *stream = (short*)_stream;
    for (int i = 0; i < size; ++i) {
        sample = m_buffer[i];

        filter->process(sample);
        //std::cout << "model " << model << " active " << filter->active << " forced " << filter->forced << "\n";
        //std::cout << sample->l << "\n";
        //std::cout << sample->r << "\n";


        short l = (short)(sample->l * 32767.0f);
        short r = (short)(sample->r * 32767.0f);

        *stream = l;
        stream++;
        *stream = r;
        stream++;

        sample->l = sample->r = 0.0;
        sample = sample->next;
    }
    //std::cout << "Amiga::mixer() done" << "\n";
    //std::flush(std::cout);
}
void Amiga::setModel(int model)
{
    this->filter->setModel(model);
}

int Amiga::isCompleted()
{
    return m_complete;
}
