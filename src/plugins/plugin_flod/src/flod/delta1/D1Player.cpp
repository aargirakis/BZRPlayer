#include "D1Player.h"
#include "D1Sample.h"
#include "D1Voice.h"
#include "BaseRow.h"
#include "AmigaChannel.h"
#include "BaseStep.h"
#include <iostream>
#include "MyEndian.h"
using namespace std;

const int D1Player::PERIODS[84] =
{
    0,6848,6464,6096,5760,5424,5120,4832,4560,4304,4064,3840,
    3616,3424,3232,3048,2880,2712,2560,2416,2280,2152,2032,1920,
    1808,1712,1616,1524,1440,1356,1280,1208,1140,1076, 960, 904,
    856, 808, 762, 720, 678, 640, 604, 570, 538, 508, 480, 452,
    428, 404, 381, 360, 339, 320, 302, 285, 269, 254, 240, 226,
    214, 202, 190, 180, 170, 160, 151, 143, 135, 127, 120, 113,
    113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113
};
D1Player::D1Player(Amiga* amiga):AmigaPlayer(amiga)
{

    samples = std::vector<D1Sample*>(21);
    voices    = std::vector<D1Voice*>(4);
    pointers = std::vector<int>(4);

    voices[0] = new D1Voice(0);
    voices[0]->next = voices[1] = new D1Voice(1);
    voices[1]->next = voices[2] = new D1Voice(2);
    voices[2]->next = voices[3] = new D1Voice(3);
}
D1Player::~D1Player()
{
    pointers.clear();


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
void D1Player::process()
{
    D1Voice* voice;
    AmigaChannel* chan;
    BaseRow* row;
    D1Sample* sample;
    int value=0;
    int loop = 0;
    int adsr = 0;

    voice = voices[0];
    do {
        chan = voice->channel;

        if (--voice->speed == 0) {
            voice->speed = speed;

            if (voice->patternPos == 0) {
                voice->step = tracks[int(pointers[voice->index] + voice->trackPos)];

                if (voice->step->pattern < 0) {
                    voice->trackPos = voice->step->transpose;
                    voice->step = tracks[int(pointers[voice->index] + voice->trackPos)];
                }
                voice->trackPos++;
            }

            row = patterns[int(voice->step->pattern + voice->patternPos)];
            if (row->effect) voice->row = row;

            if (row->note) {
                chan->setEnabled(0);
                voice->row = row;
                voice->note = row->note + voice->step->transpose;
                voice->arpeggioPos = 0;
                voice->pitchBend = 0;
                voice->status = 0;

                sample = voice->sample = samples[row->sample];
                if (!sample->synth) chan->pointer = sample->pointer;
                chan->length = sample->length;

                voice->tableCtr   = voice->tablePos = 0;
                voice->vibratoCtr = sample->vibratoWait;
                voice->vibratoPos = sample->vibratoLen;
                voice->vibratoDir = sample->vibratoLen << 1;
                voice->volume = 0;
                voice->attackCtr = 0;
                voice->decayCtr = 0;
                voice->releaseCtr = 0;
                voice->sustain = sample->sustain;
            }
            if (++voice->patternPos == 16) voice->patternPos = 0;
        }
        sample = voice->sample;

        if (sample->synth) {
            if (voice->tableCtr) {
                voice->tableCtr--;
            }
            else
            {
                voice->tableCtr = sample->tableDelay;
                do {
                    loop = 1;

                    if (voice->tablePos >= 48) voice->tablePos = 0;
                    value = sample->table[voice->tablePos];
                    voice->tablePos++;
                    if (value >= 0) {
                        chan->pointer = sample->pointer + (value << 5);
                        loop = 0;
                    } else if (value != -1) {
                        sample->tableDelay = value & 127;
                    } else {
                        voice->tablePos = sample->table[voice->tablePos];
                    }
                } while (loop);
            }
        }

        if (sample->portamento) {
            value = PERIODS[voice->note] + voice->pitchBend;

            if (voice->period) {
                if (voice->period < value) {
                    voice->period += sample->portamento;
                    if (voice->period > value) voice->period = value;
                } else {
                    voice->period -= sample->portamento;
                    if (voice->period < value) voice->period = value;
                }
            } else
            {
                voice->period = value;
            }
        }

        if (voice->vibratoCtr) {
            voice->vibratoCtr--;
        }
        else
        {
            voice->vibratoPeriod = voice->vibratoPos * sample->vibratoStep;

            if ((voice->status & 1) == 0) {
                voice->vibratoPos++;
                if (voice->vibratoPos == voice->vibratoDir) voice->status ^= 1;
            } else {
                voice->vibratoPos--;
                if (voice->vibratoPos == 0) voice->status ^= 1;
            }
        }

        if (sample->pitchBend < 0) voice->pitchBend += sample->pitchBend;
        else voice->pitchBend -= sample->pitchBend;

        if (voice->row) {
            row = voice->row;

            switch (row->effect) {
            case 0:
                break;
            case 1:
                value = row->param & 15;
                if (value) speed = value;
                break;
            case 2:
                voice->pitchBend -= row->param;
                break;
            case 3:
                voice->pitchBend += row->param;
                break;
            case 4:
                amiga->filter->active = row->param;
                break;
            case 5:
                sample->vibratoWait = row->param;
                break;
            case 6:
                sample->vibratoStep = row->param;
                break;
            case 7:
                sample->vibratoLen = row->param;
                break;
            case 8:
                sample->pitchBend = row->param;
                break;
            case 9:
                sample->portamento = row->param;
                break;
            case 10:
                value = row->param;
                if (value > 64) value = 64;
                sample->volume = 64;
                break;
            case 11:
                sample->arpeggio[0] = row->param;
                break;
            case 12:
                sample->arpeggio[1] = row->param;
                break;
            case 13:
                sample->arpeggio[2] = row->param;
                break;
            case 14:
                sample->arpeggio[3] = row->param;
                break;
            case 15:
                sample->arpeggio[4] = row->param;
                break;
            case 16:
                sample->arpeggio[5] = row->param;
                break;
            case 17:
                sample->arpeggio[6] = row->param;
                break;
            case 18:
                sample->arpeggio[7] = row->param;
                break;
            case 19:
                sample->arpeggio[0] = sample->arpeggio[4] = row->param;
                break;
            case 20:
                sample->arpeggio[1] = sample->arpeggio[5] = row->param;
                break;
            case 21:
                sample->arpeggio[2] = sample->arpeggio[6] = row->param;
                break;
            case 22:
                sample->arpeggio[3] = sample->arpeggio[7] = row->param;
                break;
            case 23:
                value = row->param;
                if (value > 64) value = 64;
                sample->attackStep = value;
                break;
            case 24:
                sample->attackDelay = row->param;
                break;
            case 25:
                value = row->param;
                if (value > 64) value = 64;
                sample->decayStep = value;
                break;
            case 26:
                sample->decayDelay = row->param;
                break;
            case 27:
                sample->sustain = row->param & (sample->sustain & 255);
                break;
            case 28:
                sample->sustain = (sample->sustain & 0xff00) + row->param;
                break;
            case 29:
                value = row->param;
                if (value > 64) value = 64;
                sample->releaseStep = value;
                break;
            case 30:
                sample->releaseDelay = row->param;
                break;
            }
        }

        if (sample->portamento)
        {
            value = voice->period;
        }
        else {
            value = PERIODS[int(voice->note + sample->arpeggio[voice->arpeggioPos])];
            voice->arpeggioPos = (++voice->arpeggioPos & 7);
            value -= (sample->vibratoLen * sample->vibratoStep);
            value += voice->pitchBend;
            voice->period = 0;
        }

        chan->setPeriod(value + voice->vibratoPeriod);
        adsr  = voice->status & 14;
        value = voice->volume;

        if (adsr == 0) {
            if (voice->attackCtr == 0) {
                voice->attackCtr = sample->attackDelay;
                value += sample->attackStep;

                if (value >= 64) {
                    adsr |= 2;
                    voice->status |= 2;
                    value = 64;
                }
            } else {
                voice->attackCtr--;
            }
        }

        if (adsr == 2) {
            if (voice->decayCtr == 0) {
                voice->decayCtr = sample->decayDelay;
                value -= sample->decayStep;

                if (value <= sample->volume) {
                    adsr |= 6;
                    voice->status |= 6;
                    value = sample->volume;
                }
            } else {
                voice->decayCtr--;
            }
        }

        if (adsr == 6) {
            if (voice->sustain == 0) {
                adsr |= 14;
                voice->status |= 14;
            } else {
                voice->sustain--;
            }
        }

        if (adsr == 14) {
            if (voice->releaseCtr == 0) {
                voice->releaseCtr = sample->releaseDelay;
                value -= sample->releaseStep;

                if (value < 0) {
                    voice->status &= 9;
                    value = 0;
                }
            } else {
                voice->releaseCtr--;
            }
        }

        voice->volume = value;
        chan->setVolume(value);

        chan->setEnabled(1);

        if (!sample->synth) {
            if (sample->loop) {
                chan->pointer = sample->loopPtr;
                chan->length  = sample->repeat;
            } else {
                chan->pointer = amiga->loopPtr;
                chan->length  = 4;
            }
        }

    }
    while(voice = voice->next);
}
void D1Player::initialize()
{
    D1Voice* voice = voices[0];
    AmigaPlayer::initialize();
    speed = 6;

    do
    {
        voice->initialize();
        voice->channel   = amiga->channels[voice->index];
        voice->sample   = samples[20];
    }
    while(voice = voice->next);
}
int D1Player::load(void* _data, unsigned long int length)
{
    unsigned char *stream = static_cast<unsigned char*>(_data);

    if(!(stream[0]=='A' && stream[1]=='L' && stream[2]=='L' && stream[3]==' '))
    {
        return -1;
    }

    unsigned int position=4;
    const int position2 = 104;

    std::vector<unsigned int>data(25);
    for (int i = 0 ; i < 25; ++i)
    {
        data[i] = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);
        position+=4;

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


    for (int i = 0; i < len; ++i) {
        BaseStep* step  = new BaseStep();
        unsigned short value = readEndian(stream[position],stream[position+1]);
        position+=2;

        if (value == 0xffff || position == index) {
            step->pattern   = -1;
            step->transpose = readEndian(stream[position],stream[position+1]);
            position+=2;
            index += data[j++];
        } else {
            position--;
            step->pattern   = ((value >> 2) & 0x3fc0) >> 2;
            step->transpose = (signed char)stream[position];
            position++;
        }
        tracks[i] = step;
    }

    len = data[4] >> 2;
    patterns = std::vector<BaseRow*>(len);
    for (int i = 0; i < len; ++i) {
        BaseRow* row = new BaseRow();
        row->sample = stream[position];
        position++;
        row->note   = stream[position];
        position++;
        row->effect = stream[position] & 31;
        position++;
        row->param  = stream[position];
        position++;
        patterns[i] = row;
    }
    index = 5;

    for (int i = 0; i < 20; ++i)
    {
        if (data[index])
        {
            D1Sample* sample = new D1Sample();
            sample->attackStep   =  stream[position];position++;
            sample->attackDelay  = stream[position];position++;
            sample->decayStep    = stream[position];position++;
            sample->decayDelay   = stream[position];position++;
            sample->sustain      = readEndian(stream[position],stream[position+1]);position+=2;
            sample->releaseStep  = stream[position];position++;
            sample->releaseDelay = stream[position];position++;
            sample->volume       = stream[position];position++;
            sample->vibratoWait  = stream[position];position++;
            sample->vibratoStep  = stream[position];position++;
            sample->vibratoLen   = stream[position];position++;
            sample->pitchBend    = (signed char)stream[position];position++;
            sample->portamento   = stream[position];position++;
            sample->synth        = stream[position];position++;
            sample->tableDelay   = stream[position];position++;

            for (j = 0; j < 8; ++j)
            {
                sample->arpeggio[j] = (signed char)stream[position];position++;
            }

            sample->length = readEndian(stream[position],stream[position+1]);position+=2;
            sample->loop   = readEndian(stream[position],stream[position+1]);position+=2;
            sample->repeat = readEndian(stream[position],stream[position+1]) << 1;position+=2;
            sample->synth  = sample->synth ? 0 : 1;

            if (sample->synth) {
                for (j = 0; j < 48; ++j)
                {
                    sample->table[j] = (signed char)stream[position];position++;

                }

                len = data[index] - 78;
            } else {
                len = sample->length;
            }


            sample->pointer = amiga->store(stream,len,position,length);
            sample->loopPtr = sample->pointer + sample->loop;
            samples[i] = sample;

        }
        else
        {
            samples[i]=0;
        }
        index++;
    }

    D1Sample* sample = new D1Sample();
    sample->pointer = sample->loopPtr = amiga->memory.size();
    sample->length  = sample->repeat  = 4;
    samples[20] = sample;

    m_version = 1;
    format = "Delta Music";
    //printData();
    return 1;

}
void D1Player::printData()
{
    //    for(unsigned int i = 0; i < patterns.size(); i++)
    //    {
    //        AmigaRow* row= patterns[i];
    //        std::cout << "Pattern [" << i << "] note: " << row->note << " sample: " << row->sample << " param: " << row->param << " effect: " << row->effect << "\n";
    //    }
    //    for(unsigned int i = 0; i < samples.size(); i++)
    //    {
    //        D1Sample* sample = samples[i];
    //        if(sample)
    //        {
    //            std::cout << "Sample [" << i << "] length: " << sample->length << " loop: " << sample->loop << " loopPtr: " << sample->loopPtr << " name: " << sample->name << " pointer: " << sample->pointer << " repeat: " << sample->repeat<< " volume: " << (int)sample->volume << "\n";
    //            std::cout << "Sample [" << i << "] synth: " << sample->synth << " attackStep: " << sample->attackStep << " attackDelay: " << sample->attackDelay << " decayStep: " << sample->decayStep << " decayDelay: " << sample->decayDelay << " releaseStep: " << sample->releaseStep << " releaseDelay: " << (int)sample->releaseDelay << " sustain: " << (int)sample->sustain << "\n";
    //            for(unsigned int j = 0; j < sample->table.size(); j++)
    //            {
    //                std::cout << "Sample [" << i << "] Table [" << j << "] " << (int)sample->table[j] << "\n";
    //            }
    //            for(unsigned int j = 0; j < sample->arpeggio.size(); j++)
    //            {
    //                std::cout << "Sample [" << i << "] Arpeggio [" << j << "] " << (int)sample->arpeggio[j] << "\n";
    //            }
    //        }
    //    }

    //    for(unsigned int i = 0; i < tracks.size(); i++)
    //    {
    //        AmigaStep* step = tracks[i];
    //        std::cout << "Tracks [" << i << "] pattern: " << step->pattern << " transpose: " << (int)step->transpose <<  "\n";
    //    }
    //    for(int i = 0; i < amiga->memory.size(); i++)
    //    {
    //        std::cout << "Memory [" << i << "]" << (int)amiga->memory[i] <<  "\n";
    //    }
    //    for(unsigned int i = 0; i < pointers.size(); i++)
    //    {
    //        std::cout << "Pointers [" << i << "] " << pointers[i] <<  "\n";
    //    }
    //    std::flush(std::cout);
}
std::vector<BaseSample*> D1Player::getSamples()
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
