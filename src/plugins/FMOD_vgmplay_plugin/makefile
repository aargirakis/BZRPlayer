# the compiler to use.
CC=gcc
CCP=g++
GLOBAL_INCLUDES= -I C:/Dev/FMOD/api/core/inc -I C:\Dev\QTCreator\BZRPlayer\BZRPlayer\src
GLOBAL_LIBS=-L C:/Dev/FMOD/api/core/lib/x86 -L C:\Dev\zlib\lib -lfmod -lz
CCPFLAGS=-Wall -O2 -DENABLE_ALL_CORES
CFLAGS=-Wall -O2
BZRPLAYER_PLUGIN=C:\Dev\QTCreator\BZRPlayer\BZRPlayer\build-BZRPlayer-Desktop_Qt_5_15_2_MinGW_32_bit-Release\release\data\plugin
#Plugin specific
DLL=codec_vgmplay.dll
CPP_SOURCES=src/main.cpp
C_SOURCES=src/vgmplay-0.40.9/VGMPlay.c src/vgmplay-0.40.9/ChipMapper.c src/vgmplay-0.40.9/pt_ioctl.c src/vgmplay-0.40.9/chips/262intf.c src/vgmplay-0.40.9/chips/2151intf.c src/vgmplay-0.40.9/chips/2203intf.c src/vgmplay-0.40.9/chips/2413intf.c src/vgmplay-0.40.9/chips/2608intf.c src/vgmplay-0.40.9/chips/2610intf.c src/vgmplay-0.40.9/chips/2612intf.c src/vgmplay-0.40.9/chips/3526intf.c src/vgmplay-0.40.9/chips/3812intf.c src/vgmplay-0.40.9/chips/8950intf.c src/vgmplay-0.40.9/chips/adlibemu_opl2.c src/vgmplay-0.40.9/chips/adlibemu_opl3.c src/vgmplay-0.40.9/chips/ay8910.c src/vgmplay-0.40.9/chips/ay_intf.c src/vgmplay-0.40.9/chips/c140.c src/vgmplay-0.40.9/chips/c352.c src/vgmplay-0.40.9/chips/c6280.c src/vgmplay-0.40.9/chips/c6280intf.c src/vgmplay-0.40.9/chips/dac_control.c src/vgmplay-0.40.9/chips/es5503.c src/vgmplay-0.40.9/chips/es5506.c src/vgmplay-0.40.9/chips/emu2149.c src/vgmplay-0.40.9/chips/emu2413.c src/vgmplay-0.40.9/chips/fm2612.c src/vgmplay-0.40.9/chips/fm.c src/vgmplay-0.40.9/chips/fmopl.c src/vgmplay-0.40.9/chips/gb.c src/vgmplay-0.40.9/chips/iremga20.c src/vgmplay-0.40.9/chips/k051649.c src/vgmplay-0.40.9/chips/k053260.c src/vgmplay-0.40.9/chips/k054539.c src/vgmplay-0.40.9/chips/multipcm.c src/vgmplay-0.40.9/chips/nes_apu.c src/vgmplay-0.40.9/chips/nes_intf.c src/vgmplay-0.40.9/chips/np_nes_apu.c src/vgmplay-0.40.9/chips/np_nes_dmc.c src/vgmplay-0.40.9/chips/np_nes_fds.c src/vgmplay-0.40.9/chips/okim6258.c src/vgmplay-0.40.9/chips/okim6295.c src/vgmplay-0.40.9/chips/Ootake_PSG.c src/vgmplay-0.40.9/chips/panning.c src/vgmplay-0.40.9/chips/pokey.c src/vgmplay-0.40.9/chips/pwm.c src/vgmplay-0.40.9/chips/qsound_ctr.c src/vgmplay-0.40.9/chips/qsound_intf.c  src/vgmplay-0.40.9/chips/rf5c68.c src/vgmplay-0.40.9/chips/segapcm.c src/vgmplay-0.40.9/chips/scd_pcm.c src/vgmplay-0.40.9/chips/scsp.c src/vgmplay-0.40.9/chips/scspdsp.c src/vgmplay-0.40.9/chips/sn76489.c src/vgmplay-0.40.9/chips/sn76496.c src/vgmplay-0.40.9/chips/sn764intf.c src/vgmplay-0.40.9/chips/upd7759.c src/vgmplay-0.40.9/chips/vsu.c src/vgmplay-0.40.9/chips/ws_audio.c src/vgmplay-0.40.9/chips/x1_010.c src/vgmplay-0.40.9/chips/ym2151.c src/vgmplay-0.40.9/chips/ym2413.c src/vgmplay-0.40.9/chips/ym2612.c src/vgmplay-0.40.9/chips/ymdeltat.c src/vgmplay-0.40.9/chips/ymf262.c src/vgmplay-0.40.9/chips/ymf271.c src/vgmplay-0.40.9/chips/ymf278b.c src/vgmplay-0.40.9/chips/ymz280b.c src/vgmplay-0.40.9/chips/ay8910_opl.c src/vgmplay-0.40.9/chips/sn76496_opl.c src/vgmplay-0.40.9/chips/ym2413hd.c src/vgmplay-0.40.9/chips/ym2413_opl.c src/vgmplay-0.40.9/chips/saa1099.c
LOCAL_INCLUDES=-I src -I src/vgmplay -I src/vgmplay-0.40.9/chips
all :
	$(CCP) -c $(CPP_SOURCES) $(CFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) $(GLOBAL_LIBS)
	$(CC) -c $(C_SOURCES) $(CFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) $(GLOBAL_LIBS)
	$(CCP) *.o $(CFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) -shared -Wl,$(GLOBAL_LIBS) -o $(DLL)
	copy $(DLL) $(BZRPLAYER_PLUGIN)\$(DLL)

clean :
	del *.o \
	$(DLL)