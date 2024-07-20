cmake_minimum_required(VERSION 3.28)

set(NAME "sc68")
project(plugin_${NAME})

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

file(GLOB SOURCES
        src/sc68/api68/*.c
        src/sc68/emu68/*.c
        src/sc68/file68/*.c
        src/sc68/io68/*.c
        src/sc68/unice68/*.c
        src/main.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        src/sc68
        src/sc68/api68
        src/sc68/emu68
        src/sc68/file68
        src/sc68/io68
        src/sc68/unice68

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
