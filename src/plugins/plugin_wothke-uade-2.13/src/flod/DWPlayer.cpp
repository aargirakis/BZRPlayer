#include "DWPlayer.h"
#include "DWVoice.h"
#include "BaseSample.h"
#include "DWSong.h"

#include <iostream>
#include "MyEndian.h"



DWPlayer::DWPlayer(Amiga* amiga):AmigaPlayer(amiga)
{
    song=0;
    songVol=0;
    master =0;
    periods=0;
    freqs=0;
    vols=0;
    transpose=0;
    slower =0;
    slowerCtr=0;
    delaySpeed   =0;
    delayCtr =0;
    fadeSpeed=0;
    fadeCtr  =0;
    wave=0;
    waveCenter   =0;
    waveLo =0;
    waveHi =0;
    waveDir=0;
    waveLen=0;
    wavePos=0;
    waveRateNeg  =0;
    waveRatePos  =0;
    active =0;
    complete =0;
    base   =0;
    com2   =0;
    com3   =0;
    com4   =0;
    readMix=USHORT;
    readLen=0;
    voices    = vector<DWVoice*>(4);
    voices[0] = new DWVoice(0,1);
    voices[1] = new DWVoice(1,2);
    voices[2] = new DWVoice(2,4);
    voices[3] = new DWVoice(3,8);
}
DWPlayer::~DWPlayer()
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


int DWPlayer::load(void* _data, unsigned long int _length)
{
    m_version = 0;
    int value = 0;
    int info=0;
    int size = 10;
    int headers = 0;
    int flag = 0;
    int index = 0;
    int lower = 0;
    int total = 0;

    stream = static_cast<unsigned char*>(_data);

    master  = 64;
    readMix = USHORT;
    readLen = 2;
    variant = 0;
    position=0;

    unsigned short val = readEndian(stream[position],stream[position+1]);position+=2;

    if (val == 0x48e7) {                               //movem.l
        position = 4;
        val = readEndian(stream[position],stream[position+1]);position+=2;
        if (val != 0x6100) return 0;                       //bsr.w

        position += readEndian(stream[position],stream[position+1]);
        position+=2;
        m_variant = 30;
    } else {
        position = 0;
    }

    do {
        value = readEndian(stream[position],stream[position+1]);position+=2;

        switch (value) {
        case 0x47fa:                                                          //lea x,a3
            base = position + (signed short)readEndian(stream[position],stream[position+1]);position+=2;
            break;
        case 0x6100:                                                          //bsr.w
            position += 2;
            info = position;

            val = readEndian(stream[position],stream[position+1]);position+=2;
            if (val == 0x6100)                           //bsr.w
                info = position + readEndian(stream[position],stream[position+1]);position+=2;
            break;
        case 0xc0fc:                                                          //mulu.w #x,d0
            size = readEndian(stream[position],stream[position+1]);position+=2;

            if (size == 18) {
                readMix = UINT;
                readLen = 4;
            } else {
                m_variant = 10;
            }

            val = readEndian(stream[position],stream[position+1]);position+=2;
            if (val == 0x41fa)
                headers = position +  readEndian(stream[position],stream[position+1]);position+=2;

            val = readEndian(stream[position],stream[position+1]);position+=2;
            if (val == 0x1230) flag = 1;
            break;
        case 0x1230:                                                          //move.b (a0,d0.w),d1
            position -= 6;

            val = readEndian(stream[position],stream[position+1]);position+=2;
            if (val == 0x41fa) {
                headers = position + readEndian(stream[position],stream[position+1]);position+=2;
                flag = 1;
            }

            position += 4;
            break;
        case 0xbe7c:                                                          //cmp.w #x,d7
            m_channels = readEndian(stream[position],stream[position+1]);position+=2;
            position += 2;

            val = readEndian(stream[position],stream[position+1]);position+=2;
            if (val == 0x377c)
                master = readEndian(stream[position],stream[position+1]);position+=2;
            break;
        }

        if (_length-position < 20) return 0;
    } while (value != 0x4e75);                                          //rts

    index = position;
    songs = vector<DWSong*>();
    lower = 0x7fffffff;

    position = headers;


    do {
        DWSong* song = new DWSong();
        song->tracks = vector<int>(m_channels);

        if (flag) {
            song->speed = stream[position];position++;
            song->delay = stream[position];position++;
        } else {
            song->speed = readEndian(stream[position],stream[position+1]);position+=2;
        }

        if (song->speed > 255) break;

        for (int i = 0; i < m_channels; ++i) {
            if(readMix==USHORT)
            {
                value = base + readEndian(stream[position],stream[position+1]);
                position+=2;
            }
            else if(readMix==UINT)
            {
                value = base + readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);
                position+=4;
            }
            else
            {
                return 0;
            }
            if (value < lower) lower = value;
            song->tracks[i] = value;
        }

        songs.push_back(song);
        if ((lower - position) < size) break;
    } while (true);

    if (songs.empty()) return 0;

    m_totalSongs = songs.size();

    position = info;

    val = readEndian(stream[position],stream[position+1]);position+=2;

    if (val != 0x4a2b) return 0;                         //tst.b x(a3)
    headers = size = 0;
    wave = 0;


    int pos = 0;
    do {
        value = readEndian(stream[position],stream[position+1]);position+=2;

        switch (value) {
        case 0x4bfa:
            if (headers) break;
            info = position + (signed short)readEndian(stream[position],stream[position+1]);position+=2;
            position++;
            total = stream[position];position++;

            position -= 10;
            value = readEndian(stream[position],stream[position+1]);position+=2;
            pos = position;

            if (value == 0x41fa || value == 0x207a) {                           //lea x,a0 | movea.l x,a0
                headers = position + readEndian(stream[position],stream[position+1]);position+=2;
            } else if (value == 0xd0fc) {                                       //adda.w #x,a0
                headers = (64 + readEndian(stream[position],stream[position+1]));position+=2;
                position -= 18;
                headers += (position + readEndian(stream[position],stream[position+1]));position+=2;
            }

            position = pos;
            break;
        case 0x84c3:                                                          //divu.w d3,d2
            if (size) break;
            position += 4;
            value = readEndian(stream[position],stream[position+1]);position+=2;

            if (value == 0xdafc) {                                              //adda.w #x,a5
                size = readEndian(stream[position],stream[position+1]);position+=2;
            } else if (value == 0xdbfc) {                                       //adda.l #x,a5
                size = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);position+=4;
            }

            if (size == 12 && m_variant < 30) m_variant = 20;

            pos = position;
            samples = vector<BaseSample*>(++total);
            position = headers;

            for (int i = 0; i < total; ++i) {
                BaseSample* sample = new BaseSample();
                sample->length   = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);position+=4;
                sample->relative = 3579545 / readEndian(stream[position],stream[position+1]);position+=2;
                sample->pointer  = amiga->store(stream, sample->length,position,_length);

                value = position;
                position = info + (i * size) + 4;
                sample->loopPtr = (signed int)readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);position+=4;

                if (!m_variant) {
                    position += 6;
                    sample->volume = readEndian(stream[position],stream[position+1]);position+=2;
                } else if (m_variant == 10) {
                    position += 4;
                    sample->volume = readEndian(stream[position],stream[position+1]);position+=2;
                    sample->finetune = (signed char)stream[position];position++;
                }

                position = value;
                samples[i] = sample;
            }

            amiga->loopLen = 64;
            _length = headers;
            position = pos;
            break;
        case 0x207a:                                                          //movea.l x,a0
            value = position + (signed short)readEndian(stream[position],stream[position+1]);position+=2;

            val = readEndian(stream[position],stream[position+1]);position+=2;
            if (val != 0x323c) {                         //move.w #x,d1
                position -= 2;
                break;
            }

            wave = samples[int((value - info) / size)];
            waveCenter = (readEndian(stream[position],stream[position+1]) + 1) << 1;position+=2;

            position += 2;
            waveRateNeg = (signed char)stream[position];position++;
            position += 12;
            waveRatePos = (signed char)stream[position];position++;
            break;
        case 0x046b:                                                          //subi.w #x,x(a3)
        case 0x066b:                                                          //addi.w #x,x(a3)
            total = readEndian(stream[position],stream[position+1]);position+=2;
            BaseSample* sample = samples[int((readEndian(stream[position],stream[position+1]) - info) / size)];position+=2;

            if (value == 0x066b) {
                sample->relative += total;
            } else {
                sample->relative -= total;
            }
            break;
        }
    } while (value != 0x4e75);                                          //rts

    if (samples.empty()) return 0;
    position = index;

    periods = freqs = vols = slower = 0;

    com2 = 0xb0;
    com3 = 0xa0;
    com4 = 0x90;

    do {
        value = readEndian(stream[position],stream[position+1]);position+=2;

        switch (value) {
        case 0x47fa:                                                          //lea x,a3
            position += 2;
            val = readEndian(stream[position],stream[position+1]);position+=2;
            if (val != 0x4a2b) break;                    //tst.b x(a3)

            pos = position;
            position += 4;
            value = readEndian(stream[position],stream[position+1]);position+=2;

            if (value == 0x103a) {                                              //move.b x,d0
                position += 4;

                val = readEndian(stream[position],stream[position+1]);position+=2;
                if (val == 0xc0fc) {                       //mulu.w #x,d0
                    value = readEndian(stream[position],stream[position+1]);position+=2;
                    total = songs.size();
                    for (int i = 0; i < total; ++i) songs[i]->delay *= value;
                    position += 6;
                }
            } else if (value == 0x532b) {                                       //subq.b #x,x(a3)
                position -= 8;
            }

            value = readEndian(stream[position],stream[position+1]);position+=2;

            if (value == 0x4a2b) {                                              //tst.b x(a3)
                position = base + readEndian(stream[position],stream[position+1]);
                slower = (signed short)stream[position];position++;
            }

            position = pos;
            break;
        case 0x0c6b:                                                          //cmpi.w #x,x(a3)
            position -= 6;
            value = readEndian(stream[position],stream[position+1]);position+=2;

            if (value == 0x546b || value == 0x526b) {                           //addq.w #2,x(a3) | addq.w #1,x(a3)
                position += 4;
                waveHi = wave->pointer + readEndian(stream[position],stream[position+1]);position+=2;
            } else if (value == 0x556b || value == 0x536b) {                    //subq.w #2,x(a3) | subq.w #1,x(a3)
                position += 4;
                waveLo = wave->pointer + readEndian(stream[position],stream[position+1]);position+=2;
            }

            waveLen = (value < 0x546b) ? 1 : 2;
            break;
        case 0x7e00:                                                          //moveq #0,d7
        case 0x7e01:                                                          //moveq #1,d7
        case 0x7e02:                                                          //moveq #2,d7
        case 0x7e03:                                                          //moveq #3,d7
            active = value & 0x0f;
            total = m_channels - 1;

            if (active) {
                voices[0]->next = 0;
                for (int i = total; i > 0;) voices[i]->next = voices[--i];
            } else {
                voices[total]->next = 0;
                for (int i = 0; i < total;) voices[i]->next = voices[++i];
            }
            break;
        case 0x0c68:                                                          //cmpi.w #x,x(a0)
            position += 22;
            val = readEndian(stream[position],stream[position+1]);position+=2;
            if (val == 0x0c11) m_variant = 40;
            break;
        case 0x322d:                                                          //move.w x(a5),d1
            pos = position;
            value = readEndian(stream[position],stream[position+1]);position+=2;

            if (value == 0x000a || value == 0x000c) {                           //10 | 12
                position -= 8;

                val = readEndian(stream[position],stream[position+1]);position+=2;
                if (val == 0x45fa)                         //lea x,a2
                    periods = position + readEndian(stream[position],stream[position+1]);position+=2;
            }

            position = pos + 2;
            break;
        case 0x0400:                                                          //subi.b #x,d0
        case 0x0440:                                                          //subi.w #x,d0
        case 0x0600:                                                          //addi.b #x,d0
            value = readEndian(stream[position],stream[position+1]);position+=2;

            if (value == 0x00c0 || value == 0x0040) {                           //192 | 64
                com2 = 0xc0;
                com3 = 0xb0;
                com4 = 0xa0;
            } else if (value == com3) {
                position += 2;

                val = readEndian(stream[position],stream[position+1]);position+=2;
                if (val == 0x45fa) {                       //lea x,a2
                    vols = position + readEndian(stream[position],stream[position+1]);position+=2;
                    if (m_variant < 40) m_variant = 30;
                }
            } else if (value == com4) {
                position += 2;

                val = readEndian(stream[position],stream[position+1]);position+=2;
                if (val == 0x45fa)                         //lea x,a2
                    freqs = position + readEndian(stream[position],stream[position+1]);position+=2;
            }
            break;
        case 0x4ef3:                                                          //jmp (a3,a2.w)
            position += 2;
        case 0x4ed2:
            //jmp a2
            lower = position;
            position -= 10;
            position += readEndian(stream[position],stream[position+1]);
            pos = position;                                              //jump table address
            position = pos + 2;                                          //effect -126
            position = base + readEndian(stream[position],stream[position+1]) + 10;
            val = readEndian(stream[position],stream[position+1]);position+=2;
            if (val == 0x4a14) m_variant = 41;             //tst.b (a4)

            position = pos + 16;                                         //effect -120
            value = base + readEndian(stream[position],stream[position+1]);position+=2;
            if (value > lower && value < pos) {
                position = value;
                value = readEndian(stream[position],stream[position+1]);position+=2;

                if (value == 0x50e8) {                                            //st x(a0)
                    m_variant = 21;
                } else if (value == 0x1759) {                                     //move.b (a1)+,x(a3)
                    m_variant = 11;
                }
            }

            position = pos + 20;                                         //effect -118
            value = base + readEndian(stream[position],stream[position+1]);position+=2;

            if (value > lower && value < pos) {
                position = value + 2;
                val = readEndian(stream[position],stream[position+1]);position+=2;
                if (val != 0x4880) m_variant = 31;           //ext.w d0
            }
            position = pos + 26;                                         //effect -115

            value = readEndian(stream[position],stream[position+1]);position+=2;
            if (value > lower && value < pos) m_variant = 32;

            if (freqs) position = _length;
            break;
        }
    } while (_length-position > 16);

    if (!periods) return 0;
    com2 -= 256;
    com3 -= 256;
    com4 -= 256;

    m_version = 1;
    format = "David Whittaker";

    return 1;
}


vector<BaseSample*> DWPlayer::getSamples()
{
    vector<BaseSample*>samp (samples.size());
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

