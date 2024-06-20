PROJECT_ROOT=../../../..
CC=gcc
CCP=g++
GLOBAL_INCLUDES= -I C:/Dev/FMOD/api/core/inc -I C:\Dev\QTCreator\BZRPlayer\BZRPlayer\src
GLOBAL_LIBS=-L C:/Dev/FMOD/api/core/lib/x86 -lfmod -L C:\Dev\QTCreator\BZRPlayer\BZRPlayer\src\external\zlib -lzdll -lshlwapi
CCPFLAGS=-O2 -fpermissive
CFLAGS=-O2 -Wno-narrowing
BZRPLAYER_PLUGIN=C:\Dev\QTCreator\BZRPlayer\BZRPlayer\build-BZRPlayer-Desktop_Qt_5_15_2_MinGW_32_bit-Release\release\data\plugin
#Plugin specific
DLL=codec_furnace.dll
CPP_SOURCES=src/main.cpp \
src/furnace-0.6.3/extern/fmt/src/format.cc \
src/furnace-0.6.3/extern/fmt/src/os.cc \
src/furnace-0.6.3/extern/SAASound/src/*.cpp \
src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/core/vox/*.cpp \
src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/es550x/*.cpp \
src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/k005289/*.cpp \
src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/k007232/*.cpp \
src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/k053260/*.cpp \
src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/msm6295/*.cpp \
src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/n163/*.cpp \
src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/scc/*.cpp \
src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/vrcvi/*.cpp \
src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/x1_010/*.cpp \
src/furnace-0.6.3/src/audio/abstract-2.cpp \
src/furnace-0.6.3/src/audio/midi.cpp \
src/furnace-0.6.3/src/audio/pipe.cpp \
src/furnace-0.6.3/src/engine/cmdStream.cpp \
src/furnace-0.6.3/src/engine/cmdStreamOps.cpp \
src/furnace-0.6.3/src/engine/config.cpp \
src/furnace-0.6.3/src/engine/configEngine.cpp \
src/furnace-0.6.3/src/engine/dispatchContainer.cpp \
src/furnace-0.6.3/src/engine/engine.cpp \
src/furnace-0.6.3/src/engine/export.cpp \
src/furnace-0.6.3/src/engine/fileOps/*.cpp \
src/furnace-0.6.3/src/engine/fileOpsIns.cpp \
src/furnace-0.6.3/src/engine/fileOpsSample.cpp \
src/furnace-0.6.3/src/engine/filter-4.cpp \
src/furnace-0.6.3/src/engine/instrument.cpp \
src/furnace-0.6.3/src/engine/macroInt.cpp \
src/furnace-0.6.3/src/engine/pattern.cpp \
src/furnace-0.6.3/src/engine/pitchTable.cpp \
src/furnace-0.6.3/src/engine/playback.cpp \
src/furnace-0.6.3/src/engine/safeReader.cpp \
src/furnace-0.6.3/src/engine/safeWriter.cpp \
src/furnace-0.6.3/src/engine/sample.cpp \
src/furnace-0.6.3/src/engine/song.cpp \
src/furnace-0.6.3/src/engine/sysDef.cpp \
src/furnace-0.6.3/src/engine/vgmOps.cpp \
src/furnace-0.6.3/src/engine/waveSynth.cpp \
src/furnace-0.6.3/src/engine/wavetable.cpp \
src/furnace-0.6.3/src/engine/wavOps.cpp \
src/furnace-0.6.3/src/engine/winStuff.cpp \
src/furnace-0.6.3/src/engine/workPool.cpp \
src/furnace-0.6.3/src/engine/zsm.cpp \
src/furnace-0.6.3/src/engine/zsmOps.cpp \
src/furnace-0.6.3/src/engine/export/*.cpp \
src/furnace-0.6.3/src/engine/effect/*.cpp \
src/furnace-0.6.3/src/engine/platform/*.cpp \
src/furnace-0.6.3/src/engine/platform/sound/*.cpp \
src/furnace-0.6.3/src/engine/platform/sound/c64/*.cc \
src/furnace-0.6.3/src/engine/platform/sound/c64_fp/*.cpp \
src/furnace-0.6.3/src/engine/platform/sound/c64_fp/resample/SincResampler.cpp \
src/furnace-0.6.3/src/engine/platform/sound/dave/*.cpp \
src/furnace-0.6.3/src/engine/platform/sound/ga20/*.cpp \
src/furnace-0.6.3/src/engine/platform/sound/lynx/*.cpp \
src/furnace-0.6.3/src/engine/platform/sound/nes_nsfplay/*.cpp \
src/furnace-0.6.3/src/engine/platform/sound/oki/*.cpp \
src/furnace-0.6.3/src/engine/platform/sound/pokey/*.cpp \
src/furnace-0.6.3/src/engine/platform/sound/sid2/*.cc \
src/furnace-0.6.3/src/engine/platform/sound/snes/*.cpp \
src/furnace-0.6.3/src/engine/platform/sound/t6w28/*.cpp \
src/furnace-0.6.3/src/engine/platform/sound/tia/*.cpp \
src/furnace-0.6.3/src/engine/platform/sound/ymfm/*.cpp \
src/furnace-0.6.3/src/baseutils.cpp \
src/furnace-0.6.3/src/fileutils.cpp \
src/furnace-0.6.3/src/log.cpp \
src/furnace-0.6.3/src/utfutils.cpp


C_SOURCES=src/furnace-0.6.3/extern/adpcm/*.c \
src/furnace-0.6.3/extern/adpcm-xq-s/*.c \
src/furnace-0.6.3/extern/emu2413/*.c \
src/furnace-0.6.3/extern/ESFMu/*.c \
src/furnace-0.6.3/extern/Nuked-OPLL/*.c \
src/furnace-0.6.3/extern/Nuked-PSG/*.c \
src/furnace-0.6.3/extern/opl/*.c \
src/furnace-0.6.3/extern/opm/*.c \
src/furnace-0.6.3/extern/opn/*.c \
src/furnace-0.6.3/extern/pwrnoise/*.c \
src/furnace-0.6.3/extern/YM2608-LLE/fmopna_2608.c \
src/furnace-0.6.3/extern/YM2608-LLE/fmopna_2610.c \
src/furnace-0.6.3/extern/YM2608-LLE/fmopna_2612.c \
src/furnace-0.6.3/extern/YM3812-LLE/*.c \
src/furnace-0.6.3/extern/YMF262-LLE/*.c \
src/furnace-0.6.3/extern/YMF276-LLE/*.c \
src/furnace-0.6.3/src/engine/*.c \
src/furnace-0.6.3/src/engine/platform/sound/*.c \
src/furnace-0.6.3/src/engine/platform/sound/atomicssg/*.c \
src/furnace-0.6.3/src/engine/platform/sound/c64_d/*.c \
src/furnace-0.6.3/src/engine/platform/sound/gb/*.c \
src/furnace-0.6.3/src/engine/platform/sound/nes/*.c \
src/furnace-0.6.3/src/engine/platform/sound/pokey/*.c \

LOCAL_INCLUDES=-I src \
-I src/furnace-0.6.3/extern/adpcm \
-I src/furnace-0.6.3/extern/adpcm-xq-s \
-I src/furnace-0.6.3/extern/emu2413 \
-I src/furnace-0.6.3/extern/ESFMu \
-I src/furnace-0.6.3/extern/fmt/include \
-I src/furnace-0.6.3/extern/IconFontCppHeaders \
-I src/furnace-0.6.3/extern/Nuked-OPLL \
-I src/furnace-0.6.3/extern/Nuked-PSG \
-I src/furnace-0.6.3/extern/opl \
-I src/furnace-0.6.3/extern/opm \
-I src/furnace-0.6.3/extern/opn \
-I src/furnace-0.6.3/extern/pwrnoise \
-I src/furnace-0.6.3/extern/SAASound/src \
-I src/furnace-0.6.3/extern/vgsound_emu-modified \
-I src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu \
-I src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/core \
-I src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/core/util \
-I src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/core/vox \
-I src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/core/e550x \
-I src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/core/k005289 \
-I src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/core/k007232 \
-I src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/core/k053260 \
-I src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/core/msm6295 \
-I src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/core/n163 \
-I src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/core/scc \
-I src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/core/template \
-I src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/core/vrcvi \
-I src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/core/x1_010 \
-I src/furnace-0.6.3/extern/YM2608-LLE \
-I src/furnace-0.6.3/extern/YM3812-LLE \
-I src/furnace-0.6.3/extern/YMF262-LLE \
-I src/furnace-0.6.3/extern/YMF276-LLE \
-I src/furnace-0.6.3/src/audio \
-I src/furnace-0.6.3/src/baseutils \
-I src/furnace-0.6.3/src/engine \
-I src/furnace-0.6.3/src/engine/fileOps \
-I src/furnace-0.6.3/src/engine/export \
-I src/furnace-0.6.3/src/engine/effect \
-I src/furnace-0.6.3/src/engine/platform \
-I src/furnace-0.6.3/src/engine/platform/sound \
-I src/furnace-0.6.3/src/engine/platform/sound/c64 \
-I src/furnace-0.6.3/src/engine/platform/sound/c64_d \
-I src/furnace-0.6.3/src/engine/platform/sound/c64_fp \
-I src/furnace-0.6.3/src/engine/platform/sound/c64_fp/resample \
-I src/furnace-0.6.3/src/engine/platform/sound/dave \
-I src/furnace-0.6.3/src/engine/platform/sound/ga20 \
-I src/furnace-0.6.3/src/engine/platform/sound/gb \
-I src/furnace-0.6.3/src/engine/platform/sound/lynx \
-I src/furnace-0.6.3/src/engine/platform/sound/nes \
-I src/furnace-0.6.3/src/engine/platform/sound/nes_nsfplay \
-I src/furnace-0.6.3/src/engine/platform/sound/oki \
-I src/furnace-0.6.3/src/engine/platform/sound/pokey \
-I src/furnace-0.6.3/src/engine/platform/sound/snes \
-I src/furnace-0.6.3/src/engine/platform/sound/t6w28 \
-I src/furnace-0.6.3/src/engine/platform/sound/tia \
-I src/furnace-0.6.3/src/engine/platform/sound/ymfm \
-I src/furnace-0.6.3/src \
-I src/furnace-0.6.3/src/icon
all : 
	$(CCP) -c $(CPP_SOURCES) $(CCPFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) $(GLOBAL_LIBS)
	$(CC) -c $(C_SOURCES) $(CFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) $(GLOBAL_LIBS)
	$(CCP) *.o $(CCPFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) -shared -Wl,$(GLOBAL_LIBS) -o $(DLL)
	copy $(DLL) $(BZRPLAYER_PLUGIN)\$(DLL)

onlymain :
	$(CC) -c src/main.cpp $(CFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) $(GLOBAL_LIBS)
	$(CCP) *.o $(CCPFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) -shared -Wl,$(GLOBAL_LIBS) -o $(DLL)
	copy $(DLL) $(BZRPLAYER_PLUGIN)\$(DLL)

ym :
	$(CC) -c src/furnace-0.6.3/extern/YM2608-LLE/*.c $(CFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) $(GLOBAL_LIBS)
	$(CCP) *.o $(CCPFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) -shared -Wl,$(GLOBAL_LIBS) -o $(DLL)

	
clean :
	del *.o \
	$(DLL)