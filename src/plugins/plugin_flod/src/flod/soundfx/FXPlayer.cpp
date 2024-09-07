#include "FXPlayer.h"
#include "FXVoice.h"
#include "AmigaChannel.h"
#include "BaseSample.h"
#include "BaseRow.h"
#include "MyEndian.h"
#include <iostream>
using namespace std;
const int FXPlayer::PERIODS[67] =
{
    1076,1016,960,906,856,808,762,720,678,640,604,570,
    538, 508,480,453,428,404,381,360,339,320,302,285,
    269, 254,240,226,214,202,190,180,170,160,151,143,
    135, 127,120,113,113,113,113,113,113,113,113,113,
    113, 113,113,113,113,113,113,113,113,113,113,113,
    113, 113,113,113,113,113,-1
};
const char* FXPlayer::NOTES[67] =
{
    "C 1", "C#1", "D 1", "D#1", "E 1", "F 1", "F#1", "G 1", "G#1", "A 1", "A#1", "B 1",
    "C 2", "C#2", "D 2", "D#2", "E 2", "F 2", "F#2", "G 2", "G#2", "A 2", "A#2", "B 2",
    "C 3", "C#3", "D 3", "D#3", "E 3", "F 3", "F#3", "G 3", "G#3", "A 3", "A#3", "B 3",
    "C 4", "C#4", "D 4", "D#4", "---", "F 4", "F#4", "G 4", "G#4", "A 4", "A#4", "B 4",
    "C 5", "C#5", "D 5", "D#5", "E 5", "F 5", "F#5", "G 5", "G#5", "A 5", "A#5", "B 5",
    "C 6", "C#6", "D 6", "D#6", "E 6", "F 6", "???"
};
FXPlayer::FXPlayer(Amiga* amiga):AmigaPlayer(amiga)
{
    trackPosBuffer = std::list<int>();
    patternPosBuffer = std::list<int>();
    track = std::vector<int>(128);

    voices    = std::vector<FXVoice*>(4);

    voices[0] = new FXVoice(0);
    voices[0]->next = voices[1] = new FXVoice(1);
    voices[1]->next = voices[2] = new FXVoice(2);
    voices[2]->next = voices[3] = new FXVoice(3);
    jumpFlag=0;
    delphine=0;

}

FXPlayer::~FXPlayer()
{
    track.clear();
    trackPosBuffer.clear();
    patternPosBuffer.clear();
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

void FXPlayer::initialize()
{

    AmigaPlayer::initialize();
    setNTSC(m_ntsc);
    speed      = 6;
    trackPos   = 0;
    patternPos = 0;
    jumpFlag   = 0;

    FXVoice* voice = voices[0];
    do {
        voice->initialize();
        voice->channel = amiga->channels[voice->index];
        voice->sample  = samples[0];
    }
    while (voice = voice->next);
    magicTempoNumber = 325000/tempo;


}
void FXPlayer::setNTSC(int value)
{
    AmigaPlayer::setNTSC(value);
    amiga->samplesTick = int((tempo / 122) * (value ? 7.5152005551 : 7.58437970472));
}

int FXPlayer::load(void* data, unsigned long int length)
{
    unsigned char *stream = static_cast<unsigned char*>(data);
    unsigned int base;
    unsigned int len;

    if(length < 1686) return -1;

    unsigned int position=64;
    if(!(stream[60]=='S' && stream[61]=='O' && stream[62]=='N' && stream[63]=='G'))
    {

        position=128;
        if(!(stream[124]=='S' && stream[125]=='O' && stream[126]=='3' && stream[127]=='1') ) return -1;
        if(length<2350) return -1 ;
        base = 544;
        len = 32;
        m_version = SOUNDFX_20;
        format = "SoundFX 2.0";
    }
    else
    {
        base = 0;
        len = 16;
        m_version = SOUNDFX_10;
        format = "SoundFX 1.0";
    }
    samples = std::vector<BaseSample*>(len);
    tempo = readEndian(stream[position],stream[position+1]);

    position=0;
    unsigned int size = 0;
    unsigned int value;
    for (unsigned int i = 1; i < len; ++i)
    {
        value = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);
        position+=4;
        if (value)
        {
            BaseSample* sample = new BaseSample();
            sample->pointer = size;
            size += value;
            samples[i] = sample;
        }
    }
    position += 20;

    for (int i = 1; i < len; ++i)
    {
        BaseSample* sample = samples[i];
        if (!sample)
        {
            position += 30;
            continue;
        }

        const int STRING_LENGTH = 22;
        for(int j = 0;j<STRING_LENGTH;j++)
        {
            if(!stream[position+j])
            {
                break;
            }
            sample->name+=stream[position+j];
        }
        position+=STRING_LENGTH;

        sample->length = readEndian(stream[position],stream[position+1]) << 1;
        position+=2;
        sample->volume = readEndian(stream[position],stream[position+1]);
        position+=2;
        sample->loopPtr   = readEndian(stream[position],stream[position+1]);
        position+=2;
        sample->repeat = readEndian(stream[position],stream[position+1]) << 1;
        position+=2;
    }

    position = base + 530;
    this->length = len = stream[position];
    position++;
    position++;
    unsigned int higher = 0;
    for (unsigned int i = 0; i < len; ++i)
    {
        value = stream[position] << 8;
        track[i] = value;
        if (value > higher) higher = value;
        position++;
    }

    if (base) base += 4;
    position = base + 660;
    higher += 256;
    patterns =  std::vector<BaseRow*>(higher);

    len = samples.size();

    for (int i = 0; i < higher; ++i)
    {
        BaseRow* row = new BaseRow();
        row->note   = (signed short)readEndian(stream[position],stream[position+1]);
        position+=2;
        value      = stream[position];
        position++;
        row->param  = stream[position];
        position++;
        row->effect = value & 0x0f;
        row->sample = value >> 4;
        patterns[i] = row;
        if (m_version == SOUNDFX_20)
        {
            if (row->note & 0x1000)
            {
                row->sample += 16;
                if (row->note > 0) row->note &= 0xefff;
            }
        }
        else
        {
            if (row->effect == 9 || row->note > 856)
            {
                m_version = SOUNDFX_18;
                format = "SoundFX 1.8";
            }

            if (row->note < -3)
            {
                m_version = SOUNDFX_19;
                format = "SoundFX 1.9";
            }
        }

        int j = 0;
        for (j = 0; j < 40; ++j)
            if (row->note >= PERIODS[j]) break;

        row->noteText = NOTES[j];
        if (row->sample >= len || !samples[row->sample]) row->sample = 0;
    }

    amiga->store(stream,size,position,length);


    BaseSample* sample;
    for (int i = 1; i < len; ++i)
    {
        sample = samples[i];

        if (!sample) continue;

        sample->loopPtr += sample->pointer;
        size = sample->pointer + 4;
        for (int j = sample->pointer; j < size; ++j) amiga->memory[j] = 0;
    }

    sample = new BaseSample();
    sample->pointer = sample->loopPtr = amiga->memory.size();
    sample->length  = sample->repeat  = 4;
    samples[0] = sample;


    position = higher = delphine = 0;
    for (int i = 0; i < 265; ++i)
    {
        higher += readEndian(stream[position],stream[position+1]);
        position+=2;
    }

    switch (higher) {
    case 172662:
    case 1391423:
    case 1458300:
    case 1706977:
    case 1920077:
    case 1920694:
    case 1677853:
    case 1931956:
    case 1926836:
    case 1385071:
    case 1720635:
    case 1714491:
    case 1731874:
    case 1437490:
        delphine = 1;
        break;
    }

    //printData();
    return 1;
}
void FXPlayer::setVersion(int value)
{
    if (value < SOUNDFX_10) value = SOUNDFX_10;
    else if (value > SOUNDFX_20) value = SOUNDFX_20;

    m_version = value;
}

void FXPlayer::process()
{

    AmigaChannel* chan;
    BaseRow* row;
    BaseSample* sample;
    int value=0;
    int period = 0;
    int index = 0;

    if(trackPosBuffer.size()==magicTempoNumber)
    {
        trackPosBuffer.pop_front();
        patternPosBuffer.pop_front();
    }
    trackPosBuffer.push_back(patternPos/4);
    patternPosBuffer.push_back(track[trackPos]/256);
    FXVoice* voice=voices[0];

    if (!tick) {
        value = track[trackPos] + patternPos;

        do {
            chan = voice->channel;

            row = patterns[int(voice->index + value)];
            voice->period = row->note;
            voice->effect = row->effect;
            voice->param  = row->param;

            if (row->note == -3) {
                voice->effect = 0;
                continue;
            }

            if (row->sample) {
                sample = voice->sample = samples[row->sample];
                voice->volume = sample->volume;
                if (voice->effect == 5)
                {
                    voice->volume += voice->param;
                }
                else if (voice->effect == 6)
                {
                    voice->volume -= voice->param;
                }
                chan->setVolume(voice->volume);

            } else {
                sample = voice->sample;
            }

            if (voice->period) {
                voice->last = voice->period;
                voice->slideSpeed = 0;
                voice->stepSpeed  = 0;

                chan->setEnabled(0);

                switch (voice->period) {
                case -2:
                    chan->setVolume(0);
                    break;
                case -4:
                    jumpFlag = 1;
                    break;
                case -5:
                    break;
                default:
                    chan->pointer = sample->pointer;
                    chan->length  = sample->length;
                    if (delphine)
                    {
                        chan->setPeriod(voice->period << 1);
                    }
                    else
                    {
                        chan->setPeriod(voice->period);
                    }
                    break;
                }

                if (row->sample)
                {
                    chan->setEnabled(1);
                    chan->pointer = sample->loopPtr;
                    chan->length  = sample->repeat;
                }
            }

        }
        while (voice = voice->next);

    } else {
        do {
            chan = voice->channel;

            if (m_version == SOUNDFX_18 && voice->period == -3) continue;


            if (voice->stepSpeed) {
                voice->stepPeriod += voice->stepSpeed;

                if (voice->stepSpeed < 0) {
                    if (voice->stepPeriod < voice->stepWanted) {
                        voice->stepPeriod = voice->stepWanted;
                        if (m_version > SOUNDFX_18) voice->stepSpeed = 0;
                    }
                } else {
                    if (voice->stepPeriod > voice->stepWanted) {
                        voice->stepPeriod = voice->stepWanted;
                        if (m_version > SOUNDFX_18) voice->stepSpeed = 0;
                    }
                }

                if (m_version > SOUNDFX_18) voice->last = voice->stepPeriod;
                chan->setPeriod(voice->stepPeriod);
            } else {
                if (voice->slideSpeed) {
                    value = voice->slideParam & 0x0f;

                    if (value) {
                        if (++voice->slideCtr == value) {
                            voice->slideCtr = 0;
                            value = (voice->slideParam << 4) << 3;

                            if (voice->slideDir) {
                                voice->slidePeriod -= 8;
                                value = voice->slideSpeed;
                            }
                            else
                            {
                                voice->slidePeriod += 8;
                                value += voice->slideSpeed;
                            }

                            if (value == voice->slidePeriod) voice->slideDir ^= 1;
                            chan->setPeriod(voice->slidePeriod);
                        } else {
                            continue;
                        }
                    }
                }

                value = 0;

                switch (voice->effect) {
                case 0:
                    break;
                case 1:   //arpeggio
                    value = tick % 3;
                    index = 0;

                    if (value == 2) {
                        chan->setPeriod(voice->last);
                        continue;
                    }

                    if (value == 1) value = voice->param & 0x0f;
                    else value = voice->param >> 4;

                    while (voice->last != PERIODS[index]) index++;
                    chan->setPeriod(PERIODS[int(index + value)]);
                    break;
                case 2:   //pitchbend
                    value = voice->param >> 4;
                    if (value) voice->period += value;
                    else voice->period -= voice->param & 0x0f;
                    chan->setPeriod(voice->period);
                    break;
                case 3:   //filter on
                    amiga->filter->active = 1;
                    break;
                case 4:   //filter off
                    amiga->filter->active = 0;
                    break;
                case 8:   //step down
                    value = -1;
                case 7:   //step up
                    voice->stepSpeed = voice->param & 0x0f;
                    voice->stepPeriod = (m_version > SOUNDFX_18) ? voice->last : voice->period;
                    if (value < 0) voice->stepSpeed = -voice->stepSpeed;
                    index = 0;

                    while (true) {
                        period = PERIODS[index];
                        if (period == voice->stepPeriod) break;
                        if (period < 0) {
                            index = -1;
                            break;
                        } else
                        {
                            index++;
                        }
                    }

                    if (index > -1) {
                        period = voice->param >> 4;
                        if (value > -1) period = -period;
                        index += period;
                        if (index < 0) index = 0;
                        voice->stepWanted = PERIODS[index];
                    } else
                    {
                        voice->stepWanted = voice->period;
                    }
                    break;
                case 9:   //auto slide
                    voice->slideSpeed = voice->slidePeriod = voice->period;
                    voice->slideParam = voice->param;
                    voice->slideDir = 0;
                    voice->slideCtr = 0;
                    break;
                }
            }
        } while (voice = voice->next);
    }

    if (++tick == speed) {
        tick = 0;
        patternPos += 4;

        if (patternPos == 256 || jumpFlag) {
            patternPos = jumpFlag = 0;

            if (++trackPos == length) {
                trackPos = 0;
                amiga->setComplete(1);
            }
        }
    }

}

void FXPlayer::printData()
{
//    for(unsigned int i = 0; i < samples.size(); i++)
//    {
//        AmigaSample* sample = samples[i];
//        if(sample)
//        {
//            std::cout << sample->name << "\n";
//        }
//    }
//    for(unsigned int i = 0; i < patterns.size(); i++)
//    {
//        AmigaRow* row= patterns[i];
//        std::cout << "Pattern [" << i << "] note: " << row->note << " sample: " << row->sample << " param: " << row->param << " effect: " << row->effect << "\n";
//    }
//    for(unsigned int i = 0; i < samples.size(); i++)
//    {
//        AmigaSample* sample = samples[i];
//        if(sample)
//        {
//            std::cout << "Sample [" << i << "] length: " << sample->length << " loop: " << sample->loop << " loopPtr: " << sample->loopPtr << " name: " << sample->name << " pointer: " << sample->pointer << " repeat: " << sample->repeat<< " volume: " << (int)sample->volume << "\n";
//        }
//    }
//    for(unsigned int i = 0; i < track.size(); i++)
//    {
//        std::cout << "Tracks [" << i << "] " << track[i] <<  "\n";
//    }
//    ////    for(int i = 0; i < amiga->memory.size(); i++)
//    ////    {
//    ////        std::cout << "Memory [" << i << "]" << (int)amiga->memory[i] <<  "\n";
//    ////    }

//    std::flush(std::cout);
}
std::vector<BaseSample*> FXPlayer::getSamples()
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
unsigned int FXPlayer::getCurrentRow()
{
    return trackPosBuffer.front();
}
unsigned int FXPlayer::getCurrentPattern()
{
    return patternPosBuffer.front();
}
void FXPlayer::getModRows(std::vector<BaseRow*>& vect)
{
    vect = patterns;
}

