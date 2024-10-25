#include "JBPlayer.h"
#include "JBVoice.h"
#include "JBSong.h"
#include "BaseSample.h"
#include "AmigaChannel.h"
#include <iostream>
#include "MyEndian.h"
using namespace std;

JBPlayer::JBPlayer(Amiga* amiga): AmigaPlayer(amiga)
{
    voices = std::vector<JBVoice*>(4);

    voices[0] = new JBVoice(0);
    voices[0]->next = voices[1] = new JBVoice(1);
    voices[1]->next = voices[2] = new JBVoice(2);
    voices[2]->next = voices[3] = new JBVoice(3);

    voices[1]->prev = voices[0];
    voices[2]->prev = voices[1];
    voices[3]->prev = voices[2];
}

JBPlayer::~JBPlayer()
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
}

void JBPlayer::initialize()
{
    AmigaPlayer::initialize();

    JBSong* song = songs[m_songNumber];
    speed = song->speed;
    tick = (m_variant < 3) ? 1 : 255;

    transpose = 0;
    waveDir = 0;
    wavePos = 0;
    waveLower = 0;
    waveUpper = 0;
    complete = 0;

    JBVoice* voice = voices[0];
    do
    {
        voice->initialize();
        voice->channel = amiga->channels[voice->index];
        complete += (1 << voice->index);

        if (m_variant < 3)
        {
            voice->channel->pointer = 0;
            voice->channel->length = 128;
            voice->channel->setVolume(0);
            voice->channel->setEnabled(1);
        }
        else
        {
            voice->trackLen = song->length[voice->index];
            voice->trackLoop = song->restart[voice->index];
        }
        voice->track = song->pointer[voice->index];
        position = voice->track;
        voice->patternPos = readEndian(stream[position], stream[position + 1]);
        position += 2;
    }
    while (voice = voice->next);
}

void JBPlayer::process()
{
    if (!oldProcess)
    {
        JBVoice* voice = voices[0];
        AmigaChannel* chan;
        BaseSample* sample;
        int temp = 0;
        int period = 0;
        int loop = 0;

        int value = (waveDir) ? waveLower : waveUpper;

        if (wavePos < value)
        {
            amiga->memory[4 + wavePos] = -128;
            wavePos++;
        }
        else if (wavePos > value)
        {
            amiga->memory[4 + wavePos] = 0;
            wavePos--;
        }
        else
        {
            waveDir = ~waveDir;
        }

        do
        {
            chan = voice->channel;
            sample = samples[voice->sample2];

            if (voice->state)
            {
                if (--voice->state)
                {
                    chan->pointer = sample->pointer;
                    chan->length = sample->length;
                }
                else
                {
                    chan->pointer = sample->loopPtr;
                    chan->length = sample->repeat;
                }
                chan->setEnabled(1);
            }

            chan->setVolume(((voice->volume >> 2) * voice->volumeMod) >> 8);

            if (voice->volCounter)
            {
                if (--voice->volCounter == 0)
                {
                    position = (1 + vblock) + voice->volPos;

                    voice->volCounter = stream[position];
                    position++;

                    if (voice->volCounter)
                    {
                        voice->volPos += 2;
                        position = vblock + voice->volPos;
                        voice->volume = (voice->volume + (signed char)stream[position]) & 0xff;
                        position++;
                    }
                }
                else
                {
                    position = vblock + voice->volPos;
                    voice->volume = (voice->volume + (signed char)stream[position]) & 0xff;
                    position++;
                }
            }
            if (m_variant > 4)
            {
                position = ptrack + voice->slidePointer;
                value = stream[position] + voice->slidePos;
                position = pblock + value;

                temp = (signed char)stream[position];
                position++;


                if (temp >= 0) voice->slidePos = 255;

                if (++temp != 0)
                {
                    temp--;
                    voice->slidePos = (voice->slidePos + 1) & 255;
                }

                if (m_variant == 5) temp = 0;
            }
            else
            {
                value = voice->slidePos + 1;
                position = pblock + value;
                temp = (signed char)stream[position];
                position++;

                if (temp < 0)
                {
                    if (++temp != 0)
                    {
                        temp += 127;
                        voice->slidePos = voice->slidePointer;
                    }
                }
                else
                {
                    voice->slidePos = value;
                }
            }


            temp += (voice->note + voice->periodMod + transpose);
            temp = (temp << 1) & 255;


            position = periods + temp;
            period = ((readEndian(stream[position], stream[position + 1]) * sample->relative) << 3) >> 16;
            position += 2;

            if (voice->flags & 64)
            {
                temp = voice->slideStep;
                if (temp == 0) temp = voice->flags & 1;

                if (voice->flags & 16)
                {
                    temp += voice->slideValue;
                    voice->slideValue = temp;
                    if (temp == voice->slideLimit) voice->flags ^= 16;
                }
                else
                {
                    voice->slideValue -= temp;
                    if (!voice->slideValue) voice->flags ^= 16;
                }

                period += (temp - (voice->slideLimit >> 1));
            }

            voice->flags ^= 1;

            if (voice->flags & 4)
            {
                if (voice->portaCounter)
                {
                    voice->portaCounter--;
                }
                else
                {
                    voice->portaPeriod += voice->portaStep;
                    period += voice->portaPeriod;
                }
            }
            chan->setPeriod(period);
        }
        while (voice = voice->next);

        if ((tick += speed) > 255)
        {
            tick &= 255;
            voice = voices[3];

            do
            {
                if (--voice->counter <= 0)
                {
                    loop = 1;
                    voice->flags &= 0x50;
                    position = voice->patternPos;


                    if (m_variant > 4)
                    {
                        do
                        {
                            value = stream[position];
                            position++;

                            if (value < command)
                            {
                                voice->delay = value;
                            }
                            else if (value < 0x60)
                            {
                                loop = fx(voice, (value - command));
                            }
                            else if (value < 0x80)
                            {
                                voice->volPointer = value - 0x60;
                            }
                            else if (value < 0xe0)
                            {
                                voice->note = value - 0x80;
                                if (voice->flags & 2) loop = 0;

                                voice->slidePos = 0;
                                voice->sample2 = voice->sample1;

                                if (!(voice->flags & 32))
                                {
                                    voice->state = 2;
                                    voice->channel->setEnabled(0);
                                }
                                break;
                            }
                            else
                            {
                                voice->sample1 = value - 0xe0;
                            }
                        }
                        while (loop);
                    }
                    else
                    {
                        do
                        {
                            value = stream[position];
                            position++;

                            if (value < 0x80)
                            {
                                voice->note = value;
                                if (voice->flags & 2) loop = 0;

                                voice->slidePos = voice->slidePointer;
                                voice->sample2 = voice->sample1;

                                if (!(voice->flags & 32))
                                {
                                    voice->state = 2;
                                    voice->channel->setEnabled(0);
                                }
                                break;
                            }
                            else if (value < command)
                            {
                                loop = fx(voice, (value - 0x80));
                            }
                            else if (value < 0xa0)
                            {
                                voice->slidePointer = stream[ptrack + (value + 0x60)];
                            }
                            else if (value < 0xc0)
                            {
                                voice->sample1 = value - 0xa0;
                            }
                            else if (value < 0xe0)
                            {
                                voice->volPointer = value - 0xc0;
                            }
                            else
                            {
                                voice->delay = value - 0xdf;
                            }
                        }
                        while (loop);
                    }
                    if (loop)
                    {
                        loop = 0;
                        temp = position;

                        position = vtrack + voice->volPointer;
                        voice->volPos = stream[position];
                        position++;
                        position = vblock + voice->volPos;
                        voice->volume = stream[position];
                        position++;
                        voice->volCounter = 1;

                        position = temp;
                    }
                    voice->counter = voice->delay;
                    voice->patternPos = position;
                }
                else if (voice->flags & 8)
                {
                    if (voice->flags & 128)
                    {
                        voice->note--;
                    }
                    else
                    {
                        voice->note++;
                    }
                }
            }
            while (voice = voice->prev);
        }
    }
    else
    {
        AmigaChannel* chan;
        int loop = 0;
        int period = 0;
        int value = 0;
        int temp = 0;
        JBVoice* voice = voices[0];

        if (--tick == 0)
        {
            tick = speed;

            do
            {
                if (--voice->counter == 0)
                {
                    loop = 1;

                    voice->flags &= 0x70;
                    position = voice->patternPos;

                    do
                    {
                        value = stream[position];
                        position++;

                        if (value < 0x80)
                        {
                            voice->note = value;

                            if (!(voice->flags & 256))
                            {
                                voice->volPos = 0;
                                voice->volume = 0;
                            }

                            voice->slidePos = voice->slidePointer;
                            break;
                        }
                        else if (value < 0x90)
                        {
                            switch (value)
                            {
                            case 0x80:
                                voice->volume = 0xc0;
                                loop = 0;
                                break;
                            case 0x81:
                                voice->flags = 0;
                                break;
                            case 0x82:
                                voice->portaStep = (signed char)stream[position];
                                position++;
                                voice->portaPeriod = 0;
                                voice->portaCounter = stream[position];
                                position++;
                                voice->flags |= 4;
                                break;
                            case 0x83:
                                voice->flags |= 136;
                                break;
                            case 0x84:
                                voice->flags |= 8;
                                break;
                            case 0x85:
                                value = voice->trackPos + 2;
                                position = voice->track + value;
                                temp = readEndian(stream[position], stream[position + 1]);
                                position += 2;

                                if (temp == 0)
                                {
                                    value = 0;
                                    position = voice->track;
                                    position = readEndian(stream[position], stream[position + 1]);
                                    position += 2;

                                    if (!complete) amiga->setComplete(1);
                                    complete &= ~(1 << voice->index);
                                }
                                else
                                {
                                    position = temp;
                                }

                                voice->trackPos = value;
                                break;
                            case 0x86:
                                voice->slideStep = stream[position];
                                position++;
                                voice->slideValue = stream[position];
                                position++;
                                temp = voice->slideValue << 1;
                                voice->slideLimit = temp & 0xff;

                                if (m_variant == 0)
                                {
                                    voice->flags = 0x40;
                                }
                                else if (m_variant == 1)
                                {
                                    voice->flags |= 64;
                                }
                                else
                                {
                                    voice->flags = (voice->slideLimit < temp) ? 0x50 : 0x40;
                                }
                                break;
                            case 0x87:
                                voice->trackPos = 0;
                                position = voice->track;
                                position = readEndian(stream[position], stream[position + 1]);
                                position += 2;
                                amiga->setComplete(1);
                                loop = 0;
                                break;
                            case 0x88:
                                voice->periodMod = stream[position];
                                position++;
                                break;
                            case 0x89:
                                position++;
                                break;
                            case 0x8a:
                                voice->flags |= 256;
                                break;
                            }
                        }
                        else if (value < 0xa0)
                        {
                            voice->slidePointer = stream[ptrack + (value + 0x60)];
                        }
                        else if (value < 0xb8)
                        {
                            voice->sample1 = value - 0xa0;
                        }
                        else if (value < 0xe0)
                        {
                            voice->volPointer = value - 0xb8;
                        }
                        else
                        {
                            voice->delay = value - 0xdf;
                        }
                    }
                    while (loop);

                    voice->counter = voice->delay;
                    voice->patternPos = position;
                }
                else if (voice->flags & 8)
                {
                    if (voice->flags & 128)
                    {
                        voice->note--;
                    }
                    else
                    {
                        voice->note++;
                    }
                }
            }
            while (voice = voice->next);

            voice = voices[0];
        }

        do
        {
            chan = voice->channel;

            if (voice->volume < 0xc0)
            {
                voice->volume -= 64;

                if (voice->volume < 0)
                {
                    position = vtrack + voice->volPointer;
                    value = stream[position] + voice->volPos;
                    position++;
                    position = vblock + value;
                    voice->volume = stream[position];
                    position++;
                    voice->volPos++;
                }
            }

            value = voice->slidePos + 1;
            position = pblock + value;
            temp = (signed char)stream[position];
            position++;

            if (temp < 0)
            {
                temp += 128;
                value = voice->slidePointer;
            }

            voice->slidePos = value;

            temp += (voice->note + voice->periodMod);
            temp = (temp << 1) & 255;

            position = periods + temp;
            period = readEndian(stream[position], stream[position + 1]);
            position += 2;

            if (voice->flags & 64)
            {
                if (m_variant == 1)
                {
                    value = voice->slideStep;
                    if (value == 0) value = voice->flags & 1;

                    if (voice->flags & 16)
                    {
                        value += voice->slideValue;
                        voice->slideValue = value;
                        if (voice->slideValue == voice->slideLimit) voice->flags ^= 16;
                    }
                    else
                    {
                        voice->slideValue -= value;
                        if (voice->slideValue == 0) voice->flags ^= 16;
                    }
                }
                else
                {
                    loop = 0;

                    if (m_variant > 0)
                    {
                        if (voice->flags & 16)
                        {
                            if (voice->flags & 1) loop = 1;
                        }
                        else
                        {
                            loop = 1;
                        }
                    }
                    else
                    {
                        loop = 1;
                    }

                    value = voice->slideValue;

                    if (loop)
                    {
                        if (voice->flags & 32)
                        {
                            value += voice->slideStep;

                            if (value >= voice->slideLimit)
                            {
                                value = voice->slideLimit;
                                voice->flags ^= 32;
                            }
                        }
                        else
                        {
                            value -= voice->slideStep;

                            if (value < 0)
                            {
                                value = 0;
                                voice->flags ^= 32;
                            }
                        }
                        voice->slideValue = value;
                    }
                }
                period += (value - (voice->slideLimit >> 1));
            }

            voice->flags ^= 1;

            if (voice->flags & 4)
            {
                if (voice->portaCounter)
                {
                    voice->portaCounter--;
                }
                else
                {
                    voice->portaPeriod += voice->portaStep;
                    period += voice->portaPeriod;
                }
            }

            chan->pointer = voice->sample1 << 7;
            chan->setPeriod(period);
            chan->setVolume(voice->volume & 63);
        }
        while (voice = voice->next);
    }
}

int JBPlayer::fx(JBVoice* voice, int value)
{
    switch (value)
    {
    case 0:
        voice->sample2 = 0;
        voice->state = 2;
        voice->channel->setEnabled(0);
        return 0;
    case 1:
        voice->flags = 0;
        break;
    case 2:
        voice->portaStep = (signed short)readEndian(stream[position], stream[position + 1]);
        position += 2;
        voice->portaPeriod = 0;
        voice->portaCounter = stream[position];
        position++;
        voice->flags |= 4;
        break;
    case 3:
        voice->flags |= 136;
        break;
    case 4:
        voice->flags |= 8;
        break;
    case 5:
        value = voice->trackPos + 2;

        if (value == voice->trackLen)
        {
            value = voice->trackLoop;
            if (!complete) amiga->setComplete(1);
            complete &= ~(1 << voice->index);
        }

        position = voice->track + value;
        position = readEndian(stream[position], stream[position + 1]);
        voice->trackPos = value;
        break;
    case 6:
        voice->slideStep = stream[position];
        position++;
        voice->slideValue = stream[position];
        position++;
        voice->slideLimit = (voice->slideValue << 1) & 255;
        voice->flags |= 64;
        break;
    case 7:
        voice->trackPos = 0;
        position = voice->track;
        position = readEndian(stream[position], stream[position + 1]);
        amiga->setComplete(1);
        return 0;
    case 8:
        voice->periodMod = stream[position];
        position++;
        break;
    case 9:
        position++;
        break;
    case 10:
        voice->flags |= 32;
        break;
    case 11:
        waveLower = stream[position];
        position++;
        waveUpper = stream[position];
        position++;
        value = 1 + stream[position];
        position++;
        samples[1]->length = value;
        samples[1]->repeat = value;
        voice->sample1 = 1;
        break;
    case 12:
        voice->volumeMod = stream[position];
        position++;
        break;
    case 13:
        voice->flags |= 2;
        break;
    case 14:
        voice->flags |= 34;
        break;
    case 15:
        position++;
        break;
    case 16:
        transpose = (signed char)stream[position];
        position++;
        break;
    case 17:
        voice->loopCounter = stream[position];
        position++;
        voice->loopPos = position;
        break;
    case 18:
        voice->loopCounter = 2;
        voice->loopPos = position;
        break;
    case 19:
        if (--voice->loopCounter != 0) position = voice->loopPos;
        break;
    case 20:
        if (m_variant == 7)
        {
            speed = stream[position];
            position++;
            break;
        }
    case 21:
        voice->slidePointer = stream[position];
        position++;
        break;
    }

    return 1;
}

void JBPlayer::printData()
{
    for (unsigned int i = 0; i < samples.size(); i++)
    {
        BaseSample* sample = samples[i];
        if (sample)
        {
            std::cout << "Sample [" << i << "] length: " << sample->length << " finetune: " << sample->finetune <<
                " relative: " << sample->relative << " loopPtr: " << sample->loopPtr << " name: " << sample->name <<
                " pointer: " << sample->pointer << " repeat: " << sample->repeat << " volume: " << (int)sample->volume
                << "\n";
        }
    }
    std::flush(std::cout);
}

int JBPlayer::load(void* _data, unsigned long int _length)
{
    unsigned long int length = _length;
    stream = static_cast<unsigned char*>(_data);
    unsigned int value = 0;
    position = 0;
    oldProcess = false;
    value = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);
    position += 4;
    if (value == 0x48e7f0f0)
    {
        return oldLoader(stream, _length);
    }


    position = 38;
    value = readEndian(stream[position], stream[position + 1]);
    position += 2;
    if (value == 0xa001)
    {
        m_variant = 3;
    }
    else
    {
        position = 50;

        value = readEndian(stream[position], stream[position + 1]);
        position += 2;
        if (value == 0xa001)
        {
            m_variant = 4;
        }
        else
        {
            position = 42;
            value = readEndian(stream[position], stream[position + 1]);
            position += 2;
            if (value != 0xa001) return -1;
            m_variant = 5;
        }
    }


    int wavesHi = 0;
    int wavesLo = 0xff000;
    int wavesLen = 0;
    int samplesLo = 0xff000;
    int i = 0;
    while (position < length - 4)
    {
        unsigned int valueTemp = 0;
        value = readEndian(stream[position], stream[position + 1]);
        position += 2;
        switch (value)
        {
        case 0x143c: //move.b [xx,d2]
            value = readEndian(stream[position], stream[position + 1]);
            position += 2;
            valueTemp = readEndian(stream[position], stream[position + 1]);
            position += 2;
            if (valueTemp == 0x7603) command = value; //moveq [#3,d3]
            break;
        case 0x43fa: //lea [xx,a1]
            value = position + readEndian(stream[position], stream[position + 1]);
            position += 2;
            valueTemp = readEndian(stream[position], stream[position + 1]);
            position += 2;
            if (valueTemp == 0x4a28) vblock = value; //tst.b [xx(a0)]
            break;
        case 0x1031: //move.b [xx(a1,d1.w),d0]
            if (m_variant == 5)
            {
                position -= 10;

                valueTemp = readEndian(stream[position], stream[position + 1]);
                position += 2;
                if (valueTemp != 0x1231)
                {
                    //move.b [xx(a1,d1.w),d1]
                    position += 8;
                    break;
                }

                position++;
                ptrack = vblock + (signed char)stream[position];
                position++;
                position += 7;
            }
            else
            {
                position -= 4;

                valueTemp = readEndian(stream[position], stream[position + 1]);
                position += 2;
                if (valueTemp != 0x5201)
                {
                    //addq.b [#1,d1]
                    position += 2;
                    break;
                }
                position += 3;
            }

            pblock = vblock + (signed char)stream[position];
            position++;
            break;
        case 0x323b: //move.w [xx(pc,d0.w),d1]
            value = position + readEndian(stream[position], stream[position + 1]);
            position += 2;
            valueTemp = readEndian(stream[position], stream[position + 1]);
            position += 2;
            if (valueTemp == 0xc2c2) periods = value; //mulu.w [d2,d1]
            break;
        case 0x45fa: //lea [xx,a2]
            value = position + readEndian(stream[position], stream[position + 1]);
            position += 2;
            i = readEndian(stream[position], stream[position + 1]);
            position += 2;

            if (i == 0x1172)
            {
                //move.b [(a2,d0.w),xx(a0)]
                ptrack = value;
            }
            else if (i == 0x1032)
            {
                //move.b [(a2,d0.w),d0]
                vtrack = value;
            }
            break;
        case 0xc2fc: //mulu.w [#10,d1]
            position += 2;

            valueTemp = readEndian(stream[position], stream[position + 1]);
            position += 2;
            if (valueTemp == 0x45fa)
            {
                //lea [xx,a2]
                samples = vector<BaseSample*>();
                int pos = position;
                position = pos + readEndian(stream[pos], stream[pos + 1]);
                i = 0;

                while (position < samplesLo)
                {
                    BaseSample* sample = new BaseSample();
                    sample->relative = readEndian(stream[position], stream[position + 1]);
                    position += 2;
                    sample->pointer = value = readEndian(stream[position], stream[position + 1], stream[position + 2],
                                                         stream[position + 3]);
                    position += 4;
                    sample->length = readEndian(stream[position], stream[position + 1]);
                    position += 2;
                    sample->repeat = (signed short)readEndian(stream[position], stream[position + 1]);
                    position += 2;
                    if (i)
                    {
                        if (value < position)
                        {
                            if (value < wavesLo) wavesLo = value;
                            if (value > wavesHi)
                            {
                                wavesHi = value;
                                wavesLen = sample->length;
                            }
                        }
                        else if (value < samplesLo)
                        {
                            samplesLo = value;
                        }
                    }
                    samples.push_back(sample);
                    i++;
                }

                amiga->loopLen = 0;
                amiga->memory.resize(4);

                wavesLen = (wavesHi + wavesLen) - wavesLo;
                position = wavesLo;
                amiga->store(stream, wavesLen, position, length);

                position = samplesLo;
                amiga->store(stream, (length - samplesLo), position, length);

                //samples.fixed = true;
                length = wavesLo;
                position = pos;
            }
            break;
        case 0x51c9: //dbf [d1,xx]
            position += 2;

            valueTemp = readEndian(stream[position], stream[position + 1]);
            position += 2;
            if (valueTemp == 0x43fa)
            {
                //lea [xx,a1]
                songs = vector<JBSong*>();
                int posTemp = position;
                position += readEndian(stream[posTemp], stream[posTemp + 1]);

                while (true)
                {
                    JBSong* song = new JBSong();
                    song->speed = stream[position];
                    position++;
                    int i = 0;

                    do
                    {
                        song->length[i] = stream[position];
                        position++;
                        song->restart[i] = stream[position];
                        position++;

                        value = readEndian(stream[position], stream[position + 1]);
                        position += 2;
                        if (value > position) break;
                        song->pointer[i++] = value;
                    }
                    while (i < 4);

                    if (i == 4)
                    {
                        songs.push_back(song);
                        int valueTemp = stream[position];
                        position++;
                        if (valueTemp != 0) break;
                    }
                    else
                    {
                        break;
                    }
                }

                if (songs.size() < 1) return -1;
                m_totalSongs = songs.size();
                //songs.fixed = true;
                position = length;
            }
            break;
        }
    }

    if (ptrack == 0 || pblock == 0 ||
        vtrack == 0 || vblock == 0 || periods == 0)
        return -1;

    BaseSample* sample = samples[0];
    sample->pointer = sample->loopPtr = 0;
    sample->length = sample->repeat = 4;

    int pos = 0;
    int len = samples.size();
    wavesLen += 4;
    wavesLo -= 4;

    for (int i = 1; i < len; ++i)
    {
        sample = samples[i];

        if (pos >= wavesLen)
        {
            sample->pointer -= (samplesLo - wavesLen);
        }
        else
        {
            sample->pointer -= wavesLo;
        }

        if (sample->repeat < 0)
        {
            sample->loopPtr = 0;
            sample->repeat = 4;
        }
        else
        {
            sample->loopPtr = sample->pointer + sample->repeat;
            sample->repeat = sample->length - sample->repeat;
        }

        pos = sample->pointer + sample->length;
    }

    int tempVal;
    if (m_variant == 5)
    {
        position = 0x290;
        tempVal = readEndian(stream[position], stream[position + 1]);
        position += 2;
        if (tempVal == 0xd028) m_variant++; //add.b [xx(a0),d0]
        position = 0x4f6;
        tempVal = readEndian(stream[position], stream[position + 1]);
        position += 2;
        if (tempVal == 0x1759) m_variant++; //move.b [(a1)+,xx(a3)]
    }


    m_version = 2;
    format = "Jason Brooke";
    //printData();
    return 1;
}

void JBPlayer::selectSong(unsigned char subsong)
{
    m_songNumber = subsong;
}

unsigned char JBPlayer::getSubsongsCount()
{
    return songs.size();
}

int JBPlayer::oldLoader(void* data, unsigned long int _length)
{
    samples.clear();
    int value;
    int valueTemp;
    int pos;
    int waves = 0;
    int upper = 0;
    int lower = 0xffff;
    while (position < _length - 4)
    {
        value = readEndian(stream[position], stream[position + 1]);
        position += 2;

        switch (value)
        {
        case 0x43fa: //lea [xx,a1]
            value = position + readEndian(stream[position], stream[position + 1]);
            position += 2;
            valueTemp = readEndian(stream[position], stream[position + 1]);
            position += 2;
            if (valueTemp == 0x7603) pblock = value; //moveq [#3,d3]
            break;
        case 0x45fa: //lea [xx,a2]
            value = position + readEndian(stream[position], stream[position + 1]);
            position += 2;
            pos = readEndian(stream[position], stream[position + 1]);
            position += 2;

            if (pos == 0xd4c0)
            {
                //adda.w [d0,a2]
                waves = value;
            }
            else if (pos == 0x103b)
            {
                //move.b [xx(pc,d0.w),d0]
                vblock = value;
                position -= 2;
            }
            break;
        case 0x117b: //move.b [xx(pc,d0.w),xx(a0)]
            value = (position + readEndian(stream[position], stream[position + 1])) - 256;;
            position += 2;
            valueTemp = readEndian(stream[position], stream[position + 1]);
            position += 2;
            if (valueTemp == 0x0016) ptrack = value; //22(a0)
            break;
        case 0x103b: //move.b [xx(pc,d0.w),d0]
            value = position + readEndian(stream[position], stream[position + 1]);
            position += 2;
            pos = readEndian(stream[position], stream[position + 1]);
            position += 2;

            if (pos == 0xd028)
            {
                vtrack = value;
                position += 6;

                valueTemp = readEndian(stream[position], stream[position + 1]);
                position += 2;
                if (valueTemp == 0x1171)
                {
                    //move.b [xx(a1,d0.w),xx(a0)]
                    vblock = pblock + readEndian(stream[position], stream[position + 1]);
                    position += 2;
                }
            }
            else if (pos == 0xd4c0)
            {
                vtrack = value;
            }
            break;
        case 0x323b: //move.b [xx(pc,d0.w),d1]
            value = position + readEndian(stream[position], stream[position + 1]);
            position += 2;
            valueTemp = readEndian(stream[position], stream[position + 1]);
            position += 2;
            if (valueTemp == 0x0810) periods = value; //btst [#6,(a0)]
            break;
        case 0x137b: //move.b [xx(pc,d0.w),xx(a1)]
            value = position + readEndian(stream[position], stream[position + 1]);
            position += 2;
            position += 2;

            valueTemp = readEndian(stream[position], stream[position + 1]);
            position += 2;
            if (valueTemp == 0x41fa)
            {
                //lea [xx,a0]
                pos = position + readEndian(stream[position], stream[position + 1]);
                position += 2;
                songs = vector<JBSong*>();
                position = value;

                while (position < pos)
                {
                    JBSong* song = new JBSong();
                    song->speed = stream[position];
                    position++;

                    for (int i = 0; i < 4; ++i)
                    {
                        value = readEndian(stream[position], stream[position + 1]);
                        position += 2;

                        if (value)
                        {
                            if (value < lower) lower = value;
                            if (value > upper) upper = value;
                        }
                        song->pointer[i] = value;
                    }

                    songs.push_back(song);
                    signed char byte = stream[position];
                    position++;
                    if (byte != 0) break;
                }

                if (songs.size() < 1) return -1;
                m_totalSongs = songs.size();
                //songs.fixed = true;
                position = _length;
            }
            break;
        }
    }

    if (ptrack == 0 || pblock == 0 ||
        vtrack == 0 || vblock == 0 || periods == 0)
        return -1;

    position = lower;
    lower = 0xffff;

    while (position < upper)
    {
        value = readEndian(stream[position], stream[position + 1]);
        position += 2;
        if (value && (value < lower)) lower = value;
    }

    position = waves;
    amiga->store(stream, (lower - waves), position, _length);
    amiga->loopLen = 0;

    m_variant = 2;
    position = 0xd6;

    valueTemp = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);;
    position += 4;
    if (valueTemp == 0x10bc0040)
    {
        //move.b [#$40,(a0)]
        m_variant = 0;
    }
    else
    {
        position = 0xf4;
        valueTemp = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);;
        position += 4;
        if (valueTemp == 0x08d00005) m_variant = 1; //bset [#5,(a0)]
    }

    this->stream = stream;
    m_version = 1;
    oldProcess = true;
    format = "Jason Brooke";
    //printData();
    return 1;
}

std::vector<BaseSample*> JBPlayer::getSamples()
{
    std::vector<BaseSample*> samp(samples.size());
    for (int i = 0; i < samples.size(); i++)
    {
        samp[i] = samples[i];
        if (!samp[i])
        {
            samp[i] = new BaseSample();
        }
    }
    //std::cout << "returning samples, size: " << samp.size() << "\n";
    return samp;
}
