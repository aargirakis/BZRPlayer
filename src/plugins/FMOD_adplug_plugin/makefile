# the compiler to use.
CC=gcc
CCP=g++
GLOBAL_INCLUDES= -I C:/Dev/FMOD/api/core/inc -I C:\Dev\includes -I C:\Dev\libmodplug
GLOBAL_LIBS=-L C:/Dev/FMOD/api/core/lib/x86 -lfmod
CFLAGS=-Wall -O2
BZRPLAYER_PLUGIN=C:\Dev\QTCreator\BZRPlayer\BZRPlayer\build-BZRPlayer-Desktop_Qt_5_15_2_MinGW_32_bit-Release\release\data\plugin
#Plugin specific
DLL=codec_adplug.dll
CPP_SOURCES=src/*.cpp src/adplug-adplug-2.3.3/src/*.cpp src/adplug/libbinio/*.cpp
C_SOURCES=src/adplug-adplug-2.3.3/src/*.c
LOCAL_INCLUDES=-I src -I src/adplug-adplug-2.3.3/src -I src/adplug/libbinio
all :
	$(CCP) -c $(CPP_SOURCES) $(CFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) $(GLOBAL_LIBS)
	$(CC) -c $(C_SOURCES) $(CFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) $(GLOBAL_LIBS)
	$(CCP) *.o $(CFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) -shared -Wl,$(GLOBAL_LIBS) -o $(DLL)
	copy $(DLL) $(BZRPLAYER_PLUGIN)\$(DLL)
clean :
	del *.o \
	$(DLL)
	
onlymain :
	$(CC) -c src/main.cpp $(CFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) $(GLOBAL_LIBS)
	$(CCP) *.o $(CCPFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) -shared -Wl,$(GLOBAL_LIBS) -o $(DLL)
	copy $(DLL) $(BZRPLAYER_PLUGIN)\$(DLL)
	copy $(DLL) $(PLAYSOUND_PLUGIN)\$(DLL)