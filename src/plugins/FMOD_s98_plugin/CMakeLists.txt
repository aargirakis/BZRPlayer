cmake_minimum_required(VERSION 3.28)

set(NAME "s98")
project(plugin_${NAME} VERSION "1.2")

find_package(ZLIB)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 14)

if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION GREATER_EQUAL 14)
    add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-incompatible-pointer-types>)
endif ()

#TODO based on: https://www.purose.net/befis/download/kmp/kpis98-1.0r10s.7z

set(LIB_NAME "tunes98_plug")
set(LIB_VERSION ${PROJECT_VERSION})
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION}")
set(LIB_FILENAME "${LIB_NAME}.lha")
set(LIB_URL "https://www.a500.org/downloads/audio/tunenet/${LIB_FILENAME}")
set(LIB_SHA_256_HASH "b93958b4546dcc1609015ae724d5fc1267152635489ce23724629d0561fe3828")
set(LIB_UNPACKED_DIR "${LIB_NAME_VERSIONED}")
download_and_patch(
        ${LIB_NAME} ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH} ${LIB_UNPACKED_DIR} #TODO
        "" #"${CMAKE_CURRENT_SOURCE_DIR}/patches"
)

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

        ${CMAKE_SOURCE_DIR}/src/app
        ${EXTERNAL_SOURCE_DIR_fmod}/api/core/inc
)

target_link_libraries(${PROJECT_NAME} PRIVATE ZLIB::ZLIB)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "codec_${NAME}"
        PREFIX ""
        SUFFIX ".dll"
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR_PARENT}/data/plugin
        COMPILE_FLAGS "-m32"
        LINK_FLAGS "-m32"
)
