cmake_minimum_required(VERSION 3.28)

set(NAME "furnace")
project(plugin_${NAME} VERSION "0.6.3")

find_package(ZLIB)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

set(LIB1_NAME "${NAME}")
set(LIB1_VERSION ${PROJECT_VERSION})
set(LIB1_NAME_VERSIONED "${LIB1_NAME}-${LIB1_VERSION}")
set(LIB1_FILENAME "v${LIB1_VERSION}.zip")
set(LIB1_URL "https://github.com/tildearrow/${LIB1_NAME}/archive/refs/tags/${LIB1_FILENAME}")
set(LIB1_SHA_256_HASH "34415b19c9fff8300fdb41bd24a349c8852be7336e6286bee368b48b6b770e4e")
download_and_patch(
        ${LIB1_NAME} ${LIB1_NAME_VERSIONED} ${LIB1_FILENAME} ${LIB1_URL} ${LIB1_SHA_256_HASH} ""
        ${CMAKE_CURRENT_SOURCE_DIR}/patches/furnace
)

set(EXTERNAL_SOURCE_DIR_1 "${EXTERNAL_SOURCE_DIR}")

set(LIB2_NAME "adpcm")
set(LIB2_VERSION "ef7a217154badc3b99978ac481b268c8aab67bd8")
set(LIB2_NAME_VERSIONED "${LIB2_NAME}-${LIB2_VERSION}")
set(LIB2_FILENAME "${LIB2_VERSION}.zip")
set(LIB2_URL "https://github.com/superctr/${LIB2_NAME}/archive/${LIB2_FILENAME}")
set(LIB2_SHA_256_HASH "d78895a68ada6dfc6de72943f005d42d7f4e9d98436a0237617c2b523766bab0")
download_and_patch(
        ${LIB2_NAME} ${LIB2_NAME_VERSIONED} ${LIB2_FILENAME} ${LIB2_URL} ${LIB2_SHA_256_HASH} ""
        ${CMAKE_CURRENT_SOURCE_DIR}/patches/adpcm
)

set(EXTERNAL_SOURCE_DIR_2 "${EXTERNAL_SOURCE_DIR}")

set(LIB3_NAME "fmt")
set(LIB3_VERSION "e57ca2e3685b160617d3d95fcd9e789c4e06ca88")
set(LIB3_NAME_VERSIONED "${LIB3_NAME}-${LIB3_VERSION}")
set(LIB3_FILENAME "${LIB3_VERSION}.zip")
set(LIB3_URL "https://github.com/fmtlib/${LIB3_NAME}/archive/${LIB3_FILENAME}")
set(LIB3_SHA_256_HASH "38004c88c72900a935cbeb01d7668af86ec1b9b10273555b02967f8fd574afc1")
download_and_patch(
        ${LIB3_NAME} ${LIB3_NAME_VERSIONED} ${LIB3_FILENAME} ${LIB3_URL} ${LIB3_SHA_256_HASH} ""
        ""
)

set(EXTERNAL_SOURCE_DIR_3 "${EXTERNAL_SOURCE_DIR}")

if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    add_compile_definitions(ENABLE_VIRTUAL_TERMINAL_PROCESSING=0x0004)
endif ()

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR_1}/extern/adpcm-xq-s/*.c
        ${EXTERNAL_SOURCE_DIR_1}/extern/emu2413/*.c
        ${EXTERNAL_SOURCE_DIR_1}/extern/ESFMu/*.c
        ${EXTERNAL_SOURCE_DIR_1}/extern/Nuked-OPLL/*.c
        ${EXTERNAL_SOURCE_DIR_1}/extern/Nuked-PSG/*.c
        ${EXTERNAL_SOURCE_DIR_1}/extern/opl/*.c
        ${EXTERNAL_SOURCE_DIR_1}/extern/opm/*.c
        ${EXTERNAL_SOURCE_DIR_1}/extern/opn/*.c
        ${EXTERNAL_SOURCE_DIR_1}/extern/pwrnoise/*.c
        ${EXTERNAL_SOURCE_DIR_1}/extern/SAASound/src/*.cpp
        ${EXTERNAL_SOURCE_DIR_1}/extern/vgsound_emu-modified/vgsound_emu/src/core/vox/*.cpp
        ${EXTERNAL_SOURCE_DIR_1}/extern/vgsound_emu-modified/vgsound_emu/src/es550x/*.cpp
        ${EXTERNAL_SOURCE_DIR_1}/extern/vgsound_emu-modified/vgsound_emu/src/k005289/*.cpp
        ${EXTERNAL_SOURCE_DIR_1}/extern/vgsound_emu-modified/vgsound_emu/src/k007232/*.cpp
        ${EXTERNAL_SOURCE_DIR_1}/extern/vgsound_emu-modified/vgsound_emu/src/k053260/*.cpp
        ${EXTERNAL_SOURCE_DIR_1}/extern/vgsound_emu-modified/vgsound_emu/src/msm6295/*.cpp
        ${EXTERNAL_SOURCE_DIR_1}/extern/vgsound_emu-modified/vgsound_emu/src/n163/*.cpp
        ${EXTERNAL_SOURCE_DIR_1}/extern/vgsound_emu-modified/vgsound_emu/src/scc/*.cpp
        ${EXTERNAL_SOURCE_DIR_1}/extern/vgsound_emu-modified/vgsound_emu/src/vrcvi/*.cpp
        ${EXTERNAL_SOURCE_DIR_1}/extern/vgsound_emu-modified/vgsound_emu/src/x1_010/*.cpp
        ${EXTERNAL_SOURCE_DIR_1}/extern/YM2608-LLE/fmopna_2608.c
        ${EXTERNAL_SOURCE_DIR_1}/extern/YM2608-LLE/fmopna_2610.c
        ${EXTERNAL_SOURCE_DIR_1}/extern/YM2608-LLE/fmopna_2612.c
        ${EXTERNAL_SOURCE_DIR_1}/extern/YM3812-LLE/*.c
        ${EXTERNAL_SOURCE_DIR_1}/extern/YMF262-LLE/*.c
        ${EXTERNAL_SOURCE_DIR_1}/extern/YMF276-LLE/*.c
        ${EXTERNAL_SOURCE_DIR_1}/src/audio/abstract.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/audio/midi.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/audio/pipe.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/baseutils.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/*.c
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/cmdStream.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/cmdStreamOps.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/config.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/configEngine.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/dispatchContainer.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/effect/*.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/engine.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/export.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/export/*.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/fileOps/*.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/fileOpsIns.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/fileOpsSample.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/filter.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/instrument.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/macroInt.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/pattern.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/pitchTable.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/platform/*.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/platform/sound/*.c
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/platform/sound/*.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/platform/sound/atomicssg/*.c
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/platform/sound/c64/*.cc
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/platform/sound/c64_d/*.c
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/platform/sound/c64_fp/*.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/platform/sound/c64_fp/resample/SincResampler.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/platform/sound/dave/*.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/platform/sound/ga20/*.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/platform/sound/gb/*.c
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/platform/sound/lynx/*.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/platform/sound/nes/*.c
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/platform/sound/nes_nsfplay/*.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/platform/sound/oki/*.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/platform/sound/pokey/*.c
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/platform/sound/pokey/*.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/platform/sound/sid2/*.cc
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/platform/sound/snes/*.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/platform/sound/t6w28/*.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/platform/sound/tia/*.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/platform/sound/ymfm/*.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/playback.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/safeReader.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/safeWriter.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/sample.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/song.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/sysDef.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/vgmOps.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/waveSynth.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/wavetable.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/wavOps.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/winStuff.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/workPool.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/zsm.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/zsmOps.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/fileutils.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/log.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/utfutils.cpp
        ${EXTERNAL_SOURCE_DIR_2}/*.c
        ${EXTERNAL_SOURCE_DIR_3}/src/format.cc
        ${EXTERNAL_SOURCE_DIR_3}/src/os.cc
        src/main.cpp
)

list(REMOVE_ITEM SOURCES ${EXTERNAL_SOURCE_DIR_1}/src/engine/platform/sound/m114s.c)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR_1}/extern/adpcm-xq-s
        ${EXTERNAL_SOURCE_DIR_1}/extern/emu2413
        ${EXTERNAL_SOURCE_DIR_1}/extern/ESFMu
        ${EXTERNAL_SOURCE_DIR_1}/extern/IconFontCppHeaders
        ${EXTERNAL_SOURCE_DIR_1}/extern/Nuked-OPLL
        ${EXTERNAL_SOURCE_DIR_1}/extern/Nuked-PSG
        ${EXTERNAL_SOURCE_DIR_1}/extern/opl
        ${EXTERNAL_SOURCE_DIR_1}/extern/opm
        ${EXTERNAL_SOURCE_DIR_1}/extern/opn
        ${EXTERNAL_SOURCE_DIR_1}/extern/pwrnoise
        ${EXTERNAL_SOURCE_DIR_1}/extern/SAASound/src
        ${EXTERNAL_SOURCE_DIR_1}/extern/vgsound_emu-modified
        ${EXTERNAL_SOURCE_DIR_1}/extern/vgsound_emu-modified/vgsound_emu
        ${EXTERNAL_SOURCE_DIR_1}/extern/vgsound_emu-modified/vgsound_emu/src/core
        ${EXTERNAL_SOURCE_DIR_1}/extern/vgsound_emu-modified/vgsound_emu/src/core/e550x
        ${EXTERNAL_SOURCE_DIR_1}/extern/vgsound_emu-modified/vgsound_emu/src/core/k005289
        ${EXTERNAL_SOURCE_DIR_1}/extern/vgsound_emu-modified/vgsound_emu/src/core/k007232
        ${EXTERNAL_SOURCE_DIR_1}/extern/vgsound_emu-modified/vgsound_emu/src/core/k053260
        ${EXTERNAL_SOURCE_DIR_1}/extern/vgsound_emu-modified/vgsound_emu/src/core/msm6295
        ${EXTERNAL_SOURCE_DIR_1}/extern/vgsound_emu-modified/vgsound_emu/src/core/n163
        ${EXTERNAL_SOURCE_DIR_1}/extern/vgsound_emu-modified/vgsound_emu/src/core/scc
        ${EXTERNAL_SOURCE_DIR_1}/extern/vgsound_emu-modified/vgsound_emu/src/core/template
        ${EXTERNAL_SOURCE_DIR_1}/extern/vgsound_emu-modified/vgsound_emu/src/core/util
        ${EXTERNAL_SOURCE_DIR_1}/extern/vgsound_emu-modified/vgsound_emu/src/core/vox
        ${EXTERNAL_SOURCE_DIR_1}/extern/vgsound_emu-modified/vgsound_emu/src/core/vrcvi
        ${EXTERNAL_SOURCE_DIR_1}/extern/vgsound_emu-modified/vgsound_emu/src/core/x1_010
        ${EXTERNAL_SOURCE_DIR_1}/extern/YM2608-LLE
        ${EXTERNAL_SOURCE_DIR_1}/extern/YM3812-LLE
        ${EXTERNAL_SOURCE_DIR_1}/extern/YMF262-LLE
        ${EXTERNAL_SOURCE_DIR_1}/extern/YMF276-LLE
        ${EXTERNAL_SOURCE_DIR_1}/src
        ${EXTERNAL_SOURCE_DIR_1}/src/audio
        ${EXTERNAL_SOURCE_DIR_1}/src/baseutils
        ${EXTERNAL_SOURCE_DIR_1}/src/engine
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/effect
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/export
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/fileOps
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/platform
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/platform/sound
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/platform/sound/c64
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/platform/sound/c64_d
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/platform/sound/c64_fp
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/platform/sound/c64_fp/resample
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/platform/sound/dave
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/platform/sound/ga20
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/platform/sound/gb
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/platform/sound/lynx
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/platform/sound/nes
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/platform/sound/nes_nsfplay
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/platform/sound/oki
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/platform/sound/pokey
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/platform/sound/snes
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/platform/sound/t6w28
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/platform/sound/tia
        ${EXTERNAL_SOURCE_DIR_1}/src/engine/platform/sound/ymfm
        ${EXTERNAL_SOURCE_DIR_1}/src/icon
        ${EXTERNAL_SOURCE_DIR_2}
        ${EXTERNAL_SOURCE_DIR_3}/include

        ${CMAKE_SOURCE_DIR}/src/app
        ${EXTERNAL_SOURCE_DIR_fmod}/api/core/inc
)

if (NOT CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows") #fix #543
    target_link_libraries(${PROJECT_NAME} PRIVATE
            ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/libstdc++.a
    )
endif ()

target_link_libraries(${PROJECT_NAME} PRIVATE
        ZLIB::ZLIB
        -lshlwapi
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "codec_${NAME}"
        PREFIX ""
        SUFFIX ".dll"
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR_PARENT}/data/plugin
        COMPILE_FLAGS "-m32"
        LINK_FLAGS "-m32"
)
