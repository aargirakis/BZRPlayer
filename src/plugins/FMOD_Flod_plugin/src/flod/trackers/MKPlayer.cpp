#include "MKPlayer.h"
#include "MKVoice.h"
#include "AmigaRow.h"
#include "AmigaSample.h"

#include "AmigaChannel.h"
#include <iostream>
#include "MyEndian.h"
#include <math.h>

const int MKPlayer::PERIODS[37] =
{
    856,808,762,720,678,640,604,570,538,508,480,453,
    428,404,381,360,339,320,302,285,269,254,240,226,
    214,202,190,180,170,160,151,143,135,127,120,113,0
};

const int MKPlayer::VIBRATO[32] =
{
    0, 24, 49, 74, 97,120,141,161,180,197,212,224,
    235,244,250,253,255,253,250,244,235,224,212,197,
    180,161,141,120, 97, 74, 49, 24
};

const char* MKPlayer::NOTES[38] =
{
    "C-1", "C#1", "D-1", "D#1", "E-1", "F-1", "F#1", "G-1", "G#1", "A-1", "A#1", "B-1",
    "C-2", "C#2", "D-2", "D#2", "E-2", "F-2", "F#2", "G-2", "G#2", "A-2", "A#2", "B-2",
    "C-3", "C#3", "D-3", "D#3", "E-3", "F-3", "F#3", "G-3", "G#3", "A-3", "A#3", "B-3",
    "---", "---"
};
MKPlayer::MKPlayer(Amiga* amiga):AmigaPlayer(amiga)
{
    trackPosBuffer = std::list<int>();
    patternPosBuffer = std::list<int>();
    track = std::vector<int>(128);
    samples    = std::vector<AmigaSample*>(32);
    voices    = std::vector<MKVoice*>(4);

    voices[0] = new MKVoice(0);
    voices[0]->next = voices[1] = new MKVoice(1);
    voices[1]->next = voices[2] = new MKVoice(2);
    voices[2]->next = voices[3] = new MKVoice(3);
}
MKPlayer::~MKPlayer()
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

void MKPlayer::initialize()
{

    AmigaPlayer::initialize();

    setForce(version);
    speed       = 6;
    trackPos    = 0;
    patternPos  = 0;
    jumpFlag    = 0;

    MKVoice* voice = voices[0];

    while (voice)
    {
        voice->initialize();
        voice->channel   = amiga->channels[voice->index];
        voice->sample  = samples[0];
        voice = voice->next;
    }


}
void MKPlayer::setForce(int value)
{
    if (value >= SOUNDTRACKER_23 && value<=NOISETRACKER_20)
    {
        version = value;
    }

    if (version == NOISETRACKER_20) vibratoDepth = 6;
    else vibratoDepth = 7;

    if (version == NOISETRACKER_10) {
        restartSave = restart;
        restart = 0;
    } else {
        restart = restartSave;
        restartSave = 0;
    }
}
int MKPlayer::load(void* data, unsigned long int _length)
{
    unsigned char *stream = static_cast<unsigned char*>(data);
    if(_length<2106) return -1;
    unsigned int position = 0;
    unsigned int higher = 0;
    unsigned int size = 0;
    unsigned int value=0;
    AmigaSample* sample;
    AmigaRow* row;

    if(
            !(stream[1080]=='M' && stream[1081]=='.' && stream[1082]=='K' && stream[1083]=='.') &&
            !(stream[1080]=='F' && stream[1081]=='L' && stream[1082]=='T' && stream[1083]=='4') &&
            !(stream[1080]=='M' && stream[1081]=='&' && stream[1082]=='K' && stream[1083]=='!'))
    {
        return -1;
    }



    const int STRING_LENGTH = 20;
    for(int j = 0;j<STRING_LENGTH;j++)
    {
        if(!stream[position+j])
        {
            break;
        }
        title+=stream[position+j];
    }
    position+=STRING_LENGTH;
    version = SOUNDTRACKER_23;

    position += 22;
    for (int i = 1; i < 32; ++i) {
        value = readEndian(stream[position],stream[position+1]);position+=2;

        if (!value) {
            samples[i] = 0;
            position += 28;
            continue;
        }

        sample = new AmigaSample();
        position -= 24;

        const int STRING_LENGTH_SAMPLE = 22;
        for(int j = 0;j<STRING_LENGTH_SAMPLE;j++)
        {
            if(!stream[position+j])
            {
                break;
            }
            sample->name+=stream[position+j];
        }
        position+=STRING_LENGTH_SAMPLE;

        sample->length = value << 1;
        position += 3;

        sample->volume = stream[position];position++;
        sample->loop   = readEndian(stream[position],stream[position+1]) << 1;position+=2;
        sample->repeat = readEndian(stream[position],stream[position+1]) << 1;position+=2;

        position += 22;
        sample->pointer = size;
        size += sample->length;
        samples[i] = sample;

        if (sample->length > 32768)
            version = SOUNDTRACKER_24;
    }

    position = 950;
    length  = stream[position];position++;
    value   = stream[position];position++;
    restart =  value < length ? value : 0;

    for (int i = 0; i < 128; ++i) {
        value = stream[position] << 8;position++;
        track[i] = value;
        if (value > higher) higher = value;
    }

    position = 1084;
    higher += 256;
    patterns    = std::vector<AmigaRow*>(higher);

    for (int i = 0; i < higher; ++i) {
        row = new AmigaRow();
        value = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);position+=4;

        row->note   = (value >> 16) & 0x0fff;
        row->effect = (value >>  8) & 0x0f;
        row->sample = (value >> 24) & 0xf0 | (value >> 12) & 0x0f;
        row->param  = value & 0xff;
        int j = 0;
        for (j = 0; j < 37; ++j)
          if (row->note >= PERIODS[j]) break;

        row->noteText = NOTES[j];

        patterns[i] = row;

        if (row->sample > 31 || !samples[row->sample]) row->sample = 0;

        if (row->effect == 3 || row->effect == 4)
            version = NOISETRACKER_10;

        if (row->effect == 5 || row->effect == 6)
            version = NOISETRACKER_20;

        if (row->effect > 6 && row->effect < 10) {
            version = 0;
            return 0;
        }
    }

    amiga->store(stream, size, position,_length);

    for (int i = 1; i < 32; ++i) {
        sample = samples[i];
        if (!sample) continue;
        if (sample->name.find("2.0") !=std::string::npos)
            version = NOISETRACKER_20;

        if (sample->loop) {
            sample->loopPtr = sample->pointer + sample->loop;
            sample->length  = sample->loop + sample->repeat;
        } else {
            sample->loopPtr = amiga->memory.size();
            sample->repeat  = 2;
        }
        size = sample->pointer + 4;

        for (int j = sample->pointer; j < size; ++j) amiga->memory[j] = 0;


    }


    sample = new AmigaSample();
    sample->pointer = sample->loopPtr = amiga->memory.size();
    sample->length  = sample->repeat  = 2;
    samples[0] = sample;

    if (version < NOISETRACKER_20 && restart != 127)
    {
        version = NOISETRACKER_11;
    }

    switch(version)
    {
    case SOUNDTRACKER_23:
        format = "Soundtracker 2.3";
        break;
    case SOUNDTRACKER_24:
        format = "Soundtracker 2.4";
        break;
    case NOISETRACKER_10:
        format = "Noisetracker 1.0";
        break;
    case NOISETRACKER_11:
        format = "Noisetracker 1.1";
        break;
    case NOISETRACKER_20:
        format = "Noisetracker 2.0";
        break;
    }

    //printData();
    return version;

}
void MKPlayer::process()
{
    int value = 0;
    int pattern = 0;
    int period = 0;
    int len = 0;
    int slide = 0;

    MKVoice* voice = voices[0];
    AmigaChannel* chan;
    AmigaRow* row;
    AmigaSample* sample;
    if(trackPosBuffer.size()==22)
    {
        trackPosBuffer.pop_front();
        patternPosBuffer.pop_front();
    }
    trackPosBuffer.push_back(patternPos/4);
    patternPosBuffer.push_back(track[trackPos]/256);
    if (!tick) {
        pattern = track[trackPos] + patternPos;

        while (voice) {
            chan = voice->channel;
            voice->enabled = 0;

            row = patterns[int(pattern + voice->index)];
            voice->effect = row->effect;
            voice->param  = row->param;

            if (row->sample) {
                sample = voice->sample = samples[row->sample];
                voice->volume = sample->volume;
                chan->setVolume(sample->volume);
            } else {
                sample = voice->sample;
            }

            if (row->note) {
                //std::cout << "note " << row->noteText << "\n";
                if (voice->effect == 3 || voice->effect == 5) {
                    if (row->note < voice->period) {
                        voice->portaDir = 1;
                        voice->portaPeriod = row->note;
                    } else if (row->note > voice->period) {
                        voice->portaDir = 0;
                        voice->portaPeriod = row->note;
                    } else {
                        voice->portaPeriod = 0;
                    }
                } else {
                    voice->enabled = 1;
                    voice->vibratoPos = 0;

                    chan->setEnabled(0);
                    chan->pointer = sample->pointer;
                    chan->length  = sample->length;
                    voice->period = row->note;
                    chan->setPeriod(row->note);
                }
            }

            switch (voice->effect) {
            case 11:  //position jump
                trackPos = voice->param - 1;
                jumpFlag ^= 1;
                break;
            case 12:  //set volume
                chan->setVolume(voice->param);

                if (version == NOISETRACKER_20)
                    voice->volume = voice->param;
                break;
            case 13:  //pattern break
                jumpFlag ^= 1;
                break;
            case 14:  //set filter
                amiga->filter->active = voice->param ^ 1;
                break;
            case 15:  //set speed
                value = voice->param;

                if (value < 1) value = 1;
                else if (value > 31) value = 31;

                speed = value;
                tick = 0;
                break;
            }

            if (voice->enabled) chan->setEnabled(1);
            chan->pointer = sample->loopPtr;
            chan->length  = sample->repeat;

            voice = voice->next;
        }
    } else {
        while (voice) {
            chan = voice->channel;

            if (!voice->effect && !voice->param) {
                chan->setPeriod(voice->period);
                voice = voice->next;
                continue;
            }

            switch (voice->effect) {
            case 0:   //arpeggio
                value = tick % 3;

                if (!value) {
                    chan->setPeriod(voice->period);
                    voice = voice->next;
                    continue;
                }

                if (value == 1) value = voice->param >> 4;
                else value = voice->param & 0x0f;

                period = voice->period & 0x0fff;
                len = 37 - value;

                for (int i = 0; i < len; ++i) {
                    if (period >= PERIODS[i]) {
                        chan->setPeriod(PERIODS[int(i + value)]);
                        break;
                    }
                }
                break;
            case 1:   //portamento up
                voice->period -= voice->param;
                if (voice->period < 113) voice->period = 113;
                chan->setPeriod(voice->period);
                break;
            case 2:   //portamento down
                voice->period += voice->param;
                if (voice->period > 856) voice->period = 856;
                chan->setPeriod(voice->period);
                break;
            case 3:   //tone portamento
            case 5:   //tone portamento + volume slide
                if (voice->effect == 5) {
                    slide = 1;
                } else if (voice->param) {
                    voice->portaSpeed = voice->param;
                    voice->param = 0;
                }

                if (voice->portaPeriod) {
                    if (voice->portaDir) {
                        voice->period -= voice->portaSpeed;

                        if (voice->period <= voice->portaPeriod) {
                            voice->period = voice->portaPeriod;
                            voice->portaPeriod = 0;
                        }
                    } else {
                        voice->period += voice->portaSpeed;

                        if (voice->period >= voice->portaPeriod) {
                            voice->period = voice->portaPeriod;
                            voice->portaPeriod = 0;
                        }
                    }
                }
                chan->setPeriod(voice->period);
                break;
            case 4:   //vibrato
            case 6:   //vibrato + volume slide
                if (voice->effect == 6) {
                    slide = 1;
                } else if (voice->param) {
                    voice->vibratoSpeed = voice->param;
                }

                value = (voice->vibratoPos >> 2) & 31;
                value = ((voice->vibratoSpeed & 0x0f) * VIBRATO[value]) >> vibratoDepth;

                if (voice->vibratoPos > 127) chan->setPeriod(voice->period - value);
                else chan->setPeriod(voice->period + value);

                value = (voice->vibratoSpeed >> 2) & 60;
                voice->vibratoPos = (voice->vibratoPos + value) & 255;
                break;
            case 10:  //volume slide
                slide = 1;
                break;
            }

            if (slide) {
                value = voice->param >> 4;
                slide = 0;

                if (value) voice->volume += value;
                else voice->volume -= voice->param & 0x0f;

                if (voice->volume < 0) voice->volume = 0;
                else if (voice->volume > 64) voice->volume = 64;

                chan->setVolume(voice->volume);
            }
            voice = voice->next;
        }
    }

    if (++tick == speed) {
        tick = 0;
        patternPos += 4;

        if (patternPos == 256 || jumpFlag) {
            patternPos = jumpFlag = 0;
            trackPos = (++trackPos & 127);

            if (trackPos == length) {
                trackPos = restart;
                amiga->setComplete(1);
            }
        }
    }
}
void MKPlayer::printData()
{
    for(unsigned int i = 0; i < patterns.size(); i++)
    {
        AmigaRow* row= patterns[i];
        std::cout << "Pattern [" << i << "] note: " << row->note << " sample: " << row->sample << " param: " << (int)row->param << " effect: " << row->effect << "\n";
    }
    for(unsigned int i = 0; i < samples.size(); i++)
    {
        AmigaSample* sample = samples[i];
        if(sample)
        {
            std::cout << "Sample [" << i << "] length: " << sample->length << " loop: " << sample->loop << " loopPtr: " << sample->loopPtr << " name: " << sample->name << " pointer: " << sample->pointer << " repeat: " << sample->repeat<< " volume: " << (int)sample->volume << "\n";
        }
    }

    for(unsigned int i = 0; i < track.size(); i++)
    {
        std::cout << "Track [" << i << "]"<< track[i] << "\n";
    }
}
std::vector<AmigaSample*> MKPlayer::getSamples()
{
    std::vector<AmigaSample*>samp (samples.size()-1);
    for(int i =1; i< samples.size() ; i++)
    {
        samp[i-1] = samples[i];
        if(!samp[i-1])
        {
            samp[i-1] = new AmigaSample();
        }
    }
    return samp;
}
bool MKPlayer::getTitle(std::string& title)
{
    title = this->title;
    return true;
}
unsigned int MKPlayer::getCurrentRow()
{
    return trackPosBuffer.front();
}
unsigned int MKPlayer::getCurrentPattern()
{
    return patternPosBuffer.front();
}
void MKPlayer::getModRows(std::vector<BaseRow*>& vect)
{
    vect = patterns;
}
