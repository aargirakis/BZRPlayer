cmake_minimum_required(VERSION 3.28)

set(NAME libxmp)
project(plugin_${NAME})

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

file(GLOB SOURCES
        src/libxmp-4.5.0/src/*.c
        src/libxmp-4.5.0/src/depackers/*.c
        src/libxmp-4.5.0/src/loaders/*.c
        src/libxmp-4.5.0/src/loaders/prowizard/*.c
        src/libxmp-4.5.0/src/win32/*.c
        src/main.cpp

        ${CMAKE_SOURCE_DIR}/src/plugins/FMOD_Flod_plugin/src/flod/BaseRow.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        src/libxmp-4.5.0/include
        src/libxmp-4.5.0/src
        src/libxmp-4.5.0/src/depackers
        src/libxmp-4.5.0/src/loaders
        src/libxmp-4.5.0/src/loaders/prowizard
        src/libxmp-4.5.0/src/win32
        src/libxmp-4.5.0/src/win32/sys

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
