#include "S1Player.h"
#include "S1Voice.h"
#include "BaseRow.h"
#include "S1Sample.h"
#include "BaseStep.h"
#include <iostream>
#include "MyEndian.h"

using namespace std;

const int S1Player::EMBEDDED[3] = {1166, 408, 908};
const int S1Player::PERIODS[540] = {
    0,
    5760, 5424, 5120, 4832, 4560, 4304, 4064, 3840, 3616, 3424, 3232, 3048,
    2880, 2712, 2560, 2416, 2280, 2152, 2032, 1920, 1808, 1712, 1616, 1524,
    1440, 1356, 1280, 1208, 1140, 1076, 1016, 960, 904, 856, 808, 762,
    720, 678, 640, 604, 570, 538, 508, 480, 452, 428, 404, 381,
    360, 339, 320, 302, 285, 269, 254, 240, 226, 214, 202, 190,
    180, 170, 160, 151, 143, 135, 127,
    0, 0, 0, 0, 0, 0, 0,
    4028, 3806, 3584, 3394, 3204, 3013, 2855, 2696, 2538, 2395, 2268, 2141,
    2014, 1903, 1792, 1697, 1602, 1507, 1428, 1348, 1269, 1198, 1134, 1071,
    1007, 952, 896, 849, 801, 754, 714, 674, 635, 599, 567, 536,
    504, 476, 448, 425, 401, 377, 357, 337, 310, 300, 284, 268,
    252, 238, 224, 213, 201, 189, 179, 169, 159, 150, 142, 134,
    0, 0, 0, 0, 0, 0, 0,
    3993, 3773, 3552, 3364, 3175, 2987, 2830, 2672, 2515, 2374, 2248, 2122,
    1997, 1887, 1776, 1682, 1588, 1494, 1415, 1336, 1258, 1187, 1124, 1061,
    999, 944, 888, 841, 794, 747, 708, 668, 629, 594, 562, 531,
    500, 472, 444, 421, 397, 374, 354, 334, 315, 297, 281, 266,
    250, 236, 222, 211, 199, 187, 177, 167, 158, 149, 141, 133,
    0, 0, 0, 0, 0, 0, 0,
    3957, 3739, 3521, 3334, 3147, 2960, 2804, 2648, 2493, 2353, 2228, 2103,
    1979, 1870, 1761, 1667, 1574, 1480, 1402, 1324, 1247, 1177, 1114, 1052,
    990, 935, 881, 834, 787, 740, 701, 662, 624, 589, 557, 526,
    495, 468, 441, 417, 394, 370, 351, 331, 312, 295, 279, 263,
    248, 234, 221, 209, 197, 185, 176, 166, 156, 148, 140, 132,
    0, 0, 0, 0, 0, 0, 0,
    3921, 3705, 3489, 3304, 3119, 2933, 2779, 2625, 2470, 2331, 2208, 2084,
    1961, 1853, 1745, 1652, 1560, 1467, 1390, 1313, 1235, 1166, 1104, 1042,
    981, 927, 873, 826, 780, 734, 695, 657, 618, 583, 552, 521,
    491, 464, 437, 413, 390, 367, 348, 329, 309, 292, 276, 261,
    246, 232, 219, 207, 195, 184, 174, 165, 155, 146, 138, 131,
    0, 0, 0, 0, 0, 0, 0,
    3886, 3671, 3457, 3274, 3090, 2907, 2754, 2601, 2448, 2310, 2188, 2065,
    1943, 1836, 1729, 1637, 1545, 1454, 1377, 1301, 1224, 1155, 1094, 1033,
    972, 918, 865, 819, 773, 727, 689, 651, 612, 578, 547, 517,
    486, 459, 433, 410, 387, 364, 345, 326, 306, 289, 274, 259,
    243, 230, 217, 205, 194, 182, 173, 163, 153, 145, 137, 130,
    0, 0, 0, 0, 0, 0, 0,
    3851, 3638, 3426, 3244, 3062, 2880, 2729, 2577, 2426, 2289, 2168, 2047,
    1926, 1819, 1713, 1622, 1531, 1440, 1365, 1289, 1213, 1145, 1084, 1024,
    963, 910, 857, 811, 766, 720, 683, 645, 607, 573, 542, 512,
    482, 455, 429, 406, 383, 360, 342, 323, 304, 287, 271, 256,
    241, 228, 215, 203, 192, 180, 171, 162, 152, 144, 136, 128,
    6848, 6464, 6096, 5760, 5424, 5120, 4832, 4560, 4304, 4064, 3840, 3616,
    3424, 3232, 3048, 2880, 2712, 2560, 2416, 2280, 2152, 2032, 1920, 1808,
    1712, 1616, 1524, 1440, 1356, 1280, 1208, 1140, 1076, 1016, 960, 904,
    856, 808, 762, 720, 678, 640, 604, 570, 538, 508, 480, 452,
    428, 404, 381, 360, 339, 320, 302, 285, 269, 254, 240, 226,
    214, 202, 190, 180, 170, 160, 151, 143, 135, 127
};

S1Player::S1Player(Amiga* amiga): AmigaPlayer(amiga)
{
    tracksPtr = std::vector<int>(4);
    voices = std::vector<S1Voice*>(4);
    voices[0] = new S1Voice(0);
    voices[0]->next = voices[1] = new S1Voice(1);
    voices[1]->next = voices[2] = new S1Voice(2);
    voices[2]->next = voices[3] = new S1Voice(3);

    speedDef = 0;
    trackLen = 0;
    patternDef = 0;
    mix1Speed = 0;
    mix2Speed = 0;
    mix1Dest = 0;
    mix2Dest = 0;
    mix1Source1 = 0;
    mix1Source2 = 0;
    mix2Source1 = 0;
    mix2Source2 = 0;
    doFilter = 0;
    doReset = 0;
    trackPos = 0;
    trackEnd = 0;
    patternPos = 0;
    patternEnd = 0;
    patternLen = 0;
    mix1Ctr = 0;
    mix2Ctr = 0;
    mix1Pos = 0;
    mix2Pos = 0;
    audioPtr = 0;
    audioLen = 0;
    audioPer = 0;
    audioVol = 0;
}

S1Player::~S1Player()
{
    tracksPtr.clear();
    patternsPtr.clear();
    waveLists.clear();

    for (unsigned int i = 0; i < tracks.size(); i++)
    {
        if (tracks[i]) delete tracks[i];
    }
    tracks.clear();
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

int S1Player::load(void* _data, unsigned long int _length)
{
    unsigned int position = 0;
    int pos = 0;
    int start = 0;
    int len = 0;
    int totPatterns = 0;
    int data = 0;
    int totSamples = 0;
    int totInstr = 0;
    int headers = 0;

    unsigned char* stream = static_cast<unsigned char*>(_data);
    do
    {
        start = readEndian(stream[position], stream[position + 1]);
        position += 2;
        if (start != 0x41fa) continue;
        int j = readEndian(stream[position], stream[position + 1]);
        position += 2;

        start = readEndian(stream[position], stream[position + 1]);
        position += 2;
        if (start != 0xd1e8) continue;
        start = readEndian(stream[position], stream[position + 1]);
        position += 2;

        if (start == 0xffd4)
        {
            if (j == 0x0fec)
            {
                m_variant = SIDMON_0FFA;
            }
            else if (j == 0x1466)
            {
                m_variant = SIDMON_1444;
            }
            else
            {
                m_variant = j;
            }

            pos = j + position - 6;
            break;
        }
    }
    while (_length - position > 8);

    if (!pos) return -1;
    position = pos;

    if (!(stream[position] == ' ' && stream[position + 1] == 'S' && stream[position + 2] == 'I' && stream[position + 3]
            == 'D' && stream[position + 4] == '-' && stream[position + 5] == 'M' && stream[position + 6] == 'O' &&
            stream[position + 7] == 'N' && stream[position + 8] == ' ' && stream[position + 9] == 'B' && stream[position
                + 10] == 'Y' && stream[position + 11] == ' ' && stream[position + 12] == 'R' && stream[position + 13] ==
            '.' && stream[position + 14] == 'v' &&
            stream[position + 15] == '.' && stream[position + 16] == 'V' && stream[position + 17] == 'L' && stream[
                position + 18] == 'I' && stream[position + 19] == 'E' && stream[position + 20] == 'T' && stream[position
                + 21] == ' ' && stream[position + 22] == ' ' &&
            stream[position + 23] == '(' && stream[position + 24] == 'c' && stream[position + 25] == ')' && stream[
                position + 26] == ' ' && stream[position + 27] == '1' && stream[position + 28] == '9' && stream[position
                + 29] == '8' && stream[position + 30] == '8' && stream[position + 31] == ' ')
        &&
        !(stream[position] == ' ' && stream[position + 1] == 'R' && stream[position + 2] == 'i' && stream[position + 3]
            == 'p' && stream[position + 4] == 'p' && stream[position + 5] == 'e' && stream[position + 6] == 'd' &&
            stream[position + 7] == ' ' && stream[position + 8] == 'w' && stream[position + 9] == 'i' && stream[position
                + 10] == 't' && stream[position + 11] == 'h' && stream[position + 12] == ' ' && stream[position + 13] ==
            'S' && stream[position + 14] == 'C' &&
            stream[position + 15] == 'X' && stream[position + 16] == ' ' && stream[position + 17] == 'R' && stream[
                position + 18] == 'i' && stream[position + 19] == 'p' && stream[position + 20] == 'p' && stream[position
                + 21] == 'e' && stream[position + 22] == 'r')
        &&
        !(stream[position] == ' ' && stream[position + 1] == 'R' && stream[position + 2] == 'i' && stream[position + 3]
            == 'p' && stream[position + 4] == 'p' && stream[position + 5] == 'e' && stream[position + 6] == 'd' &&
            stream[position + 7] == ' ' && stream[position + 8] == 'w' && stream[position + 9] == 'i' && stream[position
                + 10] == 't' && stream[position + 11] == 'h' && stream[position + 12] == ' ' && stream[position + 13] ==
            'S' && stream[position + 14] == 'i' &&
            stream[position + 15] == 'd' && stream[position + 16] == 'm' && stream[position + 17] == 'R' && stream[
                position + 18] == 'i' && stream[position + 19] == 'p' && stream[position + 20] == 'p' && stream[position
                + 21] == 'e' && stream[position + 22] == 'r')
    )

    {
        return -1;
    }

    position = pos - 44;
    start = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);
    position += 4;

    for (int i = 1; i < 4; ++i)
    {
        tracksPtr[i] = (readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]) -
            start) / 6;
        position += 4;
    }

    position = pos - 8;
    start = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);
    position += 4;
    len = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);
    position += 4;
    if (len < start) len = _length - pos;

    totPatterns = (len - start) >> 2;
    patternsPtr = std::vector<int>(totPatterns);
    position = pos + start + 4;

    for (int i = 1; i < totPatterns; ++i)
    {
        start = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]) / 5;
        position += 4;

        if (!start)
        {
            totPatterns = i;
            break;
        }
        patternsPtr[i] = start;
    }

    patternsPtr.resize(totPatterns);


    position = pos - 44;
    start = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);
    position += 4;
    position = pos - 28;
    len = (readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]) - start) / 6;
    position += 4;

    tracks = std::vector<BaseStep*>(len);
    position = pos + start;

    for (int i = 0; i < len; ++i)
    {
        BaseStep* step = new BaseStep();
        step->pattern = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);
        position += 4;
        if (step->pattern >= totPatterns) step->pattern = 0;
        position++;
        step->transpose = (signed char)stream[position];
        position++;
        if (step->transpose < -99 || step->transpose > 99) step->transpose = 0;
        tracks[i] = step;
    }

    position = pos - 24;
    start = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);
    position += 4;
    int totWaves = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]) -
        start;
    position += 4;

    //amiga->memory.resize(32);
    //amiga->store(stream, totWaves, position,_length, pos + start);
    totWaves >>= 5;

    position = pos - 16;
    start = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);
    position += 4;
    len = (readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]) - start) +
        16;;
    position += 4;
    int j = (totWaves + 2) << 4;

    waveLists = std::vector<int>(len < j ? j : len);
    position = pos + start;
    int i = 0;
    do
    {
        waveLists[i++] = i >> 4;
        waveLists[i++] = 0xff;
        waveLists[i++] = 0xff;
        waveLists[i++] = 0x10;
        i += 12;
    }
    while (i < j);

    for (int i = 16; i < len; ++i)
    {
        waveLists[i] = stream[position];
        position++;
    }

    position = pos - 20;
    position = pos + readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);

    mix1Source1 = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);
    position += 4;
    mix2Source1 = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);
    position += 4;
    mix1Source2 = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);
    position += 4;
    mix2Source2 = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);
    position += 4;
    mix1Dest = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);
    position += 4;
    mix2Dest = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);
    position += 4;
    patternDef = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);
    position += 4;
    trackLen = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);
    position += 4;
    speedDef = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);
    position += 4;
    mix1Speed = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);
    position += 4;
    mix2Speed = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);
    position += 4;

    if (mix1Source1 > totWaves) mix1Source1 = 0;
    if (mix2Source1 > totWaves) mix2Source1 = 0;
    if (mix1Source2 > totWaves) mix1Source2 = 0;
    if (mix2Source2 > totWaves) mix2Source2 = 0;
    if (mix1Dest > totWaves) mix1Speed = 0;
    if (mix2Dest > totWaves) mix2Speed = 0;
    if (!speedDef) speedDef = 4;


    position = pos - 28;
    j = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);
    position += 4;
    totInstr = (readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]) - j) >>
        5;
    position += 4;
    if (totInstr > 63) totInstr = 63;
    len = totInstr + 1;

    position = pos - 4;
    start = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);
    position += 4;

    if (start == 1)
    {
        position = 0x71c;
        start = readEndian(stream[position], stream[position + 1]);
        position += 2;

        if (start != 0x4dfa)
        {
            position = 0x6fc;
            start = readEndian(stream[position], stream[position + 1]);
            position += 2;

            if (start != 0x4dfa) return -1;
        }
        position += readEndian(stream[position], stream[position + 1]);
        position += 2;
        samples = std::vector<S1Sample*>(len + 3);

        for (int i = 0; i < 3; ++i)
        {
            S1Sample* sample = new S1Sample();
            sample->waveform = 16 + i;
            sample->length = EMBEDDED[i];
            //sample->pointer  = amiga->store(stream, sample->length,position,_length);
            sample->loopPtr = 0;
            sample->repeat = 4;
            sample->volume = 64;
            samples[int(len + i)] = sample;
            position += sample->length;
        }
    }
    else
    {
        samples = std::vector<S1Sample*>(len + 3);

        position = pos + start;
        data = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);
        position += 4;
        totSamples = (data >> 5) + 15;
        headers = position;
        data += headers;
    }

    S1Sample* sample = new S1Sample();
    samples[0] = sample;
    position = pos + j;

    for (int i = 1; i < len; ++i)
    {
        sample = new S1Sample();
        sample->waveform = readEndian(stream[position], stream[position + 1], stream[position + 2],
                                      stream[position + 3]);
        position += 4;
        for (j = 0; j < 16; ++j)
        {
            sample->arpeggio[j] = stream[position];
            position++;
        }

        sample->attackSpeed = stream[position];
        position++;
        sample->attackMax = stream[position];
        position++;
        sample->decaySpeed = stream[position];
        position++;
        sample->decayMin = stream[position];
        position++;
        sample->sustain = stream[position];
        position++;
        position++;
        sample->releaseSpeed = stream[position];
        position++;
        sample->releaseMin = stream[position];
        position++;
        sample->phaseShift = stream[position];
        position++;
        sample->phaseSpeed = stream[position];
        position++;
        sample->finetune = stream[position];
        position++;
        sample->pitchFall = (signed char)stream[position];
        position++;

        if (m_variant == SIDMON_1444)
        {
            sample->pitchFall = sample->finetune;
            sample->finetune = 0;
        }
        else
        {
            if (sample->finetune > 15) sample->finetune = 0;
            sample->finetune *= 67;
        }

        if (sample->phaseShift > totWaves)
        {
            sample->phaseShift = 0;
            sample->phaseSpeed = 0;
        }

        if (sample->waveform > 15)
        {
            if ((totSamples > 15) && (sample->waveform > totSamples))
            {
                sample->waveform = 0;
            }
            else
            {
                start = headers + ((sample->waveform - 16) << 5);
                if (start >= _length) continue;
                j = position;

                position = start;
                sample->pointer = readEndian(stream[position], stream[position + 1], stream[position + 2],
                                             stream[position + 3]);
                position += 4;
                sample->loopPtr = readEndian(stream[position], stream[position + 1], stream[position + 2],
                                             stream[position + 3]);
                position += 4;
                sample->length = readEndian(stream[position], stream[position + 1], stream[position + 2],
                                            stream[position + 3]);
                position += 4;
                const int STRING_LENGTH = 20;
                for (int j = 0; j < STRING_LENGTH; j++)
                {
                    if (!stream[position + j])
                    {
                        break;
                    }
                    sample->name += stream[position + j];
                }
                position += STRING_LENGTH;

                if (sample->loopPtr == 0 ||
                    sample->loopPtr == 99999 ||
                    sample->loopPtr == 199999 ||
                    sample->loopPtr >= sample->length)
                {
                    sample->loopPtr = 0;
                    sample->repeat = m_variant == SIDMON_0FFA ? 2 : 4;
                }
                else
                {
                    sample->repeat = sample->length - sample->loopPtr;
                    sample->loopPtr -= sample->pointer;
                }

                sample->length -= sample->pointer;
                if (sample->length < (sample->loopPtr + sample->repeat))
                {
                    sample->length = sample->loopPtr + sample->repeat;
                }

                //sample->pointer = amiga->store(stream, sample->length,position,_length,data + sample->pointer);
                if (sample->repeat < 6 || sample->loopPtr == 0)
                {
                    sample->loopPtr = 0;
                }
                else
                {
                    sample->loopPtr += sample->pointer;
                }

                position = j;
            }
        }
        else if (sample->waveform > totWaves)
        {
            sample->waveform = 0;
        }
        samples[i] = sample;
    }

    position = pos - 12;
    start = readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]);
    position += 4;
    len = (readEndian(stream[position], stream[position + 1], stream[position + 2], stream[position + 3]) - start) / 5;
    position += 4;
    patterns = std::vector<BaseRow*>(len);
    position = pos + start;
    for (int i = 0; i < len; ++i)
    {
        BaseRow* row = new BaseRow();
        row->note = stream[position];
        position++;
        row->sample = stream[position];
        position++;
        row->effect = stream[position];
        position++;
        row->param = stream[position];
        position++;
        row->speed = stream[position];
        position++;


        if (m_variant == SIDMON_1444)
        {
            if (row->note > 0 && row->note < 255) row->note += 469;
            if (row->effect > 0 && row->effect < 255) row->effect += 469;
            if (row->sample > 59) row->sample = totInstr + (row->sample - 60);
        }
        else if (row->sample > totInstr)
        {
            row->sample = 0;
        }
        patterns[i] = row;
    }

    if (m_variant == SIDMON_1170 || m_variant == SIDMON_11C6 || m_variant == SIDMON_1444)
    {
        doReset = doFilter = 0;
        if (m_variant == SIDMON_1170) mix1Speed = mix2Speed = 0;
    }
    else
    {
        doReset = doFilter = 1;
    }


    m_version = 1;
    format = "Sidmon 1";
    //printData();
    return 1;
}

std::vector<BaseSample*> S1Player::getSamples()
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
