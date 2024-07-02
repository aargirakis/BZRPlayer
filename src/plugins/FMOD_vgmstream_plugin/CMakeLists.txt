cmake_minimum_required(VERSION 3.28)

set(NAME vgmstream)
project(plugin_${NAME})

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

file(GLOB SOURCES
        src/vgmstream-r1831/*.c
        src/vgmstream-r1831/coding/*.c
        src/vgmstream-r1831/layout/*.c
        src/vgmstream-r1831/meta/*.c
        src/vgmstream-r1831/util/*.c
        src/main.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        src/vgmstream-r1831
        src/vgmstream-r1831/coding
        src/vgmstream-r1831/layout
        src/vgmstream-r1831/meta
        src/vgmstream-r1831/util

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
