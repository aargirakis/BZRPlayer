cmake_minimum_required(VERSION 3.28)

#TODO decrease zxtune priority?

set(NAME "zxtune")
set(VERSION "r4340")
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "ZXTune" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

set(NAME_VERSIONED "${NAME}-${VERSION}")
set(FILENAME "${NAME_VERSIONED}.tar.gz")
set(FILENAME_URL "${VERSION}.tar.gz")
set(URL "https://github.com/vitamin-caig/${NAME}/archive/refs/tags/${FILENAME_URL}")
set(SHA_256_HASH "d196b7f6ddb490652f4283f4dfe2aba1f4a17ecf4084d08ae4386c60f8fd7465")
download_patch_and_add(
        ${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} false ""
        ${CMAKE_CURRENT_SOURCE_DIR}/patches
)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/src/*.c
        ${EXTERNAL_SOURCE_DIR}/src/*.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/lhasa/lib/crc16.c
        ${EXTERNAL_SOURCE_DIR}/3rdparty/lhasa/lib/ext_header.c
        ${EXTERNAL_SOURCE_DIR}/3rdparty/lhasa/lib/lh1_decoder.c
        ${EXTERNAL_SOURCE_DIR}/3rdparty/lhasa/lib/lh5_decoder.c
        ${EXTERNAL_SOURCE_DIR}/3rdparty/lhasa/lib/lh6_decoder.c
        ${EXTERNAL_SOURCE_DIR}/3rdparty/lhasa/lib/lh7_decoder.c
        ${EXTERNAL_SOURCE_DIR}/3rdparty/lhasa/lib/lha_arch_unix.c
        ${EXTERNAL_SOURCE_DIR}/3rdparty/lhasa/lib/lha_arch_win32.c
        ${EXTERNAL_SOURCE_DIR}/3rdparty/lhasa/lib/lha_basic_reader.c
        ${EXTERNAL_SOURCE_DIR}/3rdparty/lhasa/lib/lha_decoder.c
        ${EXTERNAL_SOURCE_DIR}/3rdparty/lhasa/lib/lha_endian.c
        ${EXTERNAL_SOURCE_DIR}/3rdparty/lhasa/lib/lha_file_header.c
        ${EXTERNAL_SOURCE_DIR}/3rdparty/lhasa/lib/lha_input_stream.c
        ${EXTERNAL_SOURCE_DIR}/3rdparty/lhasa/lib/lha_reader.c
        ${EXTERNAL_SOURCE_DIR}/3rdparty/lhasa/lib/lz5_decoder.c
        ${EXTERNAL_SOURCE_DIR}/3rdparty/lhasa/lib/lzs_decoder.c
        ${EXTERNAL_SOURCE_DIR}/3rdparty/lhasa/lib/macbinary.c
        ${EXTERNAL_SOURCE_DIR}/3rdparty/lhasa/lib/null_decoder.c
        ${EXTERNAL_SOURCE_DIR}/3rdparty/lhasa/lib/pm2_decoder.c
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar/archive.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar/arcread.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar/cmddata.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar/consio.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar/crc.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar/crypt.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar/encname.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar/errhnd.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar/extinfo.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar/extract.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar/filcreat.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar/file.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar/filefn.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar/filestr.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar/find.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar/getbits.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar/global.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar/isnt.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar/list.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar/log.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar/match.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar/options.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar/pathfn.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar/rarvm.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar/rawread.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar/rdwrfn.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar/recvol.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar/resource.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar/rijndael.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar/rs.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar/savepos.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar/scantree.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar/secpassword.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar/sha1.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar/smallfn.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar/strfn.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar/strlist.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar/system.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar/timefn.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar/ulinks.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar/unicode.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar/unpack.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar/volume.cpp
        ${EXTERNAL_SOURCE_DIR}/3rdparty/z80ex/z80ex.c
        ${EXTERNAL_SOURCE_DIR}/3rdparty/zlib/adler32.c
        ${EXTERNAL_SOURCE_DIR}/3rdparty/zlib/compress.c
        ${EXTERNAL_SOURCE_DIR}/3rdparty/zlib/crc32.c
        ${EXTERNAL_SOURCE_DIR}/3rdparty/zlib/deflate.c
        ${EXTERNAL_SOURCE_DIR}/3rdparty/zlib/inffast.c
        ${EXTERNAL_SOURCE_DIR}/3rdparty/zlib/inflate.c
        ${EXTERNAL_SOURCE_DIR}/3rdparty/zlib/inftrees.c
        ${EXTERNAL_SOURCE_DIR}/3rdparty/zlib/trees.c
        ${EXTERNAL_SOURCE_DIR}/3rdparty/zlib/uncompr.c
        ${EXTERNAL_SOURCE_DIR}/3rdparty/zlib/zutil.c
        ${EXTERNAL_SOURCE_DIR}/apps/libzxtune/src/zxtune.cpp
        ${EXTERNAL_SOURCE_DIR}/src/analysis/src/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/binary/compression/src/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/binary/format/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/binary/src/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/archives/stub/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/players/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/players/asap/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/players/ay/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/players/dac/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/players/gme/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/players/multi/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/players/music/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/players/saa/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/players/sid/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/players/tfm/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/players/xmp/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/players/xsf/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/core/src/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/core/text/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/debug/src/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/devices/aym/src/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/devices/beeper/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/devices/dac/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/devices/fm/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/devices/saa/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/devices/z80/*.cpp

        ${EXTERNAL_SOURCE_DIR}/src/formats/archived/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/archived/multitrack/*.cpp

        ${EXTERNAL_SOURCE_DIR}/src/formats/chiptune/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/chiptune/aym/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/chiptune/digital/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/chiptune/emulation/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/chiptune/fm/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/chiptune/multidevice/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/chiptune/multitrack/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/chiptune/music/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/chiptune/saa/*.cpp



        ${EXTERNAL_SOURCE_DIR}/src/formats/image/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/instrumentation/*.cpp

        ${EXTERNAL_SOURCE_DIR}/src/formats/multitrack/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/packed/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/packed/archives/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/packed/decompilers/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/packed/lha/*.cpp



        ${EXTERNAL_SOURCE_DIR}/src/formats/text/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/l10n/stub/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/module/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/module/players/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/module/players/aym/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/module/players/dac/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/module/players/saa/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/module/players/tfm/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/module/players/xsf/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/module/src/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/module/text/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/parameters/src/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/platform/version/src/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/platform/version/text/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/sound/impl/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/strings/src/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/strings/text/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/tools/src/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/tools/text/*.cpp
        src/main.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        src/boost_1_66_0
        ${EXTERNAL_SOURCE_DIR}
        ${EXTERNAL_SOURCE_DIR}/3rdparty/lhasa/lib
        ${EXTERNAL_SOURCE_DIR}/3rdparty/unrar
        ${EXTERNAL_SOURCE_DIR}/3rdparty/z80ex/include
        ${EXTERNAL_SOURCE_DIR}/apps/libzxtune/src
        ${EXTERNAL_SOURCE_DIR}/include
        ${EXTERNAL_SOURCE_DIR}/src
        ${EXTERNAL_SOURCE_DIR}/src/module
        ${EXTERNAL_SOURCE_DIR}/src/module/players
        ${EXTERNAL_SOURCE_DIR}/src/module/players/aym
        ${EXTERNAL_SOURCE_DIR}/src/module/players/dac
        ${EXTERNAL_SOURCE_DIR}/src/module/players/saa
        ${EXTERNAL_SOURCE_DIR}/src/module/players/tfm
        ${EXTERNAL_SOURCE_DIR}/src/module/players/xs
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/players
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/players/asap
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/players/dac
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/players/gme
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/players/multi
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/players/music
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/players/saa
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/players/sid
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/players/tfm
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/players/xmp
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/players/xsf
        ${EXTERNAL_SOURCE_DIR}/src/formats/chiptune
        ${EXTERNAL_SOURCE_DIR}/src/formats/chiptune/aym
        ${EXTERNAL_SOURCE_DIR}/src/formats/chiptune/digital
        ${EXTERNAL_SOURCE_DIR}/src/formats/chiptune/emulation
        ${EXTERNAL_SOURCE_DIR}/src/formats/chiptune/fm
        ${EXTERNAL_SOURCE_DIR}/src/formats/chiptune/multidevice
        ${EXTERNAL_SOURCE_DIR}/src/formats/chiptune/multitrack
        ${EXTERNAL_SOURCE_DIR}/src/formats/chiptune/music
        ${EXTERNAL_SOURCE_DIR}/src/formats/chiptune/saa
        ${EXTERNAL_SOURCE_DIR}/src/formats
        ${EXTERNAL_SOURCE_DIR}/src/formats/archived
        ${EXTERNAL_SOURCE_DIR}/src/formats/archived/multitrack
        ${EXTERNAL_SOURCE_DIR}/src/formats/image
        ${EXTERNAL_SOURCE_DIR}/src/formats/instrumentation
        ${EXTERNAL_SOURCE_DIR}/src/formats/multitrack
        ${EXTERNAL_SOURCE_DIR}/src/formats/packed
        ${EXTERNAL_SOURCE_DIR}/src/formats/packed/archives
        ${EXTERNAL_SOURCE_DIR}/src/formats/packed/decompilers
        ${EXTERNAL_SOURCE_DIR}/src/formats/packed/lha
        ${EXTERNAL_SOURCE_DIR}/src/formats/text

        ${CMAKE_SOURCE_DIR}/src/app
        ${FMOD_API_DIR})

target_compile_definitions(${PROJECT_NAME} PRIVATE Z80EX_API_REVISION=1)
target_compile_definitions(${PROJECT_NAME} PRIVATE Z80EX_VERSION_MAJOR=1)
target_compile_definitions(${PROJECT_NAME} PRIVATE Z80EX_VERSION_MINOR=19)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
)
