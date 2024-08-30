cmake_minimum_required(VERSION 3.28)

set(NAME "pac")
project(plugin_${NAME} VERSION "0.9.0")

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

set(LIB_NAME "libpac")
set(LIB_VERSION ${PROJECT_VERSION})
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION}")
set(LIB_FILENAME "${LIB_NAME_VERSIONED}.tgz")
set(LIB_URL "http://prdownloads.sourceforge.net/${LIB_NAME}/${LIB_FILENAME}")
set(LIB_SHA_256_HASH "17516a4a9fd75c4186c6d85a8b0d46cbff98fbc2042bccf0ce4062b6f68e3e49")
download_and_patch(
        ${LIB_NAME} ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH} ""
        ${CMAKE_CURRENT_SOURCE_DIR}/patches
)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/src/*.c
        src/main.cpp
)

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
