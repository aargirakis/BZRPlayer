
extern "C"
{
#include "vgmstream.h"

}
#include <sstream>
#include <iostream>
#include <stdio.h>
#include "fmod_errors.h"
#include "info.h"

FMOD_RESULT F_CALLBACK fcopen(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo);
FMOD_RESULT F_CALLBACK fcclose(FMOD_CODEC_STATE *codec);
FMOD_RESULT F_CALLBACK fcread(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read);
FMOD_RESULT F_CALLBACK fcsetposition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype);

FMOD_CODEC_DESCRIPTION fccodec =
{
    FMOD_CODEC_PLUGIN_VERSION,
    "VGM Stream plugin",// Name.
    0x00010000,                          // Version 0xAAAABBBB   A = major, B = minor.
    0,                                   // Don't force everything using this codec to be a stream
    FMOD_TIMEUNIT_MS,					 // The time format we would like to accept into setposition/getposition.
    &fcopen,                             // Open callback.
    &fcclose,                            // Close callback.
    &fcread,                             // Read callback.
    0,                                   // Getlength callback.  (If not specified FMOD return the length in FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS or FMOD_TIMEUNIT_PCMBYTES units based on the lengthpcm member of the FMOD_CODEC structure).
    &fcsetposition,                      // Setposition callback.
    0,                                   // Getposition callback. (only used for timeunit types that are not FMOD_TIMEUNIT_PCM, FMOD_TIMEUNIT_MS and FMOD_TIMEUNIT_PCMBYTES).
    0                                    // Sound create callback (don't need it)
};

class fcplugin
{
    FMOD_CODEC_STATE *_codec;

public:
    fcplugin(FMOD_CODEC_STATE *codec)
    {
        _codec = codec;
        memset(&fcwaveformat, 0, sizeof(fcwaveformat));
    }

    ~fcplugin()
    {
        //delete some stuff
    }
    VGMSTREAM* vgmstream;
    Info* info;

    FMOD_CODEC_WAVEFORMAT fcwaveformat;
};
/*
    FMODGetCodecDescription is mandatory for every fmod plugin.  This is the symbol the registerplugin function searches for.
    Must be declared with F_API to make it export as stdcall.
    MUST BE EXTERN'ED AS C!  C++ functions will be mangled incorrectly and not load in fmod.
*/
#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) FMOD_CODEC_DESCRIPTION * __stdcall _FMODGetCodecDescription()
{
    return &fccodec;
}

#ifdef __cplusplus
}
#endif


FMOD_RESULT F_CALLBACK fcopen(FMOD_CODEC_STATE *codec, FMOD_MODE usermode, FMOD_CREATESOUNDEXINFO *userexinfo)
{

    fcplugin *fc = new fcplugin(codec);

    fc->info = (Info*)userexinfo->userdata;

    fc->vgmstream = NULL;
    fc->vgmstream = init_vgmstream(fc->info->filename.c_str());
    if (!fc->vgmstream)
    {
        return FMOD_ERR_FORMAT;
    }

    fc->vgmstream->loop_flag = 0;

    /* will we be able to play it? */
    if (fc->vgmstream->channels <= 0)
    {
        close_vgmstream(fc->vgmstream);
        fc->vgmstream=NULL;
        return FMOD_ERR_FORMAT;
    }

    int loop_count = 1;
    int fade_seconds = 0;
    int fade_delay_seconds = 0;
    fc->fcwaveformat.format       = FMOD_SOUND_FORMAT_PCM16;
    fc->fcwaveformat.channels     = fc->vgmstream->channels;
    fc->fcwaveformat.frequency    = fc->vgmstream->sample_rate;
    fc->fcwaveformat.pcmblocksize   = (16 >> 3) * fc->fcwaveformat.channels;
    fc->fcwaveformat.lengthpcm    = get_vgmstream_play_samples(loop_count,fade_seconds,fade_delay_seconds,fc->vgmstream);

    codec->waveformat   = &(fc->fcwaveformat);
    codec->numsubsounds = 0;                    /* number of 'subsounds' in this sound.  For most codecs this is 0, only multi sound codecs such as FSB or CDDA have subsounds. */
    codec->plugindata   = fc;                    /* user data value */

    fc->info->plugin = "vgmstream";
    fc->info->setSeekable(true);


    switch(fc->vgmstream->meta_type)
    {
case meta_DSP_STD: fc->info->fileformat="Nintendo standard GC ADPCM (DSP) header";break;
case meta_DSP_CSTR: fc->info->fileformat="Star Fox Assault \"Cstr\"";break;
case meta_DSP_RS03: fc->info->fileformat="Retro: Metroid Prime 2 \"RS03\"";break;
case meta_DSP_STM: fc->info->fileformat="Paper Mario 2 STM";break;
case meta_AGSC: fc->info->fileformat="Retro: Metroid Prime 2 title";break;
case meta_CSMP: fc->info->fileformat="Retro: Metroid Prime 3 (Wii), Donkey Kong Country Returns (Wii)";break;
case meta_RFRM: fc->info->fileformat="Retro: Donkey Kong Country Tropical Freeze (Wii U)";break;
case meta_DSP_MPDSP: fc->info->fileformat="Monopoly Party single header stereo";break;
case meta_DSP_JETTERS: fc->info->fileformat="Bomberman Jetters .dsp";break;
case meta_DSP_MSS: fc->info->fileformat="Free Radical GC games";break;
case meta_DSP_GCM: fc->info->fileformat="some of Traveller's Tales games";break;
case meta_DSP_STR: fc->info->fileformat="Conan .str files";break;
case meta_DSP_SADB: fc->info->fileformat=".sad";break;
case meta_DSP_WSI: fc->info->fileformat=".wsi";break;
case meta_IDSP_TT: fc->info->fileformat="Traveller's Tales games";break;
case meta_DSP_WII_MUS: fc->info->fileformat=".mus";break;
case meta_DSP_WII_WSD: fc->info->fileformat="Phantom Brave (WII)";break;
case meta_WII_NDP: fc->info->fileformat="Vertigo (Wii)";break;
case meta_DSP_YGO: fc->info->fileformat="Konami: Yu-Gi-Oh! The Falsebound Kingdom (NGC), Hikaru no Go 3 (NGC)";break;
case meta_STRM: fc->info->fileformat="Nintendo STRM";break;
case meta_RSTM: fc->info->fileformat="Nintendo RSTM (Revolution Stream, similar to STRM)";break;
case meta_AFC: fc->info->fileformat="AFC";break;
case meta_AST: fc->info->fileformat="AST";break;
case meta_RWSD: fc->info->fileformat="single-stream RWSD";break;
case meta_RWAR: fc->info->fileformat="single-stream RWAR";break;
case meta_RWAV: fc->info->fileformat="contents of RWAR";break;
case meta_CWAV: fc->info->fileformat="contents of CWAR";break;
case meta_FWAV: fc->info->fileformat="contents of FWAR";break;
case meta_THP: fc->info->fileformat="THP movie files";break;
case meta_SWAV: fc->info->fileformat="SWAV";break;
case meta_NDS_RRDS: fc->info->fileformat="Ridge Racer DS";break;
case meta_WII_BNS: fc->info->fileformat="Wii BNS Banner Sound (similar to RSTM)";break;
case meta_WIIU_BTSND: fc->info->fileformat="Wii U Boot Sound";break;
case meta_ADX_03: fc->info->fileformat="CRI ADX \"type 03\"";break;
case meta_ADX_04: fc->info->fileformat="CRI ADX \"type 04\"";break;
case meta_ADX_05: fc->info->fileformat="CRI ADX \"type 05\"";break;
case meta_AIX: fc->info->fileformat="CRI AIX";break;
case meta_AAX: fc->info->fileformat="CRI AAX";break;
case meta_UTF_DSP: fc->info->fileformat="CRI ADPCM_WII, like AAX with DSP";break;
case meta_DTK: fc->info->fileformat="DTK";break;
case meta_RSF: fc->info->fileformat="RSF";break;
case meta_HALPST: fc->info->fileformat="HAL Labs HALPST";break;
case meta_GCSW: fc->info->fileformat="GCSW (PCM)";break;
case meta_CAF: fc->info->fileformat="tri-Crescendo CAF";break;
case meta_MYSPD: fc->info->fileformat="U-Sing .myspd";break;
case meta_HIS: fc->info->fileformat="Her Ineractive .his";break;
case meta_BNSF: fc->info->fileformat="Bandai Namco Sound Format";break;
case meta_XA: fc->info->fileformat="CD-ROM XA";break;
case meta_ADS: fc->info->fileformat="ADS";break;
case meta_NPS: fc->info->fileformat="NPS";break;
case meta_RXWS: fc->info->fileformat="RXWS";break;
case meta_RAW_INT: fc->info->fileformat="RAW INT";break;
case meta_EXST: fc->info->fileformat="EXST";break;
case meta_SVAG_KCET: fc->info->fileformat="SVAG KCET";break;
case meta_PS_HEADERLESS: fc->info->fileformat="headerless PS-ADPCM";break;
case meta_MIB_MIH: fc->info->fileformat="MIB MIH";break;
case meta_PS2_MIC: fc->info->fileformat="KOEI MIC File";break;
case meta_PS2_VAGi: fc->info->fileformat="VAGi Interleaved File";break;
case meta_PS2_VAGp: fc->info->fileformat="VAGp Mono File";break;
case meta_PS2_pGAV: fc->info->fileformat="VAGp with Little Endian Header";break;
case meta_PS2_VAGp_AAAP: fc->info->fileformat="Acclaim Austin Audio VAG header";break;
case meta_SEB: fc->info->fileformat="SEB";break;
case meta_STR_WAV: fc->info->fileformat="Blitz Games STR+WAV files";break;
case meta_ILD: fc->info->fileformat="ILD";break;
case meta_PS2_PNB: fc->info->fileformat="PsychoNauts Bgm File";break;
case meta_VPK: fc->info->fileformat="VPK Audio File";break;
case meta_PS2_BMDX: fc->info->fileformat="Beatmania thing";break;
case meta_PS2_IVB: fc->info->fileformat="Langrisser 3 IVB";break;
case meta_PS2_SND: fc->info->fileformat="some Might & Magics SSND header";break;
case meta_SVS: fc->info->fileformat="Square SVS";break;
case meta_XSS: fc->info->fileformat="Dino Crisis 3";break;
case meta_SL3: fc->info->fileformat="Test Drive Unlimited";break;
case meta_HGC1: fc->info->fileformat="Knights of the Temple 2";break;
case meta_AUS: fc->info->fileformat="Various Capcom games";break;
case meta_RWS: fc->info->fileformat="RenderWare games (only when using RW Audio middleware)";break;
case meta_FSB1: fc->info->fileformat="FMOD Sample Bank: version 1";break;
case meta_FSB2: fc->info->fileformat="FMOD Sample Bank: version 2";break;
case meta_FSB3: fc->info->fileformat="FMOD Sample Bank: version 3.0/3.1";break;
case meta_FSB4: fc->info->fileformat="FMOD Sample Bank: version 4";break;
case meta_FSB5: fc->info->fileformat="FMOD Sample Bank: version 5";break;
case meta_RWX: fc->info->fileformat="Air Force Delta Storm (XBOX)";break;
case meta_XWB: fc->info->fileformat="icrosoft XACT framework (Xbox, X360, Windows)";break;
case meta_PS2_XA30: fc->info->fileformat="Driver - Parallel Lines (PS2)";break;
case meta_MUSC: fc->info->fileformat="Krome PS2 games";break;
case meta_MUSX: fc->info->fileformat="MUSX";break;
case meta_LEG: fc->info->fileformat="Legaia 2 [no header_id]";break;
case meta_FILP: fc->info->fileformat="Resident Evil - Dead Aim";break;
case meta_IKM: fc->info->fileformat="IKM";break;
case meta_STER: fc->info->fileformat="STER";break;
case meta_BG00: fc->info->fileformat="Ibara: Mushihimesama";break;
case meta_PS2_RSTM: fc->info->fileformat="Midnight Club 3";break;
case meta_PS2_KCES: fc->info->fileformat="Dance Dance Revolution";break;
case meta_HXD: fc->info->fileformat="HXS";break;
case meta_VSV: fc->info->fileformat="VSV";break;
case meta_SCD_PCM: fc->info->fileformat="Lunar - Eternal Blue";break;
case meta_PS2_PCM: fc->info->fileformat="Konami KCEJ East: Ephemeral Fantasia, Yu-Gi-Oh! The Duelists of the Roses, 7 Blades";break;
case meta_PS2_RKV: fc->info->fileformat="Legacy of Kain - Blood Omen 2 (PS2)";break;
case meta_PS2_VAS: fc->info->fileformat="Pro Baseball Spirits 5";break;
case meta_PS2_ENTH: fc->info->fileformat="Enthusia";break;
case meta_SDT: fc->info->fileformat="Baldur's Gate - Dark Alliance";break;
case meta_NGC_TYDSP: fc->info->fileformat="Ty - The Tasmanian Tiger";break;
case meta_DC_STR: fc->info->fileformat="SEGA Stream Asset Builder";break;
case meta_DC_STR_V2: fc->info->fileformat="variant of SEGA Stream Asset Builder";break;
case meta_NGC_BH2PCM: fc->info->fileformat="Bio Hazard 2";break;
case meta_SAP: fc->info->fileformat="SAP";break;
case meta_DC_IDVI: fc->info->fileformat="Eldorado Gate";break;
case meta_KRAW: fc->info->fileformat="Geometry Wars - Galaxies";break;
case meta_PS2_OMU: fc->info->fileformat="PS2 Int file with Header";break;
case meta_PS2_XA2: fc->info->fileformat="XG3 Extreme-G Racing";break;
case meta_NUB: fc->info->fileformat="NUB";break;
case meta_IDSP_NL: fc->info->fileformat="Mario Strikers Charged (Wii)";break;
case meta_IDSP_IE: fc->info->fileformat="Defencer (GC)";break;
case meta_SPT_SPD: fc->info->fileformat="Various (SPT+SPT DSP)";break;
case meta_ISH_ISD: fc->info->fileformat="Various (ISH+ISD DSP)";break;
case meta_GSP_GSB: fc->info->fileformat="Tecmo games (Super Swing Golf 1 & 2, Quamtum Theory)";break;
case meta_YDSP: fc->info->fileformat="WWE Day of Reckoning";break;
case meta_FFCC_STR: fc->info->fileformat="Final Fantasy: Crystal Chronicles";break;
case meta_UBI_JADE: fc->info->fileformat="Beyond Good & Evil, Rayman Raving Rabbids";break;
case meta_GCA: fc->info->fileformat="Metal Slug Anthology";break;
case meta_NGC_SSM: fc->info->fileformat="Golden Gashbell Full Power";break;
case meta_PS2_JOE: fc->info->fileformat="Wall-E / Pixar games";break;
case meta_NGC_YMF: fc->info->fileformat="WWE WrestleMania X8";break;
case meta_SADL: fc->info->fileformat="SADL";break;
case meta_FAG: fc->info->fileformat="Jackie Chan - Stuntmaster";break;
case meta_PS2_MIHB: fc->info->fileformat="Merged MIH+MIB";break;
case meta_NGC_PDT: fc->info->fileformat="Mario Party 6";break;
case meta_DC_ASD: fc->info->fileformat="Miss Moonligh";break;
case meta_SPSD: fc->info->fileformat="Guilty Gear X";break;
case meta_RSD: fc->info->fileformat="RSD";break;
case meta_PS2_ASS: fc->info->fileformat="ASS";break;
case meta_SEG: fc->info->fileformat="Eragon";break;
case meta_NDS_STRM_FFTA2: fc->info->fileformat="Final Fantasy Tactics A2";break;
case meta_KNON: fc->info->fileformat="KNON";break;
case meta_ZWDSP: fc->info->fileformat="Zack and Wiki";break;
case meta_VGS: fc->info->fileformat="Guitar Hero Encore - Rocks the 80s";break;
case meta_DCS_WAV: fc->info->fileformat="DCS WAV";break;
case meta_SMP: fc->info->fileformat="SMP";break;
case meta_WII_SNG: fc->info->fileformat="Excite Trucks";break;
case meta_MUL: fc->info->fileformat="MUL";break;
case meta_SAT_BAKA: fc->info->fileformat="Crypt Killer";break;
case meta_VSF: fc->info->fileformat="VSF";break;
case meta_PS2_VSF_TTA: fc->info->fileformat="Tiny Toon Adventures: Defenders of the Universe";break;
case meta_ADS_MIDWAY: fc->info->fileformat="ADS MIDWAY";break;
case meta_PS2_SPS: fc->info->fileformat="Ape Escape 2";break;
case meta_PS2_XA2_RRP: fc->info->fileformat="RC Revenge Pro";break;
case meta_NGC_DSP_KONAMI: fc->info->fileformat="Konami DSP header, found in various games";break;
case meta_UBI_CKD: fc->info->fileformat="Ubisoft CKD RIFF header (Rayman Origins Wii)";break;
case meta_RAW_WAVM: fc->info->fileformat="RAW WAVM";break;
case meta_WVS: fc->info->fileformat="WVS";break;
case meta_XBOX_MATX: fc->info->fileformat="XBOX MATX";break;
case meta_XMU: fc->info->fileformat="XMU";break;
case meta_XVAS: fc->info->fileformat="XVAS";break;
case meta_EA_SCHL: fc->info->fileformat="Electronic Arts SCHl with variable header";break;
case meta_EA_SCHL_fixed: fc->info->fileformat="Electronic Arts SCHl with fixed header";break;
case meta_EA_BNK: fc->info->fileformat="Electronic Arts BNK";break;
case meta_EA_1SNH: fc->info->fileformat="Electronic Arts 1SNh/EACS";break;
case meta_EA_EACS: fc->info->fileformat="EA EACS";break;
case meta_RAW_PCM: fc->info->fileformat="RAW PCM";break;
case meta_GENH: fc->info->fileformat="generic header";break;
case meta_AIFC: fc->info->fileformat="Audio Interchange File Format AIFF-C";break;
case meta_AIFF: fc->info->fileformat="Audio Interchange File Format";break;
case meta_STR_SNDS: fc->info->fileformat=".str with SNDS blocks and SHDR header";break;
case meta_WS_AUD: fc->info->fileformat="Westwood Studios .aud";break;
case meta_WS_AUD_old: fc->info->fileformat="Westwood Studios .aud, old style";break;
case meta_RIFF_WAVE: fc->info->fileformat="RIFF, for WAVs";break;
case meta_RIFF_WAVE_POS: fc->info->fileformat=".wav + .pos for looping (Ys Complete PC)";break;
case meta_RIFF_WAVE_labl: fc->info->fileformat="RIFF w/ loop Markers in LIST-adtl-labl";break;
case meta_RIFF_WAVE_smpl: fc->info->fileformat="RIFF w/ loop data in smpl chunk";break;
case meta_RIFF_WAVE_wsmp: fc->info->fileformat="RIFF w/ loop data in wsmp chunk";break;
case meta_RIFF_WAVE_MWV: fc->info->fileformat=".mwv RIFF w/ loop data in ctrl chunk pflt";break;
case meta_RIFX_WAVE: fc->info->fileformat="RIFX, for big-endian WAVs";break;
case meta_RIFX_WAVE_smpl: fc->info->fileformat="RIFX w/ loop data in smpl chunk";break;
case meta_XNB: fc->info->fileformat="XNA Game Studio 4.0";break;
case meta_PC_MXST: fc->info->fileformat="Lego Island MxSt";break;
case meta_SAB: fc->info->fileformat="Worms 4 Mayhem SAB+SOB file";break;
case meta_NWA: fc->info->fileformat="Visual Art's NWA";break;
case meta_NWA_NWAINFOINI: fc->info->fileformat="Visual Art's NWA w/ NWAINFO.INI for looping";break;
case meta_NWA_GAMEEXEINI: fc->info->fileformat="Visual Art's NWA w/ Gameexe.ini for looping";break;
case meta_SAT_DVI: fc->info->fileformat="Konami KCE Nagoya DVI (SAT games)";break;
case meta_DC_KCEY: fc->info->fileformat="Konami KCE Yokohama KCEYCOMP (DC games)";break;
case meta_ACM: fc->info->fileformat="InterPlay ACM header";break;
case meta_MUS_ACM: fc->info->fileformat="MUS playlist of InterPlay ACM files";break;
case meta_DEC: fc->info->fileformat="Falcom PC games (Xanadu Next, Gurumin)";break;
case meta_VS: fc->info->fileformat="Men in Black .vs";break;
case meta_FFXI_BGW: fc->info->fileformat="FFXI (PC) BGW";break;
case meta_FFXI_SPW: fc->info->fileformat="FFXI (PC) SPW";break;
case meta_STS: fc->info->fileformat="STS";break;
case meta_PS2_P2BT: fc->info->fileformat="Pop'n'Music 7 Audio File";break;
case meta_PS2_GBTS: fc->info->fileformat="Pop'n'Music 9 Audio File";break;
case meta_NGC_DSP_IADP: fc->info->fileformat="Gamecube Interleave DSP";break;
case meta_PS2_TK5: fc->info->fileformat="Tekken 5 Stream Files";break;
case meta_PS2_MCG: fc->info->fileformat="Gunvari MCG Files (was name .GCM on disk)";break;
case meta_ZSD: fc->info->fileformat="Dragon Booster ZSD";break;
case meta_REDSPARK: fc->info->fileformat="\"RedSpark\" RSD (MadWorld)";break;
case meta_IVAUD: fc->info->fileformat=".ivaud GTA IV";break;
case meta_NDS_HWAS: fc->info->fileformat="Spider-Man 3, Tony Hawk's Downhill Jam, possibly more...";break;
case meta_NGC_LPS: fc->info->fileformat="Rave Master (Groove Adventure Rave)(GC)";break;
case meta_NAOMI_ADPCM: fc->info->fileformat="NAOMI/NAOMI2 ARcade games";break;
case meta_SD9: fc->info->fileformat="beatmaniaIIDX16 - EMPRESS (Arcade)";break;
case meta_2DX9: fc->info->fileformat="beatmaniaIIDX16 - EMPRESS (Arcade)";break;
case meta_PS2_VGV: fc->info->fileformat="Rune: Viking Warlord";break;
case meta_GCUB: fc->info->fileformat="GCUB";break;
case meta_MAXIS_XA: fc->info->fileformat="Sim City 3000 (PC)";break;
case meta_NGC_SCK_DSP: fc->info->fileformat="Scorpion King (NGC)";break;
case meta_CAFF: fc->info->fileformat="iPhone .caf";break;
case meta_EXAKT_SC: fc->info->fileformat="Activision EXAKT .SC (PS2)";break;
case meta_WII_WAS: fc->info->fileformat="DiRT 2 (WII)";break;
case meta_PONA_3DO: fc->info->fileformat="Policenauts (3DO)";break;
case meta_PONA_PSX: fc->info->fileformat="Policenauts (PSX)";break;
case meta_XBOX_HLWAV: fc->info->fileformat="Half Life 2 (XBOX)";break;
case meta_AST_MV: fc->info->fileformat="AST MV";break;
case meta_AST_MMV: fc->info->fileformat="AST MMV";break;
case meta_DMSG: fc->info->fileformat="Nightcaster II - Equinox (XBOX)";break;
case meta_NGC_DSP_AAAP: fc->info->fileformat="Turok: Evolution (NGC), Vexx (NGC)";break;
case meta_PS2_WB: fc->info->fileformat="Shooting Love. ~TRIZEAL~";break;
case meta_S14: fc->info->fileformat="raw Siren 14, 24kbit mono";break;
case meta_SSS: fc->info->fileformat="raw Siren 14, 48kbit stereo";break;
case meta_PS2_GCM: fc->info->fileformat="NamCollection";break;
case meta_PS2_SMPL: fc->info->fileformat="Homura";break;
case meta_PS2_MSA: fc->info->fileformat="Psyvariar -Complete Edition-";break;
case meta_PS2_VOI: fc->info->fileformat="RAW Danger (Zettaizetsumei Toshi 2 - Itetsuita Kiokutachi) [PS2]";break;
case meta_P3D: fc->info->fileformat="Prototype P3D";break;
case meta_PS2_TK1: fc->info->fileformat="Tekken (NamCollection)";break;
case meta_NGC_RKV: fc->info->fileformat="Legacy of Kain - Blood Omen 2 (GC)";break;
case meta_DSP_DDSP: fc->info->fileformat="Various (2 dsp files stuck together";break;
case meta_NGC_DSP_MPDS: fc->info->fileformat="Big Air Freestyle, Terminator 3";break;
case meta_DSP_STR_IG: fc->info->fileformat="Micro Machines, Superman Superman: Shadow of Apokolis";break;
case meta_EA_SWVR: fc->info->fileformat="Future Cop L.A.P.D., Freekstyle";break;
case meta_PS2_B1S: fc->info->fileformat="7 Wonders of the ancient world";break;
case meta_PS2_WAD: fc->info->fileformat="The golden Compass";break;
case meta_DSP_XIII: fc->info->fileformat="XIII, possibly more (Ubisoft header???)";break;
case meta_DSP_CABELAS: fc->info->fileformat="Cabelas games";break;
case meta_PS2_ADM: fc->info->fileformat="Dragon Quest V (PS2)";break;
case meta_LPCM_SHADE: fc->info->fileformat="LPCM SHADE";break;
case meta_DSP_BDSP: fc->info->fileformat="Ah! My Goddess";break;
case meta_PS2_VMS: fc->info->fileformat="Autobahn Raser - Police Madness";break;
case meta_XAU: fc->info->fileformat="XPEC Entertainment (Beat Down (PS2 Xbox), Spectral Force Chronicle (PS2))";break;
case meta_GH3_BAR: fc->info->fileformat="Guitar Hero III Mobile .bar";break;
case meta_FFW: fc->info->fileformat="Freedom Fighters [NGC]";break;
case meta_DSP_DSPW: fc->info->fileformat="Sengoku Basara 3 [WII]";break;
case meta_PS2_JSTM: fc->info->fileformat="Tantei Jinguji Saburo - Kind of Blue (PS2)";break;
case meta_SQEX_SCD: fc->info->fileformat="Square-Enix SCD";break;
case meta_NGC_NST_DSP: fc->info->fileformat="Animaniacs [NGC]";break;
case meta_BAF: fc->info->fileformat="Bizarre Creations (Blur, James Bond)";break;
case meta_XVAG: fc->info->fileformat="Ratchet & Clank Future: Quest for Booty (PS3)";break;
case meta_CPS: fc->info->fileformat="Eternal Sonata (PS3)";break;
case meta_MSF: fc->info->fileformat="MSF";break;
case meta_PS3_PAST: fc->info->fileformat="Bakugan Battle Brawlers (PS3)";break;
case meta_SGXD: fc->info->fileformat="Sony: Folklore, Genji, Tokyo Jungle (PS3), Brave Story, Kurohyo (PSP)";break;
case meta_WII_RAS: fc->info->fileformat="Donkey Kong Country Returns (Wii)";break;
case meta_SPM: fc->info->fileformat="SPM";break;
case meta_VGS_PS: fc->info->fileformat="VGS PS";break;
case meta_PS2_IAB: fc->info->fileformat="Ueki no Housoku - Taosu ze Robert Juudan!! (PS2)";break;
case meta_VS_STR: fc->info->fileformat="The Bouncer";break;
case meta_LSF_N1NJ4N: fc->info->fileformat=".lsf n1nj4n Fastlane Street Racing (iPhone)";break;
case meta_XWAV: fc->info->fileformat="XWAV";break;
case meta_RAW_SNDS: fc->info->fileformat="RAW SNDS";break;
case meta_PS2_WMUS: fc->info->fileformat="The Warriors (PS2)";break;
case meta_HYPERSCAN_KVAG: fc->info->fileformat="Hyperscan KVAG/BVG";break;
case meta_IOS_PSND: fc->info->fileformat="Crash Bandicoot Nitro Kart 2 (iOS)";break;
case meta_BOS_ADP: fc->info->fileformat="BOS ADP";break;
case meta_QD_ADP: fc->info->fileformat="QD ADP";break;
case meta_EB_SFX: fc->info->fileformat="Excitebots .sfx";break;
case meta_EB_SF0: fc->info->fileformat="Excitebots .sf0";break;
case meta_MTAF: fc->info->fileformat="MTAF";break;
case meta_PS2_VAG1: fc->info->fileformat="Metal Gear Solid 3 VAG1";break;
case meta_PS2_VAG2: fc->info->fileformat="Metal Gear Solid 3 VAG2";break;
case meta_ALP: fc->info->fileformat="ALP";break;
case meta_WPD: fc->info->fileformat="Shuffle! (PC)";break;
case meta_MN_STR: fc->info->fileformat="Mini Ninjas (PC/PS3/WII)";break;
case meta_MSS: fc->info->fileformat="Guerilla: ShellShock Nam '67 (PS2/Xbox), Killzone (PS2)";break;
case meta_PS2_HSF: fc->info->fileformat="Lowrider (PS2)";break;
case meta_IVAG: fc->info->fileformat="IVAG";break;
case meta_PS2_2PFS: fc->info->fileformat="Konami: Mahoromatic: Moetto - KiraKira Maid-San, GANTZ (PS2)";break;
case meta_PS2_VBK: fc->info->fileformat="Disney's Stitch - Experiment 626";break;
case meta_OTM: fc->info->fileformat="Otomedius (Arcade)";break;
case meta_CSTM: fc->info->fileformat="Nintendo 3DS CSTM (Century Stream)";break;
case meta_FSTM: fc->info->fileformat="Nintendo Wii U FSTM (caFe? Stream)";break;
case meta_IDSP_NAMCO: fc->info->fileformat="IDSP NAMCO";break;
case meta_KT_WIIBGM: fc->info->fileformat="Koei Tecmo WiiBGM";break;
case meta_KTSS: fc->info->fileformat="Koei Tecmo Nintendo Stream (KNS)";break;
case meta_MCA: fc->info->fileformat="Capcom MCA \"MADP\"";break;
case meta_XB3D_ADX: fc->info->fileformat="Xenoblade Chronicles 3D ADX";break;
case meta_HCA: fc->info->fileformat="CRI HCA";break;
case meta_SVAG_SNK: fc->info->fileformat="SVAG SNK";break;
case meta_PS2_VDS_VDM: fc->info->fileformat="Graffiti Kingdom";break;
case meta_FFMPEG: fc->info->fileformat="FFMPEG";break;
case meta_FFMPEG_faulty: fc->info->fileformat="FFMPEG faulty";break;
case meta_CXS: fc->info->fileformat="Eternal Sonata (Xbox 360)";break;
case meta_AKB: fc->info->fileformat="SQEX iOS";break;
case meta_PASX: fc->info->fileformat="Namco PASX (Soul Calibur II HD X360)";break;
case meta_XMA_RIFF: fc->info->fileformat="Microsoft RIFF XMA";break;
case meta_ASTB: fc->info->fileformat="Dead Rising (X360)";break;
case meta_WWISE_RIFF: fc->info->fileformat="Audiokinetic Wwise RIFF/RIFX";break;
case meta_UBI_RAKI: fc->info->fileformat="Ubisoft RAKI header (Rayman Legends, Just Dance 2017)";break;
case meta_SXD: fc->info->fileformat="Sony SXD (Gravity Rush, Freedom Wars PSV)";break;
case meta_OGL: fc->info->fileformat="Shin'en Wii/WiiU (Jett Rocket (Wii), FAST Racing NEO (WiiU))";break;
case meta_MC3: fc->info->fileformat="Paradigm games (T3 PS2, MX Rider PS2, MI: Operation Surma PS2)";break;
case meta_GHS: fc->info->fileformat="Knights Contract (X360/PS3), Valhalla Knights 3 (PSV)";break;
case meta_AAC_TRIACE: fc->info->fileformat="TA AAC";break;
case meta_MTA2: fc->info->fileformat="MTA2";break;
case meta_NGC_ULW: fc->info->fileformat="Burnout 1 (GC only)";break;
case meta_XA_XA30: fc->info->fileformat="XA XA30";break;
case meta_XA_04SW: fc->info->fileformat="XA 04SW";break;
case meta_TXTH: fc->info->fileformat="generic text header";break;
case meta_SK_AUD: fc->info->fileformat="Silicon Knights .AUD (Eternal Darkness GC)";break;
case meta_AHX: fc->info->fileformat="CRI AHX header";break;
case meta_STM: fc->info->fileformat="Angel Studios/Rockstar San Diego Games";break;
case meta_BINK: fc->info->fileformat="RAD Game Tools BINK audio/video";break;
case meta_EA_SNU: fc->info->fileformat="Electronic Arts SNU (Dead Space)";break;
case meta_AWC: fc->info->fileformat="Rockstar AWC (GTA5, RDR)";break;
case meta_OPUS: fc->info->fileformat="Nintendo Opus [Lego City Undercover (Switch)]";break;
case meta_RAW_AL: fc->info->fileformat="RAW AL";break;
case meta_PC_AST: fc->info->fileformat="Dead Rising (PC)";break;
case meta_NAAC: fc->info->fileformat="Namco AAC (3DS)";break;
case meta_UBI_SB: fc->info->fileformat="Ubisoft banks";break;
case meta_EZW: fc->info->fileformat="EZ2DJ (Arcade) EZWAV";break;
case meta_VXN: fc->info->fileformat="Gameloft mobile games";break;
case meta_EA_SNR_SNS: fc->info->fileformat="Electronic Arts SNR+SNS (Burnout Paradise)";break;
case meta_EA_SPS: fc->info->fileformat="Electronic Arts SPS (Burnout Crash)";break;
case meta_VID1: fc->info->fileformat="VID1";break;
case meta_PC_FLX: fc->info->fileformat="Ultima IX PC";break;
case meta_MOGG: fc->info->fileformat="Harmonix Music Systems MOGG Vorbis";break;
case meta_OGG_VORBIS: fc->info->fileformat="Ogg Vorbis";break;
case meta_OGG_SLI: fc->info->fileformat="Ogg Vorbis file w/ companion .sli for looping";break;
case meta_OPUS_SLI: fc->info->fileformat="Ogg Opus file w/ companion .sli for looping";break;
case meta_OGG_SFL: fc->info->fileformat="Ogg Vorbis file w/ .sfl (RIFF SFPL) for looping";break;
case meta_OGG_KOVS: fc->info->fileformat="Ogg Vorbis with header and encryption (Koei Tecmo Games)";break;
case meta_OGG_encrypted: fc->info->fileformat="Ogg Vorbis with encryption";break;
case meta_KMA9: fc->info->fileformat="Koei Tecmo [Nobunaga no Yabou - Souzou (Vita)]";break;
case meta_XWC: fc->info->fileformat="Starbreeze games";break;
case meta_SQEX_SAB: fc->info->fileformat="Square-Enix newest middleware (sound)";break;
case meta_SQEX_MAB: fc->info->fileformat="Square-Enix newest middleware (music)";break;
case meta_WAF: fc->info->fileformat="KID WAF [Ever 17 (PC)]";break;
case meta_WAVE: fc->info->fileformat="EngineBlack games [Mighty Switch Force! (3DS)]";break;
case meta_WAVE_segmented: fc->info->fileformat="EngineBlack games, segmented [Shantae and the Pirate's Curse (PC)]";break;
case meta_SMV: fc->info->fileformat="Cho Aniki Zero (PSP)";break;
case meta_NXAP: fc->info->fileformat="Nex Entertainment games [Time Crisis 4 (PS3), Time Crisis Razing Storm (PS3)]";break;
case meta_EA_WVE_AU00: fc->info->fileformat="Electronic Arts PS movies [Future Cop - L.A.P.D. (PS), Supercross 2000 (PS)]";break;
case meta_EA_WVE_AD10: fc->info->fileformat="Electronic Arts PS movies [Wing Commander 3/4 (PS)]";break;
case meta_STHD: fc->info->fileformat="STHD .stx [Kakuto Chojin (Xbox)]";break;
case meta_MP4: fc->info->fileformat="MP4/AAC";break;
case meta_PCM_SRE: fc->info->fileformat=".PCM+SRE [Viewtiful Joe (PS2)]";break;
case meta_DSP_MCADPCM: fc->info->fileformat="Skyrim (Switch)";break;
case meta_UBI_LYN: fc->info->fileformat="Ubisoft LyN engine [The Adventures of Tintin (multi)]";break;
case meta_MSB_MSH: fc->info->fileformat="sfx companion of MIH+MIB";break;
case meta_TXTP: fc->info->fileformat="generic text playlist";break;
case meta_SMC_SMH: fc->info->fileformat="Wangan Midnight (System 246)";break;
case meta_PPST: fc->info->fileformat="PPST [Parappa the Rapper (PSP)]";break;
case meta_SPS_N1: fc->info->fileformat="SPS N1";break;
case meta_UBI_BAO: fc->info->fileformat="Ubisoft BAO";break;
case meta_DSP_SWITCH_AUDIO: fc->info->fileformat="  /* Gal Gun 2 (Switch)";break;
case meta_H4M: fc->info->fileformat="Hudson HVQM4 video [Resident Evil 0 (GC), Tales of Symphonia (GC)]";break;
case meta_ASF: fc->info->fileformat="Argonaut ASF [Croc 2 (PC)]";break;
case meta_XMD: fc->info->fileformat="Konami XMD [Silent Hill 4 (Xbox), Castlevania: Curse of Darkness (Xbox)]";break;
case meta_CKS: fc->info->fileformat="Cricket Audio stream [Part Time UFO (Android), Mega Man 1-6 (Android)]";break;
case meta_CKB: fc->info->fileformat="Cricket Audio bank [Fire Emblem Heroes (Android), Mega Man 1-6 (Android)]";break;
case meta_WV6: fc->info->fileformat="Gorilla Systems PC games";break;
case meta_WAVEBATCH: fc->info->fileformat="Firebrand Games";break;
case meta_HD3_BD3: fc->info->fileformat="Sony PS3 bank";break;
case meta_BNK_SONY: fc->info->fileformat="Sony Scream Tool bank";break;
case meta_SSCF: fc->info->fileformat="Square Enix SCD old version";break;
case meta_DSP_VAG: fc->info->fileformat="Penny-Punching Princess (Switch) sfx";break;
case meta_DSP_ITL: fc->info->fileformat="Charinko Hero (GC)";break;
case meta_A2M: fc->info->fileformat="Scooby-Doo! Unmasked (PS2)";break;
case meta_AHV: fc->info->fileformat="Headhunter (PS2)";break;
case meta_MSV: fc->info->fileformat="MSV";break;
case meta_SDF: fc->info->fileformat="SDF";break;
case meta_SVG: fc->info->fileformat="Hunter - The Reckoning - Wayward (PS2)";break;
case meta_VIS: fc->info->fileformat="AirForce Delta Strike (PS2)";break;
case meta_VAI: fc->info->fileformat="Ratatouille (GC)";break;
case meta_AIF_ASOBO: fc->info->fileformat="Ratatouille (PC)";break;
case meta_AO: fc->info->fileformat="Cloudphobia (PC)";break;
case meta_APC: fc->info->fileformat="MegaRace 3 (PC)";break;
case meta_WV2: fc->info->fileformat="Slave Zero (PC)";break;
case meta_XAU_KONAMI: fc->info->fileformat="Yu-Gi-Oh - The Dawn of Destiny (Xbox)";break;
case meta_DERF: fc->info->fileformat="Stupid Invaders (PC)";break;
case meta_SADF: fc->info->fileformat="SADF";break;
case meta_UTK: fc->info->fileformat="UTK";break;
case meta_NXA: fc->info->fileformat="NXA";break;
case meta_ADPCM_CAPCOM: fc->info->fileformat="ADPCM CAPCOM";break;
case meta_UE4OPUS: fc->info->fileformat="UE4OPUS";break;
case meta_XWMA: fc->info->fileformat="XWMA";break;
case meta_VA3: fc->info->fileformat="DDR Supernova 2 AC";break;
case meta_XOPUS: fc->info->fileformat="XOPUS";break;
case meta_VS_SQUARE: fc->info->fileformat="VS SQUARE";break;
case meta_NWAV: fc->info->fileformat="NWAV";break;
case meta_XPCM: fc->info->fileformat="XPCM";break;
case meta_MSF_TAMASOFT: fc->info->fileformat="MSF TAMASOFT";break;
case meta_XPS_DAT: fc->info->fileformat="XPS DAT";break;
case meta_ZSND: fc->info->fileformat="ZSND";break;
case meta_DSP_ADPY: fc->info->fileformat="DSP ADPY";break;
case meta_DSP_ADPX: fc->info->fileformat="DSP ADPX";break;
case meta_OGG_OPUS: fc->info->fileformat="OGG OPUS";break;
case meta_IMC: fc->info->fileformat="IMC";break;
case meta_GIN: fc->info->fileformat="GIN";break;
case meta_DSF: fc->info->fileformat="DSF";break;
case meta_208: fc->info->fileformat="208";break;
case meta_DSP_DS2: fc->info->fileformat="DSP DS2";break;
case meta_MUS_VC: fc->info->fileformat="MUS VC";break;
case meta_STRM_ABYLIGHT: fc->info->fileformat="STRM ABYLIGHT";break;
case meta_MSF_KONAMI: fc->info->fileformat="MSF KONAMI";break;
case meta_XWMA_KONAMI: fc->info->fileformat="XWMA KONAMI";break;
case meta_9TAV: fc->info->fileformat="9TAV";break;
case meta_BWAV: fc->info->fileformat="BWAV";break;
case meta_RAD: fc->info->fileformat="RAD";break;
case meta_SMACKER: fc->info->fileformat="SMACKER";break;
case meta_MZRT: fc->info->fileformat="MZRT";break;
case meta_XAVS: fc->info->fileformat="XAVS";break;
case meta_PSF: fc->info->fileformat="PSF";break;
case meta_DSP_ITL_i: fc->info->fileformat="DSP ITL i";break;
case meta_IMA: fc->info->fileformat="IMA";break;
case meta_XWV_VALVE: fc->info->fileformat="XMV VALVE";break;
case meta_UBI_HX: fc->info->fileformat="UBI HX";break;
case meta_BMP_KONAMI: fc->info->fileformat="BMP KONAMI";break;
case meta_ISB: fc->info->fileformat="ISB";break;
case meta_XSSB: fc->info->fileformat="XSSB";break;
case meta_XMA_UE3: fc->info->fileformat="XMA UE3";break;
case meta_FWSE: fc->info->fileformat="FWSE";break;
case meta_FDA: fc->info->fileformat="FDA";break;
case meta_TGC: fc->info->fileformat="TGC";break;
case meta_KWB: fc->info->fileformat="KWB";break;
case meta_LRMD: fc->info->fileformat="LRMD";break;
case meta_WWISE_FX: fc->info->fileformat="WWISE FX";break;
case meta_DIVA: fc->info->fileformat="DIVA";break;
case meta_IMUSE: fc->info->fileformat="IMUSE";break;
case meta_KTSR: fc->info->fileformat="KTSR";break;
case meta_KAT: fc->info->fileformat="KAT";break;
case meta_PCM_SUCCESS: fc->info->fileformat="PCM SUCCESS";break;
case meta_ADP_KONAMI: fc->info->fileformat="ADP KONAMI";break;
case meta_SDRH: fc->info->fileformat="SDRH";break;
case meta_WADY: fc->info->fileformat="WADY";break;
case meta_DSP_SQEX: fc->info->fileformat="DSP SQUEX";break;
case meta_DSP_WIIVOICE: fc->info->fileformat="DSP WIIVOICE";break;
case meta_SBK: fc->info->fileformat="SBK";break;
case meta_DSP_WIIADPCM: fc->info->fileformat="DSP WIIADPCM";break;
case meta_DSP_CWAC: fc->info->fileformat="DSP CWAC";break;
case meta_COMPRESSWAVE: fc->info->fileformat="COMPRESSWAVE";break;
case meta_KTAC: fc->info->fileformat="KTAC";break;
case meta_MJB_MJH: fc->info->fileformat="MJB MJH";break;
case meta_BSNF: fc->info->fileformat="BSNF";break;
case meta_TAC: fc->info->fileformat="TAC";break;
case meta_IDSP_TOSE: fc->info->fileformat="IDSP TOSE";break;
case meta_DSP_KWA: fc->info->fileformat="DSP KWA";break;
case meta_OGV_3RDEYE: fc->info->fileformat="OGV 3RDEYE";break;
case meta_PIFF_TPCM: fc->info->fileformat="PIFF TPCM";break;
case meta_WXD_WXH: fc->info->fileformat="WXD WXH";break;
case meta_BNK_RELIC: fc->info->fileformat="BNK RELIC";break;
case meta_XSH_XSD_XSS: fc->info->fileformat="XSH XSD XSS";break;
case meta_PSB: fc->info->fileformat="PSB";break;
case meta_LOPU_FB: fc->info->fileformat="LOPU FB";break;
case meta_LPCM_FB: fc->info->fileformat="LPCM FB";break;
case meta_WBK: fc->info->fileformat="WKB";break;
case meta_WBK_NSLB: fc->info->fileformat="WKB NSLB";break;
case meta_DSP_APEX: fc->info->fileformat="DSP APEX";break;
case meta_MPEG: fc->info->fileformat="MPEG";break;
case meta_SSPF: fc->info->fileformat="SSPV";break;
case meta_S3V: fc->info->fileformat="S3V";break;
case meta_ESF: fc->info->fileformat="ESF";break;
case meta_ADM3: fc->info->fileformat="ADM3";break;
case meta_TT_AD: fc->info->fileformat="TT AD";break;
case meta_SNDZ: fc->info->fileformat="SNDZ";break;
case meta_VAB: fc->info->fileformat="VAB";break;
case meta_BIGRP: fc->info->fileformat="BIGRP";break;


    default:
        string s;
        stringstream out;
        out << "Unknown VGMStream (" << fc->vgmstream->meta_type << ")";
        s = out.str();
        fc->info->fileformat=s;
    }

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fcclose(FMOD_CODEC_STATE *codec)
{
    //fcplugin* fc = (fcplugin*)codec->plugindata;
    //close_vgmstream(fc->vgmstream);
    delete (fcplugin*)codec->plugindata;
    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fcread(FMOD_CODEC_STATE *codec, void *buffer, unsigned int size, unsigned int *read)
{
    fcplugin* fc = (fcplugin*)codec->plugindata;
    render_vgmstream((sample*)buffer,size,fc->vgmstream);
    *read=size;

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fcsetposition(FMOD_CODEC_STATE *codec, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
    fcplugin* fc = (fcplugin*)codec->plugindata;

    int32_t seek_sample = (int32_t)(position * 0.001 * fc->vgmstream->sample_rate);
    seek_vgmstream(fc->vgmstream, seek_sample);
    return FMOD_OK;
}

