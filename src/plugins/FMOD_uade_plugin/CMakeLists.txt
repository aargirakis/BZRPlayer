cmake_minimum_required(VERSION 3.28)

set(NAME uade)
project(plugin_${NAME})

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O3") #TODO why dll linking on windows with debug build type fails with (default) -O0 flag ?
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fcommon") #TODO why needed for linux cross-compilation only?

file(GLOB SOURCES
        src/blazer.c
        src/flod/*.cpp
        src/md5/*.c
        src/uade-2.13-1.01/compat/memmemrep.c
        src/uade-2.13-1.01/compat/strlrep.c
        src/uade-2.13-1.01/src/audio.c
        src/uade-2.13-1.01/src/cfgfile.c
        src/uade-2.13-1.01/src/cia.c
        src/uade-2.13-1.01/src/compiler.c
        src/uade-2.13-1.01/src/cpudefs.c
        src/uade-2.13-1.01/src/cpuemu1.c
        src/uade-2.13-1.01/src/cpuemu2.c
        src/uade-2.13-1.01/src/cpuemu3.c
        src/uade-2.13-1.01/src/cpuemu4.c
        src/uade-2.13-1.01/src/cpuemu5.c
        src/uade-2.13-1.01/src/cpuemu6.c
        src/uade-2.13-1.01/src/cpuemu7.c
        src/uade-2.13-1.01/src/cpuemu8.c
        src/uade-2.13-1.01/src/cpustbl.c
        src/uade-2.13-1.01/src/custom.c
        src/uade-2.13-1.01/src/fpp.c
        src/uade-2.13-1.01/src/frontends/common/amifilemagic.c
        src/uade-2.13-1.01/src/frontends/common/eagleplayer.c
        src/uade-2.13-1.01/src/frontends/common/effects.c
        src/uade-2.13-1.01/src/frontends/common/songdb.c
        src/uade-2.13-1.01/src/frontends/common/songinfo.c
        src/uade-2.13-1.01/src/frontends/common/support.c
        src/uade-2.13-1.01/src/frontends/common/uadeconf.c
        src/uade-2.13-1.01/src/md-support.c
        src/uade-2.13-1.01/src/memory.c
        src/uade-2.13-1.01/src/missing.c
        src/uade-2.13-1.01/src/newcpu.c
        src/uade-2.13-1.01/src/ossupport.c
        src/uade-2.13-1.01/src/readcpu.c
        src/uade-2.13-1.01/src/sd-sound.c
        src/uade-2.13-1.01/src/sinctable.c
        src/uade-2.13-1.01/src/text_scope.c
        src/uade-2.13-1.01/src/uade.c
        src/uade-2.13-1.01/src/uademain.c
        src/uade-2.13-1.01/src/unixatomic.c
        src/*.c
        src/main.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        src
        src/flod
        src/md5
        src/uade-2.13-1.01/compat
        src/uade-2.13-1.01/src
        src/uade-2.13-1.01/src/frontends/common
        src/uade-2.13-1.01/src/include
        src/uade-2.13-1.01/src/machdep

        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/external/FMOD/api/core/inc
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        -L${CMAKE_SOURCE_DIR}/src/external/FMOD/api/core/lib/x86 -lfmod -lws2_32
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "codec_${NAME}"
        PREFIX ""
        SUFFIX ".dll"
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR_PARENT}/data/plugin
        COMPILE_FLAGS "-m32"
        LINK_FLAGS "-m32"
)
