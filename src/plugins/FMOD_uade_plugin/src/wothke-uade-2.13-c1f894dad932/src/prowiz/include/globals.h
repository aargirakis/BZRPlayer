#ifndef _GLOBALS_H
#define _GLOBALS_H

/*
 * ProWizard PC include file
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
/* #include <inttypes.h>*/
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>

#if BYTE_ORDER == BIG_ENDIAN
#define htonl(n) (n)
#else
#define htonl(n) (((((uint32_t)(n) & 0xFF)) << 24) | \
                  ((((uint32_t)(n) & 0xFF00)) << 8) | \
                  ((((uint32_t)(n) & 0xFF0000)) >> 8) | \
                  ((((uint32_t)(n) & 0xFF000000)) >> 24))
#endif

#define _TYPES_FILENAME     "_types_"
#define _TYPES_LINE_LENGHT  256
#define MINIMAL_FILE_LENGHT 64
#define GOOD                0
#define BAD                 1
#define BZERO(a,b)          memset(a,0,b)

enum
{
  AC1D_packer = 0,
/* version 2 / 3 */
  SoundMonitor,
  FC_M_packer,
  Hornet_packer,
  KRIS_tracker,
  Power_Music,
  Promizer_10c,
  Promizer_18a,
  Promizer_20,
  ProRunner_v1,
  ProRunner_v2,
  SKYT_packer,
  Wanton_packer,
  XANN_packer,
  Module_protector,
  Digital_illusion,
  Pha_packer,
  Promizer_01,
  Propacker_21,
  Propacker_30,
  Eureka_packer,
  Star_pack,
  Protracker,
  UNIC_v1,
  UNIC_v2,
  Fuzzac,
  GMC,
  Heatseeker,
  KSM,
  Noiserunner,
  Noisepacker1,
  Noisepacker2,
  Noisepacker3,
  P40A,
  P40B,
  P41A,
  PM40,
  PP10,
  TP1,
  TP2,
  TP3,
  ZEN,
  P50A,
  P60A,
  StarTrekker,
  /* stands for S404(data/exe),S401(data),S403(data) and S310,S300(data) */
  S404,
  StoneCracker270,
  P61A,
  STIM,
  SoundTracker,
  TPACK22,
/* stands for CrM! & CrM2 and Crunchmania Address*/
  CRM1,
/* stands for both Defjam 3.2 & 3.2 pro */
  Defjam_32,
  TPACK21,
  ICE,
/* stands for version 1.3 , 2.0 , 3.0 and Pro 1.0*/
  ByteKiller,
  XPK,
  IMP,
  RNC,
  Double_Action,
  Powerpacker3,
  Powerpacker4,
  Powerpacker23,
  SpikeCruncher,
  TPACK102,
  TimeCruncher,
  MasterCruncher,
/* stands also for Mega Cruncher 1.0/1.2 */
  MegaCruncher,
  JamCracker,
  BSIFC,
  DigiBooster,
  QuadraComposer,
  TDD,
  FuchsTracker,
  SyncroPacker,
  TNMCruncher,
  SuperCruncher,
/* not for PP20 themselves :) ... only PP20 subfiles inside PPbk */
  PP20,
  RelokIt,
  STC292data,
  FIRE,
  MaxPacker,
  SoundFX,
  arcD,
  PARA,
  CRND,
  SB_DataCruncher,
  SF,
  RLE,
  VDCO,
  SQ,
  SP,
  STK26,
  IceTracker,
  HQC,
  TryIt,
  FC13,
  FC14,
  AmnestyDesign1,
  AmnestyDesign2,
  MED,
  ACECruncherData,
  Newtron,
  GPMO,
  PolkaPacker,
  GnuPlayer,
  CJ_DataCruncher,
  AmBk,
  MasterCruncher3data,
  XM,
  MegaCruncherObj,
  TurboSqueezer61,
  STC299d,
  STC310,
  STC299b,
  STC299,
  STC300,
  ThePlayer30a,
  ThePlayer22a,
  NoiseFromHeaven,
  TMK,
  DragPack252,
  DragPack100,
  SpeedPacker3Data,
  AtomikPackerData,
  AutomationPackerData,
  /*  TreasurePattern,*/
  SGTPacker,
  GNUPacker12,
  CrunchmaniaSimple,
  dmu,
  TitanicsPlayer,
  NewtronOld,
  NovoTrade,
  GnoiPacker,
  StoneArtsPlayer,
  SLAM,
  S3M,
  MOSH,
  BNR,
  HCD,
  TPACK101,
  STRUGGLE,
  BKCloneFLT,
  Oktalizer,
  AHX,
  Sidmon1,
  Sidmon2,
  PerfSong,
  MASMDataCruncher,
  ImpulseTracker,
  HighPresCruncher,
  SP20,
  B9AB,
  HVL,
  BKClone5,
  xVdg,
  Diet,
  LSDDataCruncher,
  JamDataCruncher,
  MentalImage,
  BHC3CruncherData,
  IFF,
  SA,
  DM1,
  PMd3,
  PMD3,
  BHC2CruncherData,
  Pac1,
  DM2,
  HEADPACK,
  LOBDataCruncher,
  Devils,
  PerfSong2,
  Pretracker,
  SoundFX2,
  Promizer_10b,
  /* Must be the last : */
  Crunch13data,
  SoftwareVisionsDMF,
  UltimateSoundTracker,	// UADE HACK
  _KNOWN_FORMATS
};

extern int Get_Detected_Format_Id(); // UADE

#endif /* _GLOBALS_H */
