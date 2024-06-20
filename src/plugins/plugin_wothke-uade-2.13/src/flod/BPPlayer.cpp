#include "BPPlayer.h"
#include "BPSample.h"
#include <iostream>
#include "MyEndian.h"
#include <math.h>
using namespace std;

BPPlayer::BPPlayer(Amiga* amiga):AmigaPlayer(amiga)
{

    samples = std::vector<BPSample*>(16);

}
BPPlayer::~BPPlayer()
{

    for(unsigned int i = 0; i < samples.size(); i++)
    {
        if(samples[i]) delete samples[i];
    }
    samples.clear();

}

int BPPlayer::load(void* data, unsigned long int _length)
{
    unsigned char *stream = static_cast<unsigned char*>(data);

    unsigned int position = 0;
    unsigned int tables = 0;
    const int STRING_LENGTH = 26;
    for(int j = 0;j<STRING_LENGTH;j++)
    {
        if(!stream[position+j])
        {
            break;
        }
        m_title+=stream[position+j];
    }
    position+=STRING_LENGTH;




    if(stream[26]=='B' && stream[27]=='P' && stream[28]=='S' && stream[29]=='M')
    {
        m_version = BPSOUNDMON_V1;
        format = "Soundmon 1";
        position=30;
    }
    else
    {
        if(stream[26]=='V' && stream[27]=='.' && stream[28]=='2' )
        {
            m_version = BPSOUNDMON_V2;
            format = "Soundmon 2";
        }
        else if(stream[26]=='V' && stream[27]=='.' && stream[28]=='3' )
        {
            m_version = BPSOUNDMON_V3;
            format = "Soundmon 3";
        }
        else
        {
            return -1;
        }
        position=29;
        tables = stream[position];
        position++;
    }


    position+=2;

    for (int i = 0; ++i < 16;) {
        BPSample* sample = new BPSample();

        if (stream[position] == 0xff)
        {
            position++;
            sample->synth   = 1;
            sample->table   = stream[position];position++;
            sample->pointer = sample->table << 6;
            sample->length  = readEndian(stream[position],stream[position+1]) << 1 ; position+=2;

            sample->adsrControl = stream[position] ; position++;
            sample->adsrTable   = stream[position] << 6; position++;
            sample->adsrLen     = readEndian(stream[position],stream[position+1]); position+=2;
            sample->adsrSpeed   = stream[position] ; position++;
            sample->lfoControl  = stream[position] ; position++;
            sample->lfoTable    =  stream[position] << 6; position++;
            sample->lfoDepth    = stream[position] ; position++;
            sample->lfoLen      = readEndian(stream[position],stream[position+1]); position+=2;

            if (m_version < BPSOUNDMON_V3) {
                position++;
                sample->lfoDelay  = stream[position] ; position++;
                sample->lfoSpeed  = stream[position] ; position++;
                sample->egControl = stream[position] ; position++;
                sample->egTable   = stream[position] << 6; position++;
                position++;
                sample->egLen     = readEndian(stream[position],stream[position+1]); position+=2;
                position++;
                sample->egDelay   = stream[position] ; position++;
                sample->egSpeed   =stream[position] ; position++;
                sample->fxSpeed   = 1;
                sample->modSpeed  = 1;
                sample->volume    = stream[position] ; position++;
                position += 6;
            } else {
                sample->lfoDelay   = stream[position] ; position++;
                sample->lfoSpeed   = stream[position] ; position++;
                sample->egControl  = stream[position] ; position++;
                sample->egTable    = stream[position] << 6; position++;
                sample->egLen      = readEndian(stream[position],stream[position+1]); position+=2;
                sample->egDelay    = stream[position] ; position++;
                sample->egSpeed    = stream[position] ; position++;
                sample->fxControl  = stream[position] ; position++;
                sample->fxSpeed    = stream[position] ; position++;
                sample->fxDelay    = stream[position] ; position++;
                sample->modControl = stream[position] ; position++;
                sample->modTable   = stream[position] << 6; position++;
                sample->modSpeed   =  stream[position] ; position++;
                sample->modDelay   =  stream[position] ; position++;
                sample->volume     =  stream[position] ; position++;
                sample->modLen     = readEndian(stream[position],stream[position+1]); position+=2;
            }
        } else {

            sample->synth  = 0;
            const int STRING_LENGTH = 24;
            for(int j = 0;j<STRING_LENGTH;j++)
            {
                if(!stream[position+j])
                {
                    break;
                }
                sample->name+=stream[position+j];
            }
            position+=STRING_LENGTH;
            sample->length = readEndian(stream[position],stream[position+1]) << 1 ; position+=2;

            if (sample->length) {
                sample->loopPtr   = readEndian(stream[position],stream[position+1]); position+=2;
                sample->repeat = readEndian(stream[position],stream[position+1]) << 1 ; position+=2;
                sample->volume =readEndian(stream[position],stream[position+1]); position+=2;

                if ((sample->loopPtr + sample->repeat) >= sample->length)
                    sample->repeat = sample->length - sample->loopPtr;
            } else {
                sample->pointer--;
                sample->repeat = 2;
                position += 6;
            }
        }

        samples[i] = sample;
    }




//    for (int i = 0; ++i < 16;)
//    {
//        BPSample* sample = samples[i];
//        if (sample->synth || !sample->length) continue;
//        sample->pointer = amiga->store(stream, sample->length, position, _length);
//        sample->loopPtr += sample->pointer;
//    }
    return 1;

}

std::vector<BaseSample*> BPPlayer::getSamples()
{
    std::vector<BaseSample*>samp (samples.size()-1);
    for(int i =1; i< samples.size() ; i++)
    {
        samp[i-1] = samples[i];
        if(!samp[i-1])
        {
            samp[i-1] = new BaseSample();
        }
    }
    return samp;
}
bool BPPlayer::getTitle(std::string& title)
{
    title = this->m_title;
    return true;
}

