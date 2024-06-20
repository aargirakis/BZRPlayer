#include "JBPlayer.h"
#include "JBVoice.h"
#include "JBSong.h"
#include "BaseSample.h"

#include <iostream>
#include "MyEndian.h"
using namespace std;

JBPlayer::JBPlayer(Amiga* amiga):AmigaPlayer(amiga)
{

    voices    = std::vector<JBVoice*>(4);

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


int JBPlayer::load(void* _data, unsigned long int _length)
{
    unsigned long int length = _length;
    stream = static_cast<unsigned char*>(_data);
    unsigned int value=0;
    position = 0;
    oldProcess = false;
    value = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);position+=4;
    if (value == 0x48e7f0f0)
    {
        return oldLoader(stream,_length);
    }



    position = 38;
    value = readEndian(stream[position],stream[position+1]);position+=2;
    if (value == 0xa001)
    {
        m_variant = 3;
    }
    else
    {
        position = 50;

        value = readEndian(stream[position],stream[position+1]);position+=2;
        if (value == 0xa001)
        {
            m_variant = 4;
        }
        else
        {
            position = 42;
            value = readEndian(stream[position],stream[position+1]);position+=2;
            if (value != 0xa001) return -1;
            m_variant = 5;
        }
    }


    int wavesHi=0;
    int wavesLo=0xff000;
    int wavesLen=0;
    int samplesLo=0xff000;
    int i = 0;
    while (position < length-4)
    {
        unsigned int valueTemp=0;
        value = readEndian(stream[position],stream[position+1]);position+=2;
        switch (value) {
        case 0x143c:                                                    //move.b [xx,d2]
            value = readEndian(stream[position],stream[position+1]);position+=2;
            valueTemp = readEndian(stream[position],stream[position+1]);position+=2;
            if (valueTemp == 0x7603) command = value;    //moveq [#3,d3]
            break;
        case 0x43fa:                                                    //lea [xx,a1]
            value = position + readEndian(stream[position],stream[position+1]);position+=2;
            valueTemp = readEndian(stream[position],stream[position+1]);position+=2;
            if (valueTemp == 0x4a28) vblock = value;     //tst.b [xx(a0)]
            break;
        case 0x1031:                                                    //move.b [xx(a1,d1.w),d0]
            if (m_variant == 5) {
                position -= 10;

                valueTemp = readEndian(stream[position],stream[position+1]);position+=2;
                if (valueTemp != 0x1231) {                 //move.b [xx(a1,d1.w),d1]
                    position += 8;
                    break;
                }

                position++;
                ptrack = vblock + (signed char)stream[position];position++;
                position += 7;

            } else {
                position -= 4;

                valueTemp = readEndian(stream[position],stream[position+1]);position+=2;
                if (valueTemp != 0x5201) {                 //addq.b [#1,d1]
                    position += 2;
                    break;
                }
                position += 3;
            }

            pblock = vblock + (signed char)stream[position];position++;
            break;
        case 0x323b:                                                    //move.w [xx(pc,d0.w),d1]
            value = position + readEndian(stream[position],stream[position+1]);position+=2;
            valueTemp = readEndian(stream[position],stream[position+1]);position+=2;
            if (valueTemp == 0xc2c2) periods = value;    //mulu.w [d2,d1]
            break;
        case 0x45fa:                                                    //lea [xx,a2]
            value = position + readEndian(stream[position],stream[position+1]);position+=2;
            i = readEndian(stream[position],stream[position+1]);position+=2;

            if (i == 0x1172) {                                            //move.b [(a2,d0.w),xx(a0)]
                ptrack = value;
            } else if (i == 0x1032) {                                     //move.b [(a2,d0.w),d0]
                vtrack = value;
            }
            break;
        case 0xc2fc:                                                    //mulu.w [#10,d1]
            position += 2;

            valueTemp = readEndian(stream[position],stream[position+1]);position+=2;
            if (valueTemp == 0x45fa) {                   //lea [xx,a2]
                samples = vector<BaseSample*>();
                int pos = position;
                position = pos + readEndian(stream[pos],stream[pos+1]);
                i = 0;

                while (position < samplesLo) {
                    BaseSample* sample = new BaseSample();
                    sample->relative = readEndian(stream[position],stream[position+1]);position+=2;
                    sample->pointer  = value = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);position+=4;
                    sample->length   = readEndian(stream[position],stream[position+1]);position+=2;
                    sample->repeat   = (signed short)readEndian(stream[position],stream[position+1]);position+=2;
                    if (i) {
                        if (value < position) {
                            if (value < wavesLo) wavesLo = value;
                            if (value > wavesHi) {
                                wavesHi  = value;
                                wavesLen = sample->length;
                            }
                        } else if (value < samplesLo) {
                            samplesLo = value;
                        }
                    }
                    samples.push_back(sample);
                    i++;
                }

                //amiga->loopLen = 0;
                //amiga->memory.resize(4);

                wavesLen = (wavesHi + wavesLen) - wavesLo;
                position = wavesLo;
                //amiga->store(stream, wavesLen,position,length);

                position = samplesLo;
                //amiga->store(stream, (length - samplesLo),position,length);

                //samples.fixed = true;
                length = wavesLo;
                position = pos;
            }
            break;
        case 0x51c9:                                                    //dbf [d1,xx]
            position += 2;

            valueTemp = readEndian(stream[position],stream[position+1]);position+=2;
            if (valueTemp == 0x43fa) {                   //lea [xx,a1]
                songs = vector<JBSong*>();
                int posTemp = position;
                position += readEndian(stream[posTemp],stream[posTemp+1]);

                while (true) {
                    JBSong* song = new JBSong();
                    song->speed = stream[position];position++;
                    int i = 0;

                    do {
                        song->length[i]  = stream[position];position++;
                        song->restart[i] = stream[position];position++;

                        value = readEndian(stream[position],stream[position+1]);position+=2;
                        if (value > position) break;
                        song->pointer[i++] = value;

                    } while (i < 4);

                    if (i == 4) {
                        songs.push_back(song);
                        int valueTemp = stream[position];position++;
                        if (valueTemp != 0) break;
                    } else {
                        break;
                    }
                }

                if (songs.size() < 1) return-1;
                //m_totalSongs = songs.size();
                //songs.fixed = true;
                position = length;
            }
            break;
        }
    }

    if (ptrack == 0 || pblock == 0 ||
            vtrack == 0 || vblock == 0 || periods == 0) return -1;

    BaseSample* sample = samples[0];
    sample->pointer = sample->loopPtr = 0;
    sample->length  = sample->repeat  = 4;

    int pos = 0;
    int len = samples.size();
    wavesLen += 4;
    wavesLo  -= 4;

    for (int i = 1; i < len; ++i) {
        sample = samples[i];

        if (pos >= wavesLen) {
            sample->pointer -= (samplesLo - wavesLen);
        } else {
            sample->pointer -= wavesLo;
        }

        if (sample->repeat < 0) {
            sample->loopPtr = 0;
            sample->repeat  = 4;
        } else {
            sample->loopPtr = sample->pointer + sample->repeat;
            sample->repeat  = sample->length  - sample->repeat;
        }

        pos = sample->pointer + sample->length;
    }

    int tempVal;
    if (m_variant == 5) {
        position = 0x290;
        tempVal = readEndian(stream[position],stream[position+1]);position+=2;
        if (tempVal == 0xd028) m_variant++;            //add.b [xx(a0),d0]
        position = 0x4f6;
        tempVal = readEndian(stream[position],stream[position+1]);position+=2;
        if (tempVal == 0x1759) m_variant++;            //move.b [(a1)+,xx(a3)]
    }



    m_version = 2;
    format = "Jason Brooke";
    //printData();
    return 1;

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
    while (position < _length-4)
    {
        value = readEndian(stream[position],stream[position+1]);position+=2;

        switch (value) {
        case 0x43fa:                                                    //lea [xx,a1]
            value = position + readEndian(stream[position],stream[position+1]);position+=2;
            valueTemp = readEndian(stream[position],stream[position+1]);position+=2;
            if (valueTemp == 0x7603) pblock = value;     //moveq [#3,d3]
            break;
        case 0x45fa:                                                    //lea [xx,a2]
            value = position + readEndian(stream[position],stream[position+1]);position+=2;
            pos = readEndian(stream[position],stream[position+1]);position+=2;

            if (pos == 0xd4c0) {                                          //adda.w [d0,a2]
                waves = value;
            } else if (pos == 0x103b) {                                   //move.b [xx(pc,d0.w),d0]
                vblock = value;
                position -= 2;
            }
            break;
        case 0x117b:                                                    //move.b [xx(pc,d0.w),xx(a0)]
            value = (position + readEndian(stream[position],stream[position+1])) -256;;position+=2;
            valueTemp = readEndian(stream[position],stream[position+1]);position+=2;
            if (valueTemp == 0x0016) ptrack = value;     //22(a0)
            break;
        case 0x103b:                                                    //move.b [xx(pc,d0.w),d0]
            value = position + readEndian(stream[position],stream[position+1]);position+=2;
            pos = readEndian(stream[position],stream[position+1]);position+=2;

            if (pos == 0xd028) {
                vtrack = value;
                position += 6;

                valueTemp = readEndian(stream[position],stream[position+1]);position+=2;
                if (valueTemp == 0x1171) {                 //move.b [xx(a1,d0.w),xx(a0)]
                    vblock = pblock + readEndian(stream[position],stream[position+1]);position+=2;
                }
            } else if (pos == 0xd4c0) {
                vtrack = value;
            }
            break;
        case 0x323b:                                                    //move.b [xx(pc,d0.w),d1]
            value = position + readEndian(stream[position],stream[position+1]);position+=2;
            valueTemp = readEndian(stream[position],stream[position+1]);position+=2;
            if (valueTemp == 0x0810) periods = value;    //btst [#6,(a0)]
            break;
        case 0x137b:                                                    //move.b [xx(pc,d0.w),xx(a1)]
            value = position + readEndian(stream[position],stream[position+1]);position+=2;
            position += 2;

            valueTemp = readEndian(stream[position],stream[position+1]);position+=2;
            if (valueTemp == 0x41fa) {                   //lea [xx,a0]
                pos = position + readEndian(stream[position],stream[position+1]);position+=2;
                songs = vector<JBSong*>();
                position = value;

                while (position < pos) {
                    JBSong* song = new JBSong();
                    song->speed = stream[position];position++;

                    for (int i = 0; i < 4; ++i) {
                        value = readEndian(stream[position],stream[position+1]);position+=2;

                        if (value) {
                            if (value < lower) lower = value;
                            if (value > upper) upper = value;
                        }
                        song->pointer[i] = value;
                    }

                    songs.push_back(song);
                    signed char byte = stream[position];position++;
                    if (byte != 0) break;
                }

                if (songs.size() < 1) return -1;
                //m_totalSongs = songs.size();
                //songs.fixed = true;
                position = _length;
            }
            break;
        }
    }

    if (ptrack == 0 || pblock == 0 ||
            vtrack == 0 || vblock == 0 || periods == 0) return -1;

    position = lower;
    lower = 0xffff;

    while (position < upper) {
        value = readEndian(stream[position],stream[position+1]);position+=2;
        if (value && (value < lower)) lower = value;
    }

    position = waves;
    //amiga->store(stream, (lower - waves),position,_length);
    //amiga->loopLen = 0;

    m_variant = 2;
    position = 0xd6;

    valueTemp = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);;position+=4;
    if (valueTemp == 0x10bc0040) {                       //move.b [#$40,(a0)]
        m_variant = 0;
    } else {
        position = 0xf4;
        valueTemp = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);;position+=4;
        if (valueTemp == 0x08d00005) m_variant = 1;        //bset [#5,(a0)]
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
