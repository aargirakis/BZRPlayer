# the compiler to use.
CC=g++
GLOBAL_INCLUDES= -I C:/Dev/FMOD/api/core/inc -I C:\Dev\QTCreator\BZRPlayer\BZRPlayer\src
GLOBAL_LIBS=-L C:/Dev/FMOD/api/core/lib/x86 -lfmod
CFLAGS=-Wall -O2
BZRPLAYER_PLUGIN=C:\Dev\QTCreator\BZRPlayer\BZRPlayer\build-BZRPlayer-Desktop_Qt_5_15_2_MinGW_32_bit-Release\release\data\plugin
#Plugin specific
DLL=codec_ym.dll
SOURCES=src/main.cpp src/StSoundLibrary2.0/*.cpp src/StSoundLibrary2.0/LZH/LzhLib.cpp
LOCAL_INCLUDES=-I src/StSoundLibrary2.0 -I src/StSoundLibrary2.0/LZH
all :
	$(CC) $(SOURCES) $(CFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) -shared -Wl,$(GLOBAL_LIBS) -o $(DLL)
	copy $(DLL) $(BZRPLAYER_PLUGIN)\$(DLL)


clean :
	del \
	$(DLL)