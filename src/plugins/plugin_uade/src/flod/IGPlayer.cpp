#include "IGPlayer.h"
#include "IGVoice.h"
#include "BaseRow.h"
#include "D2Sample.h"
#include <iostream>
#include <fstream>
#include "MyEndian.h"

using namespace std;

const int IGPlayer::PERIODS[102] =
{
    27340, 25804, 24357, 22990, 21699, 20483, 19334, 18247,
    17222, 16267, 15347, 14482, 13672, 12905, 12179, 11498,
    10854, 10241, 9574, 9125, 8623, 8132, 7678, 7246,
    6844, 6454, 6092, 5750, 5427, 5121, 4836, 4565,
    4308, 4067, 3838, 3622, 3419, 3227, 3045, 2875,
    2715, 2562, 2418, 2281, 2153, 2033, 1919, 1811,
    1709, 1613, 1522, 1437, 1357, 1280, 1208, 1141,
    1077, 1016, 959, 906, 854, 806, 761, 719,
    678, 640, 604, 570, 538, 508, 480, 453,
    427, 403, 381, 359, 339, 320, 302, 285,
    269, 254, 240, 226, 214, 202, 190, 180,
    170, 160, 151, 143, 135, 127, 120, 112,
    96, 80, 64, 48, 32, 16
};
const int IGPlayer::TICKS[12] =
{
    2, 3, 4, 6, 8, 12, 16, 24, 32, 48, 64, 96
};

IGPlayer::IGPlayer(Amiga* amiga): AmigaPlayer(amiga)
{
    irqtime = 0;
    voices = std::vector<IGVoice*>(4);

    voices[0] = new IGVoice(0);
    voices[0]->next = voices[1] = new IGVoice(1);
    voices[1]->next = voices[2] = new IGVoice(2);
    voices[2]->next = voices[3] = new IGVoice(3);
}

IGPlayer::~IGPlayer()
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

    comData.clear();
    perData.clear();
    volData.clear();
}


int IGPlayer::load(void* _data, unsigned long int length, const char* filename)
{
    //AmigaPlayer::load(_data, length, filename);
    unsigned char* stream = static_cast<unsigned char*>(_data);
    string str_orgfilename = filename;
    string str_newfilename = filename;

    ifstream file;

    int cut = 4;
    do
    {
        str_newfilename = str_orgfilename.substr(0, str_orgfilename.length() - cut) + ".ins";
        file.open(str_newfilename.c_str(), ios::in | ios::binary | ios::ate);
        cut++;
    }
    while (!file.is_open() && cut < str_orgfilename.length() && str_orgfilename.substr(
        str_orgfilename.length() - cut - 1, 1) != "/");


    ifstream::pos_type fileSize;
    char* extra = 0;
    if (file.is_open())
    {
        fileSize = file.tellg();
        extra = new char[fileSize];

        file.seekg(0, ios::beg);

        if (!file.read(extra, fileSize))
        {
            //failed reading
            file.close();
            return -1;
        }
        file.close();
    }
    else
    {
        return -1;
    }
    if (!extra) return -1;
    unsigned int position = 0;
    if (fileSize != (readEndian(extra[position], extra[position + 1], extra[position + 2], extra[position + 3]) + 4))
        return -1;
    position += 4;
    switch (fileSize)
    {
    case 54832: //Gobliins 2 (all)
    case 27312: //Goblins 3 (all)
    case 82990: //Ween The Prophecy (musx)
    case 87800: //Ween The Prophecy (ween)
        irqtime = 589; //irq = $24ff
    case 37732: //Horror Zombies from the Crypt
        irqtime = 436; //irq = $1b66
    default: //remaining modules
        irqtime = 414; //irq = $19ff
    }


    unsigned int begin = readEndian(extra[position], extra[position + 1], extra[position + 2], extra[position + 3]);
    position += 4;
    unsigned int len = begin >> 4;

    samples = std::vector<BaseSample*>(len);
    position = 4;
    for (int i = 0; i < len; ++i)
    {
        BaseSample* sample = new BaseSample();
        sample->pointer = readEndian(extra[position], extra[position + 1], extra[position + 2], extra[position + 3]) -
            begin;
        position += 4;
        sample->loopPtr = readEndian(extra[position], extra[position + 1], extra[position + 2], extra[position + 3]) -
            begin;
        position += 4;

        position += 4;
        sample->length = readEndian(extra[position], extra[position + 1]) << 1;
        position += 2;
        sample->repeat = readEndian(extra[position], extra[position + 1]) << 1;
        position += 2;
        samples[i] = sample;
    }

    //amiga->store(extra, (fileSize - position),position,fileSize);

    position = 0;
    begin = readEndian(stream[position], stream[position + 1]);
    position += 2;
    speed = readEndian(stream[position], stream[position + 1]);
    position += 2;
    vector<int> pointers(8);
    for (int i = 0; i < 8; ++i)
    {
        pointers[i] = readEndian(stream[position], stream[position + 1]);
        position += 2;
    }

    if (length != (begin + pointers[6])) return -1;

    pointers[6] = pointers[7];
    pointers[7] = position - 2;

    position = begin + pointers[0];
    len = pointers[1] - pointers[0];


    volData = vector<int>(len);
    for (int i = 0; i < len; ++i)
    {
        volData[i] = (signed char)stream[position];
        position++;
    }

    position = begin + pointers[1];
    len = length - position;

    perData = vector<int>(len);
    for (int i = 0; i < len; ++i)
    {
        perData[i] = (signed char)stream[position];
        position++;
    }

    position = begin + pointers[6];
    len = pointers[0] - pointers[6];

    comData = vector<int>(len);
    for (int i = 0; i < len; ++i)
    {
        comData[i] = (signed char)stream[position];
        position++;
    }
    position = pointers[7];
    len = ((begin + pointers[2]) - pointers[7]) >> 1;

    vector<int> offsets(len);
    for (int i = 0; i < len; ++i)
    {
        offsets[i] = readEndian(stream[position], stream[position + 1]);
        position += 2;
    }

    for (int i = 2; i < 6; ++i)
    {
        position = begin + pointers[i];
        len = pointers[i + 1] - pointers[i];
        vector<int> track(len);

        int value;
        for (int j = 0; j < len; ++j)
        {
            value = stream[position];
            position++;
            if (value != 0xff)
            {
                track[j] = offsets[value] - pointers[6];
            }
            else
            {
                track[j] = value;
            }

            voices[i - 2]->track = track;
        }
    }

    m_version = 1;
    format = "Infogrames";
    //printData();
    return 1;
}

std::vector<BaseSample*> IGPlayer::getSamples()
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
    return samp;
}
