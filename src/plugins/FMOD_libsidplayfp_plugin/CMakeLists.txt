cmake_minimum_required(VERSION 3.28)

set(NAME libsidplayfp)
project(plugin_${NAME})

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

file(GLOB SOURCES
        src/libsidplayfp-2.7.1/src/*.cpp
        src/libsidplayfp-2.7.1/src/builders/residfp-builder/*.cpp
        src/libsidplayfp-2.7.1/src/builders/residfp-builder/residfp/*.cpp
        src/libsidplayfp-2.7.1/src/builders/residfp-builder/residfp/resample/*.cpp
        src/libsidplayfp-2.7.1/src/c64/*.cpp
        src/libsidplayfp-2.7.1/src/c64/CIA/*.cpp
        src/libsidplayfp-2.7.1/src/c64/CPU/*.cpp
        src/libsidplayfp-2.7.1/src/c64/VIC_II/*.cpp
        src/libsidplayfp-2.7.1/src/sidplayfp/*.cpp
        src/libsidplayfp-2.7.1/src/sidtune/*.cpp
        src/libsidplayfp-2.7.1/src/utils/*.cpp
        src/libsidplayfp-2.7.1/src/utils/MD5/*.cpp
        src/libsidplayfp-2.7.1/src/utils/STILview/*.cpp
        src/sidid.c
        src/main.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        src
        src/libsidplayfp-2.7.1/src
        src/libsidplayfp-2.7.1/src/builders/residfp-builder
        src/libsidplayfp-2.7.1/src/builders/residfp-builder/residfp
        src/libsidplayfp-2.7.1/src/c64
        src/libsidplayfp-2.7.1/src/c64/Banks
        src/libsidplayfp-2.7.1/src/c64/CIA
        src/libsidplayfp-2.7.1/src/c64/CPU
        src/libsidplayfp-2.7.1/src/c64/VIC_II
        src/libsidplayfp-2.7.1/src/sidplayfp
        src/libsidplayfp-2.7.1/src/sidtune
        src/libsidplayfp-2.7.1/src/utils
        src/libsidplayfp-2.7.1/src/utils/MD5
        src/libsidplayfp-2.7.1/src/utils/STILview

        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/external/FMOD/api/core/inc
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "codec_${NAME}"
        PREFIX ""
        SUFFIX ".dll"
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR_PARENT}/data/plugin
        COMPILE_FLAGS "-m32"
        LINK_FLAGS "-m32"
)
