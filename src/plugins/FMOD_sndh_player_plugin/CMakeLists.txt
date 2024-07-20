cmake_minimum_required(VERSION 3.28)

set(NAME "sndh-player")
project(plugin_${NAME})

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

file(GLOB SOURCES
        src/AtariAudio/*.cpp
        src/AtariAudio/external/ice_24.c
        src/AtariAudio/external/Musashi/m68kcpu.c
        src/AtariAudio/external/Musashi/m68kops.c
        src/main.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        src/AtariAudio
        src/AtariAudio/external
        src/AtariAudio/external/Musashi

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
