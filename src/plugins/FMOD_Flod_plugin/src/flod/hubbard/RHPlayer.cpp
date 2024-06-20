#include "RHPlayer.h"
#include "RHVoice.h"
#include "RHSample.h"
#include "RHSong.h"
#include "AmigaChannel.h"
#include <iostream>
#include "MyEndian.h"

RHPlayer::RHPlayer(Amiga* amiga):AmigaPlayer(amiga)
{
    voices    = std::vector<RHVoice*>(4);
    voices[3] = new RHVoice(3);
    voices[3]->next = voices[2] = new RHVoice(2);
    voices[2]->next = voices[1] = new RHVoice(1);
    voices[1]->next = voices[0] = new RHVoice(0);
}
RHPlayer::~RHPlayer()
{
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
    for(unsigned int i = 0; i < songs.size(); i++)
    {
        if(songs[i]) delete songs[i];
    }
    songs.clear();
}
void RHPlayer::initialize()
{
    AmigaPlayer::initialize();
    RHVoice* voice = voices[3];

    song = songs[m_songNumber];
    complete = 0;

    RHSample* sample;
    for (int i = 0; i < samples.size(); ++i) {
        sample = samples[i];

        if (!sample->wave.empty()) {
            for (int j = 0; j < sample->length; ++j)
            {
                amiga->memory[int(sample->pointer + j)] = sample->wave[j];
            }
        }
    }

    do
    {
        voice->initialize();
        voice->channel = amiga->channels[voice->index];

        voice->trackPtr = song->tracks[voice->index];
        voice->trackPos = 4;
        position = voice->trackPtr;
        voice->patternPos = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);
        position+=4;
        complete += (1 << voice->index);

     } while (voice = voice->next);
}
void RHPlayer::process()
{
    int loop = 0;
    int value=0;
    AmigaChannel* chan;
    RHSample* sample;
    RHVoice* voice = voices[3];
    do {
        chan = voice->channel;
        position = voice->patternPos;
        sample = voice->sample;

        if (!voice->busy) {
            voice->busy = 1;

            if (sample->loopPtr == 0) {
                chan->pointer = amiga->loopPtr;
                chan->length  = amiga->loopLen;
            } else if (sample->loopPtr > 0) {
                chan->pointer = sample->pointer + sample->loopPtr;
                chan->length  = sample->length  - sample->loopPtr;
            }
        }

        if (--voice->tick == 0) {
            voice->flags = 0;
            loop = 1;

            do {
                value = (signed char)stream[position];
                position++;

                if (value < 0) {

                    switch (value) {
                    case -121:
                        if (m_variant == 3)
                        {
                            voice->volume = stream[position];
                            position++;
                        }
                        break;
                    case -122:
                        if (m_variant == 4)
                        {
                            voice->volume = stream[position];
                            position++;
                        }
                        break;
                    case -123:
                        if (m_variant > 1) amiga->setComplete(1);
                        break;
                    case -124:
                        position = voice->trackPtr + voice->trackPos;
                        value = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);
                        position+=4;
                        voice->trackPos += 4;

                        if (!value) {
                            position = voice->trackPtr;
                            value = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);
                            position+=4;
                            voice->trackPos = 4;

                            complete &= ~(1 << voice->index);
                            if (!complete)
                            {
                                amiga->setComplete(1);
                            }

                        }

                        position = value;
                        break;
                    case -125:
                        if (m_variant == 4) voice->flags |= 4;
                        break;
                    case -126:
                        value = (signed char)stream[position];
                        position++;
                        voice->tick = song->speed * value;
                        voice->patternPos = position;
                        chan->pointer = amiga->loopPtr;
                        chan->length  = amiga->loopLen;
                        loop = 0;
                        break;
                    case -127:
                        value = (signed char)stream[position];
                        position++;
                        voice->portaSpeed = value;
                        voice->flags |= 1;
                        break;
                    case -128:
                        value = (signed char)stream[position];
                        position++;
                        if (value < 0) value = 0;
                        sample = voice->sample = samples[value];
                        voice->vibratoPtr = vibrato + sample->vibrato;
                        voice->vibratoPos = voice->vibratoPtr;
                        break;
                    }
                } else {
                    voice->tick = song->speed * value;
                    value = (signed char)stream[position];
                    position++;
                    voice->note = value;

                    voice->patternPos = position;

                    voice->synthPos = sample->loPos;
                    voice->vibratoPos = voice->vibratoPtr;

                    chan->pointer = sample->pointer;

                    chan->length  = sample->length;
                    chan->setVolume((voice->volume) ? voice->volume : sample->volume);

                    position = periods + (voice->note << 1);

                    unsigned short shortvalue = readEndian(stream[position],stream[position+1]);
                    position+=2;
                    voice->period = (shortvalue * sample->relative)>> 10;
                    chan->setPeriod(voice->period);

                    chan->setEnabled(1);
                    voice->busy = 0;
                    loop = 0;

                }
            } while (loop);
        } else {
            if (voice->tick == 1) {
                if (m_variant != 4 || !(voice->flags & 4))
                {
                    chan->setEnabled(0);
                }
            }

            if (voice->flags & 1)
            {
                chan->setPeriod(voice->period += voice->portaSpeed);
            }
            if (sample->divider) {
                position = voice->vibratoPos;
                value = (signed char)stream[position];
                position++;

                if (value == -124) {
                    position = voice->vibratoPtr;
                    value = (signed char)stream[position];
                    position++;
                }

                voice->vibratoPos = position;
                value = (voice->period / sample->divider) * value;
                chan->setPeriod(voice->period + value);
            }
        }

        if (sample->hiPos) {
            value = 0;

            if (voice->flags & 2) {
                voice->synthPos--;

                if (voice->synthPos <= sample->loPos) {
                    voice->flags &= -3;
                    value = 60;
                }
            } else {
                voice->synthPos++;

                if (voice->synthPos > sample->hiPos) {
                    voice->flags |= 2;
                    value = 60;
                }
            }

            amiga->memory[sample->pointer + voice->synthPos] = value;
        }
    } while (voice = voice->next);
}


int RHPlayer::load(void* _data, unsigned long int _length)
{
    m_version = 0;
    stream = static_cast<unsigned char*>(_data);

    position = 44;
    unsigned int value=0;
    unsigned int samplesData=0;
    unsigned int samplesHeader=0;
    unsigned int wavesPointer=0;
    unsigned int songsHeader=0;
    unsigned int samplesLen=0;
    unsigned int wavesHeader=0;

    value = readEndian(stream[position],stream[position+1]);
    position+=2;

    while (position < 1024) {
        value = readEndian(stream[position],stream[position+1]);
        position+=2;

        if (value == 0x7e10 || value == 0x7e20) {                               //moveq #16,d7 || moveq #32,d7
            value = readEndian(stream[position],stream[position+1]);
            position+=2;
            if (value == 0x41fa) {                                                //lea $x,a0
                int i = position + readEndian(stream[position],stream[position+1]);
                position+=2;
                value = readEndian(stream[position],stream[position+1]);
                position+=2;

                if (value == 0xd1fc) {                                              //adda.l
                    samplesData = i + readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);
                    position+=4;
                    amiga->loopLen = 64;
                    position += 2;
                } else {
                    samplesData = i;
                    amiga->loopLen = 512;
                }

                samplesHeader = position + readEndian(stream[position],stream[position+1]);
                position+=2;
                value = stream[position];
                position++;
                if (value == 0x72)
                {
                    samplesLen = stream[position];           //moveq #x,d1
                    position++;
                }
            }
        } else if (value == 0x51c9) {                                           //dbf d1,x
            position += 2;
            value = readEndian(stream[position],stream[position+1]);
            position+=2;

            if (value == 0x45fa) {                                                //lea $x,a2
                wavesPointer = position + readEndian(stream[position],stream[position+1]);
                position+=2;
                position+=2;

                while (true) {
                    value =  readEndian(stream[position],stream[position+1]);
                    position+=2;

                    if (value == 0x4bfa) {                                            //lea $x,a5
                        wavesHeader = position + readEndian(stream[position],stream[position+1]);
                        position+=2;
                        break;
                    }
                }
            }
        } else if (value == 0xc0fc) {                                           //mulu.w #x,d0
            position += 2;
            value = readEndian(stream[position],stream[position+1]);
            position+=2;

            if (value == 0x41eb)                                                  //lea $x(a3),a0
            {
                songsHeader = readEndian(stream[position],stream[position+1]);
                position+=2;
            }
        } else if (value == 0x346d) {                                           //movea.w x(a5),a2
            position += 2;
            value = readEndian(stream[position],stream[position+1]);
            position+=2;

            if (value == 0x49fa)                                                  //lea $x,a4
            {
                vibrato = position + readEndian(stream[position],stream[position+1]);
                position += 2;
            }
        } else if (value == 0x4240) {                                           //clr.w d0
            value = readEndian(stream[position],stream[position+1]);
            position += 2;

            if (value == 0x45fa) {                                                //lea $x,a2
                periods = position + readEndian(stream[position],stream[position+1]);
                position += 2;
            }
        }
    }
    if (!samplesHeader || !samplesData || !samplesLen || !songsHeader) return 0;

    position = samplesData;

    samples = std::vector<RHSample*>();
    samplesLen++;

    for (int i = 0; i < samplesLen; ++i) {
        RHSample* sample = new RHSample();
        sample->length   = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);
        position+=4;
        sample->relative = 3579545 / readEndian(stream[position],stream[position+1]);
        position+=2;
        sample->pointer  = amiga->store(stream, sample->length,position,_length);
        samples.push_back(sample);
    }

    position = samplesHeader;

    for (int i = 0; i < samplesLen; ++i) {
        RHSample* sample = samples[i];
        position += 4;
        sample->loopPtr = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);
        position+=4;
        position += 6;
        sample->volume = readEndian(stream[position],stream[position+1]);
        position+=2;

        if (wavesHeader) {
            sample->divider = readEndian(stream[position],stream[position+1]);position+=2;
            sample->vibrato = readEndian(stream[position],stream[position+1]);position+=2;
            sample->hiPos   = readEndian(stream[position],stream[position+1]);position+=2;
            sample->loPos   = readEndian(stream[position],stream[position+1]);position+=2;
            position += 8;
        }
    }

    if (wavesHeader) {
        position = wavesHeader;
        int i = (wavesHeader - samplesHeader) >> 5;
        int len = i + 3;
        m_variant = 1;

        if (i >= samplesLen) {
            for (int j = samplesLen; j < i; ++j)
            {
                samples.push_back(new RHSample());
            }
        }


        for (; i < len; ++i) {
            RHSample* sample = new RHSample();
            position += 4;
            sample->loopPtr   = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);position+=4;
            sample->length    = readEndian(stream[position],stream[position+1]);position+=2;
            sample->relative  = readEndian(stream[position],stream[position+1]);position+=2;

            position += 2;
            sample->volume  = readEndian(stream[position],stream[position+1]);position+=2;
            sample->divider = readEndian(stream[position],stream[position+1]);position+=2;
            sample->vibrato = readEndian(stream[position],stream[position+1]);position+=2;
            sample->hiPos   = readEndian(stream[position],stream[position+1]);position+=2;
            sample->loPos   = readEndian(stream[position],stream[position+1]);position+=2;

            int pos = position;
            position = wavesPointer;
            position = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);


            sample->pointer = amiga->memory.size();
            amiga->memory.resize(amiga->memory.size() + sample->length);

            sample->wave = std::vector<signed char>(sample->length);

            for (int j = 0; j < sample->length; ++j)
            {
                sample->wave[j] = stream[position];
                position++;
            }

            samples.push_back(sample);
            wavesPointer += 4;
            position = pos;
        }
    }


    position = songsHeader;
    songs = std::vector<RHSong*>();
    value = 0x10000;

    while (true) {
        RHSong* song = new RHSong();
        position++;
        song->tracks =  std::vector<int>(4);
        song->speed = stream[position];
        position++;

        for (int i = 0; i < 4; ++i) {
            int j = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);
            position+=4;
            if (j < value) value = j;
            song->tracks[i] = j;
        }

        songs.push_back(song);
        if ((value - position) < 18) break;
    }

    m_totalSongs = songs.size();

    position = 0x160;

    while (position < 0x200) {
        value = readEndian(stream[position],stream[position+1]);
        position+=2;

        if (value == 0xb03c) {                                                  //cmp.b #x,d0
            value = readEndian(stream[position],stream[position+1]);
            position+=2;
            if (value == 0x0085) {                                                //-123
                variant = 2;
            } else if (value == 0x0086) {                                         //-122
                variant = 4;
            } else if (value == 0x0087) {                                         //-121
                variant = 3;
            }
        }
    }

    m_version = 1;
    format = "Hubbard";
    //printData();
    return 1;
}
void RHPlayer::printData()
{
//    for(unsigned int i = 0; i < samples.size(); i++)
//    {
//        RHSample* sample = samples[i];
//        if(sample)
//        {
//            std::cout << "Sample [" << i << "] length: " << sample->length << " loop: " << sample->loop << " loopPtr: " << sample->loopPtr << " name: " << sample->name << " pointer: " << sample->pointer << " repeat: " << sample->repeat << " volume: " << sample->volume << "\n";
//            std::cout << "Sample [" << i << "] relative: " << sample->relative << " divider: " << sample->divider << " vibrato: " << sample->vibrato << " hiPos: " << sample->hiPos << " loPos: " << sample->loPos << "\n";
//            for(unsigned int j = 0; j < sample->wave.size(); j++)
//            {
//                std::cout << "Sample [" << i << "] wave [" << j << "] " << (int)sample->wave[j] << "\n";
//            }
//        }
//    }

//    for(unsigned int i = 0; i < songs.size(); i++)
//    {
//        RHSong* song = songs[i];
//        if(song)
//        {
//            std::cout << "Song [" << i << "] speed: " << song->speed << "\n";

//            for(unsigned int j = 0; j < song->tracks.size(); j++)
//            {
//                std::cout << "Song [" << i << "] track [" << j << "] " << (int)song->tracks[j] << "\n";
//            }
//        }
//    }

//    std::cout << "Periods: " << periods << "\n";
//    std::cout << "Vibrato: " << vibrato << "\n";
//    std::cout << "Variant: " << variant << "\n";


}
unsigned char RHPlayer::getSubsongsCount()
{
    return songs.size();
}
void RHPlayer::selectSong(unsigned char subsong)
{
    m_songNumber = subsong;
}
std::vector<BaseSample*> RHPlayer::getSamples()
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
    return samp;
}

