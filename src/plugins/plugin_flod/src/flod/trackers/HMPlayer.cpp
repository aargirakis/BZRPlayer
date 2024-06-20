#include "HMPlayer.h"
#include "HMVoice.h"
#include "AmigaRow.h"
#include "HMSample.h"

#include "AmigaChannel.h"
#include <iostream>
#include "MyEndian.h"

const int HMPlayer::MEGARPEGGIO[256] =
{
    0, 3, 7,12,15,12, 7, 3, 0, 3, 7,12,15,12, 7, 3,
    0, 4, 7,12,16,12, 7, 4, 0, 4, 7,12,16,12, 7, 4,
    0, 3, 8,12,15,12, 8, 3, 0, 3, 8,12,15,12, 8, 3,
    0, 4, 8,12,16,12, 8, 4, 0, 4, 8,12,16,12, 8, 4,
    0, 5, 8,12,17,12, 8, 5, 0, 5, 8,12,17,12, 8, 5,
    0, 5, 9,12,17,12, 9, 5, 0, 5, 9,12,17,12, 9, 5,
    12, 0, 7, 0, 3, 0, 7, 0,12, 0, 7, 0, 3, 0, 7, 0,
    12, 0, 7, 0, 4, 0, 7, 0,12, 0, 7, 0, 4, 0, 7, 0,
    0, 3, 7, 3, 7,12, 7,12,15,12, 7,12, 7, 3, 7, 3,
    0, 4, 7, 4, 7,12, 7,12,16,12, 7,12, 7, 4, 7, 4,
    31,27,24,19,15,12, 7, 3, 0, 3, 7,12,15,19,24,27,
    31,28,24,19,16,12, 7, 4, 0, 4, 7,12,16,19,24,28,
    0,12, 0,12, 0,12, 0,12, 0,12, 0,12, 0,12, 0,12,
    0,12,24,12, 0,12,24,12, 0,12,24,12, 0,12,24,12,
    0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3,
    0, 4, 0, 4, 0, 4, 0, 4, 0, 4, 0, 4, 0, 4, 0, 4
};

const int HMPlayer::PERIODS[37] =
{
    856,808,762,720,678,640,604,570,538,508,480,453,
    428,404,381,360,339,320,302,285,269,254,240,226,
    214,202,190,180,170,160,151,143,135,127,120,113,0
};

const int HMPlayer::VIBRATO[32] =
{
    0, 24, 49, 74, 97,120,141,161,180,197,212,224,
    235,244,250,253,255,253,250,244,235,224,212,197,
    180,161,141,120, 97, 74, 49, 24
};
const char* HMPlayer::NOTES[38] =
{
    "C-1", "C#1", "D-1", "D#1", "E-1", "F-1", "F#1", "G-1", "G#1", "A-1", "A#1", "B-1",
    "C-2", "C#2", "D-2", "D#2", "E-2", "F-2", "F#2", "G-2", "G#2", "A-2", "A#2", "B-2",
    "C-3", "C#3", "D-3", "D#3", "E-3", "F-3", "F#3", "G-3", "G#3", "A-3", "A#3", "B-3",
    "---", "---"
};
HMPlayer::HMPlayer(Amiga* amiga):AmigaPlayer(amiga)
{
    trackPosBuffer = std::list<int>();
    patternPosBuffer = std::list<int>();
    track = std::vector<int>(128);
    samples    = std::vector<HMSample*>(32);
    voices    = std::vector<HMVoice*>(4);

    voices[0] = new HMVoice(0);
    voices[0]->next = voices[1] = new HMVoice(1);
    voices[1]->next = voices[2] = new HMVoice(2);
    voices[2]->next = voices[3] = new HMVoice(3);
}
HMPlayer::~HMPlayer()
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

void HMPlayer::initialize()
{
    AmigaPlayer::initialize();

    speed       = 6;
    trackPos    = 0;
    patternPos  = 0;
    jumpFlag    = 0;

    amiga->samplesTick = 884;

    HMVoice* voice = voices[0];

    while (voice)
    {
        voice->initialize();
        voice->channel   = amiga->channels[voice->index];
        voice->sample  = samples[0];
        voice = voice->next;
    }
}
int HMPlayer::load(void* data, unsigned long int _length)
{
    unsigned char *stream = static_cast<unsigned char*>(data);
    unsigned int position = 0;
    int count = 0;


    int mupp = 0 ;
    if(_length<2106) return -1;
    if (!(stream[1080]=='F' && stream[1081]=='E' && stream[1082]=='S' && stream[1083]=='T') && !(stream[1080]=='M' && stream[1081]=='&' && stream[1082]=='K' && stream[1083]=='!'))
    {
        return -1;
    }

    HMSample* sample;
    unsigned int value = 0;
    unsigned int size = 0;
    unsigned int higher = 0;
    AmigaRow* row;

    position = 950;

    length  = stream[position];position++;
    restart = stream[position];position++;

    for (int i =0 ; i < 128; ++i)
    {
        track[i] = stream[position];position++;
    }

    position = 0;
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
    version = 1;

    for (int i = 1; i < 32; ++i) {
        samples[i] = 0;
        std::string id="";
        const int STRING_LENGTH_SAMPLE_SHORT = 4;
        for(int j = 0;j<STRING_LENGTH_SAMPLE_SHORT;j++)
        {
            if(!stream[position+j])
            {
                break;
            }
            id+=stream[position+j];
        }
        position+=STRING_LENGTH_SAMPLE_SHORT;

        if (id == "Mupp") {

            value = stream[position];position++;
            count = value - higher++;
            for (int j = 0; j < 128; ++j)
            if (track[j] && track[j] >= count) track[j]--;

            sample = new HMSample();
            sample->name = id;
            sample->length  = sample->repeat = 32;
            sample->restart = stream[position];position++;
            sample->waveLen = stream[position];position++;
            position += 17;
            sample->finetune = (signed char)stream[position];position++;
            sample->volume   = stream[position];position++;

            int pos = position + 4;
            value = 1084 + (value << 10);
            position = value;

            sample->pointer = amiga->memory.size();
            sample->waves = std::vector<int>(64);
            sample->volumes = std::vector<int>(64);
            amiga->store(stream, 896,position,_length);

            for (int j = 0; j < 64; ++j)
            {
                sample->waves[j] = stream[position] << 5;
                position++;
            }
            for (int j = 0; j < 64; ++j)
            {
                sample->volumes[j] = stream[position] & 127;
                position++;
            }

            position = value;
            //TODO check if it's correct
            //stream.writeInt(0x666c6f64);

            unsigned char myVal[4];

            myVal[0] = 0x666c6f64 >> 24;
            myVal[1] = (0x666c6f64>>16) & 0xFF;
            myVal[2] = (0x666c6f64>>8) & 0xFF;
            myVal[3] = 0x666c6f64 & 0xFF;
            for(int p=0;p<4;p++)
            {
                stream[position] = myVal[p];
                position++;
            }


            position = pos;
            mupp += 896;
        } else {
            id = id.substr(0, 2);
            if (id == "El")
                position += 18;
            else {

                position -= 4;
                id="";
                const int STRING_LENGTH_SAMPLE = 22;
                for(int j = 0;j<STRING_LENGTH_SAMPLE;j++)
                {
                    if(!stream[position+j])
                    {
                        break;
                    }
                    id+=stream[position+j];
                }
                position+=STRING_LENGTH_SAMPLE;
            }

            value = readEndian(stream[position],stream[position+1]);position+=2;
            if (!value) {
                position += 6;//???
                continue;
            }

            sample = new HMSample();
            sample->name = id;
            sample->pointer  = size;
            sample->length   = value << 1;
            sample->finetune = (signed char)stream[position];position++;
            sample->volume   = stream[position];position++;
            sample->loop     = readEndian(stream[position],stream[position+1]) << 1;position+=2;
            sample->repeat   = readEndian(stream[position],stream[position+1]) << 1;position+=2;
            size += sample->length;
        }
        samples[i] = sample;
    }


    for (int i = 0; i < 128; ++i) {
        value = track[i] << 8;
        track[i] = value;
        if (value > higher) higher = value;
    }

    position = 1084;
    higher += 256;
    patterns = std::vector<AmigaRow*>(higher);


    for (int i = 0; i < higher; ++i) {
        value = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);position+=4;
        while (value == 0x666c6f64) {
            position += 1020;
            value = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);position+=4;
        }

        row = new AmigaRow();
        row->note   = (value >> 16) & 0x0fff;
        row->sample = (value >> 24) & 0xf0 | (value >> 12) & 0x0f;
        row->effect = (value >>  8) & 0x0f;
        row->param  = value & 0xff;
        int j = 0;
        for (j = 0; j < 37; ++j)
          if (row->note >= PERIODS[j]) break;

        row->noteText = NOTES[j];
        if (row->sample > 31 || !samples[row->sample]) row->sample = 0;
        patterns[i] = row;
    }

    amiga->store(stream, size,position,_length);

    for (int i = 1; i < 32; ++i) {
        sample = samples[i];
        if (sample==0 || sample->name == "Mupp") continue;
        sample->pointer += mupp;
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

    sample = new HMSample();
    sample->pointer = sample->loopPtr = amiga->memory.size();
    sample->length  = sample->repeat  = 2;
    samples[0] = sample;
    format ="His Master's NoiseTracker";
    //printData();
    return 1;

}
void HMPlayer::process()
{

    HMVoice* voice = voices[0];
    AmigaChannel* chan;
    int pattern = 0;
    AmigaRow* row;
    HMSample* sample;
    unsigned int value = 0;
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
                voice->volume2 = sample->volume;

                if (sample->name == "Mupp") {
                    sample->loopPtr = sample->pointer + sample->waves[0];
                    voice->handler = 1;
                    voice->volume1 = sample->volumes[0];
                } else {
                    voice->handler = 0;
                    voice->volume1 = 64;
                }
            } else {
                sample = voice->sample;
            }

            if (row->note) {
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
                    voice->period     = row->note;
                    voice->vibratoPos = 0;
                    voice->wavePos    = 0;
                    voice->enabled    = 1;

                    chan->setEnabled(0);
                    value = (voice->period * sample->finetune) >> 8;
                    chan->setPeriod(voice->period + value);

                    if (voice->handler) {
                        chan->pointer = sample->loopPtr;
                        chan->length  = sample->repeat;
                    } else {
                        chan->pointer = sample->pointer;
                        chan->length  = sample->length;
                    }
                }
            }

            switch (voice->effect) {
            case 11:  //position jump
                trackPos = voice->param - 1;
                jumpFlag = 1;
                break;
            case 12:  //set volume
                voice->volume2 = voice->param;
                if (voice->volume2 > 64) voice->volume2 = 64;
                break;
            case 13:  //pattern break
                jumpFlag = 1;
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

            if (!row->note) effects(voice);
            handler(voice);

            if (voice->enabled) chan->setEnabled(1);
            chan->pointer = sample->loopPtr;
            chan->length  = sample->repeat;

            voice = voice->next;
        }
    } else {
        while (voice) {
            effects(voice);
            handler(voice);

            sample = voice->sample;
            voice->channel->pointer = sample->loopPtr;
            voice->channel->length  = sample->repeat;

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

void HMPlayer::effects(HMVoice* voice)
{
    AmigaChannel* chan = voice->channel;
    int period = voice->period & 0x0fff;
    int slide = 0;
    int value = 0;
    int len = 0;
    int j = 0;


    if (voice->effect || voice->param) {
        switch (voice->effect) {
        case 0:   //arpeggio
            value = tick % 3;
            if (!value) break;
            if (value == 1) value = voice->param >> 4;
            else value = voice->param & 0x0f;

            len = 37 - value;

            for (int i = 0; i < len; ++i) {
                if (period >= PERIODS[i]) {
                    period = PERIODS[int(i + value)];
                    break;
                }
            }
            break;
        case 1:   //portamento up
            voice->period -= voice->param;
            if (voice->period < 113) voice->period = 113;
            period = voice->period;
            break;
        case 2:   //portamento down
            voice->period += voice->param;
            if (voice->period > 856) voice->period = 856;
            period = voice->period;
            break;
        case 3:   //tone portamento
        case 5:   //tone portamento + volume slide
            if (voice->effect == 5) slide = 1;
            else if (voice->param) {
                voice->portaSpeed = voice->param;
                voice->param = 0;
            }

            if (voice->portaPeriod) {
                if (voice->portaDir) {
                    voice->period -= voice->portaSpeed;
                    if (voice->period < voice->portaPeriod) {
                        voice->period = voice->portaPeriod;
                        voice->portaPeriod = 0;
                    }
                } else {
                    voice->period += voice->portaSpeed;
                    if (voice->period > voice->portaPeriod) {
                        voice->period = voice->portaPeriod;
                        voice->portaPeriod = 0;
                    }
                }
            }
            period = voice->period;
            break;
        case 4:   //vibrato
        case 6:   //vibrato + volume slide;
            if (voice->effect == 6) slide = 1;
            else if (voice->param) voice->vibratoSpeed = voice->param;

            value = VIBRATO[int((voice->vibratoPos >> 2) & 31)];
            value = ((voice->vibratoSpeed & 0x0f) * value) >> 7;

            if (voice->vibratoPos > 127) period -= value;
            else period += value;

            value = (voice->vibratoSpeed >> 2) & 60;
            voice->vibratoPos = (voice->vibratoPos + value) & 255;
            break;
        case 7:   //mega arpeggio
            value = MEGARPEGGIO[int((voice->vibratoPos & 0x0f) + ((voice->param & 0x0f) << 4))];
            voice->vibratoPos++;

            for (j = 0; j < 37; ++j) if (period >= PERIODS[j]) break;

            value += j;
            if (value > 35) value -= 12;
            period = PERIODS[value];
            break;
        case 10:  //volume slide
            slide = 1;
            break;
        }
    }

    chan->setPeriod(period + ((period * voice->sample->finetune) >> 8));

    if (slide) {
        value = voice->param >> 4;

        if (value) voice->volume2 += value;
        else voice->volume2 -= voice->param & 0x0f;

        if (voice->volume2 > 64) voice->volume2 = 64;
        else if (voice->volume2 < 0) voice->volume2 = 0;
    }
}

void HMPlayer::handler(HMVoice* voice)
{
    HMSample* sample;
    if (voice->handler) {
        sample = voice->sample;
        sample->loopPtr = sample->pointer + sample->waves[voice->wavePos];

        voice->volume1 = sample->volumes[voice->wavePos];

        if (++voice->wavePos > sample->waveLen)
            voice->wavePos = sample->restart;
    }
    voice->channel->setVolume((voice->volume1 * voice->volume2) >> 6);
}
void HMPlayer::printData()
{
//    for(unsigned int i = 0; i < patterns.size(); i++)
//    {
//        AmigaRow* row= patterns[i];
//        std::cout << "Pattern [" << i << "] note: " << row->note << " sample: " << row->sample << " param: " << (int)row->param << " effect: " << row->effect << "\n";
//    }
    for(unsigned int i = 0; i < samples.size(); i++)
    {
        HMSample* sample = samples[i];
        if(sample)
        {
            std::cout << "Sample [" << i << "] length: " << sample->length << " loop: " << sample->loop << " loopPtr: " << sample->loopPtr << " name: " << sample->name << " pointer: " << sample->pointer << " repeat: " << sample->repeat << " volume: " << (int)sample->volume << " finetune: " << sample->finetune << " restart: " << sample->restart << " waveLen: " << sample->waveLen << "\n";
            for(int j = 0; j < sample->waves.size();j++)
            {
                std::cout << "wave " << (int)sample->waves[j] << "\n";
            }
            for(int j = 0; j < sample->volumes.size();j++)
            {
                std::cout << "volume " << (int)sample->volumes[j] << "\n";
            }
        }
    }

    for(unsigned int i = 0; i < track.size(); i++)
    {
        std::cout << "Track [" << i << "]"<< track[i] << "\n";
    }
    std::flush(std::cout);
}
std::vector<AmigaSample*> HMPlayer::getSamples()
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
bool HMPlayer::getTitle(std::string& title)
{
    title = this->title;
    return true;
}
unsigned int HMPlayer::getCurrentRow()
{
    return trackPosBuffer.front();
}
unsigned int HMPlayer::getCurrentPattern()
{
    return patternPosBuffer.front();
}
void HMPlayer::getModRows(std::vector<BaseRow*>& vect)
{
    vect = patterns;
}
