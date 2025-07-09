#include "PSPlayer.h"
#include "BaseSample.h"
#include <iostream>
#include "MyEndian.h"

using namespace std;

PSPlayer::PSPlayer(Amiga* amiga): AmigaPlayer(amiga)
{
}

PSPlayer::~PSPlayer()
{
    for (unsigned int i = 0; i < samples.size(); i++)
    {
        if (samples[i]) delete samples[i];
    }
    samples.clear();
}

int PSPlayer::load(void* _data, unsigned long int length)
{
    unsigned char* stream = static_cast<unsigned char*>(_data);


    int position = 32;
    int numsamples = 15;
    samples = std::vector<BaseSample*>(numsamples);
    for (int i = 0; i < numsamples; ++i)
    {
        BaseSample* sample = new BaseSample();
        samples[i] = sample;
        const int STRING_LENGTH = 16;
        bool illegalSample = false;
        for (int j = 0; j < STRING_LENGTH; j++)
        {
            if ((stream[position + j] < 32 || stream[position + j] > 126) && stream[position + j] != 0)
            {
                sample->name = "";
                illegalSample = true;
                break;
            }
            if (!stream[position + j])
            {
                break;
            }
            sample->name += stream[position + j];
        }
        position += 22;

        if (!illegalSample && sample->name != "")
        {
            sample->length = readEndian(stream[position], stream[position + 1]) << 1;
        }
        else
        {
            sample->length = 0;
        }

        position += 10;
    }

    m_version = 1;
    format = "Paul";

    return 1;
}

std::vector<BaseSample*> PSPlayer::getSamples()
{
    std::vector<BaseSample*> samp(samples.size());
    for (int i = 0; i < samples.size(); i++)
    {
        samp[i] = samples[i];
        if (!samp[i])
        {
            samp[i] = new BaseSample();
        }
    }
    //std::cout << "returning samples, size: " << samp.size() << "\n";
    return samp;
}
