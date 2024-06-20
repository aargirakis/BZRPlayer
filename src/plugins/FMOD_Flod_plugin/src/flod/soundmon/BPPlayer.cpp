#include "BPPlayer.h"
#include "BPVoice.h"
#include "BaseRow.h"
#include "BPSample.h"
#include "BaseStep.h"
#include "AmigaChannel.h"
#include <iostream>
#include "MyEndian.h"
#include <math.h>
using namespace std;
const int BPPlayer::PERIODS[84] =
{
    6848,6464,6080,5760,5440,5120,4832,4576,4320,4064,3840,3616,
    3424,3232,3040,2880,2720,2560,2416,2288,2160,2032,1920,1808,
    1712,1616,1520,1440,1360,1280,1208,1144,1080,1016, 960, 904,
    856, 808, 760, 720, 680, 640, 604, 572, 540, 508, 480, 452,
    428, 404, 380, 360, 340, 320, 302, 286, 270, 254, 240, 226,
    214, 202, 190, 180, 170, 160, 151, 143, 135, 127, 120, 113,
    107, 101,  95,  90,  85,  80,  76,  72,  68,  64,  60,  57
};
const int BPPlayer::VIBRATO[8] =
{
    0,64,128,64,0,-64,-128,-64
};
BPPlayer::BPPlayer(Amiga* amiga):AmigaPlayer(amiga)
{
    buffer = std::vector<int>(128);
    samples    = std::vector<BPSample*>(16);
    voices    = std::vector<BPVoice*>(4);

    voices[0] = new BPVoice(0);
    voices[0]->next = voices[1] = new BPVoice(1);
    voices[1]->next = voices[2] = new BPVoice(2);
    voices[2]->next = voices[3] = new BPVoice(3);
}
BPPlayer::~BPPlayer()
{
    buffer.clear();

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

void BPPlayer::initialize()
{

    AmigaPlayer::initialize();
    speed       = 6;
    tick        = 1;
    trackPos    = 0;
    patternPos  = 0;
    nextPos     = 0;
    jumpFlag    = 0;
    repeatCtr   = 0;
    arpeggioCtr = 0;
    vibratoPos  = 0;

    for (int i = 0; i < 128; ++i)
    {
        buffer[i] = 0;
    }
    BPVoice* voice = voices[0];

    do
    {

        voice->initialize();
        voice->channel   = amiga->channels[voice->index];
        voice->samplePtr = amiga->loopPtr;
    }
    while (voice = voice->next);

}
int BPPlayer::load(void* data, unsigned long int _length)
{
    unsigned char *stream = static_cast<unsigned char*>(data);

    unsigned int position = 0;
    unsigned int tables = 0;
    const int STRING_LENGTH = 26;
    for(int j = 0;j<STRING_LENGTH;j++)
    {
        if(!stream[position+j])
        {
            break;
        }
        m_title+=stream[position+j];
    }
    position+=STRING_LENGTH;




    if(stream[26]=='B' && stream[27]=='P' && stream[28]=='S' && stream[29]=='M')
    {
        m_version = BPSOUNDMON_V1;
        format = "SoundMon1.0";
        position=30;
    }
    else
    {
        if(stream[26]=='V' && stream[27]=='.' && stream[28]=='2' )
        {
            m_version = BPSOUNDMON_V2;
            format = "Soundmon 2";
        }
        else if(stream[26]=='V' && stream[27]=='.' && stream[28]=='3' )
        {
            m_version = BPSOUNDMON_V3;
            format = "Soundmon 3";
        }
        else
        {
            return -1;
        }
        position=29;
        tables = stream[position];
        position++;
    }

    length = readEndian(stream[position],stream[position+1]);
    position+=2;

    for (int i = 0; ++i < 16;) {
        BPSample* sample = new BPSample();

        if (stream[position] == 0xff)
        {
            position++;
            sample->synth   = 1;
            sample->table   = stream[position];position++;
            sample->pointer = sample->table << 6;
            sample->length  = readEndian(stream[position],stream[position+1]) << 1 ; position+=2;

            sample->adsrControl = stream[position] ; position++;
            sample->adsrTable   = stream[position] << 6; position++;
            sample->adsrLen     = readEndian(stream[position],stream[position+1]); position+=2;
            sample->adsrSpeed   = stream[position] ; position++;
            sample->lfoControl  = stream[position] ; position++;
            sample->lfoTable    =  stream[position] << 6; position++;
            sample->lfoDepth    = stream[position] ; position++;
            sample->lfoLen      = readEndian(stream[position],stream[position+1]); position+=2;

            if (m_version < BPSOUNDMON_V3) {
                position++;
                sample->lfoDelay  = stream[position] ; position++;
                sample->lfoSpeed  = stream[position] ; position++;
                sample->egControl = stream[position] ; position++;
                sample->egTable   = stream[position] << 6; position++;
                position++;
                sample->egLen     = readEndian(stream[position],stream[position+1]); position+=2;
                position++;
                sample->egDelay   = stream[position] ; position++;
                sample->egSpeed   =stream[position] ; position++;
                sample->fxSpeed   = 1;
                sample->modSpeed  = 1;
                sample->volume    = stream[position] ; position++;
                position += 6;
            } else {
                sample->lfoDelay   = stream[position] ; position++;
                sample->lfoSpeed   = stream[position] ; position++;
                sample->egControl  = stream[position] ; position++;
                sample->egTable    = stream[position] << 6; position++;
                sample->egLen      = readEndian(stream[position],stream[position+1]); position+=2;
                sample->egDelay    = stream[position] ; position++;
                sample->egSpeed    = stream[position] ; position++;
                sample->fxControl  = stream[position] ; position++;
                sample->fxSpeed    = stream[position] ; position++;
                sample->fxDelay    = stream[position] ; position++;
                sample->modControl = stream[position] ; position++;
                sample->modTable   = stream[position] << 6; position++;
                sample->modSpeed   =  stream[position] ; position++;
                sample->modDelay   =  stream[position] ; position++;
                sample->volume     =  stream[position] ; position++;
                sample->modLen     = readEndian(stream[position],stream[position+1]); position+=2;
            }
        } else {

            sample->synth  = 0;
            const int STRING_LENGTH = 24;
            for(int j = 0;j<STRING_LENGTH;j++)
            {
                if(!stream[position+j])
                {
                    break;
                }
                sample->name+=stream[position+j];
            }
            position+=STRING_LENGTH;
            sample->length = readEndian(stream[position],stream[position+1]) << 1 ; position+=2;

            if (sample->length) {
                sample->loopPtr   = readEndian(stream[position],stream[position+1]); position+=2;
                sample->repeat = readEndian(stream[position],stream[position+1]) << 1 ; position+=2;
                sample->volume =readEndian(stream[position],stream[position+1]); position+=2;

                if ((sample->loopPtr + sample->repeat) >= sample->length)
                    sample->repeat = sample->length - sample->loopPtr;
            } else {
                sample->pointer--;
                sample->repeat = 2;
                position += 6;
            }
        }

        samples[i] = sample;
    }

    int len = length << 2;
    tracks = std::vector<BaseStep*>(len);

    int higher=0;
    for (int i = 0; i < len; ++i) {
        BaseStep* step = new BaseStep();
        step->pattern = readEndian(stream[position],stream[position+1]); position+=2;
        step->soundTrans = (signed char)stream[position] ; position++;
        step->transpose = (signed char)stream[position] ; position++;
        if (step->pattern > higher) higher = step->pattern;
        tracks[i] = step;
    }

    len = higher << 4;
    patterns = std::vector<BaseRow*>(len);
    for (int i = 0; i < len; ++i) {
        BaseRow* row = new BaseRow();
        row->note   = (signed char)stream[position] ; position++;
        row->sample = stream[position] ; position++;
        row->effect = row->sample & 0x0f;
        row->sample = (row->sample & 0xf0) >> 4;
        row->param  = (signed char)stream[position] ; position++;
        patterns[i] = row;
    }


    amiga->store(stream, tables << 6, position, _length);
    for (int i = 0; ++i < 16;)
    {
        BPSample* sample = samples[i];
        if (sample->synth || !sample->length) continue;
        sample->pointer = amiga->store(stream, sample->length, position, _length);
        sample->loopPtr += sample->pointer;
    }
    //printData();
    return 1;

}
void BPPlayer::process()
{
    int data = 0;
    int dst = 0;
    int instr = 0;
    int len = 0;
    int note = 0;
    AmigaChannel* chan;
    int option = 0;
    BaseRow* row;
    BPSample* sample;
    int src = 0;
    BaseStep* step;

    if(trackPosBuffer.size()==22)
    {
        trackPosBuffer.pop_front();
        patternPosBuffer.pop_front();
    }
    trackPosBuffer.push_back(patternPos/4);
    patternPosBuffer.push_back(trackPos);


    BPVoice* voice = voices[0];
    arpeggioCtr = (--arpeggioCtr & 3);
    vibratoPos  = (++vibratoPos  & 7);
    do {
        chan = voice->channel;
        voice->period += voice->autoSlide;
        if (voice->vibrato)
        {
            chan->setPeriod(voice->period + (VIBRATO[vibratoPos] / voice->vibrato));
        }
        else
        {
            chan->setPeriod(voice->period);
        }

        chan->pointer = voice->samplePtr;
        chan->length  = voice->sampleLen;
        if (voice->arpeggio || voice->autoArpeggio) {
            note = voice->note;

            if (!arpeggioCtr)
            {
                note += ((voice->arpeggio & 0xf0) >> 4) + ((voice->autoArpeggio & 0xf0) >> 4);
            }
            else if (arpeggioCtr == 1)
            {
                note += (voice->arpeggio & 0x0f) + (voice->autoArpeggio & 0x0f);
            }

            voice->period = PERIODS[note + 35];
            chan->setPeriod(voice->period);
            voice->restart = 0;
        }

        if (!voice->synth || voice->sample < 0) continue;

        sample = samples[voice->sample];
        if (voice->adsrControl) {
            if (--voice->adsrCtr == 0) {
                voice->adsrCtr = sample->adsrSpeed;
                data = (128 + amiga->memory[int(sample->adsrTable + voice->adsrPtr)]) >> 2;
                chan->setVolume((data * voice->volume) >> 6);

                if (++voice->adsrPtr == sample->adsrLen) {
                    voice->adsrPtr = 0;
                    if (voice->adsrControl == 1) voice->adsrControl = 0;
                }
            }
        }

        if (voice->lfoControl) {
            if (--voice->lfoCtr == 0) {
                voice->lfoCtr = sample->lfoSpeed;
                data = amiga->memory[int(sample->lfoTable + voice->lfoPtr)];
                if (sample->lfoDepth) data = (data / sample->lfoDepth) >> 0;
                chan->setPeriod(voice->period + data);

                if (++voice->lfoPtr == sample->lfoLen) {
                    voice->lfoPtr = 0;
                    if (voice->lfoControl == 1) voice->lfoControl = 0;
                }
            }
        }

        if (voice->synthPtr < 0) continue;

        if (voice->egControl) {
            if (--voice->egCtr == 0) {
                voice->egCtr = sample->egSpeed;
                data = voice->egValue;
                voice->egValue = (128 + amiga->memory[int(sample->egTable + voice->egPtr)]) >> 3;

                if (voice->egValue != data) {
                    src = (voice->index << 5) + data;
                    dst = voice->synthPtr + data;

                    if (voice->egValue < data) {
                        data -= voice->egValue;
                        len = dst - data;
                        for (; dst > len;) amiga->memory[--dst] = buffer[--src];
                    } else {
                        data = voice->egValue - data;
                        len = dst + data;
                        for (; dst < len;) amiga->memory[dst++] = ~buffer[src++] + 1;
                    }
                }

                if (++voice->egPtr == sample->egLen) {
                    voice->egPtr = 0;
                    if (voice->egControl == 1) voice->egControl = 0;
                }
            }
        }
        switch (voice->fxControl) {
        case 0:
            break;
        case 1:   //averaging
            if (--voice->fxCtr == 0) {
                voice->fxCtr = sample->fxSpeed;
                dst = voice->synthPtr;
                len = voice->synthPtr + 32;
                data = dst > 0 ? amiga->memory[int(dst - 1)] : 0;

                for (; dst < len;) {
                    data = (data + amiga->memory[int(dst + 1)]) >> 1;
                    amiga->memory[dst++] = data;
                }
            }
            break;
        case 2:   //inversion
            src = (voice->index << 5) + 31;
            len = voice->synthPtr + 32;
            data = sample->fxSpeed;

            for (dst = voice->synthPtr; dst < len; ++dst) {
                if (buffer[src] < amiga->memory[dst]) {
                    amiga->memory[dst] -= data;
                } else if (buffer[src] > amiga->memory[dst]) {
                    amiga->memory[dst] += data;
                }
                src--;
            }
            break;
        case 3:   //backward inversion
        case 5:   //backward transform
            src = voice->index << 5;
            len = voice->synthPtr + 32;
            data = sample->fxSpeed;

            for (dst = voice->synthPtr; dst < len; ++dst) {
                if (buffer[src] < amiga->memory[dst]) {
                    amiga->memory[dst] -= data;
                } else if (buffer[src] > amiga->memory[dst]) {
                    amiga->memory[dst] += data;
                }
                src++;
            }
            break;
        case 4:   //transform
            src = voice->synthPtr + 64;
            len = voice->synthPtr + 32;
            data = sample->fxSpeed;

            for (dst = voice->synthPtr; dst < len; ++dst) {
                if (amiga->memory[src] < amiga->memory[dst]) {
                    amiga->memory[dst] -= data;
                } else if (amiga->memory[src] > amiga->memory[dst]) {
                    amiga->memory[dst] += data;
                }
                src++;
            }
            break;
        case 6:   //wave change
            if (--voice->fxCtr == 0) {
                voice->fxControl = 0;
                voice->fxCtr = 1;
                src = voice->synthPtr + 64;
                len = voice->synthPtr + 32;
                for (dst = voice->synthPtr; dst < len; ++dst) amiga->memory[dst] = amiga->memory[src++];
            }
            break;
        }

        if (voice->modControl) {
            if (--voice->modCtr == 0) {
                voice->modCtr = sample->modSpeed;
                amiga->memory[voice->synthPtr + 32] = amiga->memory[int(sample->modTable + voice->modPtr)];

                if (++voice->modPtr == sample->modLen) {
                    voice->modPtr = 0;
                    if (voice->modControl == 1) voice->modControl = 0;
                }
            }
        }
    } while (voice = voice->next);

    if (--tick == 0) {
        tick = speed;
        voice = voices[0];

        do {
            chan = voice->channel;
            voice->enabled = 0;

            step   = tracks[int((trackPos << 2) + voice->index)];
            row    = patterns[int(patternPos + ((step->pattern - 1) << 4))];
            note   = row->note;
            option = row->effect;
            data   = row->param;

            if (note) {
                voice->autoArpeggio = voice->autoSlide = voice->vibrato = 0;
                if (option != 10 || (data & 0xf0) == 0) note += step->transpose;
                voice->note = note;
                voice->period = PERIODS[int(note + 35)];

                if (option < 13)
                {
                    voice->restart = voice->volumeDef = 1;
                }
                else
                {
                    voice->restart = 0;
                }

                instr = row->sample;
                if (instr == 0) instr = voice->sample;
                if (option != 10 || (data & 0x0f) == 0) instr += step->soundTrans;

                if (option < 13 && (!voice->synth || (voice->sample != instr))) {
                    voice->sample = instr;
                    voice->enabled = 1;
                }
            }

            switch (option) {
            case 0:   //arpeggio once
                voice->arpeggio = data;
                break;
            case 1:   //set volume
                voice->volume = data;
                voice->volumeDef = 0;

                if (m_version < BPSOUNDMON_V3 || !voice->synth)
                {
                    chan->setVolume(voice->volume);
                }
                break;
            case 2:   //set speed
                tick = speed = data;
                break;
            case 3:   //set filter
                amiga->filter->active = data;
                break;
            case 4:   //portamento up
                voice->period -= data;
                voice->arpeggio = 0;
                break;
            case 5:   //portamento down
                voice->period += data;
                voice->arpeggio = 0;
                break;
            case 6:   //set vibrato
                if (m_version == BPSOUNDMON_V3) voice->vibrato = data;
                else repeatCtr = data;
                break;
            case 7:   //step jump
                if (m_version == BPSOUNDMON_V3) {
                    nextPos = data;
                    jumpFlag = 1;
                } else if (repeatCtr == 0) {
                    trackPos = data;
                }
                break;
            case 8:   //set auto slide
                voice->autoSlide = data;
                break;
            case 9:   //set auto arpeggio
                voice->autoArpeggio = data;
                if (m_version == BPSOUNDMON_V3) {
                    voice->adsrPtr = 0;
                    if (voice->adsrControl == 0) voice->adsrControl = 1;
                }
                break;
            case 11:  //change effect
                voice->fxControl = data;
                break;
            case 13:  //change inversion
                voice->autoArpeggio = data;
                voice->fxControl ^= 1;
                voice->adsrPtr = 0;
                if (voice->adsrControl == 0) voice->adsrControl = 1;
                break;
            case 14:  //no eg reset
                voice->autoArpeggio = data;
                voice->adsrPtr = 0;
                if (voice->adsrControl == 0) voice->adsrControl = 1;
                break;
            case 15:  //no eg and no adsr reset
                voice->autoArpeggio = data;
                break;
            }

        } while (voice = voice->next);

        if (jumpFlag) {
            trackPos   = nextPos;
            patternPos = jumpFlag = 0;
        } else if (++patternPos == 16) {
            patternPos = 0;

            if (++trackPos == length) {
                trackPos = 0;
                amiga->setComplete(1);
            }
        }
        voice = voices[0];

        do {
            chan = voice->channel;
            if (voice->enabled)
            {
                chan->setEnabled(0);
                voice->enabled = 0;
            }

            if (voice->restart == 0) continue;

            if (voice->synthPtr > -1) {
                src = voice->index << 5;
                len = voice->synthPtr + 32;
                for (dst = voice->synthPtr; dst < len; ++dst) amiga->memory[dst] = buffer[src++];
                voice->synthPtr = -1;
            }
        }
        while (voice = voice->next);

        voice = voices[0];

        do {
            if (voice->restart == 0 || voice->sample < 0) continue;

            chan = voice->channel;

            chan->setPeriod(voice->period);
            voice->restart = 0;
            sample = samples[voice->sample];

            if (sample->synth) {

                voice->synth   = 1;
                voice->egValue = 0;
                voice->adsrPtr = voice->lfoPtr = voice->egPtr = voice->modPtr = 0;

                voice->adsrCtr = 1;
                voice->lfoCtr  = sample->lfoDelay + 1;
                voice->egCtr   = sample->egDelay  + 1;
                voice->fxCtr   = sample->fxDelay  + 1;
                voice->modCtr  = sample->modDelay + 1;

                voice->adsrControl = sample->adsrControl;
                voice->lfoControl  = sample->lfoControl;
                voice->egControl   = sample->egControl;
                voice->fxControl   = sample->fxControl;
                voice->modControl  = sample->modControl;

                chan->pointer = voice->samplePtr = sample->pointer;
                chan->length  = voice->sampleLen = sample->length;

                if (voice->adsrControl) {
                    data = (128 + amiga->memory[sample->adsrTable]) >> 2;

                    if (voice->volumeDef) {
                        voice->volume = sample->volume;
                        voice->volumeDef = 0;
                    }

                    chan->setVolume((data * voice->volume) >> 6);
                } else {
                    chan->setVolume(voice->volumeDef ? sample->volume : voice->volume);
                }

                if (voice->egControl || voice->fxControl || voice->modControl) {
                    voice->synthPtr = sample->pointer;
                    dst = voice->index << 5;
                    len = voice->synthPtr + 32;
                    for (src = voice->synthPtr; src < len; ++src) buffer[dst++] = amiga->memory[src];
                }
            } else {
                voice->synth = voice->lfoControl = 0;

                if (sample->pointer < 0) {
                    voice->samplePtr = amiga->loopPtr;
                    voice->sampleLen = 4;
                } else {
                    chan->pointer = sample->pointer;
                    chan->setVolume(voice->volumeDef ? sample->volume : voice->volume);

                    if (sample->repeat != 2) {
                        voice->samplePtr = sample->loopPtr;
                        chan->length = voice->sampleLen = sample->repeat;
                    } else {
                        voice->samplePtr = amiga->loopPtr;
                        voice->sampleLen = 4;
                        chan->length = sample->length;
                    }
                }
            }

            voice->enabled = 1;
            chan->setEnabled(1);
        }
        while (voice = voice->next);
    }
}
void BPPlayer::printData()
{
    //    for(unsigned int i = 0; i < patterns.size(); i++)
    //    {
    //        AmigaRow* row= patterns[i];
    //        std::cout << "Pattern [" << i << "] note: " << row->note << " sample: " << row->sample << " param: " << (int)row->param << " effect: " << row->effect << "\n";
    //    }
    //    for(unsigned int i = 0; i < samples.size(); i++)
    //    {
    //        BPSample* sample = samples[i];
    //        if(sample)
    //        {
    //            std::cout << "Sample [" << i << "] length: " << sample->length << " loop: " << sample->loop << " loopPtr: " << sample->loopPtr << " name: " << sample->name << " pointer: " << sample->pointer << " repeat: " << sample->repeat<< " volume: " << (int)sample->volume << "\n";
    //            std::cout << "Sample [" << i << "] synth: " << sample->synth << " table: " << sample->table << " adsrControl: " << sample->adsrControl << " adsrTable: " << sample->adsrTable << " adsrLen: " << sample->adsrLen << " adsrSpeed: " << sample->adsrSpeed << " lfoControl: " << (int)sample->lfoControl << " lfoTable: " << (int)sample->lfoTable << "\n";
    //            std::cout << "Sample [" << i << "] lfoDepth: " << sample->lfoDepth << " lfoLen: " << sample->lfoLen << " lfoDelay: " << sample->lfoDelay << " lfoSpeed: " << sample->lfoSpeed << " egControl: " << sample->egControl << " egTable: " << sample->egTable << " egLen: " << (int)sample->egLen << " egDelay: " << (int)sample->egDelay << "\n";
    //            std::cout << "Sample [" << i << "] egSpeed: " << sample->egSpeed << " fxControl: " << sample->fxControl << " fxDelay: " << sample->fxDelay << " fxSpeed: " << sample->fxSpeed << " modControl: " << sample->modControl << " modTable: " << sample->modTable << " modLen: " << (int)sample->modLen << " modDelay: " << (int)sample->modDelay << " modSpeed: " << (int)sample->modSpeed << "\n";
    //        }
    //    }

    //    for(unsigned int i = 0; i < tracks.size(); i++)
    //    {
    //        BPStep* step = tracks[i];
    //        std::cout << "Tracks [" << i << "] pattern: " << step->pattern << " transpose: " << (int)step->transpose << " soundTranspose: " << (int)step->soundTranspose << "\n";
    //    }
}
std::vector<BaseSample*> BPPlayer::getSamples()
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
bool BPPlayer::getTitle(std::string& title)
{
    title = this->m_title;
    return true;
}
unsigned int BPPlayer::getCurrentRow()
{
    return trackPosBuffer.front();
}
unsigned int BPPlayer::getCurrentPattern()
{
    return patternPosBuffer.front();
}
std::vector<BaseRow*>& BPPlayer::getModRows()
{
    return patterns;
}
