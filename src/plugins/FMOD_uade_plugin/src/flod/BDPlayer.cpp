#include "BDPlayer.h"
#include "BDVoice.h"
#include "BDSample.h"

#include <iostream>
#include <fstream>
#include "MyEndian.h"

using namespace std;


BDPlayer::BDPlayer(Amiga* amiga):AmigaPlayer(amiga)
{
    voices    = std::vector<BDVoice*>(4);

    voices[0] = new BDVoice(0);
    voices[0]->next = voices[1] = new BDVoice(1);
    voices[1]->next = voices[2] = new BDVoice(2);
    voices[2]->next = voices[3] = new BDVoice(3);
    commands=0;
    complete = 0;
    periods = 0;
    fadeStep = 0;

}
BDPlayer::~BDPlayer()
{

     for(unsigned int i = 0; i < voices.size(); i++)
     {
     if(voices[i]) delete voices[i];
     }
     voices.clear();
     for(unsigned int i = 0; i < samples.size(); i++)
     {
     if(samples[i]) delete samples[i];
     }
     samples.clear();


    songs.clear();
    banks.clear();
}

int BDPlayer::load(void* _data, unsigned long int _length)
{
    int len;
    int lower = 0xffff;
    int pos = 0;
    int value = 0;
    int tempVal = 0;
    int tracks = 0;
    length = _length;
    banks = std::vector<int>();
    position=0;
    stream = static_cast<unsigned char*>(_data);
    do {
        value = readEndian(stream[position],stream[position+1]);position+=2;

        switch (value) {
        case 0xd040:                                                    //add.w [d0,d0]
            tempVal = readEndian(stream[position],stream[position+1]);position+=2;
            if (tempVal != 0xd040) break;              //add.w [d0,d0]
            value = readEndian(stream[position],stream[position+1]);position+=2;

            if (value == 0x47fa) {                                        //lea [xx,a3]
                periods = position + (signed short)readEndian(stream[position],stream[position+1]);position+=2;
            } else if (value == 0xd040) {                                 //add.w [d0,d0]
                tempVal = readEndian(stream[position],stream[position+1]);position+=2;
                if (tempVal == 0x41fa) {                 //lea [xx,a0]
                    tracks = position + (signed short)readEndian(stream[position],stream[position+1]);position+=2;
                }
            }
            break;
        case 0x10c2:                                                    //move.b [d2,(a0)+]
            position += 2;
            value = readEndian(stream[position],stream[position+1]);position+=2;

            if (value == 0xb43c || value == 0x0c02) {                     //cmp.b [xx,d2] || cmpi.b [xx,d2]
                value = readEndian(stream[position],stream[position+1]);position+=2;

                if (banks.size() != value) {
                    banks = std::vector<int>(value);
                }
            }
            break;
        case 0xb03c:                                                    //cmp.b [xx,d0]
        case 0x0c00:                                                    //cmpi.b [xx,d0]
            tempVal = readEndian(stream[position],stream[position+1]);position+=2;
            if (tempVal == 0x00fd) m_variant = 3;       //xx = #$fd
            break;
        case 0x294b:                                                    //move.l [a3,xx]
            position += 2;
            tempVal = readEndian(stream[position],stream[position+1]);position+=2;
            if (tempVal != 0x47fa) break;              //lea [xx,a3]
            patterns = position + (signed short)readEndian(stream[position],stream[position+1]);position+=2;

            tempVal = readEndian(stream[position],stream[position+1]);position+=2;
            if (tempVal == 0x4880) {                   //ext.w d0
                position += 6;
            } else {
                position += 4;
            }

            tempVal = readEndian(stream[position],stream[position+1]);position+=2;
            if (tempVal != 0x47fa) {                   //lea [xx,a3]
                patterns = 0;
            } else {
                commands = position + (signed short)readEndian(stream[position],stream[position+1]);position+=2;
            }
            break;
        case 0x1030:                                                    //move.b (a0,d0.w),d0
            position += 2;

            tempVal = readEndian(stream[position],stream[position+1]);position+=2;
            if (tempVal == 0x41fa) {                   //lea [xx,a0]
                pos = position + (signed short)readEndian(stream[position],stream[position+1]);position+=2;

                for (int i = 0; i < 50; ++i) {
                    value = readEndian(stream[position],stream[position+1]);position+=2;

                    if (value == 0xb03c || value == 0x0c00) {                 //cmp.b [xx,d0] || cmpi.b [xx,d0]
                        tempVal = readEndian(stream[position],stream[position+1]);position+=2;
                        if (tempVal == 0x00c1) {             //xx = $c1
                            if (m_variant) {
                                m_variant--;
                            } else {
                                m_variant++;
                            }
                            break;
                        }
                    }
                }

                position = length;
            }
            break;
        }
    } while (position < length-4);

    if (!tracks || !patterns || !commands || !periods) return -1;

    position = pos;

    std::vector<int> offsets = std::vector<int>();

    do {
        value = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);position+=4;

        if (value < lower) {
            lower = value;
            len = pos + lower;
        }

        offsets.push_back(value);
    } while (position < len);

    len = offsets.size();
    lower = 0xffff;
    samples = std::vector<BDSample*>(len);

    for (int i = 0; i < len; ++i) {
        position = pos + offsets[i];
        BDSample* sample = new BDSample();

        sample->pointer = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);position+=4;
        sample->loopPtr = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);position+=4;
        sample->length  = readEndian(stream[position],stream[position+1]) << 1;position+=2;
        sample->repeat  = readEndian(stream[position],stream[position+1]) << 1;position+=2;
        sample->volume  = readEndian(stream[position],stream[position+1]);position+=2;
        sample->word14  = (signed short)readEndian(stream[position],stream[position+1]);position+=2;
        sample->word16  = (signed short)readEndian(stream[position],stream[position+1]);position+=2;
        sample->word18  = readEndian(stream[position],stream[position+1]);position+=2;
        sample->word20  = readEndian(stream[position],stream[position+1]);position+=2;
        sample->word22  = readEndian(stream[position],stream[position+1]);position+=2;
        sample->word24  = (signed short)readEndian(stream[position],stream[position+1]);position+=2;
        sample->word26  = readEndian(stream[position],stream[position+1]);position+=2;

        if (sample->pointer < lower) lower = sample->pointer;
        samples[i] = sample;
    }

    pos += lower;
    position = pos;
    //amiga->store(stream, (length - pos),position,length);
    length = pos;

    for (int i = 0; i < len; ++i) {
        BDSample* sample = samples[i];
        sample->pointer -= lower;
        if (sample->loopPtr) sample->loopPtr -= lower;

        value = sample->pointer;
        //amiga->memory[value] = 0;
        //amiga->memory[++value] = 0;
    }

    position = tracks;
    songs = vector<int>();
    lower = 0xffff;

    do {
        value = readEndian(stream[position],stream[position+1]);position+=2;

        if (value < lower) {
            lower = value;
            len = tracks + lower;
        }

        songs.push_back(tracks + value);
    } while (position < len);

    //m_totalSongs = songs.size() >> 2;

    len = banks.size() >> 2;

    for (int i = 0; i < len; ++i) {
        banks[i] = i;
        banks[(i + len)] = i;
        banks[(i + (len * 2))] = i;
        banks[(i + (len * 3))] = i;
    }

    m_version = 1;
    format = "Ben Daglish";
    //printData();
    return 1;
}



std::vector<BaseSample*> BDPlayer::getSamples()
{
    std::vector<BaseSample*>samp (samples.size());
    for(int i =0; i< samples.size() ; i++)
    {
        samp[i] = samples[i];
        if(!samp[i])
        {
            samp[i] = new BaseSample();
        }
    }
    return samp;
}
