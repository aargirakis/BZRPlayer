# the compiler to use.
CC=gcc
CCP=g++
GLOBAL_INCLUDES= -I C:/Dev/FMOD/api/core/inc -I C:\Dev\includes -I C:\Dev\libmodplug
GLOBAL_LIBS=-L C:/Dev/FMOD/api/core/lib/x86 -lfmod
CFLAGS=-Wall -O2
BZRPLAYER_PLUGIN=C:\Dev\QTCreator\BZRPlayer\BZRPlayer\build-BZRPlayer-Desktop_Qt_5_15_2_MinGW_32_bit-Release\release\data\plugin
#Plugin specific
DLL=codec_flod.dll
CPP_SOURCES=src/main.cpp src/flod/*.cpp src/flod/delta1/*.cpp src/flod/delta2/*.cpp src/flod/brooke/*.cpp src/flod/infogrames/*.cpp src/flod/joseph/*.cpp src/flod/daglish/*.cpp src/flod/whittaker/*.cpp src/flod/fred/*.cpp src/flod/hubbard/*.cpp src/flod/hippel/*.cpp src/flod/soundfx/*.cpp src/flod/soundmon/*.cpp src/flod/sidmon/*.cpp src/flod/digitalmugician/*.cpp src/flod/trackers/PTPlayer.cpp src/flod/trackers/PTVoice.cpp
C_SOURCES=
LOCAL_INCLUDES=-I src -I src/flod -I src/flod/delta1 -I src/flod/delta2 -I src/flod/brooke -I src/flod/infogrames -I src/flod/joseph -I src/flod/daglish -I src/flod/whittaker -I src/flod/fred -I src/flod/hubbard -I src/flod/hippel -I src/flod/soundfx -I src/flod/soundmon -I src/flod/sidmon -I src/flod/digitalmugician -I src/flod/trackers
all :
	$(CCP) -c $(CPP_SOURCES) $(CFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) $(GLOBAL_LIBS)
	$(CC) -c $(C_SOURCES) $(CFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) $(GLOBAL_LIBS)
	$(CCP) *.o $(CFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) -shared -Wl,$(GLOBAL_LIBS) -o $(DLL)
	copy $(DLL) $(BZRPLAYER_PLUGIN)\$(DLL)
clean :
	del *.o \
	$(DLL)