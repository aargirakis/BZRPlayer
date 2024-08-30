cmake_minimum_required(VERSION 3.28)

set(NAME "v2m")
project(plugin_${NAME})

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

set(LIB_NAME "v2m-player")
set(LIB_VERSION "c797efd8356c0fe56420314f3683c2255cfc44d1")
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION}")
set(LIB_FILENAME "${LIB_NAME}-${LIB_VERSION}.zip")
set(LIB_FILENAME2 "${LIB_VERSION}.zip")
set(LIB_URL "https://github.com/jgilje/${LIB_NAME}/archive/${LIB_FILENAME2}")
set(LIB_SHA_256_HASH "a6ec966bc0fd95e1da43babe34aeff45f0b511664e51100907b5c25aae67b4d3")
download_and_patch(
        ${LIB_NAME} ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH} ""
        ${CMAKE_CURRENT_SOURCE_DIR}/patches
)

add_compile_definitions(V2MPLAYER_SYNC_FUNCTIONS)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/src/*.cpp
        src/main.cpp
)

list(REMOVE_ITEM SOURCES ${EXTERNAL_SOURCE_DIR}/src/tinyplayer.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/src

        ${CMAKE_SOURCE_DIR}/src/app
        ${EXTERNAL_SOURCE_DIR_fmod}/api/core/inc
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "codec_${NAME}"
        PREFIX ""
        SUFFIX ".dll"
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR_PARENT}/data/plugin
        COMPILE_FLAGS "-m32"
        LINK_FLAGS "-m32"
)
