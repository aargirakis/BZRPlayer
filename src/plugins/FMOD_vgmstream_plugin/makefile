# the compiler to use.
CC=gcc
CCP=g++
GLOBAL_INCLUDES= -I C:/Dev/FMOD/api/core/inc -I C:\Dev\includes -I C:\Dev\libmodplug
GLOBAL_LIBS=-L C:/Dev/FMOD/api/core/lib/x86 -lfmod
CFLAGS=-Wall -O2
BZRPLAYER_PLUGIN=C:\Dev\QTCreator\BZRPlayer\BZRPlayer\build-BZRPlayer-Desktop_Qt_5_15_2_MinGW_32_bit-Release\release\data\plugin
#Plugin specific
DLL=codec_vgmstream.dll
CPP_SOURCES=src/main.cpp
C_SOURCES=src/vgmstream-r1831/*.c src/vgmstream-r1831/meta/*.c src/vgmstream-r1831/layout/*.c src/vgmstream-r1831/coding/*.c src/vgmstream-r1831/util/*.c
LOCAL_INCLUDES=-I src -I src/vgmstream-r1831 -I src/vgmstream-r1831/coding -I src/vgmstream-r1831/layout -I src/vgmstream-r1831/meta -I src/vgmstream-r1831/util

all :
	$(CCP) -c $(CPP_SOURCES) $(CFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) $(GLOBAL_LIBS) $(LOCAL_LIBS)
	$(CC) -c $(C_SOURCES) $(CFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) $(GLOBAL_LIBS) $(LOCAL_LIBS)
	$(CCP) *.o $(CFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) -shared -Wl,$(GLOBAL_LIBS) $(LOCAL_LIBS) -o $(DLL)
	copy $(DLL) $(BZRPLAYER_PLUGIN)\$(DLL)

clean :
	del *.o \
	$(DLL)