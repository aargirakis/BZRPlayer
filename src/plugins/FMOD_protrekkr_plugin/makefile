# the compiler to use.
CC=gcc
CCP=g++
GLOBAL_INCLUDES= -I C:/Dev/FMOD/api/core/inc -I C:\Dev\includes -I C:\Dev\libmodplug
GLOBAL_LIBS=-L C:/Dev/FMOD/api/core/lib/x86 -lfmod -L C:\Dev\QTCreator\BZRPlayer\BZRPlayer\src\external\zlib -lzdll -lmsacm32
CCPFLAGS=-Wall -O2 
CFLAGS=-Wall -O2 -D__REPLAYER__ -D__STAND_ALONE__ -D__WINAMP__
BZRPLAYER_PLUGIN=C:\Dev\QTCreator\BZRPlayer\BZRPlayer\build-BZRPlayer-Desktop_Qt_5_15_2_MinGW_32_bit-Release\release\data\plugin
#Plugin specific
DLL=codec_protrekkr.dll
CPP_SOURCES=src/main.cpp src/protrekkr/editors/patterns_blocks.cpp src/protrekkr/files/*.cpp src/protrekkr/replay/*.cpp
C_SOURCES=
LOCAL_INCLUDES=-I src/protrekkr -I src/protrekkr/include -I src/protrekkr/replay/include -I src/protrekkr/editors/include -I src/protrekkr/files -I src/protrekkr/replay/samples/aiff/include -I src/protrekkr/replay/samples/wav/include
all :
	$(CCP) -c $(CPP_SOURCES) $(CFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) $(GLOBAL_LIBS)
	$(CC) -c $(C_SOURCES) $(CFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) $(GLOBAL_LIBS)
	$(CCP) *.o $(CFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) -shared -Wl,$(GLOBAL_LIBS) -o $(DLL)
	copy $(DLL) $(BZRPLAYER_PLUGIN)\$(DLL)

clean :
	del *.o \
	$(DLL)