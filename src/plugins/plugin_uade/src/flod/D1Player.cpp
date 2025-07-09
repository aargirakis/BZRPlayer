#include "D1Player.h"
#include "D1Sample.h"
#include "D1Voice.h"
#include "BaseRow.h"
#include "BaseStep.h"
#include <iostream>
#include "MyEndian.h"

using namespace std;

const int D1Player::PERIODS[84] =
{
    0, 6848, 6464, 6096, 5760, 5424, 5120, 4832, 4560, 4304, 4064, 3840,
    3616, 3424, 3232, 3048, 2880, 2712, 2560, 2416, 2280, 2152, 2032, 1920,
    1808, 1712, 1616, 1524, 1440, 1356, 1280, 1208, 1140, 1076, 960, 904,
    856, 808, 762, 720, 678, 640, 604, 570, 538, 508, 480, 452,
    428, 404, 381, 360, 339, 320, 302, 285, 269, 254, 240, 226,
    214, 202, 190, 180, 170, 160, 151, 143, 135, 127, 120, 113,
    113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113
};

D1Player::D1Player(Amiga* amiga): AmigaPlayer(amiga)
{
    samples = std::vector<D1Sample*>(21);
    voices = std::vector<D1Voice*>(4);
    pointers = std::vector<int>(4);

    voices[0] = new D1Voice(0);
    voices[0]->next = voices[1] = new D1Voice(1);
    voices[1]->next = voices[2] = new D1Voice(2);
    voices[2]->next = voices[3] = new D1Voice(3);
}

D1Player::~D1Player()
{
    pointers.clear();


    for (unsigned int i = 0; i < tracks.size(); i++)
    {
        if (tracks[i]) delete tracks[i];
    }
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
    for (unsigned int i = 0; i < patterns.size(); i++)
    {
        if (patterns[i]) delete patterns[i];
    }
    patterns.clear();
}

int D1Player::load(void* _data, unsigned long int length)
{
    unsigned char* stream = static_cast<unsigned char*>(_data);

    if (!(stream[0] == 'A' && stream[1] == 'L' && stream[2] == 'L' && stream[3] == ' '))
    {
        return -1;
    }

    unsigned int position = 4;
    const int position2 = 104;

    std::vector<unsigned int> data(25);
    for (int i = 0; i < 25; ++i)
    {
        data[i] = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);
        position += 4;
    }

    pointers = std::vector<int>(4);
    int j = 0;
    for (int i = 1; i < 4; ++i)
        pointers[i] = pointers[j] + (data[j++] >> 1) - 1;

    unsigned int len = pointers[3] + (data[3] >> 1) - 1;
    tracks = std::vector<BaseStep*>(len);
    int index = position2 + data[1] - 2;
    position = position2;
    j = 1;


    for (int i = 0; i < len; ++i)
    {
        BaseStep* step = new BaseStep();
        unsigned short value = readEndian(stream[position], stream[position + 1]);
        position += 2;

        if (value == 0xffff || position == index)
        {
            step->pattern = -1;
            step->transpose = readEndian(stream[position], stream[position + 1]);
            position += 2;
            index += data[j++];
        }
        else
        {
            position--;
            step->pattern = ((value >> 2) & 0x3fc0) >> 2;
            step->transpose = (signed char)stream[position];
            position++;
        }
        tracks[i] = step;
    }

    len = data[4] >> 2;
    patterns = std::vector<BaseRow*>(len);
    for (int i = 0; i < len; ++i)
    {
        BaseRow* row = new BaseRow();
        row->sample = stream[position];
        position++;
        row->note = stream[position];
        position++;
        row->effect = stream[position] & 31;
        position++;
        row->param = stream[position];
        position++;
        patterns[i] = row;
    }
    index = 5;

    for (int i = 0; i < 20; ++i)
    {
        if (data[index])
        {
            D1Sample* sample = new D1Sample();
            sample->attackStep = stream[position];
            position++;
            sample->attackDelay = stream[position];
            position++;
            sample->decayStep = stream[position];
            position++;
            sample->decayDelay = stream[position];
            position++;
            sample->sustain = readEndian(stream[position], stream[position + 1]);
            position += 2;
            sample->releaseStep = stream[position];
            position++;
            sample->releaseDelay = stream[position];
            position++;
            sample->volume = stream[position];
            position++;
            sample->vibratoWait = stream[position];
            position++;
            sample->vibratoStep = stream[position];
            position++;
            sample->vibratoLen = stream[position];
            position++;
            sample->pitchBend = (signed char)stream[position];
            position++;
            sample->portamento = stream[position];
            position++;
            sample->synth = stream[position];
            position++;
            sample->tableDelay = stream[position];
            position++;

            for (j = 0; j < 8; ++j)
            {
                sample->arpeggio[j] = (signed char)stream[position];
                position++;
            }

            sample->length = readEndian(stream[position], stream[position + 1]);
            position += 2;
            sample->loop = readEndian(stream[position], stream[position + 1]);
            position += 2;
            sample->repeat = readEndian(stream[position], stream[position + 1]) << 1;
            position += 2;
            sample->synth = sample->synth ? 0 : 1;

            if (sample->synth)
            {
                for (j = 0; j < 48; ++j)
                {
                    sample->table[j] = (signed char)stream[position];
                    position++;
                }

                len = data[index] - 78;
            }
            else
            {
                len = sample->length;
            }


            //sample->pointer = amiga->store(stream,len,position,length);
            sample->loopPtr = sample->pointer + sample->loop;
            samples[i] = sample;
        }
        else
        {
            samples[i] = 0;
        }
        index++;
    }

    D1Sample* sample = new D1Sample();
    //sample->pointer = sample->loopPtr = amiga->memory.size();
    sample->length = sample->repeat = 4;
    samples[20] = sample;

    m_version = 1;
    format = "Delta Music";
    //printData();
    return 1;
}

std::vector<BaseSample*> D1Player::getSamples()
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
