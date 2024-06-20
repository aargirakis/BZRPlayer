#include "S2Player.h"
#include "S2Voice.h"
#include "BaseRow.h"
#include "S2Sample.h"
#include "S2Instrument.h"
#include "BaseStep.h"
#include "AmigaChannel.h"
#include <iostream>
#include "MyEndian.h"

const int S2Player::PERIODS[73] =
{
    0,
    5760,5424,5120,4832,4560,4304,4064,3840,3616,3424,3232,3048,
    2880,2712,2560,2416,2280,2152,2032,1920,1808,1712,1616,1524,
    1440,1356,1280,1208,1140,1076,1016, 960, 904, 856, 808, 762,
    720, 678, 640, 604, 570, 538, 508, 480, 453, 428, 404, 381,
    360, 339, 320, 302, 285, 269, 254, 240, 226, 214, 202, 190,
    180, 170, 160, 151, 143, 135, 127, 120, 113, 107, 101,  95
};

S2Player::S2Player(Amiga* amiga):AmigaPlayer(amiga)
{
    arpeggioFx = std::vector<unsigned char>(4);
    voices    = std::vector<S2Voice*>(4);
    voices[0] = new S2Voice(0);
    voices[0]->next = voices[1] = new S2Voice(1);
    voices[1]->next = voices[2] = new S2Voice(2);
    voices[2]->next = voices[3] = new S2Voice(3);

    arpeggioPos = 0;
    length = 0;
    speedDef = 0;
    trackPos = 0;
    patternPos = 0;
    patternLen = 0;
}
S2Player::~S2Player()
{
    arpeggioFx.clear();
    arpeggios.clear();
    vibratos.clear();
    waves.clear();

    for(unsigned int i = 0; i < tracks.size(); i++)
    {
        if(tracks[i]) delete tracks[i];
    }
    for(unsigned int i = 0; i < instruments.size(); i++)
    {
        if(instruments[i]) delete instruments[i];
    }
    instruments.clear();
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
void S2Player::initialize()
{

    AmigaPlayer::initialize();
    speed       = speedDef;
    tick       =  speedDef;
    trackPos    = 0;
    patternPos  = 0;
    patternLen  = 64;

    S2Voice* voice = voices[0];
    while (voice)
    {
        voice->initialize();
        voice->channel   = amiga->channels[voice->index];
        voice->instr = instruments[0];

        arpeggioFx[voice->index] = 0;
        voice = voice->next;
    }
}
void S2Player::process()
{

    AmigaChannel* chan;
    BaseRow* row;
    S2Voice* voice=voices[0];
    S2Sample* sample;
    S2Instrument* instr;
    int value = 0;
    arpeggioPos = (++arpeggioPos & 3);

    if (++tick >= speed) {
        tick = 0;

        do
        {
            chan = voice->channel;
            voice->note = 0;
            voice->enabled = 0;

            if (!patternPos) {
                voice->step    = tracks[int(trackPos + (voice->index * length))];
                voice->pattern = voice->step->pattern;
                voice->speed   = 0;
            }

            if (--voice->speed < 0) {
                voice->row   = row = patterns[voice->pattern++];
                voice->speed = row->speed;

                if (row->note) {
                    voice->enabled = 1;
                    voice->note    = row->note + voice->step->transpose;
                    chan->setEnabled(0);
                }
            }

            voice->pitchBend = 0;

            if (voice->note) {
                voice->waveCtr      = voice->sustainCtr     = 0;
                voice->arpeggioCtr  = voice->arpeggioPos    = 0;
                voice->vibratoCtr   = voice->vibratoPos     = 0;
                voice->pitchBendCtr = voice->noteSlideSpeed = 0;
                voice->adsrPos = 4;
                voice->volume  = 0;

                if (row->sample) {
                    voice->instrument = row->sample;
                    voice->instr  = instruments[voice->instrument + voice->step->soundTrans];
                    voice->sample = samples[waves[voice->instr->wave]];

                }
                voice->original = voice->note + arpeggios[voice->instr->arpeggio];
                chan->setPeriod(PERIODS[voice->original]);
                voice->period = PERIODS[voice->original];
                sample = voice->sample;
                chan->pointer = sample->pointer;
                chan->length  = sample->length;
                chan->setEnabled(voice->enabled);
                chan->pointer = sample->loopPtr;
                chan->length  = sample->repeat;
            }
        } while (voice = voice->next);

        if (++patternPos == patternLen) {
            patternPos = 0;

            if (++trackPos == length) {
                trackPos = 0;
                amiga->setComplete(1);
            }
        }

    }
    voice = voices[0];

    do
    {
        if (!voice->sample) continue;
        chan = voice->channel;

        sample = voice->sample;

        if (sample->negToggle) continue;

        sample->negToggle = 1;

        if (sample->negCtr) {
            sample->negCtr = (--sample->negCtr & 31);
        } else {
            sample->negCtr = sample->negSpeed;
            if (!sample->negDir) continue;

            value = sample->negStart + sample->negPos;
            amiga->memory[value] = ~amiga->memory[value];
            sample->negPos += sample->negOffset;
            value = sample->negLen - 1;

            if (sample->negPos < 0) {
                if (sample->negDir == 2) {
                    sample->negPos = value;
                } else {
                    sample->negOffset = -sample->negOffset;
                    sample->negPos += sample->negOffset;
                }
            } else if (value < sample->negPos) {
                if (sample->negDir == 1) {
                    sample->negPos = 0;
                } else {
                    sample->negOffset = -sample->negOffset;
                    sample->negPos += sample->negOffset;
                }
            }
        }
    } while (voice = voice->next);

    voice = voices[0];

    do
    {
        if (!voice->sample) continue;

        voice->sample->negToggle = 0;
    }
    while (voice = voice->next);

    voice = voices[0];

    do
    {
        chan  = voice->channel;
        instr = voice->instr;

        switch (voice->adsrPos) {
        case 0:
            break;
        case 4:   //attack
            voice->volume += instr->attackSpeed;
            if (instr->attackMax <= voice->volume) {
                voice->volume = instr->attackMax;
                voice->adsrPos--;
            }
            break;
        case 3:   //decay
            if (!instr->decaySpeed) {
                voice->adsrPos--;
            } else {
                voice->volume -= instr->decaySpeed;
                if (instr->decayMin >= voice->volume) {
                    voice->volume = instr->decayMin;
                    voice->adsrPos--;
                }
            }
            break;
        case 2:   //sustain
            if (voice->sustainCtr == instr->sustain)
            {
                voice->adsrPos--;
            }
            else
            {
                voice->sustainCtr++;
            }
            break;
        case 1:   //release
            voice->volume -= instr->releaseSpeed;
            if (instr->releaseMin >= voice->volume) {
                voice->volume = instr->releaseMin;
                voice->adsrPos--;
            }
            break;
        }
        chan->setVolume(voice->volume >> 2);

        if (instr->waveLen) {
            if (voice->waveCtr == instr->waveDelay) {
                voice->waveCtr = instr->waveDelay - instr->waveSpeed;
                if (voice->wavePos == instr->waveLen)
                {
                    voice->wavePos = 0;
                }
                else
                {
                    voice->wavePos++;
                }

                voice->sample = sample = samples[waves[int(instr->wave + voice->wavePos)]];
                chan->pointer = sample->pointer;
                chan->length  = sample->length;
            }
            else
            {
                voice->waveCtr++;
            }
        }

        if (instr->arpeggioLen) {
            if (voice->arpeggioCtr == instr->arpeggioDelay) {
                voice->arpeggioCtr = instr->arpeggioDelay - instr->arpeggioSpeed;

                if (voice->arpeggioPos == instr->arpeggioLen)
                {
                    voice->arpeggioPos = 0;
                }
                else
                {
                    voice->arpeggioPos++;
                }

                value = voice->original + arpeggios[int(instr->arpeggio + voice->arpeggioPos)];
                voice->period = PERIODS[value];
            } else
            {
                voice->arpeggioCtr++;
            }
        }

        row = voice->row;

        if (tick) {
            switch (row->effect) {
            case 0:
                break;
            case 0x70:  //arpeggio
                arpeggioFx[0] = row->param >> 4;
                arpeggioFx[2] = row->param & 15;
                value = voice->original + arpeggioFx[arpeggioPos];
                voice->period = PERIODS[value];
                break;
            case 0x71:  //pitch up
                voice->pitchBend = ~row->param + 1;
                break;
            case 0x72:  //pitch down
                voice->pitchBend = row->param;
                break;
            case 0x73:  //volume up
                if (voice->adsrPos) break;
                if (voice->instrument) voice->volume = instr->attackMax;
                voice->volume += (row->param << 2);
                if (voice->volume >= 256) voice->volume = -1;
                break;
            case 0x74:  //volume down
                if (voice->adsrPos) break;
                if (voice->instrument) voice->volume = instr->attackMax;
                voice->volume -= (row->param << 2);
                if (voice->volume < 0) voice->volume = 0;
                break;
            }
        }

        switch (row->effect) {
        case 0:
            break;
        case 0x75:  //set adsr attack
            instr->attackMax   = row->param;
            instr->attackSpeed = row->param;
            break;
        case 0x76:  //set pattern length
            patternLen = row->param;
            break;
        case 0x7c:  //set volume
            chan->setVolume(row->param);
            voice->volume = row->param << 2;
            if (voice->volume >= 255) voice->volume = 255;
            break;
        case 0x7f:  //set speed
            value = row->param & 15;
            if (value) speed = value;
            break;
        }

        if (instr->vibratoLen) {
            if (voice->vibratoCtr == instr->vibratoDelay) {
                voice->vibratoCtr = instr->vibratoDelay - instr->vibratoSpeed;

                if (voice->vibratoPos == instr->vibratoLen)
                {
                    voice->vibratoPos = 0;
                }
                else
                {
                    voice->vibratoPos++;
                }

                voice->period += vibratos[int(instr->vibrato + voice->vibratoPos)];
            } else
            {
                voice->vibratoCtr++;
            }
        }

        if (instr->pitchBend) {
            if (voice->pitchBendCtr == instr->pitchBendDelay)
            {
                voice->pitchBend += instr->pitchBend;
            } else
            {
                voice->pitchBendCtr++;
            }
        }

        if (row->param) {
            if (row->effect && row->effect < 0x70) {
                voice->noteSlideTo = PERIODS[int(row->effect + voice->step->transpose)];
                value = row->param;
                if ((voice->noteSlideTo - voice->period) < 0) value = -value;
                voice->noteSlideSpeed = value;
            }
        }

        if (voice->noteSlideTo && voice->noteSlideSpeed) {
            voice->period += voice->noteSlideSpeed;

            if ((voice->noteSlideSpeed < 0 && voice->period < voice->noteSlideTo) ||
                    (voice->noteSlideSpeed > 0 && voice->period > voice->noteSlideTo)) {
                voice->noteSlideSpeed = 0;
                voice->period = voice->noteSlideTo;
            }
        }

        voice->period += voice->pitchBend;
        if (voice->period < 95) voice->period = 95;
        else if (voice->period > 5760) voice->period = 5760;

        chan->setPeriod(voice->period);
    }
    while (voice = voice->next);
}

int S2Player::load(void* data, unsigned long int _length)
{
    unsigned char *stream = static_cast<unsigned char*>(data);
    if(stream[58]=='S' && stream[59]=='I' && stream[60]=='D' && stream[61]=='M' && stream[62]=='O' && stream[63]=='N' && stream[64]==' ' &&
            stream[65]=='I' && stream[66]=='I' && stream[67]==' ' && stream[68]=='-' && stream[69]==' ' && stream[70]=='T' && stream[71]=='H' && stream[72]=='E' &&
            stream[73]==' ' && stream[74]=='M' && stream[75]=='I' && stream[76]=='D' && stream[77]=='I' && stream[78]==' ' && stream[79]=='V' && stream[80]=='E' &&
            stream[81]=='R' && stream[82]=='S' && stream[83]=='I' && stream[84]=='O' && stream[85]=='N')
    {
        int value = 0;
        int base = 0;
        unsigned int position = 2;
        length   = stream[position];
        position++;
        speedDef = stream[position];
        position++;
        samples  = std::vector<S2Sample*>(readEndian(stream[position],stream[position+1]) >> 6) ; position+=2;

        position = 14;
        int len = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]) ; position+=4;
        tracks = std::vector<BaseStep*>(len);
        position = 90;

        int higher=0;
        for (int i = 0; i < len; ++i) {
            BaseStep* step = new BaseStep();
            value = stream[position];
            position++;
            if (value > higher) higher = value;
            step->pattern = value;
            tracks[i] = step;
        }

        for (int i = 0; i < len; ++i) {
            BaseStep* step = tracks[i];
            step->transpose = (signed char)stream[position];
            position++;
        }

        for (int i = 0; i < len; ++i) {
            BaseStep* step = tracks[i];
            step->soundTrans = (signed char)stream[position];
            position++;
        }

        int pos = position;
        position = 26;
        len = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]) >> 5 ; position+=4;
        instruments = std::vector<S2Instrument*>(++len);
        position = pos;
        instruments[0] = new S2Instrument();

        for (int i = 1; i < len; ++i) {
            S2Instrument* instr = new S2Instrument();
            instr->wave           = stream[position] << 4;position++;
            instr->waveLen        = stream[position] ; position++;
            instr->waveSpeed      = stream[position] ; position++;
            instr->waveDelay      = stream[position] ; position++;
            instr->arpeggio       = stream[position] << 4;position++;
            instr->arpeggioLen    = stream[position] ; position++;
            instr->arpeggioSpeed  = stream[position] ; position++;
            instr->arpeggioDelay  = stream[position] ; position++;
            instr->vibrato        = stream[position] << 4; position++;
            instr->vibratoLen     = stream[position] ; position++;
            instr->vibratoSpeed   = stream[position] ; position++;
            instr->vibratoDelay   = stream[position] ; position++;
            instr->pitchBend      = (signed char)stream[position] ; position++;
            instr->pitchBendDelay = stream[position] ; position++;
            position+=2;
            instr->attackMax      = stream[position] ; position++;
            instr->attackSpeed    = stream[position] ; position++;
            instr->decayMin       = stream[position] ; position++;
            instr->decaySpeed     = stream[position] ; position++;
            instr->sustain        = stream[position] ; position++;
            instr->releaseMin     = stream[position] ; position++;
            instr->releaseSpeed   = stream[position] ; position++;
            instruments[i] = instr;
            position += 9;
        }
        pos = position;
        position = 30;
        len = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]) ; position+=4;
        waves = std::vector<unsigned char>(len);
        position = pos;

        for (int i = 0; i < len; ++i)
        {
            waves[i] = stream[position];
            position++;
        }


        pos = position;
        position = 34;
        len = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]) ; position+=4;
        arpeggios = std::vector<signed char>(len);
        position = pos;

        for (int i = 0; i < len; ++i)
        {
            arpeggios[i] = (signed char)stream[position];
            position++;
        }

        pos = position;
        position = 38;
        len = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]) ; position+=4;
        vibratos = std::vector<signed char>(len);
        position = pos;

        for (int i = 0; i < len; ++i)
        {
            vibratos[i] = (signed char)stream[position];
            position++;
        }

        len = samples.size();
        pos = 0;

        for (int i = 0; i < len; ++i) {
            S2Sample* sample = new S2Sample();
            position+=4;
            sample->pointer = pos;
            sample->length    = readEndian(stream[position],stream[position+1]) << 1;position+=2;
            sample->loopPtr      = (readEndian(stream[position],stream[position+1]) << 1)+pos;position+=2;
            sample->repeat    = readEndian(stream[position],stream[position+1]) << 1;position+=2;
            sample->negStart  = (readEndian(stream[position],stream[position+1]) << 1)+pos;position+=2;
            sample->negLen    = readEndian(stream[position],stream[position+1]) << 1;position+=2;
            sample->negSpeed  = readEndian(stream[position],stream[position+1]) ; position+=2;
            sample->negDir    = readEndian(stream[position],stream[position+1]) ; position+=2;
            sample->negOffset = readEndian(stream[position],stream[position+1]) ; position+=2;
            sample->negPos    = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]) ; position+=4;
            sample->negCtr    = readEndian(stream[position],stream[position+1]) ; position+=2;
            position += 6;
            const int STRING_LENGTH = 32;
            for(int j = 0;j<STRING_LENGTH;j++)
            {
                if(!stream[position+j])
                {
                    break;
                }
                sample->name+=stream[position+j];
            }
            position+=STRING_LENGTH;
            pos += sample->length;
            samples[i] = sample;
        }

        int sampleData = pos;
        len = ++higher;
        std::vector<int>pointers (++higher);
        for (int i = 0; i < len; ++i)
        {
            pointers[i]=readEndian(stream[position],stream[position+1]) ; position+=2;
        }
        pos = position;
        position = 50;
        len = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]) ; position+=4;
        patterns = std::vector<BaseRow*>();
        position = pos;
        int j = 1;

        for (int i = 0; i < len; ++i) {
            BaseRow* row   = new BaseRow();
            char value = (signed char)stream[position];
            position++;

            if (!value) {
                row->effect = (signed char)stream[position]; position++;
                row->param  = stream[position]; position++;
                i += 2;
            } else if (value < 0) {
                row->speed = ~value;
            } else if (value < 112) {
                row->note = value;
                value = (signed char)stream[position]; position++;
                i++;

                if (value < 0) {
                    row->speed = ~value;
                } else if (value < 112) {
                    row->sample = value;
                    value = (signed char)stream[position]; position++;
                    i++;

                    if (value < 0) {
                        row->speed = ~value;
                    } else {
                        row->effect = value;
                        row->param  = stream[position]; position++;
                        i++;
                    }
                } else {
                    row->effect = value;
                    row->param  = stream[position]; position++;
                    i++;
                }
            } else {
                row->effect = value;
                row->param  = stream[position]; position++;
                i++;
            }

            patterns.push_back(row);
            base++;
            if ((pos + pointers[j]) == position) pointers[j++] = base;
        }

        pointers[j] = patterns.size();


        if ((position & 1) != 0) position++;
        amiga->store(stream, sampleData,position,_length);
        len = tracks.size();

        for (int i = 0; i < len; ++i) {
            BaseStep* step = tracks[i];
            step->pattern = pointers[step->pattern];
        }
        length++;
        m_version = 2;
        format = "Sidmon 2";
        //printData();
    }
    else
    {
        return -1;
    }
    return 1;

}
void S2Player::printData()
{
//    for(unsigned int i = 0; i < patterns.size(); i++)
//    {
//        SMRow* row = patterns[i];
//        std::cout << "Pattern [" << i << "] note: " << row->note << " sample: " << row->sample << " param: " << row->param << " effect: " << row->effect << " speed: " << row->speed << "\n";
//    }
//    //    for(unsigned int i = 0; i < samples.size(); i++)
//    //    {
//    //        S2Sample* sample = samples[i];
//    //        std::cout << "Sample [" << i << "] index: " << sample->index << " length: " << sample->length << " loop: " << sample->loop << " loopPtr: " << sample->loopPtr << " name: " << sample->name << " pitchBend: " << sample->pitchBend << " pointer: " << sample->pointer << " repeat: " << sample->repeat<< " synth: " << (int)sample->synth << "\n";
//    //        for(unsigned int j = 0; j < sample->table.size(); j++)
//    //        {
//    //            std::cout << "Sample [" << i << "] Table [" << j << "] " << (int)sample->table[j] << "\n";
//    //        }
//    //        for(unsigned int j = 0; j < sample->vibratos.size(); j++)
//    //        {
//    //            std::cout << "Sample [" << i << "] Vibratos [" << j << "] " << (int)sample->vibratos[j] << "\n";
//    //        }
//    //        for(unsigned int j = 0; j < sample->volumes.size(); j++)
//    //        {
//    //            std::cout << "Sample [" << i << "] Volumes [" << j << "] " << (int)sample->volumes[j] << "\n";
//    //        }
//    //    }
//    for(int i = 0; i < arpeggios.size(); i++)
//    {
//        std::cout << "Arpeggio [" << i << "] " << (int)arpeggios[i] <<  "\n";
//    }
//    for(int i = 0; i < vibratos.size(); i++)
//    {
//        std::cout << "Vibrato [" << i << "] " << (int)vibratos[i] <<  "\n";
//    }
//    for(int i = 0; i < waves.size(); i++)
//    {
//        std::cout << "Wave [" << i << "] " << (int)waves[i] <<  "\n";
//    }
//    for(unsigned int i = 0; i < tracks.size(); i++)
//    {
//        AmigaStep* step = tracks[i];
//        std::cout << "Tracks [" << i << "] pattern: " << step->pattern << " transpose: " << (int)step->transpose <<  "\n";
//    }
//    //    for(int i = 0; i < amiga->memory.size(); i++)
//    //    {
//    //        std::cout << "Memory [" << i << "]" << (int)amiga->memory[i] <<  "\n";
//    //    }
//    std::flush(std::cout);
}
std::vector<BaseSample*> S2Player::getSamples()
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
