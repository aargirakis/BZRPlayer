# the compiler to use.
CC=gcc
CCP=g++
GLOBAL_INCLUDES= -I C:/Dev/FMOD/api/core/inc -I C:\Dev\includes -I C:\Dev\libmodplug
GLOBAL_LIBS=-L C:/Dev/FMOD/api/core/lib/x86 -lfmod
CCPFLAGS=-Wall -O2
CFLAGS=-Wall -O2 
BZRPLAYER_PLUGIN=C:\Dev\QTCreator\BZRPlayer\BZRPlayer\build-BZRPlayer-Desktop_Qt_5_15_2_MinGW_32_bit-Release\release\data\plugin
#Plugin specific
DLL=codec_libxmp.dll
CPP_SOURCES=	src/main.cpp C:\Dev\QTCreator\BZRPlayer\plugins\FMOD_Flod_plugin\src\flod/BaseRow.cpp
C_SOURCES=		src/libxmp-4.5.0/src/*.c \
				src/libxmp-4.5.0/src/depackers/*.c \
				src/libxmp-4.5.0/src/loaders/*.c \
				src/libxmp-4.5.0/src/loaders/prowizard/*.c \
				src/libxmp-4.5.0/src/win32/*.c \
				
				
LOCAL_INCLUDES=	-I src \
				-I C:/Dev/QTCreator/FMOD_Flod_plugin/src/flod \
				-I src/libxmp-4.5.0/include \
				-I src/libxmp-4.5.0/src \
				-I src/libxmp-4.5.0/src/depackers \
				-I src/libxmp-4.5.0/src/loaders \
				-I src/libxmp-4.5.0/src/loaders/prowizard \
				-I src/libxmp-4.5.0/src/win32 \
				-I src/libxmp-4.5.0/src/win32/sys
all :
	$(CCP) -c $(CPP_SOURCES) $(CCPFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) $(GLOBAL_LIBS)
	$(CC) -c $(C_SOURCES) $(CFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) $(GLOBAL_LIBS)
	$(CCP) *.o $(CCPFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) -shared -Wl,$(GLOBAL_LIBS) -o $(DLL)
	copy $(DLL) $(BZRPLAYER_PLUGIN)\$(DLL)
clean :
	del *.o \
	$(DLL)