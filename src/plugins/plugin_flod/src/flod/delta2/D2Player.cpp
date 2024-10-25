#include "D2Player.h"
#include "D2Voice.h"
#include "BaseRow.h"
#include "D2Sample.h"
#include "BaseStep.h"
#include "AmigaChannel.h"
#include <iostream>
#include "MyEndian.h"

const int D2Player::PERIODS[85] =
{
    0, 6848, 6464, 6096, 5760, 5424, 5120, 4832, 4560, 4304, 4064, 3840, 3616, 3424, 3232,
    3048, 2880, 2712, 2560, 2416, 2280, 2152, 2032, 1920, 1808, 1712, 1616, 1524, 1440, 1356,
    1280, 1208, 1140, 1076, 1016, 960, 904, 856, 808, 762, 720, 678, 640, 604, 570,
    538, 508, 480, 452, 428, 404, 381, 360, 339, 320, 302, 285, 269, 254, 240,
    226, 214, 202, 190, 180, 170, 160, 151, 143, 135, 127, 120, 113, 113, 113,
    113, 113, 113, 113, 113, 113, 113, 113, 113, 113
};

D2Player::D2Player(Amiga* amiga): AmigaPlayer(amiga)
{
    arpeggios = std::vector<signed char>(1024);
    voices = std::vector<D2Voice*>(4);

    voices[0] = new D2Voice(0);
    voices[0]->next = voices[1] = new D2Voice(1);
    voices[1]->next = voices[2] = new D2Voice(2);
    voices[2]->next = voices[3] = new D2Voice(3);
}

D2Player::~D2Player()
{
    arpeggios.clear();
    data.clear();

    for (unsigned int i = 0; i < tracks.size(); i++)
    {
        if (tracks[i]) delete tracks[i];
    }
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
    for (unsigned int i = 0; i < patterns.size(); i++)
    {
        if (patterns[i]) delete patterns[i];
    }
    patterns.clear();
}

void D2Player::initialize()
{
    AmigaPlayer::initialize();
    speed = 5;
    tick = 1;
    noise = 0;

    D2Voice* voice = voices[0];
    int i = 0;
    do
    {
        voice->initialize();
        voice->channel = amiga->channels[voice->index];
        voice->sample = samples[int(samples.size() - 1)];

        voice->trackPtr = data[voice->index];
        voice->restart = data[int(voice->index + 4)];
        voice->trackLen = data[int(voice->index + 8)];

        i++;
    }
    while (voice = voice->next);
}

void D2Player::process()
{
    int value = 0;
    D2Sample* sample;
    BaseRow* row;
    AmigaChannel* chan;
    int level = 0;
    D2Voice* voice = voices[0];

    for (int i = 0; i < 64; ++i)
    {
        noise = (noise << 7) | (noise >> 25);
        noise += 0x6eca756d;
        noise ^= 0x9e59a92b;

        value = (noise >> 24) & 255;
        if (value > 127) value |= -256;
        amiga->memory[i++] = value;

        value = (noise >> 16) & 255;
        if (value > 127) value |= -256;
        amiga->memory[i++] = value;

        value = (noise >> 8) & 255;
        if (value > 127) value |= -256;
        amiga->memory[i++] = value;

        value = noise & 255;
        if (value > 127) value |= -256;
        amiga->memory[i++] = value;
    }

    if (--tick < 0) tick = speed;

    do
    {
        if (voice->trackLen < 1) continue;

        chan = voice->channel;

        sample = voice->sample;

        if (sample->synth)
        {
            chan->pointer = sample->loopPtr;
            chan->length = sample->repeat;
        }

        if (!tick)
        {
            if (!voice->patternPos)
            {
                voice->step = tracks[int(voice->trackPtr + voice->trackPos)];

                if (++voice->trackPos == voice->trackLen)
                {
                    voice->trackPos = voice->restart;
                }
            }

            row = voice->row = patterns[int(voice->step->pattern + voice->patternPos)];

            if (row->note)
            {
                chan->setEnabled(0);
                voice->note = row->note;
                voice->period = PERIODS[int(row->note + voice->step->transpose)];

                sample = voice->sample = samples[row->sample];

                if (sample->synth < 0)
                {
                    chan->pointer = sample->pointer;
                    chan->length = sample->length;
                }

                voice->arpeggioPos = 0;
                voice->tableCtr = 0;
                voice->tablePos = 0;
                voice->vibratoCtr = sample->vibratos[1];
                voice->vibratoPos = 0;
                voice->vibratoDir = 0;
                voice->vibratoPeriod = 0;
                voice->vibratoSustain = sample->vibratos[2];
                voice->volume = 0;
                voice->volumePos = 0;
                voice->volumeSustain = 0;
            }


            switch (row->effect)
            {
            case -1:
                break;
            case 0:
                speed = row->param & 15;

                break;
            case 1:
                amiga->filter->active = row->param;

                break;
            case 2:
                voice->pitchBend = ~(row->param & 255) + 1;
                break;
            case 3:
                voice->pitchBend = row->param & 255;
                break;
            case 4:
                voice->portamento = row->param;
                break;
            case 5:
                voice->volumeMax = row->param & 63;
                break;
            case 6:
                amiga->setVolume(row->param);
                break;
            case 7:
                voice->arpeggioPtr = (row->param & 63) << 4;

                break;
            }

            voice->patternPos = (++voice->patternPos & 15);
        }
        sample = voice->sample;

        if (sample->synth >= 0)
        {
            if (voice->tableCtr)
            {
                voice->tableCtr--;
            }
            else
            {
                voice->tableCtr = sample->index;
                value = sample->table[voice->tablePos];

                if (value == 0xff)
                {
                    value = sample->table[++voice->tablePos];
                    if (value != 0xff)
                    {
                        voice->tablePos = value;
                        value = sample->table[voice->tablePos];
                    }
                }

                if (value != 0xff)
                {
                    chan->pointer = value << 8;
                    chan->length = sample->length;
                    if (++voice->tablePos > 47) voice->tablePos = 0;
                }
            }
        }
        value = sample->vibratos[voice->vibratoPos];

        if (voice->vibratoDir) voice->vibratoPeriod -= value;
        else voice->vibratoPeriod += value;

        if (--voice->vibratoCtr == 0)
        {
            voice->vibratoCtr = sample->vibratos[int(voice->vibratoPos + 1)];
            voice->vibratoDir = ~voice->vibratoDir;
        }

        if (voice->vibratoSustain)
        {
            voice->vibratoSustain--;
        }
        else
        {
            voice->vibratoPos += 3;
            if (voice->vibratoPos == 15) voice->vibratoPos = 12;
            voice->vibratoSustain = sample->vibratos[int(voice->vibratoPos + 2)];
        }

        if (voice->volumeSustain)
        {
            voice->volumeSustain--;
        }
        else
        {
            value = sample->volumes[voice->volumePos];
            level = sample->volumes[int(voice->volumePos + 1)];

            if (level < voice->volume)
            {
                voice->volume -= value;
                if (voice->volume < level)
                {
                    voice->volume = level;
                    voice->volumePos += 3;
                    voice->volumeSustain = sample->volumes[int(voice->volumePos - 1)];
                }
            }
            else
            {
                voice->volume += value;
                if (voice->volume > level)
                {
                    voice->volume = level;
                    voice->volumePos += 3;
                    if (voice->volumePos == 15) voice->volumePos = 12;
                    voice->volumeSustain = sample->volumes[int(voice->volumePos - 1)];
                }
            }
        }

        if (voice->portamento)
        {
            if (voice->period < voice->finalPeriod)
            {
                voice->finalPeriod -= voice->portamento;
                if (voice->finalPeriod < voice->period) voice->finalPeriod = voice->period;
            }
            else
            {
                voice->finalPeriod += voice->portamento;
                if (voice->finalPeriod > voice->period) voice->finalPeriod = voice->period;
            }
        }
        value = arpeggios[int(voice->arpeggioPtr + voice->arpeggioPos)];


        if (value == -128)
        {
            voice->arpeggioPos = 0;
            value = arpeggios[voice->arpeggioPtr]; //was missing ending colon in as
        }
        //gcc gives warning:  voice->arpeggioPos = ++voice->arpeggioPos & 15;
        //so instead:
        ++voice->arpeggioPos;
        voice->arpeggioPos = voice->arpeggioPos & 15;


        if (voice->portamento == 0)
        {
            value = voice->note + voice->step->transpose + value;
            if (value < 0) value = 0;
            voice->finalPeriod = PERIODS[value];
        }

        voice->vibratoPeriod -= (sample->pitchBend - voice->pitchBend);
        chan->setPeriod(voice->finalPeriod + voice->vibratoPeriod);

        value = (voice->volume >> 2) & 63;
        if (value > voice->volumeMax) value = voice->volumeMax;
        chan->setVolume(value);


        chan->setEnabled(1);
    }
    while (voice = voice->next);
}

void D2Player::printData()
{
    for (unsigned int i = 0; i < patterns.size(); i++)
    {
        BaseRow* row = patterns[i];
        std::cout << "Pattern [" << i << "] note: " << row->note << " sample: " << row->sample << " param: " << row->
            param << " effect: " << row->effect << "\n";
    }
    for (unsigned int i = 0; i < samples.size(); i++)
    {
        D2Sample* sample = samples[i];
        std::cout << "Sample [" << i << "] index: " << sample->index << " length: " << sample->length << " loopPtr: " <<
            sample->loopPtr << " loopPtr: " << sample->loopPtr << " name: " << sample->name << " pitchBend: " << sample
            ->pitchBend << " pointer: " << sample->pointer << " repeat: " << sample->repeat << " synth: " << (int)sample
            ->synth << "\n";
        for (unsigned int j = 0; j < sample->table.size(); j++)
        {
            std::cout << "Sample [" << i << "] Table [" << j << "] " << (int)sample->table[j] << "\n";
        }
        for (unsigned int j = 0; j < sample->vibratos.size(); j++)
        {
            std::cout << "Sample [" << i << "] Vibratos [" << j << "] " << (int)sample->vibratos[j] << "\n";
        }
        for (unsigned int j = 0; j < sample->volumes.size(); j++)
        {
            std::cout << "Sample [" << i << "] Volumes [" << j << "] " << (int)sample->volumes[j] << "\n";
        }
    }
    for (int i = 0; i < 1024; i++)
    {
        std::cout << "Arpeggio [" << i << "] " << (int)arpeggios[i] << "\n";
    }
    for (unsigned int i = 0; i < tracks.size(); i++)
    {
        BaseStep* step = tracks[i];
        std::cout << "Tracks [" << i << "] pattern: " << step->pattern << " transpose: " << (int)step->transpose <<
            "\n";
    }
    //    for(int i = 0; i < amiga->memory.size(); i++)
    //    {
    //        std::cout << "Memory [" << i << "]" << (int)amiga->memory[i] <<  "\n";
    //    }
    for (unsigned int i = 0; i < data.size(); i++)
    {
        std::cout << "Data [" << i << "] " << data[i] << "\n";
    }
    std::flush(std::cout);
}

int D2Player::load(void* _data, unsigned long int length)
{
    //std::string n[] = { "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"};
    if (length < 3017)
    {
        return -1;
    }
    unsigned int position = 3014;
    unsigned char* stream = static_cast<unsigned char*>(_data);
    bool isOk = -1;


    if (stream[3014] == '.' && stream[3015] == 'F' && stream[3016] == 'N' && stream[3017] == 'L')
    {
        isOk = 1;
    }
    if (!isOk)
    {
        return -1;
    }
    position = 4042;

    data = std::vector<int>(12);


    int len = 0;
    int value = 0;
    for (int i = 0; i < 4; ++i)
    {
        data[i + 4] = readEndian(stream[position], stream[position + 1]) >> 1;
        position += 2;
        value = readEndian(stream[position], stream[position + 1]) >> 1;
        position += 2;
        data[int(i + 8)] = value;
        len += value;
    }
    value = len;
    for (int i = 3; i > 0; --i)
    {
        data[i] = (value -= data[int(i + 8)]);
    }
    tracks = std::vector<BaseStep*>(len);
    for (int i = 0; i < len; ++i)
    {
        BaseStep* step = new BaseStep();
        step->pattern = stream[position] << 4;
        position++;
        step->transpose = (signed char)stream[position];
        position++;
        tracks[i] = step;
    }


    len = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]) >> 2;
    position += 4;
    patterns = std::vector<BaseRow*>(len);

    for (int i = 0; i < len; ++i)
    {
        BaseRow* row = new BaseRow();
        row->note = stream[position];
        position++;
        row->sample = stream[position];
        position++;
        row->effect = stream[position] - 1;
        position++;
        row->param = stream[position];
        position++;
        patterns[i] = row;
        //std::cout << "note: " << n[(row->note-1)%12] << (row->note/12)+1 << " sample: " << row->sample << " data1: " << row->data1 << " data2: " << row->data2 << "\n";
    }

    position += 254;
    value = readEndian(stream[position], stream[position + 1]);
    position += 2;
    int positionMark = position;
    position -= 256;
    len = 1;
    std::vector<int> offsets = std::vector<int>(128);
    for (int i = 0; i < 128; ++i)
    {
        int j = readEndian(stream[position], stream[position + 1]);
        position += 2;
        if (j != value) offsets[len++] = j;
    }
    samples = std::vector<D2Sample*>(len);


    for (int i = 0; i < len; ++i)
    {
        position = positionMark + offsets[i];
        D2Sample* sample = new D2Sample();
        sample->length = readEndian(stream[position], stream[position + 1]) << 1;
        position += 2;
        sample->loopPtr = readEndian(stream[position], stream[position + 1]);
        position += 2;
        sample->repeat = readEndian(stream[position], stream[position + 1]) << 1;
        position += 2;

        for (int j = 0; j < 15; ++j)
        {
            sample->volumes[j] = stream[position];
            position++;
        }
        for (int j = 0; j < 15; ++j)
        {
            sample->vibratos[j] = stream[position];
            position++;
        }

        sample->pitchBend = readEndian(stream[position], stream[position + 1]);
        position += 2;
        sample->synth = (signed char)stream[position];
        position++;
        sample->index = stream[position];
        position++;

        for (int j = 0; j < 48; ++j)
        {
            sample->table[j] = stream[position];
            position++;
        }

        samples[i] = sample;
    }

    len = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);
    position += 4;
    amiga->store(stream, len, position, length);

    position += 64;
    for (int i = 0; i < 8; ++i)
    {
        offsets[i] = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);
        position += 4;
    }

    len = samples.size();
    positionMark = position;

    for (int i = 0; i < len; ++i)
    {
        D2Sample* sample = samples[i];
        if (sample->synth >= 0) continue;
        position = positionMark + offsets[sample->index];
        sample->pointer = amiga->store(stream, sample->length, position, length);
        sample->loopPtr += sample->pointer;
    }
    position = 3018;
    for (int i = 0; i < 1024; ++i)
    {
        arpeggios[i] = (signed char)stream[position];
        position++;
    }

    D2Sample* sample = new D2Sample();
    sample->pointer = sample->loopPtr = amiga->memory.size();
    sample->length = sample->repeat = 4;
    samples.push_back(sample);


    len = patterns.size();
    int j = samples.size() - 1;

    for (int i = 0; i < len; ++i)
    {
        BaseRow* row = patterns[i];
        if (row->sample > j) row->sample = 0;
    }


    m_version = 2;
    format = "Delta Music 2";
    //printData();
    return 1;
}

std::vector<BaseSample*> D2Player::getSamples()
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
