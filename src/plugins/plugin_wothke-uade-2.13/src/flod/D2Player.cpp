#include "D2Player.h"
#include "D2Voice.h"
#include "BaseRow.h"
#include "D2Sample.h"
#include "BaseStep.h"

#include <iostream>
#include "MyEndian.h"

const int D2Player::PERIODS[85] =
{
    0,6848,6464,6096,5760,5424,5120,4832,4560,4304,4064,3840,3616,3424,3232,
    3048,2880,2712,2560,2416,2280,2152,2032,1920,1808,1712,1616,1524,1440,1356,
    1280,1208,1140,1076,1016, 960, 904, 856, 808, 762, 720, 678, 640, 604, 570,
    538, 508, 480, 452, 428, 404, 381, 360, 339, 320, 302, 285, 269, 254, 240,
    226, 214, 202, 190, 180, 170, 160, 151, 143, 135, 127, 120, 113, 113, 113,
    113, 113, 113, 113, 113, 113, 113, 113, 113, 113
};
D2Player::D2Player(Amiga* amiga):AmigaPlayer(amiga)
{


    arpeggios = std::vector<signed char>(1024);
    voices    = std::vector<D2Voice*>(4);

    voices[0] = new D2Voice(0);
    voices[0]->next = voices[1] = new D2Voice(1);
    voices[1]->next = voices[2] = new D2Voice(2);
    voices[2]->next = voices[3] = new D2Voice(3);
}
D2Player::~D2Player()
{
    arpeggios.clear();
    data.clear();

    for(unsigned int i = 0; i < tracks.size(); i++)
    {
        if(tracks[i]) delete tracks[i];
    }
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
    for(unsigned int i = 0; i < patterns.size(); i++)
    {
        if(patterns[i]) delete patterns[i];
    }
    patterns.clear();
}


int D2Player::load(void* _data, unsigned long int length)
{
    //std::string n[] = { "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"};
    if ( length<3017 ) {
        return -1;
    }
    unsigned int position = 3014;
    unsigned char *stream = static_cast<unsigned char*>(_data);
    bool isOk = -1;


    if(stream[3014]=='.' && stream[3015]=='F' && stream[3016]=='N' && stream[3017]=='L')
    {
        isOk = 1;
    }
    if(!isOk)
    {
        return -1;
    }
    position = 4042;

    data = std::vector<int>(12);



    int len=0;
    int value=0;
    for (int i = 0; i < 4; ++i)
    {
        data[i + 4] = readEndian(stream[position],stream[position+1]) >> 1;
        position+=2;
        value = readEndian(stream[position],stream[position+1]) >> 1;
        position+=2;
        data[int(i + 8)] = value;
        len += value;

    }
    value = len;
    for (int i = 3; i > 0; --i)
    {
        data[i] = (value -= data[int(i + 8)]);
    }
    tracks = std::vector<BaseStep*>(len);
    for (int i = 0; i < len; ++i) {
        BaseStep* step = new BaseStep();
        step->pattern   = stream[position] << 4;
        position++;
        step->transpose = (signed char)stream[position];
        position++;
        tracks[i] = step;

    }


    len = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]) >> 2;
    position+=4;
    patterns = std::vector<BaseRow*>(len);

    for (int i = 0; i < len; ++i) {
        BaseRow* row = new BaseRow();
        row->note   = stream[position];
        position++;
        row->sample = stream[position];
        position++;
        row->effect  = stream[position] - 1;
        position++;
        row->param  = stream[position];
        position++;
        patterns[i] = row;
        //std::cout << "note: " << n[(row->note-1)%12] << (row->note/12)+1 << " sample: " << row->sample << " data1: " << row->data1 << " data2: " << row->data2 << "\n";
    }

    position+=254;
    value= readEndian(stream[position],stream[position+1]);
    position+=2;
    int positionMark = position;
    position-=256;
    len = 1;
    std::vector<int> offsets = std::vector<int>(128);
    for (int i = 0; i < 128; ++i) {
        int j = readEndian(stream[position],stream[position+1]);
        position+=2;
        if (j != value) offsets[len++] = j;
    }
    samples = std::vector<D2Sample*>(len);


    for (int i = 0; i < len; ++i)
    {
        position = positionMark + offsets[i];
        D2Sample* sample = new D2Sample();
        sample->length = readEndian(stream[position],stream[position+1]) << 1;
        position+=2;
        sample->loopPtr   = readEndian(stream[position],stream[position+1]);
        position+=2;
        sample->repeat = readEndian(stream[position],stream[position+1]) << 1;
        position+=2;

        for (int j = 0; j < 15; ++j)
        {
            sample->volumes[j] = stream[position];
            position++;
        }
        for (int j = 0; j < 15; ++j)
        {
            sample->vibratos[j] = stream[position];
            position++;
        }

        sample->pitchBend = readEndian(stream[position],stream[position+1]);
        position+=2;
        sample->synth     = (signed char)stream[position];
        position++;
        sample->index     = stream[position];
        position++;

        for (int j = 0; j < 48; ++j)
        {
            sample->table[j] = stream[position];
            position++;
        }

        samples[i] = sample;
    }

    len = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);
    position+=4;
    //amiga->store(stream, len, position, length);

    position+=64;
    for (int i = 0; i < 8; ++i)
    {
        offsets[i] = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);
        position+=4;
    }

    len = samples.size();
    positionMark = position;

    for (int i = 0; i < len; ++i)
    {
      D2Sample* sample = samples[i];
      if (sample->synth >= 0) continue;
      position = positionMark + offsets[sample->index];
      //sample->pointer = amiga->store(stream, sample->length,position,length);
      sample->loopPtr+= sample->pointer;
    }
    position = 3018;
    for (int i = 0; i < 1024; ++i)
    {
        arpeggios[i] = (signed char)stream[position];
        position++;
    }

    D2Sample* sample = new D2Sample();
    //sample->pointer = sample->loopPtr = amiga->memory.size();
    sample->length  = sample->repeat  = 4;
    samples.push_back(sample);


    len = patterns.size();
    int j = samples.size() - 1;

    for (int i = 0; i < len; ++i) {
      BaseRow* row = patterns[i];
      if (row->sample > j) row->sample = 0;
    }


    m_version = 2;
    format = "Delta Music 2";
    //printData();
    return 1;

}
std::vector<BaseSample*> D2Player::getSamples()
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
    //std::cout << "returning samples, size: " << samp.size() << "\n";
    return samp;
}
