# the compiler to use.
CC=gcc
CCP=g++
GLOBAL_INCLUDES= -I C:/Dev/FMOD/api/core/inc -I C:\Dev\includes -I C:\Dev\libmodplug
GLOBAL_LIBS=-L C:/Dev/FMOD/api/core/lib/x86 -lfmod
CCPFLAGS=-Wall -O2 -std=c++1z
CFLAGS=-Wall -O2 -std=c99
BZRPLAYER_PLUGIN=C:\Dev\QTCreator\BZRPlayer\BZRPlayer\build-BZRPlayer-Desktop_Qt_5_15_2_MinGW_32_bit-Release\release\data\plugin
#Plugin specific
DLL=codec_libopenmpt.dll
CPP_SOURCES=	src/main.cpp C:\Dev\QTCreator\BZRPlayer\plugins\FMOD_Flod_plugin\src\flod/BaseRow.cpp

C_SOURCES=src/queue.c
LOCAL_INCLUDES=	-I src \
				-I src/libopenmpt-0.7.6+release/libopenmpt
LOCAL_LIBS=-L src/libopenmpt-0.7.6+release/bin -llibopenmpt
all :
	$(CCP) -c $(CPP_SOURCES) $(CCPFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) $(GLOBAL_LIBS) $(LOCAL_LIBS)
	$(CC) -c $(C_SOURCES) $(CFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) $(GLOBAL_LIBS) $(LOCAL_LIBS)
	$(CCP) *.o $(CCPFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) -shared -Wl,$(GLOBAL_LIBS) $(LOCAL_LIBS) -o $(DLL)
	copy $(DLL) $(BZRPLAYER_PLUGIN)\$(DLL)
clean :
	del *.o \
	$(DLL)