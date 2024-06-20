CC=g++
GLOBAL_INCLUDES= -I C:/Dev/FMOD/api/core/inc -I C:\Dev\QTCreator\BZRPlayer\BZRPlayer\src
GLOBAL_LIBS=-L C:/Dev/FMOD/api/core/lib/x86 -lfmod
CFLAGS=-Wall -O2
BZRPLAYER_PLUGIN=C:\Dev\QTCreator\BZRPlayer\BZRPlayer\build-BZRPlayer-Desktop_Qt_5_15_2_MinGW_32_bit-Release\release\data\plugin
#Plugin specific
DLL=codec_fc.dll
SOURCES=src/main.cpp src/libfc14audiodecoder/FC.cpp src/libfc14audiodecoder/fc14audiodecoder.cpp src/libfc14audiodecoder/FC_Data.cpp src/libfc14audiodecoder/LamePaulaMixer.cpp src/libfc14audiodecoder/LamePaulaVoice.cpp src/libfc14audiodecoder/Paula.cpp
LOCAL_INCLUDES=-I src/libfc14audiodecoder
all :
	$(CC) $(SOURCES) $(CFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) -shared -Wl,$(GLOBAL_LIBS) -o $(DLL)
	copy $(DLL) $(BZRPLAYER_PLUGIN)\$(DLL)

clean :
	del \
	FC.o \
	FC_Data.o \
	fc14audiodecoder.o \
	LamePaulaMixer.o \
	LamePaulaVoice.o \
	Paula.o \
	Dump.o \
	codec_fc_dll.a \
	main.o \
	$(DLL)