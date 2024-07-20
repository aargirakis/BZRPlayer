cmake_minimum_required(VERSION 3.28)

set(NAME "s98")
project(plugin_${NAME})

set(CMAKE_C_STANDARD 17)

if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(CMAKE_CXX_STANDARD 17)
else ()
    set(CMAKE_CXX_STANDARD 14)
endif ()

file(GLOB SOURCES
        src/s98/*.cpp
        src/s98/device/*.c
        src/s98/device/*.cpp
        src/s98/device/emu2413/*.c
        src/s98/device/fmgen/*.cpp
        src/main.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        src/s98
        src/s98/device
        src/s98/device/emu2413
        src/s98/device/fmgen

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
