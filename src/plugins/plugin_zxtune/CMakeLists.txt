cmake_minimum_required(VERSION 3.28)

set(NAME "zxtune")
set(VERSION "5110")
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "ZXTune" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

find_package(ZLIB REQUIRED)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(NAME_VERSIONED "${NAME}-${VERSION}")
set(TAG "r${VERSION}")
set(NAME_TAGGED "${NAME}-${TAG}")
set(FILENAME "${NAME_TAGGED}.tar.gz")
set(FILENAME_URL "${TAG}.tar.gz")
set(URL "https://github.com/vitamin-caig/${NAME}/archive/refs/tags/${FILENAME_URL}")
set(SHA_256_HASH "1b64fe16d60ce0c971b37e67ca0130b917b60b7dfda2a05ec1c606c37bd8dd58")
set(UNPACKED_DIR "${NAME_TAGGED}")
download_patch_and_add(${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} true ${UNPACKED_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/patches)

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
        ${EXTERNAL_SOURCE_DIR}/src/analysis/src/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/binary/compression/src/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/binary/format/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/binary/format/full/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/binary/src/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/archives/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/archives/full/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/players/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/players/ay/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/players/dac/dac_plugin.cpp
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/players/multi/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/players/saa/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/players/tfm/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/core/src/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/devices/aym/src/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/devices/dac/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/devices/fm/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/devices/saa/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/archived/other/zxstate.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/archived/trdos/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/chiptune/aym/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/chiptune/common/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/chiptune/digital/chiptracker.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/chiptune/digital/digital.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/chiptune/digital/digitalmusicmaker.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/chiptune/digital/digitalstudio.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/chiptune/digital/extremetracker1.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/chiptune/digital/prodigitracker.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/chiptune/digital/sampletracker.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/chiptune/digital/sqdigitaltracker.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/chiptune/emulation/ay.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/chiptune/fm/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/chiptune/multidevice/multitrackcontainer.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/chiptune/saa/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/packed/common/container.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/packed/decompilers/compiledasc.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/packed/decompilers/compiledpt2.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/packed/decompilers/compiledptu13.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/packed/decompilers/compiledst3.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/packed/decompilers/compiledstp.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/packed/images/dsk.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/packed/images/fulldiskimage.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/packed/images/teledisk.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/packed/images/utils.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/packed/snapshots/sna128.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/packed/snapshots/z80.cpp
        ${EXTERNAL_SOURCE_DIR}/src/formats/packed/zx/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/module/players/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/module/players/aym/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/module/players/dac/chiptracker.cpp
        ${EXTERNAL_SOURCE_DIR}/src/module/players/dac/dac_base.cpp
        ${EXTERNAL_SOURCE_DIR}/src/module/players/dac/dac_factory.cpp
        ${EXTERNAL_SOURCE_DIR}/src/module/players/dac/dac_parameters.cpp
        ${EXTERNAL_SOURCE_DIR}/src/module/players/dac/dac_properties_helper.cpp
        ${EXTERNAL_SOURCE_DIR}/src/module/players/dac/dac_simple.cpp
        ${EXTERNAL_SOURCE_DIR}/src/module/players/dac/digitalmusicmaker.cpp
        ${EXTERNAL_SOURCE_DIR}/src/module/players/dac/digitalstudio.cpp
        ${EXTERNAL_SOURCE_DIR}/src/module/players/dac/extremetracker1.cpp
        ${EXTERNAL_SOURCE_DIR}/src/module/players/dac/prodigitracker.cpp
        ${EXTERNAL_SOURCE_DIR}/src/module/players/dac/sampletracker.cpp
        ${EXTERNAL_SOURCE_DIR}/src/module/players/dac/sqdigitaltracker.cpp
        ${EXTERNAL_SOURCE_DIR}/src/module/players/multitrack/*.cpp
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
        ${EXTERNAL_SOURCE_DIR}/src/core/plugins/players/multitrack_plugin.cpp
        ${EXTERNAL_SOURCE_DIR}/src/module/players/aym/ayemul.cpp
        ${EXTERNAL_SOURCE_DIR}/src/sound/impl/resampler.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}
        ${EXTERNAL_SOURCE_DIR}/3rdparty/fmt/include
        ${EXTERNAL_SOURCE_DIR}/include
        ${EXTERNAL_SOURCE_DIR}/src

        ${CMAKE_SOURCE_DIR}/src/app
        ${LOGGER_API_DIR}
        ${FMOD_API_DIR})

set(CONFIG_FILENAME "${PROJECT_NAME}.cfg")
set(PLUGIN_${NAME}_CONFIG_FILENAME "${CONFIG_FILENAME}" PARENT_SCOPE)
target_compile_definitions(${PROJECT_NAME} PRIVATE CONFIG_FILENAME="${CONFIG_FILENAME}")
target_compile_definitions(${PROJECT_NAME} PRIVATE PLUGIN_CONFIGS_DIR="${PLUGIN_CONFIGS_DIR}")
target_compile_definitions(${PROJECT_NAME} PRIVATE NO_L10N)
target_compile_definitions(${PROJECT_NAME} PRIVATE SOURCES_ROOT="${EXTERNAL_SOURCE_DIR}")

if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE NO_DEBUG_LOGS)
endif ()

# this avoid multiple issues (eg in track samples gathering)
target_compile_options(${PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-funsigned-char>)

target_link_libraries(${PROJECT_NAME} PRIVATE ZLIB::ZLIB logger)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_PATH}/${CMAKE_INSTALL_LIBDIR}${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_PATH}${DATA_DIR}${PLUGINS_DIR})

if (NOT WIN32)
    install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}${PLUGINS_DIR})
endif ()