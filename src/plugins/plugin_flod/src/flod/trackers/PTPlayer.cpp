#include "PTPlayer.h"
#include "PTVoice.h"
#include "BaseRow.h"
#include "BaseSample.h"

#include "AmigaChannel.h"
#include <iostream>
#include "MyEndian.h"
#include <sstream>
#include <string.h>
#include <stdlib.h>     /* strtol */


using namespace std;

const int PTPlayer::FUNKREP[16] =
{
    0,5,6,7,8,10,11,13,16,19,22,26,32,43,64,128
};

const int PTPlayer::PERIODS[592] =
{
    856,808,762,720,678,640,604,570,538,508,480,453,
    428,404,381,360,339,320,302,285,269,254,240,226,
    214,202,190,180,170,160,151,143,135,127,120,113,0,
    850,802,757,715,674,637,601,567,535,505,477,450,
    425,401,379,357,337,318,300,284,268,253,239,225,
    213,201,189,179,169,159,150,142,134,126,119,113,0,
    844,796,752,709,670,632,597,563,532,502,474,447,
    422,398,376,355,335,316,298,282,266,251,237,224,
    211,199,188,177,167,158,149,141,133,125,118,112,0,
    838,791,746,704,665,628,592,559,528,498,470,444,
    419,395,373,352,332,314,296,280,264,249,235,222,
    209,198,187,176,166,157,148,140,132,125,118,111,0,
    832,785,741,699,660,623,588,555,524,495,467,441,
    416,392,370,350,330,312,294,278,262,247,233,220,
    208,196,185,175,165,156,147,139,131,124,117,110,0,
    826,779,736,694,655,619,584,551,520,491,463,437,
    413,390,368,347,328,309,292,276,260,245,232,219,
    206,195,184,174,164,155,146,138,130,123,116,109,0,
    820,774,730,689,651,614,580,547,516,487,460,434,
    410,387,365,345,325,307,290,274,258,244,230,217,
    205,193,183,172,163,154,145,137,129,122,115,109,0,
    814,768,725,684,646,610,575,543,513,484,457,431,
    407,384,363,342,323,305,288,272,256,242,228,216,
    204,192,181,171,161,152,144,136,128,121,114,108,0,
    907,856,808,762,720,678,640,604,570,538,508,480,
    453,428,404,381,360,339,320,302,285,269,254,240,
    226,214,202,190,180,170,160,151,143,135,127,120,0,
    900,850,802,757,715,675,636,601,567,535,505,477,
    450,425,401,379,357,337,318,300,284,268,253,238,
    225,212,200,189,179,169,159,150,142,134,126,119,0,
    894,844,796,752,709,670,632,597,563,532,502,474,
    447,422,398,376,355,335,316,298,282,266,251,237,
    223,211,199,188,177,167,158,149,141,133,125,118,0,
    887,838,791,746,704,665,628,592,559,528,498,470,
    444,419,395,373,352,332,314,296,280,264,249,235,
    222,209,198,187,176,166,157,148,140,132,125,118,0,
    881,832,785,741,699,660,623,588,555,524,494,467,
    441,416,392,370,350,330,312,294,278,262,247,233,
    220,208,196,185,175,165,156,147,139,131,123,117,0,
    875,826,779,736,694,655,619,584,551,520,491,463,
    437,413,390,368,347,328,309,292,276,260,245,232,
    219,206,195,184,174,164,155,146,138,130,123,116,0,
    868,820,774,730,689,651,614,580,547,516,487,460,
    434,410,387,365,345,325,307,290,274,258,244,230,
    217,205,193,183,172,163,154,145,137,129,122,115,0,
    862,814,768,725,684,646,610,575,543,513,484,457,
    431,407,384,363,342,323,305,288,272,256,242,228,
    216,203,192,181,171,161,152,144,136,128,121,114,0
};

const int PTPlayer::VIBRATO[32] =
{
    0, 24, 49, 74, 97,120,141,161,180,197,212,224,
    235,244,250,253,255,253,250,244,235,224,212,197,
    180,161,141,120, 97, 74, 49, 24
};
const char* PTPlayer::NOTES[38] =
{
    "C-1", "C#1", "D-1", "D#1", "E-1", "F-1", "F#1", "G-1", "G#1", "A-1", "A#1", "B-1",
    "C-2", "C#2", "D-2", "D#2", "E-2", "F-2", "F#2", "G-2", "G#2", "A-2", "A#2", "B-2",
    "C-3", "C#3", "D-3", "D#3", "E-3", "F-3", "F#3", "G-3", "G#3", "A-3", "A#3", "B-3",
    "---", "---"
};
PTPlayer::PTPlayer(Amiga* amiga):AmigaPlayer(amiga)
{
    trackPosBuffer = std::list<int>();
    patternPosBuffer = std::list<int>();
    track = std::vector<int>(128);
    samples    = std::vector<BaseSample*>(32);
}
PTPlayer::~PTPlayer()
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

void PTPlayer::setNTSC(bool value)
{
    ntsc = value;
    if (m_version == ULTIMATE_SOUNDTRACKER)
    {
        amiga->samplesTick = int((240 - tempo) * (value ? 7.5152005551 : 7.58437970472));
    }
}

void PTPlayer::initialize()
{

    AmigaPlayer::initialize();

    tempo        = 125;
    speed        = 6;
    trackPos     = 0;
    patternPos   = 0;
    patternBreak = 0;
    patternDelay = 0;
    breakPos     = 0;
    jumpFlag     = 0;

    restartCopy = 0; //this is not really used?
    ntsc = m_ntsc;
    setVersion(m_version);

    PTVoice* voice = voices[0];

    do
    {
        voice->initialize();
        voice->channel   = amiga->channels[voice->index];
        voice->sample  = samples[0];
    }
    while (voice = voice->next);

}
void PTPlayer::setVersion(int value)
{
    if ((m_version < SOUNDTRACKER_24 && value > DOC_SOUNDTRACKER_20) ||
            (m_version > DOC_SOUNDTRACKER_20 && value < SOUNDTRACKER_24)) return;

    if (value < ULTIMATE_SOUNDTRACKER) {
        value = ULTIMATE_SOUNDTRACKER;
    } else if (value > FASTTRACKER_10) {
        value = FASTTRACKER_10;
    }

    m_version = value;

    if (value >= PROTRACKER_10) {
        vibratoDepth = 6;
    } else {
        vibratoDepth = 7;
    }

    //maybe FastTracker uses restart too?
    if (value == NOISETRACKER_11 || value == NOISETRACKER_20) {
        restart = restartCopy;
    } else {
        restart = 0;
    }
}

int PTPlayer::load(void* data, unsigned long int _length)
{
    unsigned char *stream = static_cast<unsigned char*>(data);

    if (_length < 2106) return -1;
    if(
            !(stream[1080]=='M' && stream[1081]=='.' && stream[1082]=='K' && stream[1083]=='.') &&
            !(stream[1080]=='M' && stream[1081]=='!' && stream[1082]=='K' && stream[1083]=='!'))
    {

        char id[5] = {stream[1080],stream[1081],stream[1082],stream[1083]};
        string str_id = id;
        unsigned found = str_id.find("CH");
        if (found!=std::string::npos)
        {
            //std::string id = std::string(stream[1082]) + std::string(stream[1083]);
            int value = strtol (id,NULL,10);
            if (value < 2 || value > 32) return -1;
            m_channels = value;
            m_version = FASTTRACKER_10;
        }
        else {
            return -1;
        }

    }
    else {
        m_version = PROTRACKER_10;
    }

    patternLen = m_channels << 6;

    unsigned int position = 0;
    unsigned int value = 0;
    unsigned int size = 0;
    unsigned int higher = 0;
    BaseSample* sample;
    BaseRow* row;
    position = 0;
    const int STRING_LENGTH = 20;
    for(int j = 0;j<STRING_LENGTH;j++)
    {
        if(!stream[position+j])
        {
            break;
        }
        m_title+=stream[position+j];
    }
    position+=STRING_LENGTH;


    position = 42;



    for (int i = 1; i < 32; ++i) {
        value = readEndian(stream[position],stream[position+1]);position+=2;

        if (!value) {
            samples[i] = 0;
            position += 28;
            continue;
        }

        sample = new BaseSample();
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
        position += 2;

        sample->finetune = stream[position] * 37;position++;
        sample->volume   = stream[position];position++;
        sample->loopPtr  = readEndian(stream[position],stream[position+1]) << 1; position+=2;
        sample->repeat   = readEndian(stream[position],stream[position+1]) << 1; position+=2;

        position += 22;
        sample->pointer = size;
        size += sample->length;
        samples[i] = sample;
    }


    position = 950;
    length =  stream[position];position++;
    position++;

    for (int i = 0; i < 128; ++i) {
        value =  stream[position] * patternLen;position++;
        track[i] = value;
        if (value > higher) higher = value;
    }

    position = 1084;
    higher += patternLen;
    patterns    = std::vector<BaseRow*>(higher);

    for (int i = 0; i < higher; ++i) {
        row = new BaseRow();
        row->step = value = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);position+=4;

        row->note   = (value >> 16) & 0x0fff;
        row->effect = (value >>  8) & 0x0f;
        row->sample = (value >> 24) & 0xf0 | (value >> 12) & 0x0f;
        row->param  = value & 0xff;
        int j = 0;
        for (j = 0; j < 37; ++j)
            if (row->note >= PERIODS[j]) break;

        row->noteText = NOTES[j];



        if (row->sample > 31 || !samples[row->sample]) row->sample = 0;

        if (m_version != FASTTRACKER_10) {
            if (row->effect == 15 && row->param > 31) m_version = PROTRACKER_11;

            if (row->effect == 8) m_version = PROTRACKER_12;
        }
        patterns[i] = row;
    }

    amiga->store(stream, size,position,_length);

    for (int i = 1; i < 32; ++i) {
        sample = samples[i];
        if (!sample) continue;

        if (sample->loopPtr || sample->repeat > 4) {
            sample->length = sample->loopPtr + sample->repeat;
            sample->loopPtr += sample->pointer;
        } else {
            sample->repeat  = 4;
            sample->loopPtr = amiga->memory.size();

        }

        size = sample->pointer + 2;
        for (int j = sample->pointer; j < size; ++j) amiga->memory[j] = 0;
    }

    sample = new BaseSample();
    sample->pointer = sample->loopPtr = amiga->memory.size();
    sample->length  = sample->repeat  = 4;
    samples[0] = sample;

    voices    = std::vector<PTVoice*>(m_channels);
    voices[0] = new PTVoice(0);

    for (int i = 1; i < m_channels; ++i)
    {
        voices[i] = voices[int(i - 1)]->next = new PTVoice(i);
    }


    switch(m_version)
    {
    case PROTRACKER_10:
        format = "ProTracker 1.0";
        break;
    case PROTRACKER_11:
        format = "ProTracker 1.1/2.1";
        break;
    case PROTRACKER_12:
        format = "ProTracker 1.2/2.0";
        break;
    case FASTTRACKER_10:
        format = "FastTracker 1.0";
        break;
    default:
        format = "Error getting fileformat";
        break;
    }

    //printData();
    return 1;

}
void PTPlayer::process()
{

    int value = 0;
    int pattern = 0;

    AmigaChannel* chan;
    BaseRow* row;
    BaseSample* sample;

    PTVoice* voice = voices[0];
    if(trackPosBuffer.size()==22)
    {
        trackPosBuffer.pop_front();
        patternPosBuffer.pop_front();
    }

    trackPosBuffer.push_back(patternPos/m_channels);
    patternPosBuffer.push_back(track[trackPos]/patternLen);

    if (!tick) {
        if (patternDelay) {
            fx();
        } else {
            pattern = track[trackPos] + patternPos;

            do {
                chan = voice->channel;
                voice->enabled = 0;

                if (!voice->step)
                {
                    chan->setPeriod(voice->period);
                }

                row = patterns[int(voice->index + pattern)];
                voice->step   = row->step;
                voice->effect = row->effect;
                voice->param  = row->param;

                if (row->sample) {
                    sample = voice->sample = samples[row->sample];

                    voice->pointer  = sample->pointer;
                    voice->length   = sample->length;
                    voice->loopPtr  = sample->loopPtr;
                    voice->funkWave = sample->loopPtr;
                    voice->repeat   = sample->repeat;
                    voice->finetune = sample->finetune;
                    voice->volume   = sample->volume;

                    chan->setVolume(sample->volume);

                } else {
                    sample = voice->sample;
                }

                if (!row->note) {
                    moreFx(voice);
                    continue;
                } else {
                    if ((voice->step & 0x0ff0) == 0x0e50) {
                        voice->finetune = (voice->param & 0x0f) * 37;
                    } else if (voice->effect == 3 || voice->effect == 5) {
                        if (row->note == voice->period) {
                            voice->portaPeriod = 0;
                        } else {
                            int i = voice->finetune;
                            value = i + 37;

                            for (; i < value; ++i)
                            {
                                if (row->note >= PERIODS[i]) break;
                            }

                            if (i == value) value--;

                            if (i > 0) {
                                value = (voice->finetune / 37) & 8;
                                if (value) i--;
                            }

                            voice->portaPeriod = PERIODS[i];
                            voice->portaDir = (row->note > voice->period) ? 0 : 1;
                        }
                        moreFx(voice);
                        continue;
                    } else if (voice->effect == 9) {
                        moreFx(voice);
                    }
                }

                int i = 0;
                for (i = 0; i < 37; ++i)
                {
                    if (row->note >= PERIODS[i]) break;
                }

                voice->period = PERIODS[int(voice->finetune + i)];

                if ((voice->step & 0x0ff0) == 0x0ed0) {
                    if (voice->funkSpeed) updateFunk(voice);
                    extendedFx(voice);
                    continue;
                }

                if (voice->vibratoWave < 4) voice->vibratoPos = 0;
                if (voice->tremoloWave < 4) voice->tremoloPos = 0;

                chan->setEnabled(0);
                chan->pointer = voice->pointer;
                chan->length  = voice->length;
                chan->setPeriod(voice->period);

                voice->enabled = 1;
                moreFx(voice);

            }
            while (voice = voice->next);

            voice = voices[0];

            do {
                chan = voice->channel;
                if (voice->enabled) chan->setEnabled(1);

                chan->pointer = voice->loopPtr;
                chan->length  = voice->repeat;
            }
            while (voice = voice->next);
        }
    } else {
        fx();
    }

    if (++tick == speed) {
        tick = 0;
        patternPos += m_channels;

        if (patternDelay)
        {
            if (--patternDelay) patternPos -= m_channels;
        }

        if (patternBreak) {
            patternBreak = 0;
            patternPos = breakPos;
            breakPos = 0;
        }

        if (patternPos == patternLen || jumpFlag) {
            patternPos = breakPos;
            breakPos = 0;
            jumpFlag = 0;

            if (++trackPos == length) {
                trackPos = 0;
                amiga->setComplete(1);
            }
        }
    }

}
void PTPlayer::fx()
{

    PTVoice* voice = voices[0];
    AmigaChannel* chan;
    int value = 0;
    int pos = 0;
    int slide = 0;
    int wave = 0;
    int i = 0;
    do {
        chan = voice->channel;
        if (voice->funkSpeed) updateFunk(voice);

        if (!(voice->step & 0x0fff)) {
            chan->setPeriod(voice->period);
            continue;
        }

        switch (voice->effect) {
        case 0:   //arpeggio
            value = tick % 3;

            if (!value) {
                chan->setPeriod(voice->period);
                continue;
            }

            if (value == 1) value = voice->param >> 4;
            else value = voice->param & 0x0f;

            i = voice->finetune;
            pos = i + 37;

            for (; i < pos; ++i)
            {
                if (voice->period >= PERIODS[i]) {
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
            } else if (voice->param){
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

                if (voice->glissando) {
                    i = voice->finetune;
                    pos = i + 37;

                    for (; i < value; ++i)
                    {
                        if (voice->period >= PERIODS[i]) break;
                    }

                    if (i == value) i--;
                    chan->setPeriod(PERIODS[i]);
                } else {
                    chan->setPeriod(voice->period);
                }
            }
            break;
        case 4:   //vibrato
        case 6:   //vibrato + volume slide
            if (voice->effect == 6) {
                slide = 1;
            } else if (voice->param) {
                value = voice->param & 0x0f;
                if (value) voice->vibratoParam = (voice->vibratoParam & 0xf0) | value;
                value = voice->param & 0xf0;
                if (value) voice->vibratoParam = (voice->vibratoParam & 0x0f) | value;
            }

            pos = (voice->vibratoPos >> 2) & 31;
            wave = voice->vibratoWave & 3;

            if (wave) {
                value = 255;
                pos <<= 3;

                if (wave == 1) {
                    if (voice->vibratoPos > 127) value -= pos;
                    else value = pos;
                }
            } else {
                value = VIBRATO[pos];
            }
            value = ((voice->vibratoParam & 0x0f) * value) >> vibratoDepth;

            if (voice->vibratoPos > 127)
            {
                chan->setPeriod(voice->period - value);
            }
            else
            {
                chan->setPeriod(voice->period + value);
            }

            value = (voice->vibratoParam >> 2) & 60;
            voice->vibratoPos = (voice->vibratoPos + value) & 255;

            break;
        case 7:   //tremolo
            chan->setPeriod(voice->period);

            if (voice->param) {
                value = voice->param & 0x0f;
                if (value) voice->tremoloParam = (voice->tremoloParam & 0xf0) | value;
                value = voice->param & 0xf0;
                if (value) voice->tremoloParam = (voice->tremoloParam & 0x0f) | value;
            }

            pos = (voice->tremoloPos >> 2) & 31;
            wave = voice->tremoloWave & 3;

            if (wave) {
                value = 255;
                pos <<= 3;

                if (wave == 1) {
                    if (voice->tremoloPos > 127) value -= pos;
                    else value = pos;
                }
            } else {
                value = VIBRATO[pos];
            }

            value = ((voice->tremoloParam & 0x0f) * value) >> 6;

            if (voice->tremoloPos > 127) chan->setVolume(voice->volume - value);
            else chan->setVolume(voice->volume + value);

            value = (voice->tremoloParam >> 2) & 60;
            voice->tremoloPos = (voice->tremoloPos + value) & 255;
            break;
        case 8:   //set panning
            std::cout << "Fx Set Panning\n";
            break;
        case 10:  //volume slide
            chan->setPeriod(voice->period);
            slide = 1;
            break;
        case 14:  //extended effects
            extendedFx(voice);
            break;
        }

        if (slide) {
            slide = 0;
            value = voice->param >> 4;

            if (value) voice->volume += value;
            else voice->volume -= voice->param & 0x0f;

            if (voice->volume < 0) voice->volume = 0;
            else if (voice->volume > 64) voice->volume = 64;

            chan->setVolume(voice->volume);
        }

    }
    while (voice = voice->next);
}
void PTPlayer::moreFx(PTVoice* voice)
{
    int value = 0;
    if (voice->funkSpeed) updateFunk(voice);

    switch (voice->effect) {
    case 9:   //sample offset
        if (voice->param) voice->offset = voice->param;
        value = voice->offset << 8;

        if (value >= voice->length) {
            voice->length = 2;
        } else {
            voice->pointer += value;
            voice->length  -= value;
        }
        break;
    case 11:  //position jump
        trackPos = voice->param - 1;
        breakPos = 0;
        jumpFlag = 1;
        break;
    case 12:  //set volume
        voice->volume = voice->param;
        if (voice->volume > 64) voice->volume = 64;
        voice->channel->setVolume(voice->volume);
        break;
    case 13:  //pattern break
        breakPos = ((voice->param >> 4) * 10) + (voice->param & 0x0f);

        if (breakPos > 63) breakPos = 0;
        else breakPos <<= 2;

        jumpFlag = 1;
        break;
    case 14:  //extended effects
        extendedFx(voice);
        break;
    case 15:  //set speed
        if (!voice->param) return;

        if (voice->param < 32) speed = voice->param;
        else amiga->samplesTick = 110250 / voice->param;

        tick = 0;
        break;
    }
}

void PTPlayer::updateFunk(PTVoice* voice)
{
    int p1 = 0;
    int p2 = 0;
    int value = FUNKREP[voice->funkSpeed];
    voice->funkPos += value;
    if (voice->funkPos < 128) return;
    voice->funkPos = 0;

    if (m_version == PROTRACKER_10) {
        p1 = voice->pointer + (voice->sample->length - voice->repeat);
        p2 = voice->funkWave + voice->repeat;

        if (p2 > p1) {
            p2 = voice->loopPtr;
            voice->channel->length = voice->repeat;
        }
        voice->funkWave = p2;
        voice->channel->pointer = p2;
    } else {
        p1 = voice->loopPtr + voice->repeat;
        p2 = voice->funkWave + 1;

        if (p2 >= p1) p2 = voice->loopPtr;

        amiga->memory[p2] = -amiga->memory[p2];
    }
}

void PTPlayer::extendedFx(PTVoice *voice)
{

    AmigaChannel* chan = voice->channel;
    int effect = voice->param >> 4;
    int param = voice->param & 0x0f;
    int len = 0;
    int i = 0;

    switch (effect) {
    case 0:   //set filter
        amiga->filter->active = param;
        break;
    case 1:   //fine portamento up
        if (tick) return;
        voice->period -= param;
        if (voice->period < 113) voice->period = 113;
        chan->setPeriod(voice->period);
        break;
    case 2:   //fine portamento down
        if (tick) return;
        voice->period += param;
        if (voice->period > 856) voice->period = 856;
        chan->setPeriod(voice->period);
        break;
    case 3:   //glissando control
        voice->glissando = param;
        break;
    case 4:   //vibrato control
        voice->vibratoWave = param;
        break;
    case 5:   //set finetune
        voice->finetune = param * 37;
        break;
    case 6:   //pattern loop
        if (tick) return;

        if (param) {
            if (voice->loopCtr) voice->loopCtr--;
            else voice->loopCtr = param;

            if (voice->loopCtr) {
                breakPos = voice->loopPos << 2;
                patternBreak = 1;
            }
        } else {
            voice->loopPos = patternPos >> 2;
        }
        break;
    case 7:   //tremolo control
        voice->tremoloWave = param;
        break;
    case 8:   //karplus strong
        len = voice->length - 2;

        for (i = voice->loopPtr; i < len;)
        {
            amiga->memory[i] = (amiga->memory[i] + amiga->memory[++i]) * 0.5;
        }

        amiga->memory[++i] = (amiga->memory[i] + amiga->memory[0]) * 0.5;
        break;
    case 9:   //retrig note
        if (tick || !param || !voice->period) return;
        if (tick % param) return;

        chan->setEnabled(0);
        chan->pointer = voice->pointer;
        chan->length  = voice->length;
        chan->delay   = 30;

        chan->setEnabled(1);
        chan->pointer = voice->loopPtr;
        chan->length  = voice->repeat;
        chan->setPeriod(voice->period);
        break;
    case 10:  //fine volume up
        if (tick) return;
        voice->volume += param;
        if (voice->volume > 64) voice->volume = 64;
        chan->setVolume(voice->volume);
        break;
    case 11:  //fine volume down
        if (tick) return;
        voice->volume -= param;
        if (voice->volume < 0) voice->volume = 0;
        chan->setVolume(voice->volume);
        break;
    case 12:  //note cut
        if (tick == param)
        {
            voice->volume = 0;
            chan->setVolume(0);
        }
        break;
    case 13:  //note delay
        if (tick != param || !voice->period) return;

        chan->setEnabled(0);
        chan->pointer = voice->pointer;
        chan->length  = voice->length;
        chan->delay   = 30;

        chan->setEnabled(1);
        chan->pointer = voice->loopPtr;
        chan->length  = voice->repeat;
        chan->setPeriod(voice->period);
        break;
    case 14:  //pattern delay
        if (tick || patternDelay) return;
        patternDelay = +param+1;
        break;
    case 15:  //funk repeat | invert loop
        if (tick) return;
        voice->funkSpeed = param;
        if (param) updateFunk(voice);
        break;
    }
}

void PTPlayer::printData()
{
    //    for(unsigned int i = 0; i < patterns.size(); i++)
    //    {
    //        PTRow* row= patterns[i];
    //        std::cout << "Pattern [" << i << "] note: " << row->note << " sample: " << row->sample << " param: " << (int)row->param << " effect: " << row->effect << " step: " << row->step << "\n";
    //    }
    //    for(unsigned int i = 0; i < samples.size(); i++)
    //    {
    //        PTSample* sample = samples[i];
    //        if(sample)
    //        {
    //            std::cout << "Sample [" << i << "] length: " << sample->length << " loop: " << sample->loop << " loopPtr: " << sample->loopPtr << " name: " << sample->name << " pointer: " << sample->pointer << " repeat: " << sample->repeat<< " volume: " << (int)sample->volume << " finetune: " << sample->finetune << " realLen: " << sample->realLen << "\n";
    //        }
    //    }

    //    for(unsigned int i = 0; i < track.size(); i++)
    //    {
    //        std::cout << "Track [" << i << "]"<< track[i] << "\n";
    //    }
}
std::vector<BaseSample*> PTPlayer::getSamples()
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
bool PTPlayer::getTitle(std::string& title)
{
    title = m_title;
    return true;
}
unsigned int PTPlayer::getCurrentRow()
{
    return trackPosBuffer.front();
}
unsigned int PTPlayer::getCurrentPattern()
{
    return patternPosBuffer.front();
}
void PTPlayer::getModRows(std::vector<BaseRow*>& vect)
{
    std::vector<BaseRow*> patts(patterns.size());
    for(int i =0; i< patterns.size() ; i++)
    {
        patts[i] = dynamic_cast<BaseRow*>(patterns[i]);
    }
    vect=patts;
}
