#include "MGPlayer.h"
#include "MGVoice.h"
#include "MGSample.h"
#include "MGSong.h"
#include "BaseStep.h"
#include "BaseRow.h"

#include <iostream>
#include "MyEndian.h"
using namespace std;

const int MGPlayer::PERIODS[1017] = {3220,3040,2869,2708,2556,2412,2277,2149,2029,1915,1807,1706,
                                     1610,1520,1434,1354,1278,1206,1139,1075,1014, 957, 904, 853,
                                     805, 760, 717, 677, 639, 603, 569, 537, 507, 479, 452, 426,
                                     403, 380, 359, 338, 319, 302, 285, 269, 254, 239, 226, 213,
                                     201, 190, 179, 169, 160, 151, 142, 134, 127,
                                     4842,4571,4314,4072,3843,3628,3424,3232,3051,2879,2718,2565,
                                     2421,2285,2157,2036,1922,1814,1712,1616,1525,1440,1359,1283,
                                     1211,1143,1079,1018, 961, 907, 856, 808, 763, 720, 679, 641,
                                     605, 571, 539, 509, 480, 453, 428, 404, 381, 360, 340, 321,
                                     303, 286, 270, 254, 240, 227, 214, 202, 191, 180, 170, 160,
                                     151, 143, 135, 127,
                                     4860,4587,4330,4087,3857,3641,3437,3244,3062,2890,2728,2574,
                                     2430,2294,2165,2043,1929,1820,1718,1622,1531,1445,1364,1287,
                                     1215,1147,1082,1022, 964, 910, 859, 811, 765, 722, 682, 644,
                                     607, 573, 541, 511, 482, 455, 430, 405, 383, 361, 341, 322,
                                     304, 287, 271, 255, 241, 228, 215, 203, 191, 181, 170, 161,
                                     152, 143, 135, 128,
                                     4878,4604,4345,4102,3871,3654,3449,3255,3073,2900,2737,2584,
                                     2439,2302,2173,2051,1936,1827,1724,1628,1536,1450,1369,1292,
                                     1219,1151,1086,1025, 968, 914, 862, 814, 768, 725, 684, 646,
                                     610, 575, 543, 513, 484, 457, 431, 407, 384, 363, 342, 323,
                                     305, 288, 272, 256, 242, 228, 216, 203, 192, 181, 171, 161,
                                     152, 144, 136, 128,
                                     4895,4620,4361,4116,3885,3667,3461,3267,3084,2911,2747,2593,
                                     2448,2310,2181,2058,1943,1834,1731,1634,1542,1455,1374,1297,
                                     1224,1155,1090,1029, 971, 917, 865, 817, 771, 728, 687, 648,
                                     612, 578, 545, 515, 486, 458, 433, 408, 385, 364, 343, 324,
                                     306, 289, 273, 257, 243, 229, 216, 204, 193, 182, 172, 162,
                                     153, 144, 136, 129,
                                     4913,4637,4377,4131,3899,3681,3474,3279,3095,2921,2757,2603,
                                     2456,2319,2188,2066,1950,1840,1737,1639,1547,1461,1379,1301,
                                     1228,1159,1094,1033, 975, 920, 868, 820, 774, 730, 689, 651,
                                     614, 580, 547, 516, 487, 460, 434, 410, 387, 365, 345, 325,
                                     307, 290, 274, 258, 244, 230, 217, 205, 193, 183, 172, 163,
                                     154, 145, 137, 129,
                                     4931,4654,4393,4146,3913,3694,3486,3291,3106,2932,2767,2612,
                                     2465,2327,2196,2073,1957,1847,1743,1645,1553,1466,1384,1306,
                                     1233,1163,1098,1037, 978, 923, 872, 823, 777, 733, 692, 653,
                                     616, 582, 549, 518, 489, 462, 436, 411, 388, 366, 346, 326,
                                     308, 291, 275, 259, 245, 231, 218, 206, 194, 183, 173, 163,
                                     154, 145, 137, 130,
                                     4948,4671,4409,4161,3928,3707,3499,3303,3117,2942,2777,2621,
                                     2474,2335,2204,2081,1964,1854,1750,1651,1559,1471,1389,1311,
                                     1237,1168,1102,1040, 982, 927, 875, 826, 779, 736, 694, 655,
                                     619, 584, 551, 520, 491, 463, 437, 413, 390, 368, 347, 328,
                                     309, 292, 276, 260, 245, 232, 219, 206, 195, 184, 174, 164,
                                     155, 146, 138, 130,
                                     4966,4688,4425,4176,3942,3721,3512,3315,3129,2953,2787,2631,
                                     2483,2344,2212,2088,1971,1860,1756,1657,1564,1477,1394,1315,
                                     1242,1172,1106,1044, 985, 930, 878, 829, 782, 738, 697, 658,
                                     621, 586, 553, 522, 493, 465, 439, 414, 391, 369, 348, 329,
                                     310, 293, 277, 261, 246, 233, 219, 207, 196, 185, 174, 164,
                                     155, 146, 138, 131,
                                     4984,4705,4441,4191,3956,3734,3524,3327,3140,2964,2797,2640,
                                     2492,2352,2220,2096,1978,1867,1762,1663,1570,1482,1399,1320,
                                     1246,1176,1110,1048, 989, 934, 881, 832, 785, 741, 699, 660,
                                     623, 588, 555, 524, 495, 467, 441, 416, 392, 370, 350, 330,
                                     312, 294, 278, 262, 247, 233, 220, 208, 196, 185, 175, 165,
                                     156, 147, 139, 131,
                                     5002,4722,4457,4206,3970,3748,3537,3339,3151,2974,2807,2650,
                                     2501,2361,2228,2103,1985,1874,1769,1669,1576,1487,1404,1325,
                                     1251,1180,1114,1052, 993, 937, 884, 835, 788, 744, 702, 662,
                                     625, 590, 557, 526, 496, 468, 442, 417, 394, 372, 351, 331,
                                     313, 295, 279, 263, 248, 234, 221, 209, 197, 186, 175, 166,
                                     156, 148, 139, 131,
                                     5020,4739,4473,4222,3985,3761,3550,3351,3163,2985,2818,2659,
                                     2510,2369,2236,2111,1992,1881,1775,1675,1581,1493,1409,1330,
                                     1255,1185,1118,1055, 996, 940, 887, 838, 791, 746, 704, 665,
                                     628, 592, 559, 528, 498, 470, 444, 419, 395, 373, 352, 332,
                                     314, 296, 280, 264, 249, 235, 222, 209, 198, 187, 176, 166,
                                     157, 148, 140, 132,
                                     5039,4756,4489,4237,3999,3775,3563,3363,3174,2996,2828,2669,
                                     2519,2378,2244,2118,2000,1887,1781,1681,1587,1498,1414,1335,
                                     1260,1189,1122,1059,1000, 944, 891, 841, 794, 749, 707, 667,
                                     630, 594, 561, 530, 500, 472, 445, 420, 397, 374, 353, 334,
                                     315, 297, 281, 265, 250, 236, 223, 210, 198, 187, 177, 167,
                                     157, 149, 140, 132,
                                     5057,4773,4505,4252,4014,3788,3576,3375,3186,3007,2838,2679,
                                     2528,2387,2253,2126,2007,1894,1788,1688,1593,1503,1419,1339,
                                     1264,1193,1126,1063,1003, 947, 894, 844, 796, 752, 710, 670,
                                     632, 597, 563, 532, 502, 474, 447, 422, 398, 376, 355, 335,
                                     316, 298, 282, 266, 251, 237, 223, 211, 199, 188, 177, 167,
                                     158, 149, 141, 133,
                                     5075,4790,4521,4268,4028,3802,3589,3387,3197,3018,2848,2688,
                                     2538,2395,2261,2134,2014,1901,1794,1694,1599,1509,1424,1344,
                                     1269,1198,1130,1067,1007, 951, 897, 847, 799, 754, 712, 672,
                                     634, 599, 565, 533, 504, 475, 449, 423, 400, 377, 356, 336,
                                     317, 299, 283, 267, 252, 238, 224, 212, 200, 189, 178, 168,
                                     159, 150, 141, 133,
                                     5093,4808,4538,4283,4043,3816,3602,3399,3209,3029,2859,2698,
                                     2547,2404,2269,2142,2021,1908,1801,1700,1604,1514,1429,1349,
                                     1273,1202,1134,1071,1011, 954, 900, 850, 802, 757, 715, 675,
                                     637, 601, 567, 535, 505, 477, 450, 425, 401, 379, 357, 337,
                                     318, 300, 284, 268, 253, 238, 225, 212, 201, 189, 179, 169,
                                     159, 150, 142, 134};

MGPlayer::MGPlayer(Amiga* amiga):AmigaPlayer(amiga)
{

    buffer1=0;
    buffer2=0;
    trackPos=0;
    patternPos=0;
    patternLen=0;
    patternEnd=0;
    stepEnd=0;
    chans=0;
    mixPeriod=0;

    songs = std::vector<MGSong*>(8);
    arpeggios = std::vector<int>(256);
    voices    = std::vector<MGVoice*>(7);

    voices[0] = new MGVoice(0);
    voices[0]->next = voices[1] = new MGVoice(1);
    voices[1]->next = voices[2] = new MGVoice(2);
    voices[2]->next = voices[3] = new MGVoice(3);

    voices[4] = new MGVoice(4);
    voices[4]->next = voices[5] = new MGVoice(5);
    voices[5]->next = voices[6] = new MGVoice(6);
    tables();
}
MGPlayer::~MGPlayer()
{
    arpeggios.clear();
    averages.clear();
    volumes.clear();

    for(unsigned int i = 0; i < songs.size(); i++)
    {
        if(songs[i]) delete songs[i];
    }
    songs.clear();
    for(unsigned int i = 1; i < samples.size(); i++)
    {
        if(samples[i]) delete samples[i];
    }
    samples.clear();
    for(unsigned int i = 0; i < voices.size(); i++)
    {
        if(voices[i]) delete voices[i];
    }
    voices.clear();
    for(unsigned int i = 0; i < patterns.size(); i++)
    {
        if(patterns[i]) delete patterns[i];
    }
    patterns.clear();

    subSongsList.clear();
}
void MGPlayer::tables()
{
    averages  = std::vector<int>(1024);
    volumes   = std::vector<int>(16384);
    mixPeriod = 203;
    int vol = 128;
    int step = 0;
    int idx = 0;
    int pos = 0;
    int v1 = 0;
    int v2 = 0;

    for (int i = 0; i < 1024; ++i) {
        if (vol > 127) vol -= 256;
        averages[i] = vol;
        if (i > 383 && i < 639) vol = ++vol & 255;
    }

    for (int i = 0; i < 64; ++i) {
        v1 = -128;
        v2 =  128;

        for (int j = 0; j < 256; ++j) {
            vol = ((v1 * step) / 63) + 128;
            idx = pos + v2;
            volumes[idx] = vol & 255;

            if (i != 0 && i != 63 && v2 >= 128) --volumes[idx];
            v1++;
            v2 = ++v2 & 255;
        }
        pos += 256;
        step++;
    }
}



int MGPlayer::load(void* _data, unsigned long int _length)
{
    unsigned char *stream = static_cast<unsigned char*>(_data);
    if(stream[0]==' ' && stream[1]=='M' && stream[2]=='U' && stream[3]=='G' && stream[4]=='I' && stream[5]=='C' && stream[6]=='I' && stream[7]=='A' && stream[8]=='N' && stream[9]=='/'&& stream[10]=='S' && stream[11]=='O' && stream[12]=='F' && stream[13]=='T' && stream[14]=='E' && stream[15]=='Y' && stream[16]=='E' && stream[17]=='S' && stream[18]==' ' && stream[19]=='1' && stream[20]=='9' && stream[21]=='9' && stream[22]=='0' && stream[23]==' ')
    {
        m_version = MUGICIAN_V1;
        format = "Digital Mugician 1";
        chans = 4;
        voices[3]->next = 0;
    }
    else if(stream[0]==' ' && stream[1]=='M' && stream[2]=='U' && stream[3]=='G' && stream[4]=='I' && stream[5]=='C' && stream[6]=='I' && stream[7]=='A' && stream[8]=='N' && stream[9]=='2'&& stream[10]=='/' && stream[11]=='S' && stream[12]=='O' && stream[13]=='F' && stream[14]=='T' && stream[15]=='E' && stream[16]=='Y' && stream[17]=='E' && stream[18]=='S' && stream[19]==' ' && stream[20]=='1' && stream[21]=='9' && stream[22]=='9' && stream[23]=='0')
    {
        m_version = MUGICIAN_V2;
        format = "Digital Mugician 2";
        chans = 7;
        voices[3]->next = voices[4];
    }
    else
    {
        return -1;
    }

    int len = 0;
    unsigned int position;
    position = 28;
    std::vector<int> index(8);


    for (int i=0; i < 8; ++i)
    {
        index[i] = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);
        position+=4;
    }

    position = 76;

    for (int i = 0; i < 8; ++i) {
        MGSong* song = new MGSong();
        song->loop     = stream[position];position++;
        song->loopStep = stream[position] << 2;position++;
        song->speed    = stream[position];position++;
        song->length   = stream[position] << 2;position++;
        const int STRING_LENGTH = 12;
        for(int j = 0;j<STRING_LENGTH;j++)
        {
            if(!stream[position+j])
            {
                break;
            }
            song->title+=stream[position+j];
        }
        position+=STRING_LENGTH;
        songs[i] = song;
    }

    position = 204;
    m_totalSongs = songs.size();
    subSongsList = std::vector<unsigned char>();


    for (int i = 0; i < 8; ++i) {
        MGSong* song = songs[i];
        len = index[i] << 2;

        unsigned int patternSize = 0;
        for (int j = 0; j < len; ++j)
        {
            BaseStep* step = new BaseStep();
            step->pattern = stream[position] << 6;position++;
            step->transpose = (signed char)stream[position];position++;
            song->tracks.push_back(step);
            patternSize+=step->pattern;
        }
        if(patternSize>0)
        {
            if((m_version == MUGICIAN_V1) || (m_version == MUGICIAN_V2 && (i+1) % 2==1))
            {
                subSongsList.push_back(i);
            }
        }

    }

    int pos = position;
    position = 60;
    len = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);position+=4;
    samples = std::vector<MGSample*>(++len);
    position = pos;

    for (int i = 1; i < len; ++i) {
        MGSample* sample = new MGSample();
        sample->wave        = stream[position];position++;
        sample->waveLen     = stream[position] << 1;position++;
        sample->volume      = stream[position];position++;
        sample->volSpeed = stream[position];position++;
        sample->arpeggio    = stream[position];position++;
        sample->pitch       = stream[position];position++;
        sample->fxStep  = stream[position];position++;
        sample->pitchDelay  = stream[position];position++;
        sample->finetune    = stream[position] << 6;position++;
        sample->pitchLoop   = stream[position];position++;
        sample->pitchSpeed  = stream[position];position++;
        sample->fx      = stream[position];position++;
        sample->source1     = stream[position];position++;
        sample->source2     = stream[position];position++;
        sample->fxSpeed = stream[position];position++;
        sample->volLoop  = stream[position];position++;
        samples[i] = sample;
    }
    samples[0] = samples[1];

    pos = position;
    position = 64;
    len = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]) << 7;position+=4;
    position = pos;
    amiga->store(stream, len,position,_length);

    pos = position;
    position = 68;
    int instr = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);position+=4;


    position = 26;
    len = readEndian(stream[position],stream[position+1]) << 6;position+=2;

    patterns.resize(len);

    position = pos + (instr << 5);

    if (instr) instr = pos;

    for (int i = 0; i < len; ++i) {
        BaseRow* row = new BaseRow();
        row->note   = stream[position];position++;
        row->sample = stream[position] & 63;position++;
        row->effect  = stream[position];position++;
        row->param  = (signed char)stream[position];position++;
        patterns[i]=row;
    }

    pos = position;
    position = 72;

    if (instr) {
        len = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);position+=4;
        position = pos;
        int data = amiga->store(stream, len,position,_length);
        pos = position;

        amiga->memory.resize(amiga->memory.size() + 350);
        buffer1 = amiga->memory.size();
        amiga->memory.resize(amiga->memory.size() + 350);
        buffer2 = amiga->memory.size();
        amiga->memory.resize(amiga->memory.size() + 350);
        amiga->loopLen = 8;

        len = samples.size();

        for (int i = 1; i < len; ++i) {
            MGSample* sample = samples[i];
            if (sample->wave < 32) continue;
            position = instr + ((sample->wave - 32) << 5);


            sample->pointer = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);position+=4;
            sample->length  = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]) - sample->pointer;position+=4;
            sample->loop    = readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);position+=4;
            const int STRING_LENGTH = 12;
            for(int j = 0;j<STRING_LENGTH;j++)
            {
                if(!stream[position+j])
                {
                    break;
                }
                sample->name+=stream[position+j];
            }
            position+=STRING_LENGTH;

            if (sample->loop) {
                sample->loop  -= sample->pointer;
                sample->repeat = sample->length - sample->loop;
                if (sample->repeat & 1) sample->repeat--;
            } else {
                sample->loopPtr = amiga->memory.size();
                sample->repeat  = 8;
            }

            if (sample->pointer & 1) sample->pointer--;
            if (sample->length  & 1) sample->length--;

            sample->pointer += data;
            if (!sample->loopPtr) sample->loopPtr = sample->pointer + sample->loop;
        }
    } else {
        pos += readEndian(stream[position],stream[position+1],stream[position+2],stream[position+3]);position+=4;
    }

    position = 24;
    if (readEndian(stream[position],stream[position+1]) == 1) {
        position+=2;
        position = pos;
        len = _length - pos;
        if (len > 256) len = 256;
        for (int i = 0; i < len; ++i) {
            arpeggios[i] = stream[position];position++;
        }
    }

    //printData();
    return 1;
}

std::vector<BaseSample*> MGPlayer::getSamples()
{
    std::vector<BaseSample*>samp (samples.size()-1);
    for(int i =1; i< samples.size() ; i++)
    {
        samp[i-1] = samples[i];
        if(!samp[i-1])
        {
            samp[i-1] = new BaseSample();
        }
    }
    return samp;
}

bool MGPlayer::getTitle(std::string& title)
{
    title = songs[0]->title;
    return true;
}
