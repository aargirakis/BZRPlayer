cmake_minimum_required(VERSION 3.28)

set(NAME "zxtune")
set(VERSION "5100")
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "ZXTune" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(NAME_VERSIONED "${NAME}-${VERSION}")
set(TAG "r${VERSION}")
set(NAME_TAGGED "${NAME}-${TAG}")
set(FILENAME "${NAME_TAGGED}.tar.gz")
set(FILENAME_URL "${TAG}.tar.gz")
set(URL "https://github.com/vitamin-caig/${NAME}/archive/refs/tags/${FILENAME_URL}")
set(SHA_256_HASH "2123cc21bc422eaec3ae358246ebe518bc7b9066d6f0726d2eb7a264ff9359fb")
set(UNPACKED_DIR "${NAME_TAGGED}")
download_patch_and_add(
        ${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} true ${UNPACKED_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/patches
)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/3rdparty/fmt/src/format.cc
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
        ${EXTERNAL_SOURCE_DIR}/src/analysis/src/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/binary/compression/src/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/binary/format/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/binary/src/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/archives/stub/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/players/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/players/ay/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/players/dac/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/players/multi/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/players/saa/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/players/tfm/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/core/src/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/devices/aym/src/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/devices/dac/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/devices/fm/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/devices/saa/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/chiptune/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/chiptune/aym/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/chiptune/digital/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/chiptune/fm/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/chiptune/multidevice/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/chiptune/multitrack/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/chiptune/saa/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/multitrack/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/packed/container.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/packed/lha_file.cpp
        ${EXTERNAL_SOURCE_DIR}/src/module/players/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/module/players/aym/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/module/players/dac/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/module/players/saa/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/module/players/tfm/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/module/src/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/parameters/src/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/sound/impl/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/strings/src/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/tools/src/*.cpp
        src/main.cpp)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND SOURCES ${EXTERNAL_SOURCE_DIR}/src/debug/src/log.cpp)
endif ()

list(REMOVE_ITEM SOURCES
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/players/ay/ay_supp.cpp
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/players/dac/ahx_supp.cpp
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/players/dac/v2m_supp.cpp
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/players/multitrack_plugin.cpp
        ${EXTERNAL_SOURCE_DIR}/src/module/players/aym/ayemul.cpp
        ${EXTERNAL_SOURCE_DIR}/src/module/players/dac/abysshighestexperience.cpp
        ${EXTERNAL_SOURCE_DIR}/src/module/players/dac/v2m.cpp
        ${EXTERNAL_SOURCE_DIR}/src/sound/impl/resampler.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}
        ${EXTERNAL_SOURCE_DIR}/3rdparty/fmt/include
        ${EXTERNAL_SOURCE_DIR}/3rdparty/lhasa/lib
        ${EXTERNAL_SOURCE_DIR}/include
        ${EXTERNAL_SOURCE_DIR}/src
        ${EXTERNAL_SOURCE_DIR}/src/module

        ${CMAKE_SOURCE_DIR}/src/app
        ${FMOD_API_DIR})

target_compile_definitions(${PROJECT_NAME} PRIVATE NO_L10N)
target_compile_definitions(${PROJECT_NAME} PRIVATE SOURCES_ROOT="${EXTERNAL_SOURCE_DIR}")

if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE NO_DEBUG_LOGS)
endif ()

# this avoid multiple issues (eg in track samples gathering)
target_compile_options(${PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-funsigned-char>)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
)
