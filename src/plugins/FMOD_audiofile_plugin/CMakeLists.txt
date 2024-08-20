cmake_minimum_required(VERSION 3.28)

set(NAME "audiofile")
project(plugin_${NAME})

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")

set(LIB_NAME "${NAME}")
set(LIB_VERSION "d9363a5d16af4ce55eb35c5aad9ca19bb9c53cbe")
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION}")
set(LIB_FILENAME "${LIB_VERSION}.zip")
set(LIB_URL "https://github.com/mpruett/${LIB_NAME}/archive/${LIB_FILENAME}")
set(LIB_SHA_256_HASH "83835f1705a0cec378c1c4fc5d7d6385737be6dcfbc480049bd82e3657069194")
download_and_patch(
        ${LIB_NAME} ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH} ""
        ${CMAKE_CURRENT_SOURCE_DIR}/patches
)

configure_file(
        src/config.h.in
        ${EXTERNAL_SOURCE_DIR}/libaudiofile/config.h
        @ONLY NEWLINE_STYLE UNIX
)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/libaudiofile/*.c
        ${EXTERNAL_SOURCE_DIR}/libaudiofile/*.cpp
        ${EXTERNAL_SOURCE_DIR}/libaudiofile/alac/*.c
        ${EXTERNAL_SOURCE_DIR}/libaudiofile/alac/*.cpp
        ${EXTERNAL_SOURCE_DIR}/libaudiofile/modules/*.cpp
        src/main.cpp
)

list(REMOVE_ITEM SOURCES ${EXTERNAL_SOURCE_DIR}/libaudiofile/modules/UT_RebufferModule.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/libaudiofile
        ${EXTERNAL_SOURCE_DIR}/libaudiofile/alac
        ${EXTERNAL_SOURCE_DIR}/libaudiofile/modules

        ${CMAKE_SOURCE_DIR}/src/app
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
