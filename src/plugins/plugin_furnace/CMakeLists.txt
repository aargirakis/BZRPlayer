cmake_minimum_required(VERSION 3.28)

set(NAME "furnace")
set(VERSION "0.6.8.3")
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "Furnace" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

find_package(ZLIB REQUIRED)

add_subdirectory(deps/adpcm)
add_subdirectory(deps/fmt)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

set(NAME_VERSIONED "${NAME}-${VERSION}")
set(FILENAME "${NAME_VERSIONED}.tar.gz")
set(FILENAME_URL "v${VERSION}.tar.gz")
set(URL "https://github.com/tildearrow/${NAME}/archive/refs/tags/${FILENAME_URL}")
set(SHA_256_HASH "c54b93dc7211a8e86828ea6b436628920df7a7dc87b130b3752f2251f5466e9c")
download_patch_and_add(
        ${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} false ""
        ${CMAKE_CURRENT_SOURCE_DIR}/patches
)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/extern/adpcm-xq-s/*.c
        ${EXTERNAL_SOURCE_DIR}/extern/blip_buf/*.c
        ${EXTERNAL_SOURCE_DIR}/extern/emu2413/*.c
        ${EXTERNAL_SOURCE_DIR}/extern/ESFMu/*.c
        ${EXTERNAL_SOURCE_DIR}/extern/itcompress/compression.c
        ${EXTERNAL_SOURCE_DIR}/extern/Nuked-OPLL/*.c
        ${EXTERNAL_SOURCE_DIR}/extern/Nuked-PSG/*.c
        ${EXTERNAL_SOURCE_DIR}/extern/opl/*.c
        ${EXTERNAL_SOURCE_DIR}/extern/opm/*.c
        ${EXTERNAL_SOURCE_DIR}/extern/opn/*.c
        ${EXTERNAL_SOURCE_DIR}/extern/pwrnoise/*.c
        ${EXTERNAL_SOURCE_DIR}/extern/SAASound/src/*.cpp
        ${EXTERNAL_SOURCE_DIR}/extern/vgsound_emu-modified/vgsound_emu/src/core/vox/*.cpp
        ${EXTERNAL_SOURCE_DIR}/extern/vgsound_emu-modified/vgsound_emu/src/es550x/*.cpp
        ${EXTERNAL_SOURCE_DIR}/extern/vgsound_emu-modified/vgsound_emu/src/k005289/*.cpp
        ${EXTERNAL_SOURCE_DIR}/extern/vgsound_emu-modified/vgsound_emu/src/k007232/*.cpp
        ${EXTERNAL_SOURCE_DIR}/extern/vgsound_emu-modified/vgsound_emu/src/k053260/*.cpp
        ${EXTERNAL_SOURCE_DIR}/extern/vgsound_emu-modified/vgsound_emu/src/msm6295/*.cpp
        ${EXTERNAL_SOURCE_DIR}/extern/vgsound_emu-modified/vgsound_emu/src/n163/*.cpp
        ${EXTERNAL_SOURCE_DIR}/extern/vgsound_emu-modified/vgsound_emu/src/scc/*.cpp
        ${EXTERNAL_SOURCE_DIR}/extern/vgsound_emu-modified/vgsound_emu/src/vrcvi/*.cpp
        ${EXTERNAL_SOURCE_DIR}/extern/vgsound_emu-modified/vgsound_emu/src/x1_010/*.cpp
        ${EXTERNAL_SOURCE_DIR}/extern/YM2608-LLE/fmopna_2608.c
        ${EXTERNAL_SOURCE_DIR}/extern/YM2608-LLE/fmopna_2610.c
        ${EXTERNAL_SOURCE_DIR}/extern/YM2608-LLE/fmopna_2612.c
        ${EXTERNAL_SOURCE_DIR}/extern/YM3812-LLE/*.c
        ${EXTERNAL_SOURCE_DIR}/extern/YMF262-LLE/*.c
        ${EXTERNAL_SOURCE_DIR}/extern/YMF276-LLE/*.c
        ${EXTERNAL_SOURCE_DIR}/src/audio/abstract.cpp
        ${EXTERNAL_SOURCE_DIR}/src/audio/midi.cpp
        ${EXTERNAL_SOURCE_DIR}/src/audio/pipe.cpp
        ${EXTERNAL_SOURCE_DIR}/src/baseutils.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/*.c
        ${EXTERNAL_SOURCE_DIR}/src/engine/cmdStream.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/cmdStreamOps.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/config.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/configEngine.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/dispatchContainer.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/effect/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/engine.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/export.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/exportDef.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/export/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/fileOps/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/fileOpsIns.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/fileOpsSample.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/filter.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/instrument.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/macroInt.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/pattern.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/pitchTable.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform/sound/*.c
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform/sound/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform/sound/atomicssg/*.c
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform/sound/c64/*.cc
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform/sound/c64_d/*.c
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform/sound/c64_fp/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform/sound/c64_fp/resample/SincResampler.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform/sound/dave/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform/sound/ga20/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform/sound/gb/*.c
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform/sound/lynx/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform/sound/nes/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform/sound/nes_nsfplay/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform/sound/oki/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform/sound/pokey/*.c
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform/sound/pokey/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform/sound/sid2/*.cc
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform/sound/snes/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform/sound/t6w28/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform/sound/tia/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform/sound/ymf278b/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform/sound/ymfm/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/playback.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/safeReader.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/safeWriter.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/sample.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/song.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/sysDef.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/vgmOps.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/waveSynth.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/wavetable.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/wavOps.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/workPool.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/zsm.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/zsmOps.cpp
        ${EXTERNAL_SOURCE_DIR}/src/fileutils.cpp
        ${EXTERNAL_SOURCE_DIR}/src/log.cpp
        src/main.cpp)

if (WIN32)
    list(APPEND SOURCES ${EXTERNAL_SOURCE_DIR}/src/utfutils.cpp)
    list(APPEND SOURCES ${EXTERNAL_SOURCE_DIR}/src/engine/winStuff.cpp)
endif ()

list(REMOVE_ITEM SOURCES
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform/swan_before.cpp
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform/sound/m114s.c)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/extern/adpcm-xq-s
        ${EXTERNAL_SOURCE_DIR}/extern/blip_buf
        ${EXTERNAL_SOURCE_DIR}/extern/emu2413
        ${EXTERNAL_SOURCE_DIR}/extern/ESFMu
        ${EXTERNAL_SOURCE_DIR}/extern/IconFontCppHeaders
        ${EXTERNAL_SOURCE_DIR}/extern/Nuked-OPLL
        ${EXTERNAL_SOURCE_DIR}/extern/Nuked-PSG
        ${EXTERNAL_SOURCE_DIR}/extern/opl
        ${EXTERNAL_SOURCE_DIR}/extern/opm
        ${EXTERNAL_SOURCE_DIR}/extern/opn
        ${EXTERNAL_SOURCE_DIR}/extern/pwrnoise
        ${EXTERNAL_SOURCE_DIR}/extern/SAASound/src
        ${EXTERNAL_SOURCE_DIR}/extern/vgsound_emu-modified
        ${EXTERNAL_SOURCE_DIR}/extern/vgsound_emu-modified/vgsound_emu
        ${EXTERNAL_SOURCE_DIR}/extern/vgsound_emu-modified/vgsound_emu/src/core
        ${EXTERNAL_SOURCE_DIR}/extern/vgsound_emu-modified/vgsound_emu/src/core/e550x
        ${EXTERNAL_SOURCE_DIR}/extern/vgsound_emu-modified/vgsound_emu/src/core/k005289
        ${EXTERNAL_SOURCE_DIR}/extern/vgsound_emu-modified/vgsound_emu/src/core/k007232
        ${EXTERNAL_SOURCE_DIR}/extern/vgsound_emu-modified/vgsound_emu/src/core/k053260
        ${EXTERNAL_SOURCE_DIR}/extern/vgsound_emu-modified/vgsound_emu/src/core/msm6295
        ${EXTERNAL_SOURCE_DIR}/extern/vgsound_emu-modified/vgsound_emu/src/core/n163
        ${EXTERNAL_SOURCE_DIR}/extern/vgsound_emu-modified/vgsound_emu/src/core/scc
        ${EXTERNAL_SOURCE_DIR}/extern/vgsound_emu-modified/vgsound_emu/src/core/template
        ${EXTERNAL_SOURCE_DIR}/extern/vgsound_emu-modified/vgsound_emu/src/core/util
        ${EXTERNAL_SOURCE_DIR}/extern/vgsound_emu-modified/vgsound_emu/src/core/vox
        ${EXTERNAL_SOURCE_DIR}/extern/vgsound_emu-modified/vgsound_emu/src/core/vrcvi
        ${EXTERNAL_SOURCE_DIR}/extern/vgsound_emu-modified/vgsound_emu/src/core/x1_010
        ${EXTERNAL_SOURCE_DIR}/extern/YM2608-LLE
        ${EXTERNAL_SOURCE_DIR}/extern/YM3812-LLE
        ${EXTERNAL_SOURCE_DIR}/extern/YMF262-LLE
        ${EXTERNAL_SOURCE_DIR}/extern/YMF276-LLE
        ${EXTERNAL_SOURCE_DIR}/src
        ${EXTERNAL_SOURCE_DIR}/src/audio
        ${EXTERNAL_SOURCE_DIR}/src/baseutils
        ${EXTERNAL_SOURCE_DIR}/src/engine
        ${EXTERNAL_SOURCE_DIR}/src/engine/effect
        ${EXTERNAL_SOURCE_DIR}/src/engine/export
        ${EXTERNAL_SOURCE_DIR}/src/engine/fileOps
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform/sound
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform/sound/c64
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform/sound/c64_d
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform/sound/c64_fp
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform/sound/c64_fp/resample
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform/sound/dave
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform/sound/ga20
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform/sound/gb
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform/sound/lynx
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform/sound/nes
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform/sound/nes_nsfplay
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform/sound/oki
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform/sound/pokey
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform/sound/snes
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform/sound/t6w28
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform/sound/tia
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform/sound/ymf278b
        ${EXTERNAL_SOURCE_DIR}/src/engine/platform/sound/ymfm
        ${EXTERNAL_SOURCE_DIR}/src/icon

        ${CMAKE_SOURCE_DIR}/src/app
        ${FMOD_API_DIR})

if (WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE ENABLE_VIRTUAL_TERMINAL_PROCESSING=0x0004)
endif ()

target_link_libraries(${PROJECT_NAME} PRIVATE
        adpcm
        fmt
        ZLIB::ZLIB)

if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE -lshlwapi)
endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
)
