# the compiler to use.
CC=gcc
CCP=g++
GLOBAL_INCLUDES= -I C:/Dev/FMOD/api/core/inc -I C:\Dev\includes -I C:\Dev\libmodplug
GLOBAL_LIBS=-L C:/Dev/FMOD/api/core/lib/x86 -lfmod -lws2_32
CCPFLAGS=-Wall -O2
CFLAGS=-Wall -O2 
BZRPLAYER_PLUGIN=C:\Dev\QTCreator\BZRPlayer\BZRPlayer\build-BZRPlayer-Desktop_Qt_5_15_2_MinGW_32_bit-Release\release\data\plugin
#Plugin specific
DLL=codec_uade.dll
CPP_SOURCES=	src/main.cpp
C_SOURCES=		src/uade-2.13-1.01/src/newcpu.c src/uade-2.13-1.01/src/memory.c src/uade-2.13-1.01/src/custom.c src/uade-2.13-1.01/src/cia.c src/uade-2.13-1.01/src/audio.c src/uade-2.13-1.01/src/compiler.c src/uade-2.13-1.01/src/cpustbl.c src/uade-2.13-1.01/src/missing.c src/uade-2.13-1.01/src/sd-sound.c src/uade-2.13-1.01/src/md-support.c src/uade-2.13-1.01/src/cfgfile.c src/uade-2.13-1.01/src/fpp.c src/uade-2.13-1.01/src/readcpu.c src/uade-2.13-1.01/src/cpudefs.c src/uade-2.13-1.01/src/cpuemu1.c src/uade-2.13-1.01/src/cpuemu2.c src/uade-2.13-1.01/src/cpuemu3.c src/uade-2.13-1.01/src/cpuemu4.c src/uade-2.13-1.01/src/cpuemu5.c src/uade-2.13-1.01/src/cpuemu6.c src/uade-2.13-1.01/src/cpuemu7.c src/uade-2.13-1.01/src/cpuemu8.c src/uade-2.13-1.01/src/uade.c src/uade-2.13-1.01/src/unixatomic.c src/uade-2.13-1.01/src/ossupport.c src/uade-2.13-1.01/src/uademain.c src/uade-2.13-1.01/src/sinctable.c src/uade-2.13-1.01/src/text_scope.c src/uade-2.13-1.01/src/frontends/common/effects.c src/uade-2.13-1.01/src/frontends/common/uadeconf.c src/uade-2.13-1.01/src/frontends/common/support.c src/uade-2.13-1.01/src/frontends/common/songinfo.c src/uade-2.13-1.01/src/frontends/common/songdb.c src/uade-2.13-1.01/src/frontends/common/amifilemagic.c src/uade-2.13-1.01/src/frontends/common/eagleplayer.c src/uade-2.13-1.01/compat/strlrep.c src/blazer.c src/uade-2.13-1.01/compat/memmemrep.c src/md5/*.c src/flod/*.cpp

			
LOCAL_INCLUDES=	-I src/uade-2.13-1.01/src/include \
                -I src/uade-2.13-1.01/src \
                -I src/uade-2.13-1.01/src/frontends/common \
                -I src/uade-2.13-1.01/compat \
                -I src/uade-2.13-1.01/src/machdep \
				-I src/md5 \
				-I src/flod \
                -I src

all :
	$(CCP) -c $(CPP_SOURCES) $(CCPFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) $(GLOBAL_LIBS)
	$(CC) -c $(C_SOURCES) $(CFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) $(GLOBAL_LIBS)
	$(CCP) *.o $(CCPFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) -shared -Wl,$(GLOBAL_LIBS) -o $(DLL)
	copy $(DLL) $(BZRPLAYER_PLUGIN)\$(DLL)

clean :
	del *.o \
	$(DLL)

onlymain :
	$(CC) -c src/main.cpp src/uade-2.13-1.01/src/uade.c  src/uade-2.13-1.01/src/audio.c src/uade-2.13-1.01/src/frontends/common/uadeconf.c $(CFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) $(GLOBAL_LIBS)
	$(CCP) *.o $(CCPFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) -shared -Wl,$(GLOBAL_LIBS) -o $(DLL)
	copy $(DLL) $(BZRPLAYER_PLUGIN)\$(DLL)

	
samples :
	$(CC) -c src/main.cpp src/uade-2.13-1.01/src/uade.c  src/uade-2.13-1.01/src/audio.c src/uade-2.13-1.01/src/frontends/common/uadeconf.c src/flod/*.cpp $(CFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) $(GLOBAL_LIBS)
	$(CCP) *.o $(CCPFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) -shared -Wl,$(GLOBAL_LIBS) -o $(DLL)
	copy $(DLL) $(BZRPLAYER_PLUGIN)\$(DLL)
