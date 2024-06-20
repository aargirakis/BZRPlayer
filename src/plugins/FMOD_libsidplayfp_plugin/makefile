# the compiler to use.
CC=gcc
CCP=g++
GLOBAL_INCLUDES= -I C:/Dev/FMOD/api/core/inc -I C:\Dev\includes -I C:\Dev\libmodplug
GLOBAL_LIBS=-L C:/Dev/FMOD/api/core/lib/x86 -lfmod

CFLAGS=-Wall -O2
BZRPLAYER_PLUGIN=C:\Dev\QTCreator\BZRPlayer\BZRPlayer\build-BZRPlayer-Desktop_Qt_5_15_2_MinGW_32_bit-Release\release\data\plugin
#Plugin specific
DLL=codec_libsidplayfp.dll
CPP_SOURCES=src/main.cpp \
			src/libsidplayfp-2.7.1/src/*.cpp \
			src/libsidplayfp-2.7.1/src/builders/residfp-builder/*.cpp src/libsidplayfp-2.7.1/src/builders/residfp-builder/residfp/*.cpp src/libsidplayfp-2.7.1/src/builders/residfp-builder/residfp/resample/*.cpp \
			src/libsidplayfp-2.7.1/src/c64/*.cpp src/libsidplayfp-2.7.1/src/c64/CIA/*.cpp src/libsidplayfp-2.7.1/src/c64/CPU/*.cpp src/libsidplayfp-2.7.1/src/c64/VIC_II/*.cpp \
			src/libsidplayfp-2.7.1/src/sidplayfp/*.cpp \
			src/libsidplayfp-2.7.1/src/sidtune/*.cpp \
			src/libsidplayfp-2.7.1/src/utils/*.cpp \
			src/libsidplayfp-2.7.1/src/utils/MD5/*.cpp \
			src/libsidplayfp-2.7.1/src/utils/STILview/*.cpp \


C_SOURCES=	src/sidid.c

LOCAL_INCLUDES= -I src \
				-I src/libsidplayfp-2.7.1/src \
				-I src/libsidplayfp-2.7.1/src/builders/residfp-builder -I src/libsidplayfp-2.7.1/src/builders/residfp-builder/residfp \
				-I src/libsidplayfp-2.7.1/src/c64 -I src/libsidplayfp-2.7.1/src/c64/Banks -I src/libsidplayfp-2.7.1/src/c64/CIA -I src/libsidplayfp-2.7.1/src/c64/CPU -I src/libsidplayfp-2.7.1/src/c64/VIC_II \
				-I src/libsidplayfp-2.7.1/src/sidplayfp \
				-I src/libsidplayfp-2.7.1/src/sidtune \
				-I src/libsidplayfp-2.7.1/src/utils -I src/libsidplayfp-2.7.1/src/utils/MD5 -I src/libsidplayfp-2.7.1/src/utils/STILview 

all :
	$(CCP) -c $(CPP_SOURCES) $(CFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) $(GLOBAL_LIBS)
	$(CC) -c $(C_SOURCES) $(CFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) $(GLOBAL_LIBS)
	$(CCP) *.o $(CFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) -shared -Wl,$(GLOBAL_LIBS) -o $(DLL)
	copy $(DLL) $(BZRPLAYER_PLUGIN)\$(DLL)
	
onlymain :
	$(CC) -c src/main.cpp $(CFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) $(GLOBAL_LIBS)
	$(CCP) *.o $(CCPFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) -shared -Wl,$(GLOBAL_LIBS) -o $(DLL)
	copy $(DLL) $(BZRPLAYER_PLUGIN)\$(DLL)
	
clean :
	del *.o \
	$(DLL)