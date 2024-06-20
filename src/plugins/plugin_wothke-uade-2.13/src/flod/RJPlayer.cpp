#include "RJPlayer.h"
#include "RJVoice.h"
#include "RJSample.h"

#include <iostream>
#include <fstream>
#include <algorithm> //transform
#include "MyEndian.h"

using namespace std;
const int RJPlayer::PERIODS[36] =
{
    453,480,508,538,570,604,640,678,720,762,
    808,856,226,240,254,269,285,302,320,339,
    360,381,404,428,113,120,127,135,143,151,
    160,170,180,190,202,214
};

RJPlayer::RJPlayer(Amiga* amiga):AmigaPlayer(amiga)
{
    voices    = std::vector<RJVoice*>(4);

    voices[0] = new RJVoice(0);
    voices[0]->next = voices[1] = new RJVoice(1);
    voices[1]->next = voices[2] = new RJVoice(2);
    voices[2]->next = voices[3] = new RJVoice(3);

}
RJPlayer::~RJPlayer()
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


    songs.clear();
    tracks.clear();
    patterns.clear();
    envelope.clear();



}



int RJPlayer::load(void* _data, unsigned long int length, const char* filename)
{
    //AmigaPlayer::load(_data, length, filename);
    unsigned char *stream = static_cast<unsigned char*>(_data);


    ifstream file;
    string str_orgfilename = filename;




    bool usesPrefix = false;
    unsigned found = str_orgfilename.find_last_of("/");
    string filenameOnly = str_orgfilename.substr(found+1);
    string pathOnly = str_orgfilename.substr(0,found+1);
    string suffix = filenameOnly.substr(0,4);
    transform(suffix.begin(), suffix.end(), suffix.begin(), ::tolower);

    if(suffix=="rjp.")
    {
        string samplefilename = pathOnly + "smp." + filenameOnly.substr(4);
        file.open(samplefilename.c_str() , ios::in | ios::binary | ios::ate);
        if(file.is_open())
        {
            usesPrefix=true;
        }
    }



    if(!usesPrefix)
    {
        string str_newfilename = filename;
        int cut=4;
        do
        {
            str_newfilename = str_orgfilename.substr(0,str_orgfilename.length()-cut) + ".ins";
            file.open(str_newfilename.c_str(), ios::in | ios::binary | ios::ate);
            cut++;

        }
        while(!file.is_open() && cut<str_orgfilename.length() && str_orgfilename.substr(str_orgfilename.length()-cut-1,1)!="/");
    }


    ifstream::pos_type fileSize;
    char* extra=0;
    if(file.is_open())
    {
        fileSize = file.tellg();
        extra = new char[fileSize];

        file.seekg(0, ios::beg);

        if(!file.read(extra, fileSize))
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

    if (!extra) return-1;

    if(!(extra[0]=='R' && extra[1]=='J' && extra[2]=='P'))
    {
        return-1;
    }


    unsigned int position=4;


    //amiga->store(extra, (fileSize - 4),position,fileSize);


    position=8;
    unsigned int len = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]) >> 5;position+=4;
    samples = std::vector<RJSample*>(len);

    for (int i = 0; i < len; ++i) {
        RJSample* sample;
        sample = new RJSample();
        sample->pointer     = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);position+=4;
        sample->periodPtr   = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);position+=4;
        sample->volumePtr   = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);position+=4;
        sample->envelopePos = readEndian(stream[position],stream[position+1]);position+=2;
        sample->volumeScale = (signed short)readEndian(stream[position],stream[position+1]);position+=2;
        sample->offset      = readEndian(stream[position],stream[position+1])<< 1;position+=2;
        sample->length      = readEndian(stream[position],stream[position+1])<< 1;position+=2;
        sample->loopPtr     = sample->pointer + (readEndian(stream[position],stream[position+1]) << 1);position+=2;
        sample->repeat      = readEndian(stream[position],stream[position+1])<< 1;position+=2;
        sample->periodStart = readEndian(stream[position],stream[position+1])<< 1;position+=2;
        sample->periodLen   = readEndian(stream[position],stream[position+1])<< 1;position+=2;
        sample->volumeStart = readEndian(stream[position],stream[position+1])<< 1;position+=2;
        sample->volumeLen   = readEndian(stream[position],stream[position+1])<< 1;position+=2;
        samples[i] = sample;
    }

    len = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);position+=4;
    envelope = std::vector<int>(len);
    for (int i = 0; i < len; ++i)
    {
        envelope[i] = stream[position];position++;
    }

    int pos = position;
    position = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]) + position;position+=4;

    len = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]) >> 2;position+=4;
    vector<int> offsets(len);
    for (int i = 0; i < len; ++i)
    {
        offsets[i] = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]) + 1;position+=4;
    }

    int i = position;
    position = pos;
    pos = i;

    len = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);position+=4;
    songs = std::vector<int>(len);
    m_totalSongs = (len >> 2);

    int flag = 0;
    for (int i = 0; i < len; ++i) {
        flag = stream[position];position++;
        if (!flag || flag >= offsets.size()) continue;
        songs[i] = offsets[flag];
    }

    position = pos;

    len = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]) >> 2;position+=4;
    offsets.resize(len);
    for (i = 0; i < len; ++i)
    {
        offsets[i] = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]) + 1;position+=4;
    }

    len = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]) + 1;position+=4;
    tracks = std::vector<int>(len);
    flag = 0;

    for (int i = 1; i < len; ++i) {
        pos = stream[position];position++;

        if (pos == 0) {
            flag ^= 1;
        } else if (pos > 0) {
            if (!flag) {
                pos = offsets[pos];
            } else {
                flag = 0;
            }
        }
        tracks[i] = pos;
    }

    len = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]) + 1;position+=4;
    patterns =  std::vector<int>(len);
    for (int i = 1; i < len; ++i)
    {
        patterns[i] = stream[position];position++;
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


std::vector<BaseSample*> RJPlayer::getSamples()
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

