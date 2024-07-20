cmake_minimum_required(VERSION 3.28)

set(NAME "furnace")
project(plugin_${NAME})

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

file(GLOB SOURCES
        src/furnace-0.6.3/extern/adpcm-xq-s/*.c
        src/furnace-0.6.3/extern/adpcm/*.c
        src/furnace-0.6.3/extern/emu2413/*.c
        src/furnace-0.6.3/extern/ESFMu/*.c
        src/furnace-0.6.3/extern/fmt/src/format.cc
        src/furnace-0.6.3/extern/fmt/src/os.cc
        src/furnace-0.6.3/extern/Nuked-OPLL/*.c
        src/furnace-0.6.3/extern/Nuked-PSG/*.c
        src/furnace-0.6.3/extern/opl/*.c
        src/furnace-0.6.3/extern/opm/*.c
        src/furnace-0.6.3/extern/opn/*.c
        src/furnace-0.6.3/extern/pwrnoise/*.c
        src/furnace-0.6.3/extern/SAASound/src/*.cpp
        src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/core/vox/*.cpp
        src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/es550x/*.cpp
        src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/k005289/*.cpp
        src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/k007232/*.cpp
        src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/k053260/*.cpp
        src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/msm6295/*.cpp
        src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/n163/*.cpp
        src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/scc/*.cpp
        src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/vrcvi/*.cpp
        src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/x1_010/*.cpp
        src/furnace-0.6.3/extern/YM2608-LLE/fmopna_2608.c
        src/furnace-0.6.3/extern/YM2608-LLE/fmopna_2610.c
        src/furnace-0.6.3/extern/YM2608-LLE/fmopna_2612.c
        src/furnace-0.6.3/extern/YM3812-LLE/*.c
        src/furnace-0.6.3/extern/YMF262-LLE/*.c
        src/furnace-0.6.3/extern/YMF276-LLE/*.c
        src/furnace-0.6.3/src/audio/abstract-2.cpp
        src/furnace-0.6.3/src/audio/midi.cpp
        src/furnace-0.6.3/src/audio/pipe.cpp
        src/furnace-0.6.3/src/baseutils.cpp
        src/furnace-0.6.3/src/engine/*.c
        src/furnace-0.6.3/src/engine/cmdStream.cpp
        src/furnace-0.6.3/src/engine/cmdStreamOps.cpp
        src/furnace-0.6.3/src/engine/config.cpp
        src/furnace-0.6.3/src/engine/configEngine.cpp
        src/furnace-0.6.3/src/engine/dispatchContainer.cpp
        src/furnace-0.6.3/src/engine/effect/*.cpp
        src/furnace-0.6.3/src/engine/engine.cpp
        src/furnace-0.6.3/src/engine/export.cpp
        src/furnace-0.6.3/src/engine/export/*.cpp
        src/furnace-0.6.3/src/engine/fileOps/*.cpp
        src/furnace-0.6.3/src/engine/fileOpsIns.cpp
        src/furnace-0.6.3/src/engine/fileOpsSample.cpp
        src/furnace-0.6.3/src/engine/filter-4.cpp
        src/furnace-0.6.3/src/engine/instrument.cpp
        src/furnace-0.6.3/src/engine/macroInt.cpp
        src/furnace-0.6.3/src/engine/pattern.cpp
        src/furnace-0.6.3/src/engine/pitchTable.cpp
        src/furnace-0.6.3/src/engine/platform/*.cpp
        src/furnace-0.6.3/src/engine/platform/sound/*.c
        src/furnace-0.6.3/src/engine/platform/sound/*.cpp
        src/furnace-0.6.3/src/engine/platform/sound/atomicssg/*.c
        src/furnace-0.6.3/src/engine/platform/sound/c64/*.cc
        src/furnace-0.6.3/src/engine/platform/sound/c64_d/*.c
        src/furnace-0.6.3/src/engine/platform/sound/c64_fp/*.cpp
        src/furnace-0.6.3/src/engine/platform/sound/c64_fp/resample/SincResampler.cpp
        src/furnace-0.6.3/src/engine/platform/sound/dave/*.cpp
        src/furnace-0.6.3/src/engine/platform/sound/ga20/*.cpp
        src/furnace-0.6.3/src/engine/platform/sound/gb/*.c
        src/furnace-0.6.3/src/engine/platform/sound/lynx/*.cpp
        src/furnace-0.6.3/src/engine/platform/sound/nes/*.c
        src/furnace-0.6.3/src/engine/platform/sound/nes_nsfplay/*.cpp
        src/furnace-0.6.3/src/engine/platform/sound/oki/*.cpp
        src/furnace-0.6.3/src/engine/platform/sound/pokey/*.c
        src/furnace-0.6.3/src/engine/platform/sound/pokey/*.cpp
        src/furnace-0.6.3/src/engine/platform/sound/sid2/*.cc
        src/furnace-0.6.3/src/engine/platform/sound/snes/*.cpp
        src/furnace-0.6.3/src/engine/platform/sound/t6w28/*.cpp
        src/furnace-0.6.3/src/engine/platform/sound/tia/*.cpp
        src/furnace-0.6.3/src/engine/platform/sound/ymfm/*.cpp
        src/furnace-0.6.3/src/engine/playback.cpp
        src/furnace-0.6.3/src/engine/safeReader.cpp
        src/furnace-0.6.3/src/engine/safeWriter.cpp
        src/furnace-0.6.3/src/engine/sample.cpp
        src/furnace-0.6.3/src/engine/song.cpp
        src/furnace-0.6.3/src/engine/sysDef.cpp
        src/furnace-0.6.3/src/engine/vgmOps.cpp
        src/furnace-0.6.3/src/engine/waveSynth.cpp
        src/furnace-0.6.3/src/engine/wavetable.cpp
        src/furnace-0.6.3/src/engine/wavOps.cpp
        src/furnace-0.6.3/src/engine/winStuff.cpp
        src/furnace-0.6.3/src/engine/workPool.cpp
        src/furnace-0.6.3/src/engine/zsm.cpp
        src/furnace-0.6.3/src/engine/zsmOps.cpp
        src/furnace-0.6.3/src/fileutils.cpp
        src/furnace-0.6.3/src/log.cpp
        src/furnace-0.6.3/src/utfutils.cpp
        src/main.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        src/furnace-0.6.3/extern/adpcm
        src/furnace-0.6.3/extern/adpcm-xq-s
        src/furnace-0.6.3/extern/emu2413
        src/furnace-0.6.3/extern/ESFMu
        src/furnace-0.6.3/extern/fmt/include
        src/furnace-0.6.3/extern/IconFontCppHeaders
        src/furnace-0.6.3/extern/Nuked-OPLL
        src/furnace-0.6.3/extern/Nuked-PSG
        src/furnace-0.6.3/extern/opl
        src/furnace-0.6.3/extern/opm
        src/furnace-0.6.3/extern/opn
        src/furnace-0.6.3/extern/pwrnoise
        src/furnace-0.6.3/extern/SAASound/src
        src/furnace-0.6.3/extern/vgsound_emu-modified
        src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu
        src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/core
        src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/core/e550x
        src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/core/k005289
        src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/core/k007232
        src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/core/k053260
        src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/core/msm6295
        src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/core/n163
        src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/core/scc
        src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/core/template
        src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/core/util
        src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/core/vox
        src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/core/vrcvi
        src/furnace-0.6.3/extern/vgsound_emu-modified/vgsound_emu/src/core/x1_010
        src/furnace-0.6.3/extern/YM2608-LLE
        src/furnace-0.6.3/extern/YM3812-LLE
        src/furnace-0.6.3/extern/YMF262-LLE
        src/furnace-0.6.3/extern/YMF276-LLE
        src/furnace-0.6.3/src
        src/furnace-0.6.3/src/audio
        src/furnace-0.6.3/src/baseutils
        src/furnace-0.6.3/src/engine
        src/furnace-0.6.3/src/engine/effect
        src/furnace-0.6.3/src/engine/export
        src/furnace-0.6.3/src/engine/fileOps
        src/furnace-0.6.3/src/engine/platform
        src/furnace-0.6.3/src/engine/platform/sound
        src/furnace-0.6.3/src/engine/platform/sound/c64
        src/furnace-0.6.3/src/engine/platform/sound/c64_d
        src/furnace-0.6.3/src/engine/platform/sound/c64_fp
        src/furnace-0.6.3/src/engine/platform/sound/c64_fp/resample
        src/furnace-0.6.3/src/engine/platform/sound/dave
        src/furnace-0.6.3/src/engine/platform/sound/ga20
        src/furnace-0.6.3/src/engine/platform/sound/gb
        src/furnace-0.6.3/src/engine/platform/sound/lynx
        src/furnace-0.6.3/src/engine/platform/sound/nes
        src/furnace-0.6.3/src/engine/platform/sound/nes_nsfplay
        src/furnace-0.6.3/src/engine/platform/sound/oki
        src/furnace-0.6.3/src/engine/platform/sound/pokey
        src/furnace-0.6.3/src/engine/platform/sound/snes
        src/furnace-0.6.3/src/engine/platform/sound/t6w28
        src/furnace-0.6.3/src/engine/platform/sound/tia
        src/furnace-0.6.3/src/engine/platform/sound/ymfm
        src/furnace-0.6.3/src/icon

        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/external/FMOD/api/core/inc
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        -L${CMAKE_SOURCE_DIR}/src/external/zlib -lzdll -lshlwapi
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "codec_${NAME}"
        PREFIX ""
        SUFFIX ".dll"
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR_PARENT}/data/plugin
        COMPILE_FLAGS "-m32"
        LINK_FLAGS "-m32"
)
