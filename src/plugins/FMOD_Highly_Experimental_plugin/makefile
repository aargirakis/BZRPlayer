# the compiler to use.
CC=gcc
CCP=g++
GLOBAL_INCLUDES= -I C:/Dev/FMOD/api/core/inc -I C:\Dev\includes -I C:\Dev\libmodplug
GLOBAL_LIBS=-L C:/Dev/FMOD/api/core/lib/x86 -lfmod -L C:\Dev\QTCreator\BZRPlayer\BZRPlayer\src\external\zlib -lzdll
CCPFLAGS=-Wall -O2
CFLAGS=-Wall -O2 -DEMU_COMPILE -DEMU_LITTLE_ENDIAN
BZRPLAYER_PLUGIN=C:\Dev\QTCreator\BZRPlayer\BZRPlayer\build-BZRPlayer-Desktop_Qt_5_15_2_MinGW_32_bit-Release\release\data\plugin
#Plugin specific
DLL=codec_highly_experimental.dll
CPP_SOURCES=src/main.cpp 
C_SOURCES=src/Core/*.c src/psflib/psf2fs.c src/psflib/psflib.c
LOCAL_INCLUDES=-I src -I src/Core -I src/psflib 
all : 
	$(CCP) -c $(CPP_SOURCES) $(CFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) $(GLOBAL_LIBS)
	$(CC) -c $(C_SOURCES) $(CFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) $(GLOBAL_LIBS)
	$(CCP) *.o $(CFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) -shared -Wl,$(GLOBAL_LIBS) -o $(DLL)
	copy $(DLL) $(BZRPLAYER_PLUGIN)\$(DLL)
clean :
	del *.o \
	$(DLL)