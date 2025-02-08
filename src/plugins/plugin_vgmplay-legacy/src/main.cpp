#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cstring>
#include <cmath>
#include "fmod.h"
#include "info.h"
#include "plugins.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "chips/mamedef.h"	// for (U)INTxx types
#include "VGMPlay.h"
#include "VGMPlay_Intf.h"
#ifdef __cplusplus
}
#endif

using namespace std;


FMOD_RESULT handle_error(const char* str)
{
    if (str)
    {
        return FMOD_ERR_INTERNAL;
    }
    else
        return FMOD_OK;
}

FMOD_RESULT F_CALLBACK open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo);
FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE* codec);
FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read);
FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);
FMOD_RESULT F_CALLBACK getlength(FMOD_CODEC_STATE* codec, unsigned int* length, FMOD_TIMEUNIT lengthtype);

FMOD_CODEC_DESCRIPTION codecDescription =
{
    FMOD_CODEC_PLUGIN_VERSION,
    PLUGIN_vgmplay_legacy_NAME, // Name.
    0x00012300, // Version 0xAAAABBBB   A = major, B = minor.
    1, // Force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS, // The time format we would like to accept into setposition/getposition.
    &open, // Open callback.
    &close, // Close callback.
    &read, // Read callback.
    nullptr,
    // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &setposition, // Setposition callback.
    nullptr,
    // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    nullptr // Sound create callback (don't need it)
};

class pluginVgmplayLegacy
{
    FMOD_CODEC_STATE* _codec;

public:
    pluginVgmplayLegacy(FMOD_CODEC_STATE* codec)
    {
        _codec = codec;
        //LogFile = new CLogFile("gameemu.log");
        memset(&waveformat, 0, sizeof(waveformat));
    }

    ~pluginVgmplayLegacy()
    {
        //delete some stuff
        //gme_delete( emu );
    }

    //Music_Emu* emu;
    FMOD_CODEC_WAVEFORMAT waveformat;
};

/*
    FMODGetCodecDescription is mandatory for every fmod plugin.  This is the symbol the registerplugin function searches for.
    Must be declared with F_API to make it export as stdcall.
    MUST BE EXTERN'ED AS C!  C++ functions will be mangled incorrectly and not load in fmod.
*/
#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) FMOD_CODEC_DESCRIPTION* __stdcall _FMODGetCodecDescription()
{
    return &codecDescription;
}

#ifdef __cplusplus
}
#endif


static string msToNiceStringExact(unsigned int lenms)
{
    string songLength = "";

    int ms;
    int sec;
    int min;
    int hour;
    string strMs;
    string strSec;
    string strMin;
    string strHour;

    unsigned int length = lenms / 1000;

    ms = lenms % 1000;
    sec = length % 60;
    min = length / 60 % 60;
    hour = length / 3600;
    stringstream ss2;
    stringstream ss3;
    stringstream ss4;
    stringstream ss5;

    if (ms < 10)
        ss2 << "00" << ms;
    else if (ms < 100)
        ss2 << "0" << ms;
    else
        ss2 << ms;

    if (sec < 10 && (min > 0 || hour > 0))
        ss3 << "0" << sec;
    else
        ss3 << sec;

    ss2 >> strMs;
    ss3 >> strSec;

    ss4 << min;
    ss4 >> strMin;
    if (hour > 0)
    {
        ss5 << hour;
        ss5 >> strHour;
        songLength = strHour + ":" + strMin + ":" + strSec + "." + strMs;
    }
    else if (min > 0)
    {
        songLength = strMin + ":" + strSec + "." + strMs;
    }
    else
    {
        songLength = strSec + "." + strMs;
    }
    if (lenms == 0xffffffff)
    {
        songLength = "??:??";
    }
    return songLength;
}

static string PrintChipStr(UINT8 ChipID, UINT8 SubType, UINT32 Clock)
{
    string chips;
    if (!Clock)
        return "";

    if (ChipID == 0x00 && (Clock & 0x80000000))
        Clock &= ~0x40000000;
    if (Clock & 0x80000000)
    {
        Clock &= ~0x80000000;
        ChipID |= 0x80;
    }

    if (Clock & 0x40000000)
        chips.append("2x");
    chips.append(GetAccurateChipName(ChipID, SubType));
    chips.append(", ");

    return chips;
}

FMOD_RESULT F_CALLBACK open(FMOD_CODEC_STATE* codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO* userexinfo)
{
    FMOD_RESULT result;
    auto* plugin = new pluginVgmplayLegacy(codec);


    Info* info = static_cast<Info*>(userexinfo->userdata);


    VGMPlay_Init();
    if (!OpenVGMFile(info->filename.c_str()))
    {
        if (GetGZFileLength(info->filename.c_str()) == 0xFFFFFFFF)
        {
            return FMOD_ERR_FORMAT; // file not found
        }
        else
        {
            return FMOD_ERR_FORMAT; // file invalid
        }
    }


    int freq = 44100;

    plugin->waveformat.format = FMOD_SOUND_FORMAT_PCM16;
    plugin->waveformat.channels = 2;
    plugin->waveformat.frequency = freq;
    plugin->waveformat.pcmblocksize = (16 >> 3) * plugin->waveformat.channels;
    plugin->waveformat.lengthpcm = 0xffffffff;

    codec->waveformat = &(plugin->waveformat);
    codec->numsubsounds = 0;
    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata = plugin; /* user data value */


    VGM_HEADER header;
    GD3_TAG tag;
    GetVGMFileInfo(info->filename.c_str(), &header, &tag);


    wstring title_ws(tag.strTrackNameE);
    string title_s(title_ws.begin(), title_ws.end());
    info->title = title_s;

    wstring artist_ws(tag.strAuthorNameE);
    string artist_s(artist_ws.begin(), artist_ws.end());
    info->artist = artist_s;

    wstring system_ws(tag.strSystemNameE);
    string system_s(system_ws.begin(), system_ws.end());
    info->system = system_s;

    wstring game_ws(tag.strGameNameE);
    string game_s(game_ws.begin(), game_ws.end());
    info->game = game_s;

    wstring comments_ws(tag.strNotes);
    string comments_s(comments_ws.begin(), comments_ws.end());
    info->comments = comments_s;

    wstring date_ws(tag.strReleaseDate);
    string date_s(date_ws.begin(), date_ws.end());
    info->date = date_s;


    wstring dumper_ws(tag.strCreator);
    string dumper_s(dumper_ws.begin(), dumper_ws.end());
    info->dumper = dumper_s;

    info->version = tag.lngVersion;


    INT16 VolMod;
    if (header.bytVolumeModifier <= VOLUME_MODIF_WRAP)
        VolMod = header.bytVolumeModifier;
    else if (header.bytVolumeModifier == (VOLUME_MODIF_WRAP + 0x01))
        VolMod = VOLUME_MODIF_WRAP - 0x100;
    else
        VolMod = header.bytVolumeModifier - 0x100;


    info->gain = pow(2.0, VolMod / (double)0x20);


    UINT8 CurChip;
    UINT8 ChpType;
    UINT32 ChpClk;

    for (CurChip = 0x00; CurChip < CHIP_COUNT; CurChip++)
    {
        ChpClk = GetChipClock(&header, CurChip, &ChpType);
        if (ChpClk && GetChipClock(&header, 0x80 | CurChip, NULL))
            ChpClk |= 0x40000000;
        info->chips.append(PrintChipStr(CurChip, ChpType, ChpClk));
    }

    info->plugin = PLUGIN_vgmplay_legacy;
    info->pluginName = PLUGIN_vgmplay_legacy_NAME;
    info->fileformat = "Video Game Music File";
    info->setSeekable(true);


    UINT32 trackLen;
    if (header.lngLoopSamples)
    {
        trackLen = header.lngTotalSamples;
        plugin->waveformat.lengthpcm = header.lngTotalSamples + (header.lngLoopSamples);
    }
    else
    {
        trackLen = header.lngTotalSamples;
        plugin->waveformat.lengthpcm = header.lngTotalSamples;
    }

    double trackLenMs;
    trackLenMs = (double)trackLen / 44.1;
    info->loopInfo.append(msToNiceStringExact(trackLenMs));

    if (!header.lngLoopSamples)
    {
        info->loopInfo.append(" (no loop)");
    }
    else
    {
        UINT32 introLen = header.lngTotalSamples - header.lngLoopSamples;

        if (introLen < 30000)
        {
            info->loopInfo.append(" (looped)");
        }
        else
        {
            double intro_ms = (double)introLen / 44.1;
            double loop_ms = (double)header.lngLoopSamples / 44.1;
            info->loopInfo.append(" (");
            info->loopInfo.append(msToNiceStringExact(intro_ms));
            info->loopInfo.append(" intro and ");
            info->loopInfo.append(msToNiceStringExact(loop_ms));
            info->loopInfo.append(" loop)");
        }
    }

    VGMPlay_Init2();
    PlayVGM();
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK close(FMOD_CODEC_STATE* codec)
{
    StopVGM();
    VGMPlay_Deinit();
    auto* plugin = static_cast<pluginVgmplayLegacy*>(codec->plugindata);
    delete static_cast<pluginVgmplayLegacy*>(codec->plugindata);
    //delete LogFile;
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK read(FMOD_CODEC_STATE* codec, void* buffer, unsigned int size, unsigned int* read)
{
    auto* plugin = static_cast<pluginVgmplayLegacy*>(codec->plugindata);

    int RetSamples = FillBuffer(static_cast<WAVE_16BS*>(buffer), size);

    *read = size;

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK setposition(FMOD_CODEC_STATE* codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    auto* plugin = static_cast<pluginVgmplayLegacy*>(codec->plugindata);

    if (postype == FMOD_TIMEUNIT_MS)
    {
        UINT32 u = static_cast<UINT32>(position * 44.1);
        //I'm not sure why the above seems to work...
        //The one below will overflow and other shit
        //UINT32 u = (UINT32)((position * 44100 * 2) / 1000);
        //cout << "position: " << position << " ms / " << u << " samples\n";
        SeekVGM(false, u);
    }
    else
    {
        return FMOD_ERR_UNSUPPORTED;
    }
    return FMOD_OK;
}
