#include "JHPlayer.h"
#include "JHVoice.h"
#include "JHSong.h"

#include "BaseSample.h"
#include <iostream>
#include "MyEndian.h"

const int JHPlayer::PERIODS[84] = {1712,1616,1524,1440,1356,1280,1208,1140,1076,1016,
                                   960, 906, 856, 808, 762, 720, 678, 640, 604, 570,
                                   538, 508, 480, 453, 428, 404, 381, 360, 339, 320,
                                   302, 285, 269, 254, 240, 226, 214, 202, 190, 180,
                                   170, 160, 151, 143, 135, 127, 120, 113, 113, 113,
                                   113, 113, 113, 113, 113, 113, 113, 113, 113, 113,
                                   3424,3232,3048,2880,2712,2560,2416,2280,2152,2032,
                                   1920,1812,6848,6464,6096,5760,5424,5120,4832,4560,
                                   4304,4064,3840,3624};


JHPlayer::JHPlayer(Amiga* amiga):AmigaPlayer(amiga)
{
    voices    = std::vector<JHVoice*>(4);
    voices[0] = new JHVoice(0);
    voices[0]->next = voices[1] = new JHVoice(1);
    voices[1]->next = voices[2] = new JHVoice(2);
    voices[2]->next = voices[3] = new JHVoice(3);
    base=0;
    patterns=0;
    patternLen=0;
    periods=0;
    freqs=0;
    vols=0;
    sampleData=0;
    coso=0;
    m_variant=0;

}
JHPlayer::~JHPlayer()
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


int JHPlayer::load(void* _data, unsigned long int _length)
{
    stream = static_cast<unsigned char*>(_data);
    m_version = 0;
    position = 4;
    base = periods = 0;
    int value=0;
    int tracks=0;
    int songData=0;
    int headers=0;
    int id = 0;
    int len = 0;
    int pos = 0;

    coso = (stream[0]=='C' && stream[1]=='O' && stream[2]=='S' && stream[3]=='O');

    if (coso) {

        for (int i = 0; i < 7; ++i)
        {
            value += readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);
            position+=4;
        }



        position = 47;
        value += stream[position];
        position++;

        switch (value) {

        case 22670:   //Astaroth
        case 18845:
        case 30015:   //Chambers of Shaolin
        case 22469:
        case 3549:    //Over the Net
            m_variant = 1;
            break;
        case 16948:   //Dragonflight
        case 18337:
        case 13704:
            m_variant = 2;
            break;
        case 18548:   //Wings of Death
        case 13928:
        case 8764:
        case 17244:
        case 11397:
        case 14496:
        case 14394:
        case 13578:   //Dragonflight

        case 6524:
            m_variant = 3;
            break;
        default:
            m_variant = 4;
            break;
        }


        m_version = 2;
        position = 4;


        freqs     = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]); position+=4;
        vols     = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]); position+=4;
        patterns    = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]); position+=4;
        tracks      = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]); position+=4;
        songData   = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]); position+=4;
        headers     = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]); position+=4;
        sampleData = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]); position+=4;

        position = 0;
        //All this code to write three values..............
        unsigned char myVal[4];

        myVal[0] = 0x1000000 >> 24;
        myVal[1] = (0x1000000>>16) & 0xFF;
        myVal[2] = (0x1000000>>8) & 0xFF;
        myVal[3] = 0x1000000 & 0xFF;
        for(int p=0;p<4;p++)
        {
            stream[p] = myVal[p];
        }

        position+=4;
        myVal[0] = 0xe1 >> 24;
        myVal[1] = (0xe1>>16) & 0xFF;
        myVal[2] = (0xe1>>8) & 0xFF;
        myVal[3] = 0xe1 & 0xFF;
        for(int p=0;p<4;p++)
        {
            stream[p+4] = myVal[p];
        }
        position+=4;
        myVal[0] = (0xffff>>8) & 0xFF;
        myVal[1] = 0xffff & 0xFF;
        for(int p=0;p<2;p++)
        {
            stream[p+8] = 255;
        }
        position+=2;


        //stream.writeInt(0x1000000);
        //stream.writeInt(0xe1);
        //stream.writeShort(0xffff);


        len = ((sampleData - headers) / 10) - 1;

        if(len<1 || len>255)
        {
            m_version = 0;
            return 0;
        }

        //m_totalSongs = (headers - songData) / 6;
    } else {
        do {
            value = readEndian(stream[position],stream[position+1]);
            position+=2;

            switch (value) {
            case 0x0240:                                                        //andi.w #x,d0
                value = readEndian(stream[position],stream[position+1]);
                position+=2;

                if (value == 0x007f) {                                            //andi.w #$7f,d0
                    position += 2;
                    periods = position +  readEndian(stream[position],stream[position+1]);
                    position+=2;
                }
                break;
            case 0x7002:                                                        //moveq #2,d0
            case 0x7003:                                                        //moveq #3,d0
                //m_channels = (value & 0xff) + 1;
                value = readEndian(stream[position],stream[position+1]);
                position+=2;
                if (value == 0x7600)
                {
                    value = readEndian(stream[position],stream[position+1]);         //moveq #0,d3
                    position+=2;
                }

                if (value == 0x41fa) {                                            //lea x,a0
                    position += 4;
                    base = position + readEndian(stream[position],stream[position+1]);
                    position+=2;
                }
                break;
            case 0x5446:                                                        //"TF"
                value = readEndian(stream[position],stream[position+1]);
                position+=2;

                if (value == 0x4d58) {                                            //"MX"
                    id = position - 4;
                    position = _length;
                }
                break;
            }
        } while (_length-position > 12);

        if (!id || !base || !periods)
        {
            m_version = 0;
            return 0;
        }
        m_version = 1;

        position = id + 4;
        freqs = pos = id + 32;
        value = readEndian(stream[position],stream[position+1]);position+=2;
        vols = (pos += (++value << 6));

        value = readEndian(stream[position],stream[position+1]);position+=2;
        patterns = (pos += (++value << 6));
        value = readEndian(stream[position],stream[position+1]);position+=2;
        position += 2;
        patternLen = readEndian(stream[position],stream[position+1]);position+=2;
        tracks = (pos += (++value * patternLen));

        position -= 4;
        value = readEndian(stream[position],stream[position+1]);position+=2;
        songData = (pos += (++value * 12));

        position = id + 16;
        m_totalSongs = readEndian(stream[position],stream[position+1]);position+=2;
        headers = (pos += (++m_totalSongs * 6));

        len = readEndian(stream[position],stream[position+1]);position+=2;

        sampleData = pos + (len * 30);
    }

    position = headers;

    samples = vector<BaseSample*>(len);
    value = 0;


    for (int i = 0; i < len; ++i) {
        BaseSample* sample = new BaseSample();
        if (!coso)
        {
            const int STRING_LENGTH = 18;
            for(int j = 0;j<STRING_LENGTH;j++)
            {
                if(!stream[position+j])
                {
                    break;
                }
                sample->name+=stream[position+j];
            }
            position+=STRING_LENGTH;
        }

        sample->pointer = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]); position+=4;
        sample->length  = readEndian(stream[position],stream[position+1]) << 1;position+=2;
        if (!coso)
        {
            sample->volume  = readEndian(stream[position],stream[position+1]);position+=2;
        }
        sample->loopPtr = readEndian(stream[position],stream[position+1]) + sample->pointer;position+=2;
        sample->repeat  = readEndian(stream[position],stream[position+1]) << 1;position+=2;

        if (sample->loopPtr & 1) sample->loopPtr--;
        value += sample->length;
        samples[i] = sample;
    }

    position = sampleData;
    //amiga->store(stream, value,position,_length);

    position = songData;
    songs = vector<JHSong*>();
    value = 0;

    for (int i = 0; i < m_totalSongs; ++i) {
        JHSong* song = new JHSong();
        song->pointer = readEndian(stream[position],stream[position+1]);position+=2;
        song->length  = readEndian(stream[position],stream[position+1]) - (song->pointer + 1);position+=2;
        song->speed   = readEndian(stream[position],stream[position+1]);position+=2;

        song->pointer = (song->pointer * 12) + tracks;
        song->length *= 12;
        if (song->length > 12) songs.push_back(song);
    }

    m_totalSongs = songs.size();

    if (!coso) {
        position = 0;
        m_variant = 1;

        do {
            value = readEndian(stream[position],stream[position+1]);position+=2;

            if (value == 0xb03c || value == 0x0c00) {                             //cmp.b #x,d0 | cmpi.b #x,d0
                value = readEndian(stream[position],stream[position+1]);position+=2;

                if (value == 0x00e5 || value == 0x00e6 || value == 0x00e9) {        //effects
                    m_variant = 2;
                    break;
                }
            } else if (value == 0x4efb) {                                         //jmp $(pc,d0.w)
                m_variant = 3;
                break;
            }
        } while (position < id);
    }



    m_version = 1;
    if(!coso)
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


std::vector<BaseSample*> JHPlayer::getSamples()
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

