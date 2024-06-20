#include "STPlayer.h"
#include "STVoice.h"
#include "AmigaRow.h"
#include "AmigaSample.h"

#include "AmigaChannel.h"
#include <iostream>
#include "MyEndian.h"
#include <math.h>

const int STPlayer::PERIODS[39] =
{
    856,808,762,720,678,640,604,570,538,508,480,453,
    428,404,381,360,339,320,302,285,269,254,240,226,
    214,202,190,180,170,160,151,143,135,127,120,113,
    0,0,0
};
const char* STPlayer::NOTES[38] =
{
    "C-1", "C#1", "D-1", "D#1", "E-1", "F-1", "F#1", "G-1", "G#1", "A-1", "A#1", "B-1",
    "C-2", "C#2", "D-2", "D#2", "E-2", "F-2", "F#2", "G-2", "G#2", "A-2", "A#2", "B-2",
    "C-3", "C#3", "D-3", "D#3", "E-3", "F-3", "F#3", "G-3", "G#3", "A-3", "A#3", "B-3",
    "---", "---"
};
STPlayer::STPlayer(Amiga* amiga):AmigaPlayer(amiga)
{
    trackPosBuffer = std::list<int>();
    patternPosBuffer = std::list<int>();
    track = std::vector<int>(128);
    samples    = std::vector<AmigaSample*>(16);
    voices    = std::vector<STVoice*>(4);

    voices[0] = new STVoice(0);
    voices[0]->next = voices[1] = new STVoice(1);
    voices[1]->next = voices[2] = new STVoice(2);
    voices[2]->next = voices[3] = new STVoice(3);
}
STPlayer::~STPlayer()
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

void STPlayer::initialize()
{

    AmigaPlayer::initialize();
    speed       = 6;
    trackPos    = 0;
    patternPos  = 0;
    jumpFlag    = 0;

    STVoice* voice = voices[0];

    while (voice)
    {
        voice->initialize();
        voice->channel   = amiga->channels[voice->index];
        voice->sample  = samples[0];
        voice = voice->next;
    }


}
void STPlayer::setNTSC(int value)
{
    AmigaPlayer::setNTSC(value);
    if (version < DOC_SOUNDTRACKER_9) {
        amiga->samplesTick = int((240 - tempo) * (value ? 7.5152005551 : 7.58437970472));
    } else {
        amiga->samplesTick = 876;
    }
}
void STPlayer::setForce(int value)
{
    if (value >= ULTIMATE_SOUNDTRACKER && value <=DOC_SOUNDTRACKER_20)
    {
        version = value;
    }
    std::cout << "setforce: version = " << version << "\n";
}
int STPlayer::load(void* data, unsigned long int _length)
{
    int score = 0;
    int value=0;
    unsigned char *stream = static_cast<unsigned char*>(data);
    std::cout << "loading st...\n";
    std::flush(std::cout);
    if(_length<1626) return -1;
    unsigned int position = 0;
    unsigned int higher = 0;
    unsigned int size = 0;
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
    score += isLegal(title);
    version = ULTIMATE_SOUNDTRACKER;
    position = 42;



    AmigaSample* sample;
    for (int i = 1; i < 16; ++i) {
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
        sample->loop   = readEndian(stream[position],stream[position+1]);position+=2;
        sample->repeat = readEndian(stream[position],stream[position+1]) << 1;position+=2;
        if(sample->loop > sample->length || sample->repeat > sample->length)
        {
            version = 0;
            return 0;
        }

        position += 22;
        sample->pointer = size;
        size += sample->length;
        samples[i] = sample;

        score += isLegal(sample->name);
        if (sample->length > 9999) version = MASTER_SOUNDTRACKER;
    }

    position = 470;
    length = stream[position];position++;
    tempo  = stream[position];position++;

    for (int i = 0; i < 128; ++i) {
        value = stream[position] << 8; position++;
        if (value > 16384) score--;
        track[i] = value;
        if (value > higher) higher = value;
    }

    position = 600;
    higher += 256;
    patterns    = std::vector<AmigaRow*>(higher);

    int v = (_length - size - 600) >> 2;
    //if (higher > v) higher = v;
    //TODO what does this do?
    std::cout << "pattern  size " << patterns.size() << "\n";
    std::cout << "higher " << higher << "\n";
    for (int i = 0; i < higher; ++i) {
        AmigaRow* row = new AmigaRow();

        row->note   = readEndian(stream[position],stream[position+1]);position+=2;
        value      = stream[position];position++;
        row->param  = stream[position];position++;
        row->effect = value & 0x0f;
        row->sample = value >> 4;
        int j = 0;
        for (j = 0; j < 37; ++j)
          if (row->note >= PERIODS[j]) break;

        row->noteText = NOTES[j];
        patterns[i] = row;

        if (row->effect > 2 && row->effect < 11) score--;
        if (row->note) {
            if (row->note < 113 || row->note > 856) score--;
        }

        if (row->sample)
            if (row->sample > 15 || !samples[row->sample]) {
                if (row->sample > 15) score--;
                row->sample = 0;
            }

        if (row->effect > 2 || (!row->effect && row->param != 0))
            version = DOC_SOUNDTRACKER_9;

        if (row->effect == 11 || row->effect == 13)
            version = DOC_SOUNDTRACKER_20;
    }

    amiga->store(stream, size, position,_length);

    for (int i = 1; i < 16; ++i) {
        sample = samples[i];
        if (!sample) continue;

        if (sample->loop) {
            sample->loopPtr = sample->pointer + sample->loop;
            sample->pointer = sample->loopPtr;
            sample->length  = sample->repeat;
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

    std::cout << "final score:" << score  << "\n";
    if (score < 1) version = 0;

    switch(version)
    {
    case ULTIMATE_SOUNDTRACKER:
        format = "Ultimate Soundtracker";
        break;
    case DOC_SOUNDTRACKER_9:
        format = "D.O.C. Soundtracker 9";
        break;
    case MASTER_SOUNDTRACKER:
        format = "Master Soundtracker";
        break;
    case DOC_SOUNDTRACKER_20:
        format = "DOC Soundtracker 2.0/2.2";
        break;
    }

    //printData();
    return version;

}
void STPlayer::process()
{
    int value = 0;
    STVoice* voice = voices[0];
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
        value = track[trackPos] + patternPos;
        std::cout << "value "  << value << "\n";
        std::flush(std::cout);
        while (voice) {
            chan = voice->channel;
            voice->enabled = 0;

            row = patterns[int(value + voice->index)];
            voice->period = row->note;
            voice->effect = row->effect;
            voice->param  = row->param;
            std::cout << "time to play2"  << "\n";
            std::flush(std::cout);
            if (row->sample) {
                sample = voice->sample = samples[row->sample];

                if (((version & 2) == 2) && voice->effect == 12) chan->setVolume(voice->param);
                else chan->setVolume(sample->volume);
            } else {
                sample = voice->sample;
            }
            std::cout << "time to play3"  << "\n";
            std::flush(std::cout);
            if (voice->period) {
                voice->enabled = 1;

                chan->setEnabled(0);
                chan->pointer = sample->pointer;
                chan->length  = sample->length;
                chan->setPeriod(voice->period);
                voice->last = voice->period;
            }

            if (voice->enabled) chan->setEnabled(1);
            chan->pointer = sample->loopPtr;
            chan->length  = sample->repeat;

            if (version < DOC_SOUNDTRACKER_20) {
                voice = voice->next;
                continue;
            }

            switch (voice->effect) {
            case 11:  //position jump
                trackPos = voice->param - 1;
                jumpFlag ^= 1;
                break;
            case 12:  //set volume
                chan->setVolume(voice->param);
                break;
            case 13:  //pattern break
                jumpFlag ^= 1;
                break;
            case 14:  //set filter
                amiga->filter->active = voice->param ^ 1;
                break;
            case 15:  //set speed
                if (!voice->param) break;
                speed = voice->param & 0x0f;
                tick = 0;
                break;
            }

            voice = voice->next;
        }
    } else {
        while (voice) {
            if (!voice->param) {
                voice = voice->next;
                continue;
            }
            chan = voice->channel;

            if (version == ULTIMATE_SOUNDTRACKER) {
                if (voice->effect == 1) {
                    arpeggio(voice);
                } else if (voice->effect == 2) {
                    value = voice->param >> 4;

                    if (value) voice->period += value;
                    else voice->period -= (voice->param & 0x0f);

                    chan->setPeriod(voice->period);
                }
            } else {
                switch (voice->effect) {
                case 0: //arpeggio
                    arpeggio(voice);
                    break;
                case 1: //portamento up
                    voice->last -= voice->param & 0x0f;
                    if (voice->last < 113) voice->last = 113;
                    chan->setPeriod(voice->last);
                    break;
                case 2: //portamento down
                    voice->last += voice->param & 0x0f;
                    if (voice->last > 856) voice->last = 856;
                    chan->setPeriod(voice->last);
                    break;
                }

                if ((version & 2) != 2) {
                    voice = voice->next;
                    continue;
                }

                switch (voice->effect) {
                case 12:  //set volume
                    chan->setVolume(voice->param);
                    break;
                case 14:  //set filter
                    amiga->filter->active = 0;
                    break;
                case 15:  //set speed
                    speed = voice->param & 0x0f;
                    break;
                }
            }
            voice = voice->next;
        }
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
void STPlayer::printData()
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
std::vector<AmigaSample*> STPlayer::getSamples()
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
bool STPlayer::getTitle(std::string& title)
{
    title = this->title;
    return true;
}
int STPlayer::isLegal(std::string text)
{
    int ascii=0;
    int i = 0;
    int len = text.size();
    //if (!len) return 0;

    for (; i < len; ++i) {
        ascii = text[i];
        if (ascii && (ascii < 32 || ascii > 127)) return 0;
    }
    return 1;
}

void STPlayer::arpeggio(STVoice* voice)
{
  AmigaChannel* chan = voice->channel;
  int i = 0;
  int param = tick % 3;

  if (!param) {
    chan->setPeriod(voice->last);
    return;
  }

  if (param == 1) param = voice->param >> 4;
    else param = voice->param & 0x0f;

  while (voice->last != PERIODS[i]) i++;
  chan->setPeriod(PERIODS[int(i + param)]);
}
unsigned int STPlayer::getCurrentRow()
{
    return trackPosBuffer.front();
}
unsigned int STPlayer::getCurrentPattern()
{
    return patternPosBuffer.front();
}
void STPlayer::getModRows(std::vector<BaseRow*>& vect)
{
    vect = patterns;
}
