#include "BDPlayer.h"
#include "BDVoice.h"
#include "BDSample.h"
#include "AmigaChannel.h"
#include <iostream>
#include <fstream>
#include "MyEndian.h"

using namespace std;


BDPlayer::BDPlayer(Amiga* amiga):AmigaPlayer(amiga)
{
    voices    = std::vector<BDVoice*>(4);

    voices[0] = new BDVoice(0);
    voices[0]->next = voices[1] = new BDVoice(1);
    voices[1]->next = voices[2] = new BDVoice(2);
    voices[2]->next = voices[3] = new BDVoice(3);
    commands=0;
    complete = 0;
    periods = 0;
    fadeStep = 0;

}
BDPlayer::~BDPlayer()
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
    banks.clear();
}
void BDPlayer::initialize()
{
    AmigaPlayer::initialize();

    BDVoice* voice = voices[0];
    int len = banks.size() >> 2;

    do {
        voice->initialize();
        voice->channel  = amiga->channels[voice->index];
        voice->sample   = samples[0];
        voice->bank     = voice->index * len;
        voice->trackPos = songs[((m_songNumber << 2) + voice->index)];
    } while (voice = voice->next);
}
void BDPlayer::process()
{
    BDVoice* voice = voices[0];
    int loop = 0;
    int temp = 0;
    int pos = 0;
    int value = 0;
    AmigaChannel* chan;
    complete = 0;

    do {

        complete |= voice->s1byte22;

        if (!voice->state) continue;

        chan = voice->channel;

        if (voice->s2word14 > 0) voice->s2word14--;

        if (voice->s2word16 != -1) {
            if (voice->s2word16) {
                voice->s2word22 += voice->s2word18;
                voice->s2word16--;
            }

            if (!voice->s2word16) {
                voice->s2word22 += voice->s2word24;

                if (--voice->s2word20 == 0) {
                    if (voice->sample->word20) {
                        voice->s2word20 = voice->sample->word20;
                        voice->s2word24 = -voice->s2word24;
                    }
                }
            }
        }

        if (m_variant == 1) {
            if (voice->v1word14) {
                if (--voice->v1word18 == 0) {
                    voice->v1word18 = voice->v1word14;
                    voice->volume += voice->v1word16;
                }
            }
        }

        if (voice->type == 1) {
            if (voice->state < 0x8000) {
                if (--voice->state == 0) {
                    voice->state = 0xffff;
                    chan->pointer = amiga->loopPtr;
                    chan->length  = amiga->loopLen;
                }
            } else {
                if (voice->s2word14 == 1) voice->state = 0x8000;
            }
        } else if (voice->type == 2) {
            if (voice->state < 0x8000) {
                if (--voice->state == 0) {
                    voice->state = 0xffff;
                    chan->pointer = voice->sample->loopPtr;
                    chan->length  = voice->sample->repeat;
                }
            } else {
                if (voice->s2word14 == 1) voice->type = 0;
            }
        } else {
            voice->state = 0x8000;
            voice->volume += voice->sample->word14;
        }

        if (voice->volume > 0) {
            chan->setPeriod((voice->period + voice->s2word22 + voice->s2word10) & 0xffff);
            chan->setVolume(voice->volume);
            chan->setEnabled(1);
        } else {
            voice->state = 0;
            chan->reset();
            chan->setEnabled(0);
        }

    } while (voice = voice->next);

    if (!complete) return;
    voice = voices[0];

    do {
        if (!voice->s1byte22) continue;
        chan = voice->channel;
        loop = 1;

        if (m_variant) {
            if (voice->s1byte35) {
                if (voice->s1byte35 > 0x7f) {
                    temp = voice->period * (voice->s1long38 & 0xffff);
                    temp = (temp >> 16) & 0xffff;
                    temp += voice->period * ((voice->s1long38 >> 16) & 0xffff);
                    temp -= voice->period;
                    voice->s1long38 = temp / voice->s1byte37;
                    voice->s1byte35 &= 0x7f;
                }

                if (voice->s1byte36) {
                    voice->s1byte36--;
                } else if (voice->s1byte37) {
                    voice->s1byte37--;
                    voice->s2word10 += voice->s1long38;
                }
            }

            if (voice->s1byte48) {
                if (voice->s1byte51) {
                    if (--voice->s1byte50 == 0) {
                        voice->s1byte51--;
                        voice->s1byte50 = voice->s1byte49;
                        voice->s1word54 += voice->s1word52;
                        value = voice->s1word54 + voice->sample->volume;

                        if (value < 0) {
                            value = voice->s1byte51 = 0;
                        } else if (value > 64) {
                            value = 64;
                        }

                        voice->volume = value;
                        setSample(voice, voice->s1byte50);
                    }
                }
            }
        }

        do {

            if (voice->s1byte20) {

                if (--voice->s1byte18 != 0) {
                    voice->patternPos = voice->patternPtr;
                    voice->s1byte20 = 0;
                    position = voice->patternPos;

                } else {
                    voice->s1byte18 = 1;
                    position = voice->trackPos;

                    do {
                        value = stream[position];position++;

                        if (value == 0xff) {
                            if (!voice->state || voice->state == 0x8000) voice->s1byte22 = 0;
                            loop = 0;
                        } else if (value == 0xfe) {
                            voice->s1byte19 = (signed char)stream[position];position++;
                        } else if (m_variant < 2) {
                            if (value < 0x80) {
                                voice->s1byte20 = 0;
                                voice->trackPos = position;

                                position = patterns + (value << 1);

                                position = commands + readEndian(stream[position],stream[position+1]);

                                voice->patternPtr = position;
                                break;
                            } else if (value < 0xc0) {
                                voice->s1byte18 = value & 0x1f;
                            } else {
                                value = (value & 7) + voice->bank;
                                banks[value] = stream[position] >> 2;position++;
                            }
                        } else {

                            if (value == 0xfd) {
                                fadeStep = (signed char)stream[position];position++;
                            } else if (value < 0xc8) {
                                voice->s1byte20 = 0;
                                voice->trackPos = position;

                                position = patterns + (value << 1);

                                position = commands + readEndian(stream[position],stream[position+1]);

                                voice->patternPtr = position;

                                break;
                            } else if (value < 0xf0) {
                                voice->s1byte18 = value - 0xc8;
                            } else {
                                value = (value - 0xf0) + voice->bank;
                                banks[value] = stream[position] >> 2;position++;
                            }
                        }
                    } while (loop);
                }

            } else {
                position = voice->patternPos;

            }

            voice->s1byte21 = (--voice->s1byte21 & 0xff);

            if (voice->s1byte21) {
                if (stream[position] > 0x7f) fx(voice);
                voice->patternPos = position;
                loop = 0;

            } else {
                do {
                    if (stream[position] > 0x7f) {
                        fx(voice);

                        if (voice->s1byte20) {
                            if (m_variant > 1 && !voice->s1byte21) voice->s1byte21 = 1;
                            break;
                        }

                    } else {
                        value = stream[position];position++;

                        if (value != 0x7f) {
                            voice->s1byte23 = value + voice->s1byte19;

                            if (m_variant) {
                                voice->s2word10 = 0;
                                voice->s1byte35 = voice->s1byte31;
                                pos = position;

                                if (voice->s1byte35) {
                                    voice->s1byte36 = voice->s1byte32;
                                    voice->s1byte37 = voice->s1byte33;
                                    value = (-voice->s1byte34) << 2;

                                    position = periods + value;
                                    voice->s1long38 = (signed int)readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);position+=4;
                                }

                                voice->s1byte48 = voice->s1byte43;

                                if (voice->s1byte48) {
                                    voice->s1byte49 = voice->s1byte44;
                                    voice->s1byte50 = voice->s1byte44;
                                    voice->s1byte51 = voice->s1byte45;
                                    voice->s1word52 = voice->s1word46;
                                    voice->s1word54 = 0;
                                }

                                if (voice->s1byte42) {
                                    voice->s1byte35 = 0xff;
                                    voice->s1byte36 = 0;
                                    voice->s1byte37 = voice->s1byte33;

                                    temp = voice->s1byte23;
                                    if (!voice->s1byte24) voice->s1byte23 = voice->s1byte30;
                                    temp -= voice->s1byte23;
                                    temp = (-temp) << 2;

                                    position = periods + temp;
                                    voice->s1long38 = (signed int)readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);position+=4;
                                }

                                voice->s1byte30 = voice->s1byte23;
                                temp = voice->sample->volume * voice->s1word66;
                                voice->volume = (temp >> 14) & 0xffff;
                                voice->s1byte24 = 0;

                                if (m_variant == 1) {
                                    voice->v1word14 = voice->s1word68;
                                    voice->v1word16 = voice->s1word68;
                                    voice->v1word18 = voice->s1word70;
                                }

                                position = pos;

                            } else {
                                voice->volume = voice->sample->volume;
                            }

                            value = stream[position];position++;

                            if (!value && m_variant > 1) {
                                value = -1;
                                voice->s1byte21 = stream[position];position++;
                            } else {
                                voice->s1byte21 = value;
                            }

                            voice->sample = voice->sample2;
                            voice->patternPos = position;
                            setSample(voice, value);

                        } else {
                            voice->s1byte21 = stream[position];position++;
                            voice->patternPos = position;
                        }

                        loop = 0;
                        break;
                    }
                } while (true);
            }
        } while (loop);

    } while (voice = voice->next);
}



int BDPlayer::load(void* _data, unsigned long int _length)
{
    int len;
    int lower = 0xffff;
    int pos = 0;
    int value = 0;
    int tempVal = 0;
    int tracks = 0;
    length = _length;
    banks = std::vector<int>();
    position=0;
    stream = static_cast<unsigned char*>(_data);
    do {
        value = readEndian(stream[position],stream[position+1]);position+=2;

        switch (value) {
        case 0xd040:                                                    //add.w [d0,d0]
            tempVal = readEndian(stream[position],stream[position+1]);position+=2;
            if (tempVal != 0xd040) break;              //add.w [d0,d0]
            value = readEndian(stream[position],stream[position+1]);position+=2;

            if (value == 0x47fa) {                                        //lea [xx,a3]
                periods = position + (signed short)readEndian(stream[position],stream[position+1]);position+=2;
            } else if (value == 0xd040) {                                 //add.w [d0,d0]
                tempVal = readEndian(stream[position],stream[position+1]);position+=2;
                if (tempVal == 0x41fa) {                 //lea [xx,a0]
                    tracks = position + (signed short)readEndian(stream[position],stream[position+1]);position+=2;
                }
            }
            break;
        case 0x10c2:                                                    //move.b [d2,(a0)+]
            position += 2;
            value = readEndian(stream[position],stream[position+1]);position+=2;

            if (value == 0xb43c || value == 0x0c02) {                     //cmp.b [xx,d2] || cmpi.b [xx,d2]
                value = readEndian(stream[position],stream[position+1]);position+=2;

                if (banks.size() != value) {
                    banks = std::vector<int>(value);
                }
            }
            break;
        case 0xb03c:                                                    //cmp.b [xx,d0]
        case 0x0c00:                                                    //cmpi.b [xx,d0]
            tempVal = readEndian(stream[position],stream[position+1]);position+=2;
            if (tempVal == 0x00fd) m_variant = 3;       //xx = #$fd
            break;
        case 0x294b:                                                    //move.l [a3,xx]
            position += 2;
            tempVal = readEndian(stream[position],stream[position+1]);position+=2;
            if (tempVal != 0x47fa) break;              //lea [xx,a3]
            patterns = position + (signed short)readEndian(stream[position],stream[position+1]);position+=2;

            tempVal = readEndian(stream[position],stream[position+1]);position+=2;
            if (tempVal == 0x4880) {                   //ext.w d0
                position += 6;
            } else {
                position += 4;
            }

            tempVal = readEndian(stream[position],stream[position+1]);position+=2;
            if (tempVal != 0x47fa) {                   //lea [xx,a3]
                patterns = 0;
            } else {
                commands = position + (signed short)readEndian(stream[position],stream[position+1]);position+=2;
            }
            break;
        case 0x1030:                                                    //move.b (a0,d0.w),d0
            position += 2;

            tempVal = readEndian(stream[position],stream[position+1]);position+=2;
            if (tempVal == 0x41fa) {                   //lea [xx,a0]
                pos = position + (signed short)readEndian(stream[position],stream[position+1]);position+=2;

                for (int i = 0; i < 50; ++i) {
                    value = readEndian(stream[position],stream[position+1]);position+=2;

                    if (value == 0xb03c || value == 0x0c00) {                 //cmp.b [xx,d0] || cmpi.b [xx,d0]
                        tempVal = readEndian(stream[position],stream[position+1]);position+=2;
                        if (tempVal == 0x00c1) {             //xx = $c1
                            if (m_variant) {
                                m_variant--;
                            } else {
                                m_variant++;
                            }
                            break;
                        }
                    }
                }

                position = length;
            }
            break;
        }
    } while (position < length-4);

    if (!tracks || !patterns || !commands || !periods) return -1;

    position = pos;

    std::vector<int> offsets = std::vector<int>();

    do {
        value = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);position+=4;

        if (value < lower) {
            lower = value;
            len = pos + lower;
        }

        offsets.push_back(value);
    } while (position < len);

    len = offsets.size();
    lower = 0xffff;
    samples = std::vector<BDSample*>(len);

    for (int i = 0; i < len; ++i) {
        position = pos + offsets[i];
        BDSample* sample = new BDSample();

        sample->pointer = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);position+=4;
        sample->loopPtr = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);position+=4;
        sample->length  = readEndian(stream[position],stream[position+1]) << 1;position+=2;
        sample->repeat  = readEndian(stream[position],stream[position+1]) << 1;position+=2;
        sample->volume  = readEndian(stream[position],stream[position+1]);position+=2;
        sample->word14  = (signed short)readEndian(stream[position],stream[position+1]);position+=2;
        sample->word16  = (signed short)readEndian(stream[position],stream[position+1]);position+=2;
        sample->word18  = readEndian(stream[position],stream[position+1]);position+=2;
        sample->word20  = readEndian(stream[position],stream[position+1]);position+=2;
        sample->word22  = readEndian(stream[position],stream[position+1]);position+=2;
        sample->word24  = (signed short)readEndian(stream[position],stream[position+1]);position+=2;
        sample->word26  = readEndian(stream[position],stream[position+1]);position+=2;

        if (sample->pointer < lower) lower = sample->pointer;
        samples[i] = sample;
    }

    pos += lower;
    position = pos;
    amiga->store(stream, (length - pos),position,length);
    length = pos;

    for (int i = 0; i < len; ++i) {
        BDSample* sample = samples[i];
        sample->pointer -= lower;
        if (sample->loopPtr) sample->loopPtr -= lower;

        value = sample->pointer;
        amiga->memory[value] = 0;
        amiga->memory[++value] = 0;
    }

    position = tracks;
    songs = vector<int>();
    lower = 0xffff;

    do {
        value = readEndian(stream[position],stream[position+1]);position+=2;

        if (value < lower) {
            lower = value;
            len = tracks + lower;
        }

        songs.push_back(tracks + value);
    } while (position < len);

    m_totalSongs = songs.size() >> 2;

    len = banks.size() >> 2;

    for (int i = 0; i < len; ++i) {
        banks[i] = i;
        banks[(i + len)] = i;
        banks[(i + (len * 2))] = i;
        banks[(i + (len * 3))] = i;
    }

    m_version = 1;
    format = "Ben Daglish";
    //printData();
    return 1;
}
void BDPlayer::setSample(BDVoice* voice, int counter)
{
    AmigaChannel* chan = voice->channel;
    BDSample* sample = voice->sample;
    int value = 0;
    int temp = 0;

    chan->setEnabled(0);
    chan->pointer = sample->pointer;
    chan->length  = sample->length;
    chan->setVolume(voice->volume);

    voice->s2word14 = counter;

    value = -(voice->s1byte23 & 0x7f);
    value = (value + sample->word24) << 2;
    value = periods + value;

    if (value >= 0) {
      position = value;
      value = sample->word26 * readEndian(stream[position],stream[position+1]);position+=2;
      temp  = sample->word26 * readEndian(stream[position],stream[position+1]);position+=2;
      voice->period = ((temp >> 16) & 0xffff) + value;
    } else {
      voice->period = 0;
    }

    voice->s2word16 = sample->word16;

    if (sample->word16 >= 0) {
      value = sample->word20 >> 1;
      if (value & 1) value++;
      voice->s2word20 = value;

      value = sample->word18 * voice->period;
      voice->s2word18 = (value >> 14) & 0xffff;

      value = sample->word22 * voice->period;
      voice->s2word24 = (value >> 14) & 0xffff;
    }

    voice->s2word22 = 0;
  //voice->s2word10 = 0;
    voice->type = (sample->word14) ? 2 : 1;
    voice->state = 2;
}


void BDPlayer::fx(BDVoice* voice)
{
    int still = 0;
    int value = stream[position];position++;
    if (m_variant > 2) {
      if (value <= 0x8a) {
        value = (value - 0x80) + voice->bank;
        voice->sample2 = samples[banks[value]];
      } else if (value == 0xff) {
        voice->s1byte20 = value;
      } else if (value < 0x9b) {
        voice->s1byte56 = (value - 0x9b);
      } else {
        value += 0x25;
        still = 1;
      }
    } else {
      if (value <= 0x88) {
        value = (value & 7) + voice->bank;
        voice->sample2 = samples[banks[value]];
      } else if (value == 0xff) {
        voice->s1byte20 = value;
      } else if (value < 0xc0) {
        voice->s1byte56 = (value & 0x0f);
      } else if (!m_variant) {
        if (value != 0xc2) position += 3;
      } else {
        still = 1;
      }
    }

    if (!still) return;

    switch (value) {
      case 0xc0:
        voice->s1byte31 = 0xff;
        voice->s1byte42 = 0;
        voice->s1byte32 = stream[position];position++;
        voice->s1byte33 = stream[position];position++;
        voice->s1byte34 = (signed char)stream[position];position++;
        break;
      case 0xc1:
        voice->s1byte31 = 0;
        break;
      case 0xc2:
        voice->s1byte43 = 0xff;
        voice->s1byte44 = stream[position];position++;
        voice->s1byte45 = stream[position];position++;
        voice->s1word46 = (signed char)stream[position];position++;
        break;
      case 0xc3:
        voice->s1byte43 = 0;
        break;
      case 0xc4:
        voice->s1byte42 = 0xff;
        voice->s1byte31 = 0;
        voice->s1byte33 = stream[position];position++;
      case 0xc5:
        voice->s1byte42 = 0;
        break;
      case 0xc6:
        voice->s1word66 = (stream[position] << 8) | 0xff;position++;
        if (m_variant == 1) {
          voice->s1word68 = stream[position];position++;
          voice->s1word70 = (signed char)stream[position];position++;
        }
        break;
      case 0xc7:
        if (m_variant != 1) break;
        voice->s1word68 = 0;
        voice->s1word70 = 0xffff;
        break;
    }
}

unsigned char BDPlayer::getSubsongsCount()
{
    return m_totalSongs;
}
void BDPlayer::selectSong(unsigned char subsong)
{
    m_songNumber = subsong;
}
std::vector<BaseSample*> BDPlayer::getSamples()
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
void BDPlayer::printData()
{
    //    for(unsigned int i = 0; i < patterns.size(); i++)
    //    {
    //        AmigaRow* row= patterns[i];
    //        std::cout << "Pattern [" << i << "] note: " << row->note << " sample: " << row->sample << " param: " << row->param << " effect: " << row->effect << "\n";
    //    }
    for(unsigned int i = 0; i < samples.size(); i++)
    {
        BaseSample* sample = samples[i];
        if(sample)
        {
            std::cout << "Sample [" << i << "] length: " << sample->length << " finetune: " << sample->finetune << " relative: " << sample->relative << " loopPtr: " << sample->loopPtr << " name: " << sample->name << " pointer: " << sample->pointer << " repeat: " << sample->repeat<< " volume: " << (int)sample->volume << "\n";
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
