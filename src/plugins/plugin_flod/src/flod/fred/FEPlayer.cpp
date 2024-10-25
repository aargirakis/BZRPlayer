#include "FEPlayer.h"
#include "FEVoice.h"
#include "FESample.h"
#include "FESong.h"
#include "AmigaChannel.h"
#include <iostream>
#include "MyEndian.h"

const int FEPlayer::PERIODS[72] = {
    8192, 7728, 7296, 6888, 6504, 6136, 5792, 5464, 5160,
    4872, 4600, 4336, 4096, 3864, 3648, 3444, 3252, 3068,
    2896, 2732, 2580, 2436, 2300, 2168, 2048, 1932, 1824,
    1722, 1626, 1534, 1448, 1366, 1290, 1218, 1150, 1084,
    1024, 966, 912, 861, 813, 767, 724, 683, 645,
    609, 575, 542, 512, 483, 456, 430, 406, 383,
    362, 341, 322, 304, 287, 271, 256, 241, 228,
    215, 203, 191, 181, 170, 161, 152, 143, 135
};

FEPlayer::FEPlayer(Amiga* amiga): AmigaPlayer(amiga)
{
    voices = vector<FEVoice*>(4);
    voices[3] = new FEVoice(3);
    voices[3]->next = voices[2] = new FEVoice(2);
    voices[2]->next = voices[1] = new FEVoice(1);
    voices[1]->next = voices[0] = new FEVoice(0);
}

FEPlayer::~FEPlayer()
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
    for (unsigned int i = 0; i < songs.size(); i++)
    {
        if (songs[i]) delete songs[i];
    }
    songs.clear();
    if (patterns)
    {
        delete[] patterns;
    }
}

void FEPlayer::initialize()
{
    AmigaPlayer::initialize();
    FEVoice* voice = voices[3];
    int i = 0;
    int len = 0;
    song = songs[m_songNumber];
    speed = song->speed;

    complete = 0;

    do
    {
        voice->initialize();
        voice->channel = amiga->channels[voice->index];
        voice->patternPos = song->tracks[voice->index][0];

        complete += (1 << voice->index);
        i = voice->synth;
        len = i + 64;
        for (; i < len; ++i)
        {
            amiga->memory[i] = 0;
        }
    }
    while (voice = voice->next);
}

void FEPlayer::process()
{
    FEVoice* voice = voices[3];
    AmigaChannel* chan;
    FESample* sample;
    int value = 0;
    int j = 0;
    int i = 0;
    int pos = 0;
    int len = 0;

    do
    {
        chan = voice->channel;
        int loop = 0;

        do
        {
            position = voice->patternPos;

            sample = voice->sample;
            sampleFlag = 0;

            if (!voice->busy)
            {
                voice->busy = 1;
                if (sample->loopPtr == 0)
                {
                    chan->pointer = amiga->loopPtr;
                    chan->length = amiga->loopLen;
                }
                else if (sample->loopPtr > 0)
                {
                    chan->pointer = (sample->type) ? voice->synth : (sample->pointer + sample->loopPtr);
                    chan->length = sample->length - sample->loopPtr;
                }
            }

            if (--voice->tick == 0)
            {
                loop = 2;

                while (loop > 1)
                {
                    value = patterns[position];
                    position++;
                    if (value < 0)
                    {
                        switch (value)
                        {
                        case -125:
                            sample = voice->sample = samples[patterns[position]];
                            position++;
                            sampleFlag = 1;
                            voice->patternPos = position;

                            break;
                        case -126:
                            speed = patterns[position];
                            position++;
                            voice->patternPos = position;
                            break;
                        case -127:
                            value = (sample) ? sample->relative : 428;
                            voice->portaSpeed = patterns[position] * speed;
                            position++;
                            voice->portaNote = patterns[position];
                            position++;
                            voice->portaLimit = (PERIODS[voice->portaNote] * value) >> 10;
                            voice->portamento = 0;
                            voice->portaDelay = patterns[position] * speed;
                            position++;
                            voice->portaFlag = 1;
                            voice->patternPos = position;
                            break;
                        case -124:
                            chan->setEnabled(0);
                            voice->tick = speed;
                            voice->busy = 1;
                            voice->patternPos = position;
                            loop = 0;
                            break;
                        case -128:
                            voice->trackPos++;

                            while (true)
                            {
                                value = song->tracks[voice->index][voice->trackPos];

                                if (value == 65535)
                                {
                                    amiga->setComplete(1);
                                }
                                else if (value > 32767)
                                {
                                    voice->trackPos = (value ^ 32768) >> 1;

                                    if (!loopSong)
                                    {
                                        complete &= ~(1 << voice->index);
                                        if (!complete) amiga->setComplete(1);
                                    }
                                }
                                else
                                {
                                    voice->patternPos = value;
                                    voice->tick = 1;
                                    loop = 1;
                                    break;
                                }
                            }
                            break;
                        default:
                            voice->tick = speed * -value;
                            voice->patternPos = position;
                            loop = 0;
                            break;
                        }
                    }
                    else
                    {
                        loop = 0;
                        voice->patternPos = position;

                        voice->note = value;
                        voice->arpeggioPos = 0;
                        voice->vibratoFlag = -1;
                        voice->vibrato = 0;

                        voice->arpeggioSpeed = sample->arpeggioSpeed;
                        voice->vibratoDelay = sample->vibratoDelay;
                        voice->vibratoSpeed = sample->vibratoSpeed;
                        voice->vibratoDepth = sample->vibratoDepth;

                        i = 0;
                        j = 0;
                        len = 0;
                        if (sample->type == 1)
                        {
                            if (sampleFlag || (sample->synchro & 2))
                            {
                                voice->pulseCounter = sample->pulseCounter;
                                voice->pulseDelay = sample->pulseDelay;
                                voice->pulseDir = 0;
                                voice->pulsePos = sample->pulsePosL;
                                voice->pulseSpeed = sample->pulseSpeed;
                                i = voice->synth;
                                len = i + sample->pulsePosL;
                                for (; i < len; ++i)
                                {
                                    amiga->memory[i] = sample->pulseRateNeg;
                                }
                                len += (sample->length - sample->pulsePosL);
                                for (; i < len; ++i)
                                {
                                    amiga->memory[i] = sample->pulseRatePos;
                                }
                            }

                            chan->pointer = voice->synth;
                        }
                        else if (sample->type == 2)
                        {
                            voice->blendCounter = sample->blendCounter;
                            voice->blendDelay = sample->blendDelay;
                            voice->blendDir = 0;
                            voice->blendPos = 1;

                            i = sample->pointer;
                            j = voice->synth;
                            len = i + 31;
                            for (; i < len; ++i)
                            {
                                amiga->memory[j++] = amiga->memory[i];
                            }

                            chan->pointer = voice->synth;
                        }
                        else
                        {
                            chan->pointer = sample->pointer;
                        }

                        voice->tick = speed;
                        voice->busy = 0;
                        voice->period = (PERIODS[voice->note] * sample->relative) >> 10;

                        voice->volume = 0;
                        voice->envelopePos = 0;
                        voice->sustainTime = sample->sustainTime;

                        chan->length = sample->length;
                        chan->setPeriod(voice->period);

                        chan->setVolume(0);
                        chan->setEnabled(1);

                        if (voice->portaFlag)
                        {
                            if (!voice->portamento)
                            {
                                voice->portamento = voice->period;
                                voice->portaCounter = 1;
                                voice->portaPeriod = voice->portaLimit - voice->period;
                            }
                        }
                    }
                }
            }
            else if (voice->tick == 1)
            {
                value = (patterns[voice->patternPos] - 160) & 255;

                if (value > 127)
                {
                    chan->setEnabled(0);
                }
            }
        }
        while (loop > 0);


        if (!chan->enabled()) continue;


        value = voice->note + sample->arpeggio[voice->arpeggioPos];


        if (--voice->arpeggioSpeed == 0)
        {
            voice->arpeggioSpeed = sample->arpeggioSpeed;

            if (++voice->arpeggioPos == sample->arpeggioLimit)
            {
                voice->arpeggioPos = 0;
            }
        }

        voice->period = (PERIODS[value] * sample->relative) >> 10;


        if (voice->portaFlag)
        {
            if (voice->portaDelay)
            {
                voice->portaDelay--;
            }
            else
            {
                voice->period += ((voice->portaCounter * voice->portaPeriod) / voice->portaSpeed);

                if (++voice->portaCounter > voice->portaSpeed)
                {
                    voice->note = voice->portaNote;
                    voice->portaFlag = 0;
                }
            }
        }

        if (voice->vibratoDelay)
        {
            voice->vibratoDelay--;
        }
        else
        {
            if (voice->vibratoFlag)
            {
                if (voice->vibratoFlag < 0)
                {
                    voice->vibrato += voice->vibratoSpeed;

                    if (voice->vibrato == voice->vibratoDepth)
                    {
                        voice->vibratoFlag ^= 0x80000000;
                    }
                }
                else
                {
                    voice->vibrato -= voice->vibratoSpeed;

                    if (voice->vibrato == 0)
                    {
                        voice->vibratoFlag ^= 0x80000000;
                    }
                }

                if (!voice->vibrato) voice->vibratoFlag ^= 1;

                if (voice->vibratoFlag & 1)
                {
                    voice->period += voice->vibrato;
                }
                else
                {
                    voice->period -= voice->vibrato;
                }
            }
        }

        chan->setPeriod(voice->period);

        switch (voice->envelopePos)
        {
        case 4:
            break;
        case 0:
            voice->volume += sample->attackSpeed;

            if (voice->volume >= sample->attackVol)
            {
                voice->volume = sample->attackVol;
                voice->envelopePos = 1;
            }
            break;
        case 1:
            voice->volume -= sample->decaySpeed;

            if (voice->volume <= sample->decayVol)
            {
                voice->volume = sample->decayVol;
                voice->envelopePos = 2;
            }
            break;
        case 2:
            if (voice->sustainTime)
            {
                voice->sustainTime--;
            }
            else
            {
                voice->envelopePos = 3;
            }
            break;
        case 3:
            voice->volume -= sample->releaseSpeed;

            if (voice->volume <= sample->releaseVol)
            {
                voice->volume = sample->releaseVol;
                voice->envelopePos = 4;
            }
            break;
        }

        value = sample->envelopeVol << 12;
        value >>= 8;
        value >>= 4;
        value *= voice->volume;
        value >>= 8;
        value >>= 1;
        chan->setVolume(value);

        if (sample->type == 1)
        {
            if (voice->pulseDelay)
            {
                voice->pulseDelay--;
            }
            else
            {
                if (voice->pulseSpeed)
                {
                    voice->pulseSpeed--;
                }
                else
                {
                    if (voice->pulseCounter || !(sample->synchro & 1))
                    {
                        voice->pulseSpeed = sample->pulseSpeed;

                        if (voice->pulseDir & 4)
                        {
                            while (true)
                            {
                                if (voice->pulsePos >= sample->pulsePosL)
                                {
                                    loop = 1;
                                    break;
                                }

                                voice->pulseDir &= -5;
                                voice->pulsePos++;
                                voice->pulseCounter--;

                                if (voice->pulsePos <= sample->pulsePosH)
                                {
                                    loop = 2;
                                    break;
                                }

                                voice->pulseDir |= 4;
                                voice->pulsePos--;
                                voice->pulseCounter--;
                            }
                        }
                        else
                        {
                            while (true)
                            {
                                if (voice->pulsePos <= sample->pulsePosH)
                                {
                                    loop = 2;
                                    break;
                                }

                                voice->pulseDir |= 4;
                                voice->pulsePos--;
                                voice->pulseCounter--;

                                if (voice->pulsePos >= sample->pulsePosL)
                                {
                                    loop = 1;
                                    break;
                                }

                                voice->pulseDir &= -5;
                                voice->pulsePos++;
                                voice->pulseCounter++;
                            }
                        }

                        pos = voice->synth + voice->pulsePos;

                        if (loop == 1)
                        {
                            amiga->memory[pos] = sample->pulseRatePos;
                            voice->pulsePos--;
                        }
                        else
                        {
                            amiga->memory[pos] = sample->pulseRateNeg;
                            voice->pulsePos++;
                        }
                    }
                }
            }
        }
        else if (sample->type == 2)
        {
            if (voice->blendDelay)
            {
                voice->blendDelay--;
            }
            else
            {
                if (voice->blendCounter || !(sample->synchro & 4))
                {
                    if (voice->blendDir)
                    {
                        if (voice->blendPos != 1)
                        {
                            voice->blendPos--;
                        }
                        else
                        {
                            voice->blendDir ^= 1;
                            voice->blendCounter--;
                        }
                    }
                    else
                    {
                        if (voice->blendPos != (sample->blendRate << 1))
                        {
                            voice->blendPos++;
                        }
                        else
                        {
                            voice->blendDir ^= 1;
                            voice->blendCounter--;
                        }
                    }

                    i = sample->pointer;
                    j = voice->synth;
                    len = i + 31;
                    pos = len + 1;

                    for (; i < len; ++i)
                    {
                        value = (voice->blendPos * amiga->memory[pos++]) >> sample->blendRate;
                        amiga->memory[pos++] = value + amiga->memory[i];
                    }
                }
            }
        }
    }
    while (voice = voice->next);
}

int FEPlayer::load(void* _data, unsigned long int _length)
{
    m_version = 0;
    unsigned char* stream = static_cast<unsigned char*>(_data);
    int value = 0;
    position = 0;
    int data = 0;
    int base = 0;
    unsigned int pos = 0x7fffffff;

    while (position < 16)
    {
        value = readEndian(stream[position], stream[position + 1]);
        position += 2;
        position += 2;
        if (value != 0x4efa) return 0; //jmp
    }

    while (position < 1024)
    {
        value = readEndian(stream[position], stream[position + 1]);
        position += 2;

        if (value == 0x123a)
        {
            //move.b $x,d1
            position += 2;
            value = readEndian(stream[position], stream[position + 1]);
            position += 2;

            if (value == 0xb001)
            {
                //cmp.b d1,d0
                position -= 4;
                data = (position + readEndian(stream[position], stream[position + 1])) - 0x895;
                position += 2;
            }
        }
        else if (value == 0x214a)
        {
            //move.l a2,(a0)
            position += 2;
            value = readEndian(stream[position], stream[position + 1]);
            position += 2;

            if (value == 0x47fa)
            {
                //lea $x,a3
                base = position + (signed short)readEndian(stream[position], stream[position + 1]);
                position += 2;
                m_version = 1;
                break;
            }
        }
    }
    if (!m_version) return 0;

    position = data + 0x8a2;

    position = base + readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);

    samples = vector<FESample*>();

    while (pos > position)
    {
        value = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);
        position += 4;

        if (value)
        {
            if ((value < position) || (value >= _length))
            {
                position -= 4;
                break;
            }

            if (value < pos) pos = base + value;
        }

        FESample* sample = new FESample();
        sample->pointer = value;
        sample->loopPtr = (signed short)readEndian(stream[position], stream[position + 1]);
        position += 2;
        sample->length = readEndian(stream[position], stream[position + 1]) << 1;
        position += 2;
        sample->relative = readEndian(stream[position], stream[position + 1]);
        position += 2;

        sample->vibratoDelay = stream[position];
        position++;
        position++;
        sample->vibratoSpeed = stream[position];
        position++;
        sample->vibratoDepth = stream[position];
        position++;
        sample->envelopeVol = stream[position];
        position++;
        sample->attackSpeed = stream[position];
        position++;
        sample->attackVol = stream[position];
        position++;
        sample->decaySpeed = stream[position];
        position++;
        sample->decayVol = stream[position];
        position++;
        sample->sustainTime = stream[position];
        position++;
        sample->releaseSpeed = stream[position];
        position++;
        sample->releaseVol = stream[position];
        position++;

        sample->arpeggio = vector<signed char>(16);
        for (int i = 0; i < 16; ++i)
        {
            sample->arpeggio[i] = stream[position];
            position++;
        }

        sample->arpeggioSpeed = stream[position];
        position++;
        sample->type = stream[position];
        position++;
        sample->pulseRateNeg = stream[position];
        position++;
        sample->pulseRatePos = stream[position];
        position++;
        sample->pulseSpeed = stream[position];
        position++;
        sample->pulsePosL = stream[position];
        position++;
        sample->pulsePosH = stream[position];
        position++;
        sample->pulseDelay = stream[position];
        position++;
        sample->synchro = stream[position];
        position++;
        sample->blendRate = stream[position];
        position++;
        sample->blendDelay = stream[position];
        position++;
        sample->pulseCounter = stream[position];
        position++;
        sample->blendCounter = stream[position];
        position++;
        sample->arpeggioLimit = stream[position];
        position++;

        position += 12;
        samples.push_back(sample);
        if (position > _length) break;
    }
    if (pos != 0x7fffffff)
    {
        amiga->store(stream, _length - pos, position, _length);
        int len = samples.size();


        for (int i = 0; i < len; ++i)
        {
            FESample* sample = samples[i];
            if (sample->pointer)
            {
                sample->pointer -= (base + pos);
            }
        }
    }

    pos = amiga->memory.size();
    amiga->memory.resize(amiga->memory.size() + 256);
    amiga->loopLen = 100;

    for (int i = 0; i < 4; ++i)
    {
        voices[i]->synth = pos;
        pos += 64;
    }


    position = data + 0x8a2;
    unsigned int len = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);
    position += 4;
    pos = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);
    position += 4;

    position = base + pos;
    patterns = new signed char[len - pos];

    for (int i = 0; i < len - pos; i++)
    {
        patterns[i] = stream[position];
        position++;
    }

    pos += base;

    position = data + 0x895;
    len = stream[position] + 1;
    position++;
    m_totalSongs = len;

    songs = vector<FESong*>(len);
    base = data + 0xb0e;
    unsigned int tracksLen = pos - base;
    pos = 0;

    int size = 0;
    for (int i = 0; i < len; ++i)
    {
        FESong* song = new FESong();

        for (int j = 0; j < 4; ++j)
        {
            position = base + pos;
            value = readEndian(stream[position], stream[position + 1]);
            position += 2;

            if (j == 3 && (i == (len - 1)))
            {
                size = tracksLen;
            }
            else
            {
                size = readEndian(stream[position], stream[position + 1]);
                position += 2;
            }

            size = (size - value) >> 1;
            if (size > song->length) song->length = size;

            song->tracks[j] = vector<int>(size);
            position = base + value;

            for (int ptr = 0; ptr < size; ++ptr)
            {
                song->tracks[j][ptr] = readEndian(stream[position], stream[position + 1]);
                position += 2;
            }

            pos += 2;
        }

        position = data + i + 0x897;
        song->speed = stream[position];
        position++;
        songs[i] = song;
    }

    m_version = 1;
    format = "Fred Editor";
    //printData();
    return 1;
}

void FEPlayer::printData()
{
    for (unsigned int i = 0; i < samples.size(); i++)
    {
        FESample* sample = samples[i];
        if (sample)
        {
            //cout << "Sample [" << i << "] length: " << sample->length << " loop: " << sample->loop << " loopPtr: " << sample->loopPtr << " name: " << sample->name << " pointer: " << sample->pointer << " repeat: " << sample->repeat << " volume: " << sample->volume << "\n";
            //cout << "Sample [" << i << "] relative: " << sample->relative << " type: " << (int)sample->type << " synchro: " << sample->synchro << " envelopeVol: " << sample->envelopeVol << " attackSpeed: " << sample->attackSpeed << "\n";
            //cout << "Sample [" << i << "] attackVol: " << sample->attackVol << " decaySpeed: " << sample->decaySpeed << " decayVol: " << sample->decayVol << " sustainTime: " << sample->sustainTime << " releaseSpeed: " << sample->releaseSpeed << "\n";
            //cout << "Sample [" << i << "] releaseVol: " << sample->releaseVol << " arpeggioLimit: " << sample->arpeggioLimit << " arpeggioSpeed: " << sample->arpeggioSpeed << " vibratoDelay: " << sample->vibratoDelay << " vibratoDepth: " << sample->vibratoDepth << "\n";
            //cout << "Sample [" << i << "] vibratoSpeed: " << sample->vibratoSpeed << " pulseCounter: " << sample->pulseCounter << " pulseDelay: " << sample->pulseDelay << " pulsePosL: " << sample->pulsePosL << " pulsePosH: " << sample->pulsePosH << "\n";
            //cout << "Sample [" << i << "] pulseSpeed: " << sample->pulseSpeed << " pulseRateNeg: " << (int)sample->pulseRateNeg << " pulseRatePos: " << sample->pulseRatePos << " blendCounter: " << sample->blendCounter << " blendDelay: " << sample->blendDelay << " blendRate: " << sample->blendRate << "\n";
            for (unsigned int j = 0; j < sample->arpeggio.size(); j++)
            {
                //cout << "Sample [" << i << "] arpeggio [" << j << "] " << (int)sample->arpeggio[j] << "\n";
            }
        }
    }


    for (unsigned int i = 0; i < songs.size(); i++)
    {
        FESong* song = songs[i];
        if (song)
        {
            //cout << "Song [" << i << "] speed: " << song->speed << " length: " << song->length << "\n";

            for (unsigned int j = 0; j < song->tracks.size(); j++)
            {
                for (unsigned int k = 0; k < song->tracks[j].size(); k++)
                {
                    //cout << "Song [" << i << "] track [" << j << "][ " << k << "] " << (int)song->tracks[j][k] << "\n";
                }
            }
        }
    }
    flush(cout);

    //     //cout << "Periods: " << periods << "\n";
    //     //cout << "Vibrato: " << vibrato << "\n";
    //     //cout << "Variant: " << variant << "\n";
}

unsigned char FEPlayer::getSubsongsCount()
{
    return songs.size();
}

void FEPlayer::selectSong(unsigned char subsong)
{
    m_songNumber = subsong;
}

vector<BaseSample*> FEPlayer::getSamples()
{
    vector<BaseSample*> samp(samples.size());
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
