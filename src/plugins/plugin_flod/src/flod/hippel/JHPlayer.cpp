#include "JHPlayer.h"
#include "JHVoice.h"
#include "JHSong.h"
#include "AmigaChannel.h"
#include "BaseSample.h"
#include <iostream>
#include "MyEndian.h"

const int JHPlayer::PERIODS[84] = {
    1712, 1616, 1524, 1440, 1356, 1280, 1208, 1140, 1076, 1016,
    960, 906, 856, 808, 762, 720, 678, 640, 604, 570,
    538, 508, 480, 453, 428, 404, 381, 360, 339, 320,
    302, 285, 269, 254, 240, 226, 214, 202, 190, 180,
    170, 160, 151, 143, 135, 127, 120, 113, 113, 113,
    113, 113, 113, 113, 113, 113, 113, 113, 113, 113,
    3424, 3232, 3048, 2880, 2712, 2560, 2416, 2280, 2152, 2032,
    1920, 1812, 6848, 6464, 6096, 5760, 5424, 5120, 4832, 4560,
    4304, 4064, 3840, 3624
};


JHPlayer::JHPlayer(Amiga* amiga): AmigaPlayer(amiga)
{
    voices = std::vector<JHVoice*>(4);
    voices[0] = new JHVoice(0);
    voices[0]->next = voices[1] = new JHVoice(1);
    voices[1]->next = voices[2] = new JHVoice(2);
    voices[2]->next = voices[3] = new JHVoice(3);
    base = 0;
    patterns = 0;
    patternLen = 0;
    periods = 0;
    freqs = 0;
    vols = 0;
    sampleData = 0;
    coso = 0;
    m_variant = 0;
}

JHPlayer::~JHPlayer()
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

void JHPlayer::initialize()
{
    AmigaPlayer::initialize();
    JHVoice* voice = voices[0];
    song = songs[m_songNumber];
    speed = song->speed;
    tick = (coso || m_variant > 1) ? 1 : speed;

    do
    {
        voice->initialize();
        voice->channel = amiga->channels[voice->index];
        voice->trackPtr = song->pointer + (voice->index * 3);

        if (coso)
        {
            voice->trackPos = 0;
            voice->patternPos = 8;
        }
        else
        {
            position = voice->trackPtr;
            voice->patternPtr = patterns + (stream[position] * patternLen);
            position++;
            voice->trackTrans = stream[position];
            position++;
            voice->volTrans = stream[position];
            position++;

            voice->freqsPtr = base;
            voice->volsPtr = base;
        }
    }
    while (voice = voice->next);
}

void JHPlayer::process()
{
    AmigaChannel* chan;
    int loop = 0;
    int period = 0;
    int pos1 = 0;
    int pos2 = 0;
    BaseSample* sample;
    int value = 0;
    JHVoice* voice = voices[0];

    if (--tick == 0)
    {
        tick = speed;

        do
        {
            chan = voice->channel;

            if (coso)
            {
                if (--voice->cosoCtr < 0)
                {
                    voice->cosoCtr = voice->cosoSpeed;

                    do
                    {
                        position = voice->patternPos;

                        do
                        {
                            loop = 0;
                            value = (signed char)stream[position];
                            position++;

                            if (value == -1)
                            {
                                if (voice->trackPos == song->length)
                                {
                                    voice->trackPos = 0;
                                    amiga->setComplete(1);
                                }

                                position = voice->trackPtr + voice->trackPos;
                                value = stream[position];
                                position++;
                                voice->trackTrans = (signed char)stream[position];
                                position++;

                                pos1 = stream[position];

                                if ((m_variant > 3) && (pos1 > 127))
                                {
                                    pos2 = (pos1 >> 4) & 15;
                                    pos1 &= 15;

                                    if (pos2 == 15)
                                    {
                                        pos2 = 100;

                                        if (pos1)
                                        {
                                            pos2 = (15 - pos1) + 1;
                                            pos2 <<= 1;
                                            pos1 = pos2;
                                            pos2 <<= 1;
                                            pos2 += pos1;
                                        }

                                        voice->volFade = pos2;
                                    }
                                    else if (pos2 == 8)
                                    {
                                        amiga->setComplete(1);
                                    }
                                    else if (pos2 == 14)
                                    {
                                        speed = pos1;
                                    }
                                }
                                else
                                {
                                    voice->volTrans = (signed char)stream[position];
                                    position++;
                                }

                                position = patterns + (value << 1);
                                voice->patternPos = readEndian(stream[position], stream[position + 1]);
                                position += 2;
                                voice->trackPos += 12;
                                loop = 1;
                            }
                            else if (value == -2)
                            {
                                voice->cosoCtr = voice->cosoSpeed = stream[position];
                                position++;
                                loop = 3;
                            }
                            else if (value == -3)
                            {
                                voice->cosoCtr = voice->cosoSpeed = stream[position];
                                position++;
                                voice->patternPos = position;
                            }
                            else
                            {
                                voice->note = value;
                                voice->info = (signed char)stream[position];
                                position++;

                                if (voice->info & 224)
                                {
                                    voice->infoPrev = (signed char)stream[position];
                                    position++;
                                }

                                voice->patternPos = position;
                                voice->portaDelta = 0;
                                if (value >= 0)
                                {
                                    if (m_variant == 1) chan->setEnabled(0);

                                    value = (voice->info & 31) + voice->volTrans;
                                    position = vols + (value << 1);
                                    position = readEndian(stream[position], stream[position + 1]);

                                    voice->volCtr = voice->volSpeed = stream[position];
                                    position++;
                                    voice->volSustain = 0;
                                    value = (signed char)stream[position];
                                    position++;

                                    voice->vibSpeed = (signed char)stream[position];
                                    position++;
                                    voice->vibrato = 64;
                                    voice->vibDepth = voice->vibDelta = (signed char)stream[position];
                                    position++;
                                    voice->vibDelay = stream[position];
                                    position++;

                                    voice->volsPtr = position;
                                    voice->volsPos = 0;

                                    if (value != -128)
                                    {
                                        if (m_variant > 1 && (voice->info & 64)) value = voice->infoPrev;
                                        position = freqs + (value << 1);
                                        voice->freqsPtr = readEndian(stream[position], stream[position + 1]);
                                        position += 2;
                                        voice->freqsPos = 0;

                                        voice->tick = 0;
                                    }
                                }
                            }
                        }
                        while (loop > 2);
                    }
                    while (loop > 0);
                }
            }
            else
            {
                position = voice->patternPtr + voice->patternPos;
                value = (signed char)stream[position];
                position++;

                if ((voice->patternPos == patternLen) || ((value & 127) == 1))
                {
                    if (voice->trackPos == song->length)
                    {
                        voice->trackPos = 0;
                        amiga->setComplete(1);
                    }

                    position = voice->trackPtr + voice->trackPos;
                    value = stream[position];
                    position++;
                    voice->trackTrans = (signed char)stream[position];
                    position++;
                    voice->volTrans = (signed char)stream[position];
                    position++;

                    if (voice->volTrans == -128) amiga->setComplete(1);

                    voice->patternPtr = patterns + (value * patternLen);
                    voice->patternPos = 0;
                    voice->trackPos += 12;

                    position = voice->patternPtr;
                    value = (signed char)stream[position];
                    position++;
                }

                if (value & 127)
                {
                    voice->note = value & 127;
                    voice->portaDelta = 0;

                    pos1 = position;
                    if (!voice->patternPos) position += patternLen;
                    position -= 2;

                    voice->infoPrev = (signed char)stream[position];
                    position++;
                    position = pos1;
                    voice->info = (signed char)stream[position];
                    position++;

                    if (value >= 0)
                    {
                        if (m_variant == 1) chan->setEnabled(0);
                        value = (voice->info & 31) + voice->volTrans;
                        position = vols + (value << 6);

                        voice->volCtr = voice->volSpeed = stream[position];
                        position++;
                        voice->volSustain = 0;
                        value = (signed char)stream[position];
                        position++;

                        voice->vibSpeed = (signed char)stream[position];
                        position++;
                        voice->vibrato = 64;
                        voice->vibDepth = voice->vibDelta = (signed char)stream[position];
                        position++;
                        voice->vibDelay = stream[position];
                        position++;

                        voice->volsPtr = position;
                        voice->volsPos = 0;

                        if (m_variant > 1 && (voice->info & 64)) value = voice->infoPrev;

                        voice->freqsPtr = freqs + (value << 6);
                        voice->freqsPos = 0;

                        voice->tick = 0;
                    }
                }
                voice->patternPos += 2;
            }
        }
        while (voice = voice->next);
        voice = voices[0];
    }


    do
    {
        chan = voice->channel;
        voice->enabled = 0;

        do
        {
            loop = 0;
            if (voice->tick)
            {
                voice->tick--;
            }
            else
            {
                position = voice->freqsPtr + voice->freqsPos;

                do
                {
                    value = (signed char)stream[position];
                    position++;
                    if (value == -31) break;
                    loop = 3;

                    if (m_variant == 3 && coso)
                    {
                        if (value == -27)
                        {
                            value = -30;
                        }
                        else if (value == -26)
                        {
                            value = -28;
                        }
                    }

                    unsigned char c1;
                    unsigned char c2;
                    switch (value)
                    {
                    case -32:
                        voice->freqsPos = (stream[position] & 63);
                        position++;
                        position = voice->freqsPtr + voice->freqsPos;
                        break;
                    case -30:
                        sample = samples[stream[position]];
                        position++;
                        voice->sample = -1;

                        voice->loopPtr = sample->loopPtr;
                        voice->repeat = sample->repeat;
                        voice->enabled = 1;

                        chan->setEnabled(0);
                        chan->pointer = sample->pointer;
                        chan->length = sample->length;

                        voice->volsPos = 0;
                        voice->volCtr = 1;
                        voice->slide = 0;
                        voice->freqsPos += 2;
                        break;
                    case -29:
                        voice->vibSpeed = (signed char)stream[position];
                        position++;
                        voice->vibDepth = (signed char)stream[position];
                        position++;
                        voice->freqsPos += 3;
                        break;
                    case -28:
                        sample = samples[stream[position]];
                        position++;
                        voice->loopPtr = sample->loopPtr;
                        voice->repeat = sample->repeat;

                        chan->pointer = sample->pointer;
                        chan->length = sample->length;

                        voice->slide = 0;
                        voice->freqsPos += 2;
                        break;
                    case -27:
                        if (m_variant < 2) break;
                        sample = samples[stream[position]];
                        position++;
                        chan->setEnabled(0);
                        voice->enabled = 1;

                        if (m_variant == 2)
                        {
                            pos1 = stream[position] * sample->length;
                            position++;

                            voice->loopPtr = sample->loopPtr + pos1;
                            voice->repeat = sample->repeat;

                            chan->pointer = sample->pointer + pos1;
                            chan->length = sample->length;

                            voice->freqsPos += 3;
                        }
                        else
                        {
                            voice->sldPointer = sample->pointer;
                            voice->sldEnd = sample->pointer + sample->length;
                            value = readEndian(stream[position], stream[position + 1]);
                            position += 2;

                            if (value == 0xffff)
                            {
                                voice->sldLoopPtr = sample->length;
                            }
                            else
                            {
                                voice->sldLoopPtr = value << 1;
                            }

                            voice->sldLen = readEndian(stream[position], stream[position + 1]) << 1;
                            position += 2;
                            voice->sldDelta = (signed short)readEndian(stream[position], stream[position + 1]) << 1;
                            position += 2;
                            voice->sldActive = 0;
                            voice->sldCtr = 0;
                            voice->sldSpeed = stream[position];
                            position++;
                            voice->slide = 1;
                            voice->sldDone = 0;

                            voice->freqsPos += 9;
                        }
                        voice->volsPos = 0;
                        voice->volCtr = 1;
                        break;
                    case -26:
                        if (m_variant < 3) break;

                        voice->sldLen = readEndian(stream[position], stream[position + 1]) << 1;
                        position += 2;
                        voice->sldDelta = (signed short)readEndian(stream[position], stream[position + 1]) << 1;
                        position += 2;
                        voice->sldActive = 0;
                        voice->sldCtr = 0;
                        voice->sldSpeed = stream[position];
                        position++;
                        voice->sldDone = 0;

                        voice->freqsPos += 6;
                        break;
                    case -25:
                        if (m_variant == 1)
                        {
                            voice->freqsPtr = freqs + (stream[position] << 6);
                            position++;
                            voice->freqsPos = 0;

                            position = voice->freqsPtr;
                            loop = 3;
                        }
                        else
                        {
                            value = stream[position];
                            position++;

                            if (value != voice->sample)
                            {
                                sample = samples[value];
                                voice->sample = value;

                                voice->loopPtr = sample->loopPtr;
                                voice->repeat = sample->repeat;
                                voice->enabled = 1;

                                chan->setEnabled(0);
                                chan->pointer = sample->pointer;
                                chan->length = sample->length;
                            }

                            voice->volsPos = 0;
                            voice->volCtr = 1;
                            voice->slide = 0;
                            voice->freqsPos += 2;
                        }
                        break;
                    case -24:
                        voice->tick = stream[position];
                        position++;
                        voice->freqsPos += 2;
                        loop = 1;
                        break;
                    case -23:
                        if (m_variant < 2) break;
                        sample = samples[stream[position]];
                        position++;
                        voice->sample = -1;
                        voice->enabled = 1;

                        pos2 = stream[position];
                        position++;
                        pos1 = position;
                        chan->setEnabled(0);

                        position = sampleData + (sample->pointer + 4);
                        c1 = stream[position];
                        position++;
                        c2 = stream[position];
                        position++;
                        value = (c1 * 24) + (c2 << 2);
                        position += (pos2 * 24);

                        voice->loopPtr = readEndian(stream[position], stream[position + 1], stream[position + 2],
                                                    stream[position + 3]) & 0xfffffffe;
                        position += 4;
                        chan->length = (readEndian(stream[position], stream[position + 1], stream[position + 2],
                                                   stream[position + 3]) & 0xfffffffe) - voice->loopPtr;
                        position += 4;
                        voice->loopPtr += (sample->pointer + (value + 8));
                        chan->pointer = voice->loopPtr;
                        voice->repeat = 2;

                        position = pos1;
                        pos1 = voice->loopPtr + 1;
                        amiga->memory[pos1] = amiga->memory[voice->loopPtr];

                        voice->volsPos = 0;
                        voice->volCtr = 1;
                        voice->slide = 0;
                        voice->freqsPos += 3;
                        break;
                    default:
                        voice->transpose = value;
                        voice->freqsPos++;
                        loop = 0;
                    }
                }
                while (loop > 2);
            }
        }
        while (loop > 0);

        if (voice->slide)
        {
            if (!voice->sldDone)
            {
                if (--voice->sldCtr < 0)
                {
                    voice->sldCtr = voice->sldSpeed;

                    if (voice->sldActive)
                    {
                        value = voice->sldLoopPtr + voice->sldDelta;

                        if (value < 0)
                        {
                            voice->sldDone = 1;
                            value = voice->sldLoopPtr - voice->sldDelta;
                        }
                        else
                        {
                            pos1 = voice->sldPointer + (voice->sldLen + value);

                            if (pos1 > voice->sldEnd)
                            {
                                voice->sldDone = 1;
                                value = voice->sldLoopPtr - voice->sldDelta;
                            }
                        }
                        voice->sldLoopPtr = value;
                    }
                    else
                    {
                        voice->sldActive = 1;
                    }

                    voice->loopPtr = voice->sldPointer + voice->sldLoopPtr;
                    voice->repeat = voice->sldLen;
                    chan->pointer = voice->loopPtr;
                    chan->length = voice->repeat;
                }
            }
        }

        do
        {
            loop = 0;

            if (voice->volSustain)
            {
                voice->volSustain--;
            }
            else
            {
                if (--voice->volCtr) break;
                voice->volCtr = voice->volSpeed;

                do
                {
                    position = voice->volsPtr + voice->volsPos;
                    value = (signed char)stream[position];
                    position++;
                    if ((value <= -25) && (value >= -31)) break;

                    switch (value)
                    {
                    case -24:
                        voice->volSustain = stream[position];
                        position++;
                        voice->volsPos += 2;
                        loop = 1;

                        break;
                    case -32:
                        voice->volsPos = (stream[position] & 63) - 5;
                        position++;
                        loop = 3;
                        break;
                    default:
                        voice->volume = value;
                        voice->volsPos++;
                        loop = 0;
                    }
                }
                while (loop > 2);
            }
        }
        while (loop > 0);

        value = voice->transpose;
        if (value >= 0) value += (voice->note + voice->trackTrans);
        value &= 127;

        if (coso)
        {
            if (value > 83) value = 0;
            period = PERIODS[value];
            value <<= 1;
        }
        else
        {
            value <<= 1;
            position = periods + value;
            period = readEndian(stream[position], stream[position + 1]);
            position += 2;
        }

        if (voice->vibDelay)
        {
            voice->vibDelay--;
        }
        else
        {
            if (m_variant > 3)
            {
                if (voice->vibrato & 32)
                {
                    value = voice->vibDelta + voice->vibSpeed;

                    if (value > voice->vibDepth)
                    {
                        voice->vibrato &= ~32;
                        value = voice->vibDepth;
                    }
                }
                else
                {
                    value = voice->vibDelta - voice->vibSpeed;

                    if (value < 0)
                    {
                        voice->vibrato |= 32;
                        value = 0;
                    }
                }

                voice->vibDelta = value;
                value = (value - (voice->vibDepth >> 1)) * period;
                period += (value >> 10);
            }
            else if (m_variant > 2)
            {
                value = voice->vibSpeed;

                if (value < 0)
                {
                    value &= 127;
                    voice->vibrato ^= 1;
                }

                if (!(voice->vibrato & 1))
                {
                    if (voice->vibrato & 32)
                    {
                        voice->vibDelta += value;
                        pos1 = voice->vibDepth << 1;

                        if (voice->vibDelta > pos1)
                        {
                            voice->vibrato &= ~32;
                            voice->vibDelta = pos1;
                        }
                    }
                    else
                    {
                        voice->vibDelta -= value;

                        if (voice->vibDelta < 0)
                        {
                            voice->vibrato |= 32;
                            voice->vibDelta = 0;
                        }
                    }
                }

                period += (value - voice->vibDepth);
            }
            else
            {
                if (voice->vibrato >= 0 || !(voice->vibrato & 1))
                {
                    if (voice->vibrato & 32)
                    {
                        voice->vibDelta += voice->vibSpeed;
                        pos1 = voice->vibDepth << 1;

                        if (voice->vibDelta >= pos1)
                        {
                            voice->vibrato &= ~32;
                            voice->vibDelta = pos1;
                        }
                    }
                    else
                    {
                        voice->vibDelta -= voice->vibSpeed;

                        if (voice->vibDelta < 0)
                        {
                            voice->vibrato |= 32;
                            voice->vibDelta = 0;
                        }
                    }
                }

                pos1 = voice->vibDelta - voice->vibDepth;

                if (pos1)
                {
                    value += 160;

                    while (value < 256)
                    {
                        pos1 += pos1;
                        value += 24;
                    }

                    period += pos1;
                }
            }
        }

        if (m_variant < 3) voice->vibrato ^= 1;

        if (voice->info & 32)
        {
            value = voice->infoPrev;

            if (m_variant > 3)
            {
                if (value < 0)
                {
                    voice->portaDelta -= value;
                    value = voice->portaDelta * period;
                    period += (value >> 10);
                }
                else
                {
                    voice->portaDelta += value;
                    value = voice->portaDelta * period;
                    period -= (value >> 10);
                }
            }
            else
            {
                if (value < 0)
                {
                    voice->portaDelta += (-value << 11);
                    period += (voice->portaDelta >> 16);
                }
                else
                {
                    voice->portaDelta += (value << 11);
                    period -= (voice->portaDelta >> 16);
                }
            }
        }

        if (m_variant > 3)
        {
            value = (voice->volFade * voice->volume) / 100;
        }
        else
        {
            value = voice->volume;
        }

        chan->setPeriod(period);
        chan->setVolume(value);

        if (voice->enabled)
        {
            chan->setEnabled(1);
            chan->pointer = voice->loopPtr;
            chan->length = voice->repeat;
        }
    }
    while (voice = voice->next);
}

int JHPlayer::load(void* _data, unsigned long int _length)
{
    stream = static_cast<unsigned char*>(_data);
    m_version = 0;
    position = 4;
    base = periods = 0;
    int value = 0;
    int tracks = 0;
    int songData = 0;
    int headers = 0;
    int id = 0;
    int len = 0;
    int pos = 0;

    coso = (stream[0] == 'C' && stream[1] == 'O' && stream[2] == 'S' && stream[3] == 'O');

    if (coso)
    {
        for (int i = 0; i < 7; ++i)
        {
            value += readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);
            position += 4;
        }


        position = 47;
        value += stream[position];
        position++;

        switch (value)
        {
        case 22670: //Astaroth
        case 18845:
        case 30015: //Chambers of Shaolin
        case 22469:
        case 3549: //Over the Net
            m_variant = 1;
            break;
        case 16948: //Dragonflight
        case 18337:
        case 13704:
            m_variant = 2;
            break;
        case 18548: //Wings of Death
        case 13928:
        case 8764:
        case 17244:
        case 11397:
        case 14496:
        case 14394:
        case 13578: //Dragonflight

        case 6524:
            m_variant = 3;
            break;
        default:
            m_variant = 4;
            break;
        }


        m_version = 2;
        position = 4;


        freqs = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);
        position += 4;
        vols = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);
        position += 4;
        patterns = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);
        position += 4;
        tracks = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);
        position += 4;
        songData = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);
        position += 4;
        headers = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);
        position += 4;
        sampleData = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);
        position += 4;

        position = 0;
        //All this code to write three values..............
        unsigned char myVal[4];

        myVal[0] = 0x1000000 >> 24;
        myVal[1] = (0x1000000 >> 16) & 0xFF;
        myVal[2] = (0x1000000 >> 8) & 0xFF;
        myVal[3] = 0x1000000 & 0xFF;
        for (int p = 0; p < 4; p++)
        {
            stream[p] = myVal[p];
        }

        position += 4;
        myVal[0] = 0xe1 >> 24;
        myVal[1] = (0xe1 >> 16) & 0xFF;
        myVal[2] = (0xe1 >> 8) & 0xFF;
        myVal[3] = 0xe1 & 0xFF;
        for (int p = 0; p < 4; p++)
        {
            stream[p + 4] = myVal[p];
        }
        position += 4;
        myVal[0] = (0xffff >> 8) & 0xFF;
        myVal[1] = 0xffff & 0xFF;
        for (int p = 0; p < 2; p++)
        {
            stream[p + 8] = 255;
        }
        position += 2;


        //stream.writeInt(0x1000000);
        //stream.writeInt(0xe1);
        //stream.writeShort(0xffff);


        len = ((sampleData - headers) / 10) - 1;

        if (len < 1 || len > 255)
        {
            m_version = 0;
            return 0;
        }

        m_totalSongs = (headers - songData) / 6;
    }
    else
    {
        do
        {
            value = readEndian(stream[position], stream[position + 1]);
            position += 2;

            switch (value)
            {
            case 0x0240: //andi.w #x,d0
                value = readEndian(stream[position], stream[position + 1]);
                position += 2;

                if (value == 0x007f)
                {
                    //andi.w #$7f,d0
                    position += 2;
                    periods = position + readEndian(stream[position], stream[position + 1]);
                    position += 2;
                }
                break;
            case 0x7002: //moveq #2,d0
            case 0x7003: //moveq #3,d0
                m_channels = (value & 0xff) + 1;
                value = readEndian(stream[position], stream[position + 1]);
                position += 2;
                if (value == 0x7600)
                {
                    value = readEndian(stream[position], stream[position + 1]); //moveq #0,d3
                    position += 2;
                }

                if (value == 0x41fa)
                {
                    //lea x,a0
                    position += 4;
                    base = position + readEndian(stream[position], stream[position + 1]);
                    position += 2;
                }
                break;
            case 0x5446: //"TF"
                value = readEndian(stream[position], stream[position + 1]);
                position += 2;

                if (value == 0x4d58)
                {
                    //"MX"
                    id = position - 4;
                    position = _length;
                }
                break;
            }
        }
        while (_length - position > 12);

        if (!id || !base || !periods)
        {
            m_version = 0;
            return 0;
        }
        m_version = 1;

        position = id + 4;
        freqs = pos = id + 32;
        value = readEndian(stream[position], stream[position + 1]);
        position += 2;
        vols = (pos += (++value << 6));

        value = readEndian(stream[position], stream[position + 1]);
        position += 2;
        patterns = (pos += (++value << 6));
        value = readEndian(stream[position], stream[position + 1]);
        position += 2;
        position += 2;
        patternLen = readEndian(stream[position], stream[position + 1]);
        position += 2;
        tracks = (pos += (++value * patternLen));

        position -= 4;
        value = readEndian(stream[position], stream[position + 1]);
        position += 2;
        songData = (pos += (++value * 12));

        position = id + 16;
        m_totalSongs = readEndian(stream[position], stream[position + 1]);
        position += 2;
        headers = (pos += (++m_totalSongs * 6));

        len = readEndian(stream[position], stream[position + 1]);
        position += 2;

        sampleData = pos + (len * 30);
    }

    position = headers;

    samples = vector<BaseSample*>(len);
    value = 0;


    for (int i = 0; i < len; ++i)
    {
        BaseSample* sample = new BaseSample();
        if (!coso)
        {
            const int STRING_LENGTH = 18;
            for (int j = 0; j < STRING_LENGTH; j++)
            {
                if (!stream[position + j])
                {
                    break;
                }
                sample->name += stream[position + j];
            }
            position += STRING_LENGTH;
        }

        sample->pointer = readEndian(stream[position], stream[position + 1], stream[position + 2],
                                     stream[position + 3]);
        position += 4;
        sample->length = readEndian(stream[position], stream[position + 1]) << 1;
        position += 2;
        if (!coso)
        {
            sample->volume = readEndian(stream[position], stream[position + 1]);
            position += 2;
        }
        sample->loopPtr = readEndian(stream[position], stream[position + 1]) + sample->pointer;
        position += 2;
        sample->repeat = readEndian(stream[position], stream[position + 1]) << 1;
        position += 2;

        if (sample->loopPtr & 1) sample->loopPtr--;
        value += sample->length;
        samples[i] = sample;
    }

    position = sampleData;
    amiga->store(stream, value, position, _length);

    position = songData;
    songs = vector<JHSong*>();
    value = 0;

    for (int i = 0; i < m_totalSongs; ++i)
    {
        JHSong* song = new JHSong();
        song->pointer = readEndian(stream[position], stream[position + 1]);
        position += 2;
        song->length = readEndian(stream[position], stream[position + 1]) - (song->pointer + 1);
        position += 2;
        song->speed = readEndian(stream[position], stream[position + 1]);
        position += 2;

        song->pointer = (song->pointer * 12) + tracks;
        song->length *= 12;
        if (song->length > 12) songs.push_back(song);
    }

    m_totalSongs = songs.size();

    if (!coso)
    {
        position = 0;
        m_variant = 1;

        do
        {
            value = readEndian(stream[position], stream[position + 1]);
            position += 2;

            if (value == 0xb03c || value == 0x0c00)
            {
                //cmp.b #x,d0 | cmpi.b #x,d0
                value = readEndian(stream[position], stream[position + 1]);
                position += 2;

                if (value == 0x00e5 || value == 0x00e6 || value == 0x00e9)
                {
                    //effects
                    m_variant = 2;
                    break;
                }
            }
            else if (value == 0x4efb)
            {
                //jmp $(pc,d0.w)
                m_variant = 3;
                break;
            }
        }
        while (position < id);
    }


    m_version = 1;
    if (!coso)
    {
        format = "Hippel";
    }
    else
    {
        format = "Hippel COSO";
    }
    //printData();
    return 1;
}

void JHPlayer::printData()
{
    //    cout << "base: " << base << " patterns: " << patterns << " patternLen: " << patternLen << " periods: " << periods << " frqseqs: " << frqseqs << " volseqs: " << volseqs << " samplesData: " << samplesData << " coso: " << (bool)coso << " variant: " << variant << "\n";

    //    for(unsigned int i = 0; i < songs.size(); i++)
    //    {
    //        JHSong* song = songs[i];
    //        if(song)
    //        {
    //            cout << "Song [" << i << "] speed: " << song->speed << " pointer: " << song->pointer << " length: " << song->length <<"\n";
    //        }
    //    }

    //    for(unsigned int i = 0; i < samples.size(); i++)
    //    {
    //        AmigaSample* sample = samples[i];
    //        if(sample)
    //        {
    //            cout << "Sample [" << i << "] length: " << sample->length << " loop: " << sample->loop << " loopPtr: " << sample->loopPtr << " name: " << sample->name << " pointer: " << sample->pointer << " repeat: " << sample->repeat << " volume: " << sample->volume << "\n";
    //        }
    //    }
}

unsigned char JHPlayer::getSubsongsCount()
{
    return songs.size();
}

void JHPlayer::selectSong(unsigned char subsong)
{
    m_songNumber = subsong;
}

std::vector<BaseSample*> JHPlayer::getSamples()
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
    return samp;
}
