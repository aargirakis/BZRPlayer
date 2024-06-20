# the compiler to use.
CC=gcc
CCP=g++
GLOBAL_INCLUDES= -I C:/Dev/FMOD/api/core/inc -I C:\Dev\QTCreator\BZRPlayer\BZRPlayer\src
GLOBAL_LIBS=-L C:/Dev/FMOD/api/core/lib/x86 -lfmod
CCPFLAGS=-Wall -O2
CFLAGS=-Wall -O2
BZRPLAYER_PLUGIN=C:\Dev\QTCreator\BZRPlayer\BZRPlayer\build-BZRPlayer-Desktop_Qt_5_15_2_MinGW_32_bit-Release\release\data\plugin
#Plugin specific
DLL=codec_zxtune.dll
CPP_SOURCES=	src/*.cpp \
				src/zxtune-r4110/apps/libzxtune/src/zxtune.cpp \
				src/zxtune-r4110/src/analysis/src/*.cpp \
				src/zxtune-r4110/src/binary/src/*.cpp \
				src/zxtune-r4110/src/binary/compression/src/*.cpp \
				src/zxtune-r4110/src/binary/format/*.cpp \
				src/zxtune-r4110/src/core/src/*.cpp \
				src/zxtune-r4110/src/core/text/*.cpp \
				src/zxtune-r4110/src/core/plugins/*.cpp \
				src/zxtune-r4110/src/core/plugins/archives/stub/*.cpp \
				src/zxtune-r4110/src/core/plugins/players/*.cpp \
				src/zxtune-r4110/src/core/plugins/players/ay/*.cpp \
				src/zxtune-r4110/src/core/plugins/players/dac/chi_supp.cpp \
				src/zxtune-r4110/src/core/plugins/players/dac/dac_plugin.cpp \
				src/zxtune-r4110/src/core/plugins/players/dac/dmm_supp.cpp \
				src/zxtune-r4110/src/core/plugins/players/dac/dst_supp.cpp \
				src/zxtune-r4110/src/core/plugins/players/dac/et1_supp.cpp \
				src/zxtune-r4110/src/core/plugins/players/dac/pdt_supp.cpp \
				src/zxtune-r4110/src/core/plugins/players/dac/sqd_supp.cpp \
				src/zxtune-r4110/src/core/plugins/players/dac/str_supp.cpp \
				src/zxtune-r4110/src/core/plugins/players/saa/*.cpp \
				src/zxtune-r4110/src/core/plugins/players/tfm/*.cpp \
				src/zxtune-r4110/src/debug/src/*.cpp \
				src/zxtune-r4110/src/devices/aym/src/*.cpp \
				src/zxtune-r4110/src/devices/beeper/*.cpp \
				src/zxtune-r4110/src/devices/dac/*.cpp \
				src/zxtune-r4110/src/devices/fm/*.cpp \
				src/zxtune-r4110/src/devices/saa/*.cpp \
				src/zxtune-r4110/src/devices/z80/*.cpp \
				src/zxtune-r4110/src/formats/chiptune/*.cpp \
				src/zxtune-r4110/src/formats/chiptune/aym/*.cpp \
				src/zxtune-r4110/src/formats/chiptune/digital/*.cpp \
				src/zxtune-r4110/src/formats/chiptune/emulation/*.cpp \
				src/zxtune-r4110/src/formats/chiptune/fm/*.cpp \
				src/zxtune-r4110/src/formats/chiptune/multidevice/*.cpp \
				src/zxtune-r4110/src/formats/chiptune/multitrack/*.cpp \
				src/zxtune-r4110/src/formats/chiptune/saa/*.cpp \
				src/zxtune-r4110/src/formats/multitrack/*.cpp \
				src/zxtune-r4110/src/formats/text/*.cpp \
				src/zxtune-r4110/src/formats/packed/*.cpp \
				src/zxtune-r4110/src/l10n/stub/*.cpp \
				src/zxtune-r4110/src/module/src/*.cpp \
				src/zxtune-r4110/src/module/text/*.cpp \
				src/zxtune-r4110/src/module/players/*.cpp \
				src/zxtune-r4110/src/module/players/aym/*.cpp \
				src/zxtune-r4110/src/module/players/dac/dac_base.cpp \
				src/zxtune-r4110/src/module/players/dac/dac_chiptracker.cpp \
				src/zxtune-r4110/src/module/players/dac/dac_digitalmusicmaker.cpp \
				src/zxtune-r4110/src/module/players/dac/dac_digitalstudio.cpp \
				src/zxtune-r4110/src/module/players/dac/dac_extremetracker1.cpp \
				src/zxtune-r4110/src/module/players/dac/dac_parameters.cpp \
				src/zxtune-r4110/src/module/players/dac/dac_prodigitracker.cpp \
				src/zxtune-r4110/src/module/players/dac/dac_properties_helper.cpp \
				src/zxtune-r4110/src/module/players/dac/dac_sampletracker.cpp \
				src/zxtune-r4110/src/module/players/dac/dac_simple.cpp \
				src/zxtune-r4110/src/module/players/dac/dac_sqdigitaltracker.cpp \
				src/zxtune-r4110/src/module/players/saa/*.cpp \
				src/zxtune-r4110/src/module/players/tfm/*.cpp \
				src/zxtune-r4110/src/parameters/src/*.cpp \
				src/zxtune-r4110/src/platform/version/src/*.cpp \
				src/zxtune-r4110/src/platform/version/text/*.cpp \
				src/zxtune-r4110/src/sound/impl/*.cpp \
				src/zxtune-r4110/src/strings/src/*.cpp \
				src/zxtune-r4110/src/strings/text/*.cpp \
				src/zxtune-r4110/src/tools/src/*.cpp \
				src/zxtune-r4110/src/tools/text/*.cpp \
				src/zxtune-r4110/3rdparty/unrar/rdwrfn.cpp \
				src/zxtune-r4110/3rdparty/unrar/rijndael.cpp \
				src/zxtune-r4110/3rdparty/unrar/volume.cpp \
				src/zxtune-r4110/3rdparty/unrar/crypt.cpp \
				src/zxtune-r4110/3rdparty/unrar/file.cpp \
				src/zxtune-r4110/3rdparty/unrar/secpassword.cpp \
				src/zxtune-r4110/3rdparty/unrar/sha1.cpp \
				src/zxtune-r4110/3rdparty/unrar/errhnd.cpp \
				src/zxtune-r4110/3rdparty/unrar/timefn.cpp \
				src/zxtune-r4110/3rdparty/unrar/isnt.cpp \
				src/zxtune-r4110/3rdparty/unrar/consio.cpp \
				src/zxtune-r4110/3rdparty/unrar/log_.cpp \
				src/zxtune-r4110/3rdparty/unrar/resource.cpp \
				src/zxtune-r4110/3rdparty/unrar/filefn.cpp \
				src/zxtune-r4110/3rdparty/unrar/crc.cpp \
				src/zxtune-r4110/3rdparty/unrar/pathfn.cpp \
				src/zxtune-r4110/3rdparty/unrar/unicode.cpp \
				src/zxtune-r4110/3rdparty/unrar/strfn.cpp \
				src/zxtune-r4110/3rdparty/unrar/savepos.cpp \
				src/zxtune-r4110/3rdparty/unrar/system.cpp \
				src/zxtune-r4110/3rdparty/unrar/find.cpp \
				src/zxtune-r4110/3rdparty/unrar/options.cpp \
				src/zxtune-r4110/3rdparty/unrar/archive.cpp \
				src/zxtune-r4110/3rdparty/unrar/recvol.cpp \
				src/zxtune-r4110/3rdparty/unrar/arcread.cpp \
				src/zxtune-r4110/3rdparty/unrar/rawread.cpp \
				src/zxtune-r4110/3rdparty/unrar/encname.cpp \
				src/zxtune-r4110/3rdparty/unrar/unpack.cpp \
				src/zxtune-r4110/3rdparty/unrar/getbits.cpp \
				src/zxtune-r4110/3rdparty/unrar/rarvm.cpp \
				src/zxtune-r4110/3rdparty/unrar/rs.cpp \
				src/zxtune-r4110/3rdparty/unrar/smallfn.cpp \
				src/zxtune-r4110/3rdparty/unrar/extract.cpp \
				src/zxtune-r4110/3rdparty/unrar/cmddata.cpp \
				src/zxtune-r4110/3rdparty/unrar/extinfo.cpp \
				src/zxtune-r4110/3rdparty/unrar/filcreat.cpp \
				src/zxtune-r4110/3rdparty/unrar/strlist.cpp \
				src/zxtune-r4110/3rdparty/unrar/match.cpp \
				src/zxtune-r4110/3rdparty/unrar/list.cpp \
				src/zxtune-r4110/3rdparty/unrar/scantree.cpp \
				src/zxtune-r4110/3rdparty/unrar/filestr.cpp \
				src/zxtune-r4110/3rdparty/unrar/ulinks.cpp \
				src/zxtune-r4110/3rdparty/unrar/global.cpp \


C_SOURCES=		src/zxtune-r4110/3rdparty/zlib/uncompr.c \
				src/zxtune-r4110/3rdparty/zlib/compress.c \
				src/zxtune-r4110/3rdparty/zlib/deflate.c \
				src/zxtune-r4110/3rdparty/zlib/adler32.c \
				src/zxtune-r4110/3rdparty/zlib/crc32.c \
				src/zxtune-r4110/3rdparty/zlib/trees.c \
				src/zxtune-r4110/3rdparty/zlib/zutil.c \
				src/zxtune-r4110/3rdparty/zlib/inflate.c \
				src/zxtune-r4110/3rdparty/zlib/inffast.c \
				src/zxtune-r4110/3rdparty/zlib/inftrees.c \
				src/zxtune-r4110/3rdparty/lhasa/lib/crc16.c \
				src/zxtune-r4110/3rdparty/lhasa/lib/ext_header.c \
				src/zxtune-r4110/3rdparty/lhasa/lib/lha_arch_unix.c \
				src/zxtune-r4110/3rdparty/lhasa/lib/lha_arch_win32.c \
				src/zxtune-r4110/3rdparty/lhasa/lib/lha_decoder.c \
				src/zxtune-r4110/3rdparty/lhasa/lib/lha_endian.c \
				src/zxtune-r4110/3rdparty/lhasa/lib/lha_file_header.c \
				src/zxtune-r4110/3rdparty/lhasa/lib/lha_input_stream.c \
				src/zxtune-r4110/3rdparty/lhasa/lib/lha_basic_reader.c \
				src/zxtune-r4110/3rdparty/lhasa/lib/lha_reader.c \
				src/zxtune-r4110/3rdparty/lhasa/lib/macbinary.c \
				src/zxtune-r4110/3rdparty/lhasa/lib/null_decoder.c \
				src/zxtune-r4110/3rdparty/lhasa/lib/lh1_decoder.c \
				src/zxtune-r4110/3rdparty/lhasa/lib/lh5_decoder.c \
				src/zxtune-r4110/3rdparty/lhasa/lib/lh6_decoder.c \
				src/zxtune-r4110/3rdparty/lhasa/lib/lh7_decoder.c \
				src/zxtune-r4110/3rdparty/lhasa/lib/lz5_decoder.c \
				src/zxtune-r4110/3rdparty/lhasa/lib/lzs_decoder.c \
				src/zxtune-r4110/3rdparty/lhasa/lib/pm2_decoder.c \
				src/zxtune-r4110/3rdparty/z80ex/z80ex.c

Z80EX_SOURCES=	src/zxtune-r4110/3rdparty/z80ex/z80ex.c \

LHASA_SOURCES=	src/zxtune-r4110/3rdparty/lhasa/lib/crc16.c \
				src/zxtune-r4110/3rdparty/lhasa/lib/ext_header.c \
				src/zxtune-r4110/3rdparty/lhasa/lib/lha_arch_unix.c \
				src/zxtune-r4110/3rdparty/lhasa/lib/lha_arch_win32.c \
				src/zxtune-r4110/3rdparty/lhasa/lib/lha_decoder.c \
				src/zxtune-r4110/3rdparty/lhasa/lib/lha_endian.c \
				src/zxtune-r4110/3rdparty/lhasa/lib/lha_file_header.c \
				src/zxtune-r4110/3rdparty/lhasa/lib/lha_input_stream.c \
				src/zxtune-r4110/3rdparty/lhasa/lib/lha_basic_reader.c \
				src/zxtune-r4110/3rdparty/lhasa/lib/lha_reader.c \
				src/zxtune-r4110/3rdparty/lhasa/lib/macbinary.c \
				src/zxtune-r4110/3rdparty/lhasa/lib/null_decoder.c \
				src/zxtune-r4110/3rdparty/lhasa/lib/lh1_decoder.c \
				src/zxtune-r4110/3rdparty/lhasa/lib/lh5_decoder.c \
				src/zxtune-r4110/3rdparty/lhasa/lib/lh6_decoder.c \
				src/zxtune-r4110/3rdparty/lhasa/lib/lh7_decoder.c \
				src/zxtune-r4110/3rdparty/lhasa/lib/lz5_decoder.c \
				src/zxtune-r4110/3rdparty/lhasa/lib/lzs_decoder.c \
				src/zxtune-r4110/3rdparty/lhasa/lib/pm2_decoder.c

LOCAL_INCLUDES=	-I src/zxtune-r4110 \
				-I src/zxtune-r4110/include \
				-I src/zxtune-r4110/src \
				-I src/zxtune-r4110/3rdparty/unrar \
				-I src/zxtune-r4110/3rdparty/z80ex/include \
				-I src/zxtune-r4110/3rdparty/lhasa/lib \
				-I C:\Dev\boost\boost_1_66_0 \

all :
	$(CCP) -c $(CPP_SOURCES) $(CCPFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) $(GLOBAL_LIBS)
	$(CC) -c $(C_SOURCES) $(CFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) $(GLOBAL_LIBS)
	$(CCP) *.o $(CCPFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) -shared -Wl,$(GLOBAL_LIBS) -o $(DLL)
	copy $(DLL) $(BZRPLAYER_PLUGIN)\$(DLL)

clean :
	del *.o \
	$(DLL)

z80ex :
	$(CC) -c $(Z80EX_SOURCES) $(CFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) $(GLOBAL_LIBS)
	$(CCP) *.o $(CCPFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) -shared -Wl,$(GLOBAL_LIBS) -o $(DLL)

lhasa :
	$(CC) -c $(LHASA_SOURCES) $(CFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) $(GLOBAL_LIBS)
	$(CCP) *.o $(CCPFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) -shared -Wl,$(GLOBAL_LIBS) -o $(DLL)
	
onlymain :
	$(CC) -c src/main.cpp src/zxtune-r4110/apps/libzxtune/src/zxtune.cpp $(CFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) $(GLOBAL_LIBS)
	$(CCP) *.o $(CCPFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) -shared -Wl,$(GLOBAL_LIBS) -o $(DLL)
	copy $(DLL) $(BZRPLAYER_PLUGIN)\$(DLL)


testme :
	$(CCP) -c src/zxtune-r4110/src/core/plugins/archives/zdata_supp.cpp $(CCPFLAGS) $(GLOBAL_INCLUDES) $(LOCAL_INCLUDES) $(GLOBAL_LIBS)

