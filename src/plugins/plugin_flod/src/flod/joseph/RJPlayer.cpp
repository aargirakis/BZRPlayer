#include "RJPlayer.h"
#include "RJVoice.h"
#include "RJSample.h"
#include "AmigaChannel.h"
#include <iostream>
#include <fstream>
#include <algorithm> //transform
#include "MyEndian.h"

using namespace std;
const int RJPlayer::PERIODS[36] =
{
    453, 480, 508, 538, 570, 604, 640, 678, 720, 762,
    808, 856, 226, 240, 254, 269, 285, 302, 320, 339,
    360, 381, 404, 428, 113, 120, 127, 135, 143, 151,
    160, 170, 180, 190, 202, 214
};

RJPlayer::RJPlayer(Amiga* amiga): AmigaPlayer(amiga)
{
    voices = std::vector<RJVoice*>(4);

    voices[0] = new RJVoice(0);
    voices[0]->next = voices[1] = new RJVoice(1);
    voices[1]->next = voices[2] = new RJVoice(2);
    voices[2]->next = voices[3] = new RJVoice(3);
}

RJPlayer::~RJPlayer()
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


    songs.clear();
    tracks.clear();
    patterns.clear();
    envelope.clear();
}

void RJPlayer::initialize()
{
    AmigaPlayer::initialize();
    complete = 0;

    RJVoice* voice = voices[0];
    int index = 0;
    do
    {
        voice->initialize();
        voice->channel = amiga->channels[voice->index];
        voice->sample = samples[0];

        index = songs[((m_songNumber << 2) + voice->index)];
        voice->trackPos = index + 1;
        index = tracks[index];
        voice->patternPos = index;

        if (index)
        {
            voice->active = 1;
            complete += (1 << voice->index);
        }
    }
    while (voice = voice->next);
}

void RJPlayer::process()
{
    AmigaChannel* chan;
    RJVoice* voice = voices[0];
    RJSample* sample;
    int value = 0;
    int loop = 0;

    do
    {
        if (!voice->active) continue;

        loop = 1;
        chan = voice->channel;
        sample = voice->sample;

        if (voice->enabled)
        {
            chan->pointer = sample->loopPtr;
            chan->length = sample->repeat;
            voice->enabled = 0;
        }

        if (voice->note)
        {
            chan->setEnabled(1);
            voice->note = 0;
            voice->enabled = 1;
        }

        if (voice->patternPos)
        {
            if (--voice->tick1 == 0)
            {
                if (--voice->tick2 == 0)
                {
                    do
                    {
                        value = patterns[voice->patternPos++];

                        if (value > 127)
                        {
                            switch (value)
                            {
                            case 128:
                                voice->speed2 = 1;
                                value = tracks[voice->trackPos++];

                                if (!value)
                                {
                                    value = tracks[voice->trackPos];

                                    if (!value)
                                    {
                                        voice->active = 0;
                                    }
                                    else if (value > 127)
                                    {
                                        voice->trackPos = (tracks[++voice->trackPos] >> 1) & 255;
                                        voice->patternPos = tracks[voice->trackPos++];
                                    }
                                    else
                                    {
                                        voice->trackPos -= value;
                                        value = tracks[voice->trackPos++];

                                        if ((voice->patternPos - value) < 6)
                                        {
                                            voice->active = 0;
                                            loop = 0;
                                        }
                                        else
                                        {
                                            voice->patternPos = value;
                                        }
                                    }

                                    complete &= ~(1 << voice->index);
                                    if (!complete) amiga->setComplete(1);
                                }
                                else
                                {
                                    voice->patternPos = value;
                                }
                                break;
                            case 129:
                                voice->envelStart = 0;
                                voice->envelEnd1 = envelope[int(voice->envelPos + 5)];
                                voice->envelEnd2 = voice->envelEnd1;
                                voice->envelScale = -voice->envelVolume;
                                voice->envelStep = -1;
                                loop = 0;
                                break;
                            case 130:
                                voice->speed1 = patterns[voice->patternPos++];
                                break;
                            case 131:
                                voice->speed2 = patterns[voice->patternPos++];
                                break;
                            case 132:
                                value = patterns[voice->patternPos++];

                                if (value < samples.size())
                                {
                                    sample = voice->sample = samples[value];
                                    voice->volumeScale = sample->volumeScale;
                                    voice->periodPos = 0;
                                    voice->volumePos = 0;
                                }
                                break;
                            case 133:
                                voice->volumeScale = patterns[voice->patternPos++];
                                break;
                            case 134:
                                voice->portaCounter = patterns[voice->patternPos++];
                                voice->portaPeriod = 0;

                                voice->portaStep = patterns[voice->patternPos++] << 24;
                                voice->portaStep |= patterns[voice->patternPos++] << 16;
                                voice->portaStep |= patterns[voice->patternPos++] << 8;
                                voice->portaStep |= patterns[voice->patternPos++];
                                break;
                            case 135:
                                loop = 0;
                                break;
                            }
                        }
                        else
                        {
                            voice->period = PERIODS[int(value >> 1)];
                            voice->periodMod = voice->period;
                            voice->portaPeriod = 0;

                            value = sample->pointer + sample->offset;
                            chan->setEnabled(0);
                            chan->pointer = value;
                            chan->length = sample->length;

                            amiga->memory[value] = 0;
                            amiga->memory[++value] = 0;

                            value = sample->envelopePos;
                            voice->envelPos = value;

                            voice->envelStart = envelope[int(value + 1)];
                            voice->envelScale = voice->envelStart - envelope[value];
                            voice->envelEnd1 = envelope[int(value + 2)];
                            voice->envelEnd2 = voice->envelEnd1;

                            voice->envelStep = 4;
                            voice->note = 1;
                            break;
                        }
                    }
                    while (loop);
                    voice->tick2 = voice->speed2;
                }
                voice->tick1 = voice->speed1;
            }
        }

        if (voice->envelStep)
        {
            value = voice->envelScale;

            if (voice->envelScale)
            {
                if (voice->envelEnd1)
                {
                    value *= voice->envelEnd1;

                    if (voice->envelEnd2)
                    {
                        value /= voice->envelEnd2;
                    }
                    else
                    {
                        value = 0;
                    }
                }
                else
                {
                    value = 0;
                }
            }

            voice->envelVolume = voice->envelStart - value;
            voice->envelEnd1--;

            if (voice->envelEnd1 == -1)
            {
                if (voice->envelStep == 4)
                {
                    value = voice->envelPos;

                    voice->envelStart = envelope[int(value + 3)];
                    voice->envelScale = voice->envelStart - envelope[int(value + 1)];
                    voice->envelEnd1 = envelope[int(value + 4)];
                    voice->envelEnd2 = voice->envelEnd1;

                    voice->envelStep = 2;
                }
                else
                {
                    voice->envelStep = 0;
                }
            }
        }

        voice->volume = voice->envelVolume;

        if (sample->volumePtr)
        {
            value = amiga->memory[(sample->volumePtr + voice->volumePos)];
            value = (value * voice->volume) >> 7;
            voice->volume += value;

            if (++voice->volumePos == sample->volumeLen)
            {
                voice->volumePos = sample->volumeStart;
            }
        }

        chan->setVolume((voice->volume * voice->volumeScale) >> 6);

        if (sample->periodPtr)
        {
            value = amiga->memory[(sample->periodPtr + voice->periodPos)];
            value = -((value * voice->period) >> 7);
            if (value < 0) value >>= 1;
            voice->periodMod = voice->period + value;

            if (++voice->periodPos == sample->periodLen)
            {
                voice->periodPos = sample->periodStart;
            }
        }

        if (voice->portaCounter)
        {
            voice->portaPeriod += voice->portaStep;
            voice->portaCounter--;
        }

        chan->setPeriod(voice->periodMod + (voice->portaPeriod >> 16));
    }
    while (voice = voice->next);
}


int RJPlayer::load(void* _data, unsigned long int length, const char* filename)
{
    //AmigaPlayer::load(_data, length, filename);
    unsigned char* stream = static_cast<unsigned char*>(_data);


    ifstream file;
    string str_orgfilename = filename;


    bool usesPrefix = false;
    unsigned found = str_orgfilename.find_last_of("/");
    string filenameOnly = str_orgfilename.substr(found + 1);
    string pathOnly = str_orgfilename.substr(0, found + 1);
    string suffix = filenameOnly.substr(0, 4);
    transform(suffix.begin(), suffix.end(), suffix.begin(), ::tolower);

    if (suffix == "rjp.")
    {
        string samplefilename = pathOnly + "smp." + filenameOnly.substr(4);
        file.open(samplefilename.c_str(), ios::in | ios::binary | ios::ate);
        if (file.is_open())
        {
            usesPrefix = true;
        }
    }


    if (!usesPrefix)
    {
        string str_newfilename = filename;
        int cut = 4;
        do
        {
            str_newfilename = str_orgfilename.substr(0, str_orgfilename.length() - cut) + ".ins";
            file.open(str_newfilename.c_str(), ios::in | ios::binary | ios::ate);
            cut++;
        }
        while (!file.is_open() && cut < str_orgfilename.length() && str_orgfilename.substr(
            str_orgfilename.length() - cut - 1, 1) != "/");
    }


    ifstream::pos_type fileSize;
    char* extra = 0;
    if (file.is_open())
    {
        fileSize = file.tellg();
        extra = new char[fileSize];

        file.seekg(0, ios::beg);

        if (!file.read(extra, fileSize))
        {
            //failed reading
            file.close();
            return -1;
        }
        file.close();
    }
    else
    {
        return -1;
    }

    if (!extra) return -1;

    if (!(extra[0] == 'R' && extra[1] == 'J' && extra[2] == 'P'))
    {
        return -1;
    }


    unsigned int position = 4;


    amiga->store(extra, (fileSize - 4), position, fileSize);


    position = 8;
    unsigned int len = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]) >>
        5;
    position += 4;
    samples = std::vector<RJSample*>(len);

    for (int i = 0; i < len; ++i)
    {
        RJSample* sample;
        sample = new RJSample();
        sample->pointer = readEndian(stream[position], stream[position + 1], stream[position + 2],
                                     stream[position + 3]);
        position += 4;
        sample->periodPtr = readEndian(stream[position], stream[position + 1], stream[position + 2],
                                       stream[position + 3]);
        position += 4;
        sample->volumePtr = readEndian(stream[position], stream[position + 1], stream[position + 2],
                                       stream[position + 3]);
        position += 4;
        sample->envelopePos = readEndian(stream[position], stream[position + 1]);
        position += 2;
        sample->volumeScale = (signed short)readEndian(stream[position], stream[position + 1]);
        position += 2;
        sample->offset = readEndian(stream[position], stream[position + 1]) << 1;
        position += 2;
        sample->length = readEndian(stream[position], stream[position + 1]) << 1;
        position += 2;
        sample->loopPtr = sample->pointer + (readEndian(stream[position], stream[position + 1]) << 1);
        position += 2;
        sample->repeat = readEndian(stream[position], stream[position + 1]) << 1;
        position += 2;
        sample->periodStart = readEndian(stream[position], stream[position + 1]) << 1;
        position += 2;
        sample->periodLen = readEndian(stream[position], stream[position + 1]) << 1;
        position += 2;
        sample->volumeStart = readEndian(stream[position], stream[position + 1]) << 1;
        position += 2;
        sample->volumeLen = readEndian(stream[position], stream[position + 1]) << 1;
        position += 2;
        samples[i] = sample;
    }

    len = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);
    position += 4;
    envelope = std::vector<int>(len);
    for (int i = 0; i < len; ++i)
    {
        envelope[i] = stream[position];
        position++;
    }

    int pos = position;
    position = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]) +
        position;
    position += 4;

    len = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]) >> 2;
    position += 4;
    vector<int> offsets(len);
    for (int i = 0; i < len; ++i)
    {
        offsets[i] = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]) + 1;
        position += 4;
    }

    int i = position;
    position = pos;
    pos = i;

    len = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);
    position += 4;
    songs = std::vector<int>(len);
    m_totalSongs = (len >> 2);

    int flag = 0;
    for (int i = 0; i < len; ++i)
    {
        flag = stream[position];
        position++;
        if (!flag || flag >= offsets.size()) continue;
        songs[i] = offsets[flag];
    }

    position = pos;

    len = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]) >> 2;
    position += 4;
    offsets.resize(len);
    for (i = 0; i < len; ++i)
    {
        offsets[i] = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]) + 1;
        position += 4;
    }

    len = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]) + 1;
    position += 4;
    tracks = std::vector<int>(len);
    flag = 0;

    for (int i = 1; i < len; ++i)
    {
        pos = stream[position];
        position++;

        if (pos == 0)
        {
            flag ^= 1;
        }
        else if (pos > 0)
        {
            if (!flag)
            {
                pos = offsets[pos];
            }
            else
            {
                flag = 0;
            }
        }
        tracks[i] = pos;
    }

    len = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]) + 1;
    position += 4;
    patterns = std::vector<int>(len);
    for (int i = 1; i < len; ++i)
    {
        patterns[i] = stream[position];
        position++;
    }


    m_version = 1;
    format = "Richard Joseph";
    //printData();
    return 1;
}

unsigned char RJPlayer::getSubsongsCount()
{
    return m_totalSongs;
}

void RJPlayer::selectSong(unsigned char subsong)
{
    m_songNumber = subsong;
}

std::vector<BaseSample*> RJPlayer::getSamples()
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

void RJPlayer::printData()
{
    //    for(unsigned int i = 0; i < patterns.size(); i++)
    //    {
    //        AmigaRow* row= patterns[i];
    //        std::cout << "Pattern [" << i << "] note: " << row->note << " sample: " << row->sample << " param: " << row->param << " effect: " << row->effect << "\n";
    //    }
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
    std::flush(std::cout);
}
