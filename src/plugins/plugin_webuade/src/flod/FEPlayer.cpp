#include "FEPlayer.h"
#include "FEVoice.h"
#include "FESample.h"
#include "FESong.h"
#include <iostream>
#include "MyEndian.h"

const int FEPlayer::PERIODS[72] = {
    8192, 7728, 7296, 6888, 6504, 6136, 5792, 5464, 5160,
    4872, 4600, 4336, 4096, 3864, 3648, 3444, 3252, 3068,
    2896, 2732, 2580, 2436, 2300, 2168, 2048, 1932, 1824,
    1722, 1626, 1534, 1448, 1366, 1290, 1218, 1150, 1084,
    1024, 966, 912, 861, 813, 767, 724, 683, 645,
    609, 575, 542, 512, 483, 456, 430, 406, 383,
    362, 341, 322, 304, 287, 271, 256, 241, 228,
    215, 203, 191, 181, 170, 161, 152, 143, 135
};

FEPlayer::FEPlayer(Amiga* amiga): AmigaPlayer(amiga)
{
    voices = vector<FEVoice*>(4);
    voices[3] = new FEVoice(3);
    voices[3]->next = voices[2] = new FEVoice(2);
    voices[2]->next = voices[1] = new FEVoice(1);
    voices[1]->next = voices[0] = new FEVoice(0);
}

FEPlayer::~FEPlayer()
{
    for (unsigned int i = 0; i < voices.size(); i++)
    {
        if (voices[i]) delete voices[i];
    }
    voices.clear();
    for (unsigned int i = 0; i < samples.size(); i++)
    {
        if (samples[i]) delete samples[i];
    }
    samples.clear();
    for (unsigned int i = 0; i < songs.size(); i++)
    {
        if (songs[i]) delete songs[i];
    }
    songs.clear();
    /*if (patterns)
    {
        delete[] patterns;
    }*/
}

int FEPlayer::load(void* _data, unsigned long int _length)
{
    m_version = 0;
    unsigned char* stream = static_cast<unsigned char*>(_data);
    int value = 0;
    position = 0;
    int data = 0;
    int base = 0;
    unsigned int pos = 0x7fffffff;

    while (position < 16)
    {
        value = readEndian(stream[position], stream[position + 1]);
        position += 2;
        position += 2;
        if (value != 0x4efa) return 0; //jmp
    }

    while (position < 1024)
    {
        value = readEndian(stream[position], stream[position + 1]);
        position += 2;

        if (value == 0x123a)
        {
            //move.b $x,d1
            position += 2;
            value = readEndian(stream[position], stream[position + 1]);
            position += 2;

            if (value == 0xb001)
            {
                //cmp.b d1,d0
                position -= 4;
                data = (position + readEndian(stream[position], stream[position + 1])) - 0x895;
                position += 2;
            }
        }
        else if (value == 0x214a)
        {
            //move.l a2,(a0)
            position += 2;
            value = readEndian(stream[position], stream[position + 1]);
            position += 2;

            if (value == 0x47fa)
            {
                //lea $x,a3
                base = position + (signed short)readEndian(stream[position], stream[position + 1]);
                position += 2;
                m_version = 1;
                break;
            }
        }
    }
    if (!m_version) return 0;

    position = data + 0x8a2;

    position = base + readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);

    samples = vector<FESample*>();

    while (pos > position)
    {
        value = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);
        position += 4;

        if (value)
        {
            if ((value < position) || (value >= _length))
            {
                position -= 4;
                break;
            }

            if (value < pos) pos = base + value;
        }

        FESample* sample = new FESample();
        sample->pointer = value;
        sample->loopPtr = (signed short)readEndian(stream[position], stream[position + 1]);
        position += 2;
        sample->length = readEndian(stream[position], stream[position + 1]) << 1;
        position += 2;
        sample->relative = readEndian(stream[position], stream[position + 1]);
        position += 2;

        sample->vibratoDelay = stream[position];
        position++;
        position++;
        sample->vibratoSpeed = stream[position];
        position++;
        sample->vibratoDepth = stream[position];
        position++;
        sample->envelopeVol = stream[position];
        position++;
        sample->attackSpeed = stream[position];
        position++;
        sample->attackVol = stream[position];
        position++;
        sample->decaySpeed = stream[position];
        position++;
        sample->decayVol = stream[position];
        position++;
        sample->sustainTime = stream[position];
        position++;
        sample->releaseSpeed = stream[position];
        position++;
        sample->releaseVol = stream[position];
        position++;

        sample->arpeggio = vector<signed char>(16);
        for (int i = 0; i < 16; ++i)
        {
            sample->arpeggio[i] = stream[position];
            position++;
        }

        sample->arpeggioSpeed = stream[position];
        position++;
        sample->type = stream[position];
        position++;
        sample->pulseRateNeg = stream[position];
        position++;
        sample->pulseRatePos = stream[position];
        position++;
        sample->pulseSpeed = stream[position];
        position++;
        sample->pulsePosL = stream[position];
        position++;
        sample->pulsePosH = stream[position];
        position++;
        sample->pulseDelay = stream[position];
        position++;
        sample->synchro = stream[position];
        position++;
        sample->blendRate = stream[position];
        position++;
        sample->blendDelay = stream[position];
        position++;
        sample->pulseCounter = stream[position];
        position++;
        sample->blendCounter = stream[position];
        position++;
        sample->arpeggioLimit = stream[position];
        position++;

        position += 12;
        samples.push_back(sample);
        if (position > _length) break;
    }
    if (pos != 0x7fffffff)
    {
        //amiga->store(stream, _length - pos,position,_length);
        int len = samples.size();


        for (int i = 0; i < len; ++i)
        {
            FESample* sample = samples[i];
            if (sample->pointer)
            {
                sample->pointer -= (base + pos);
            }
        }
    }

    //    pos = amiga->memory.size();
    //    amiga->memory.resize(amiga->memory.size()+256);
    //    amiga->loopLen = 100;

    //    for (int i = 0; i < 4; ++i) {
    //        voices[i]->synth = pos;
    //        pos += 64;
    //    }


    //    position = data + 0x8a2;
    //    unsigned int len = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);
    //    position+=4;
    //    pos = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);
    //    position+=4;

    //    position = base + pos;
    //    patterns = new signed char[len - pos];

    //    for(int i = 0;i<len - pos;i++)
    //    {
    //        patterns[i] = stream[position];
    //        position++;
    //    }

    //    pos += base;

    //    position = data + 0x895;
    //    len = stream[position]+1;
    //    position++;
    //    m_totalSongs = len;

    //    songs = vector<FESong*>(len);
    //    base = data + 0xb0e;
    //    unsigned int tracksLen = pos - base;
    //    pos = 0;

    //    int size = 0;
    //    for (int i = 0; i < len; ++i) {
    //        FESong* song = new FESong();

    //        for (int j = 0; j < 4; ++j) {
    //            position = base + pos;
    //            value = readEndian(stream[position],stream[position+1]);
    //            position+=2;

    //            if (j == 3 && (i == (len - 1)))
    //            {
    //                size = tracksLen;
    //            }
    //            else
    //            {
    //                size = readEndian(stream[position],stream[position+1]);
    //                position+=2;
    //            }

    //            size = (size - value) >> 1;
    //            if (size > song->length) song->length = size;

    //            song->tracks[j] = vector<int>(size);
    //            position = base + value;

    //            for (int ptr = 0; ptr < size; ++ptr)
    //            {
    //                song->tracks[j][ptr] = readEndian(stream[position],stream[position+1]);
    //                position+=2;
    //            }

    //            pos += 2;
    //        }

    //        position = data + i + 0x897;
    //        song->speed = stream[position];
    //        position++;
    //        songs[i] = song;
    //    }

    m_version = 1;
    format = "Fred Editor";
    //printData();
    return 1;
}


vector<BaseSample*> FEPlayer::getSamples()
{
    vector<BaseSample*> samp(samples.size());
    for (int i = 0; i < samples.size(); i++)
    {
        samp[i] = samples[i];
        if (!samp[i])
        {
            samp[i] = new BaseSample();
        }
    }
    return samp;
}
