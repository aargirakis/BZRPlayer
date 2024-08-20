cmake_minimum_required(VERSION 3.28)

set(NAME "asap")
project(plugin_${NAME} VERSION "5.2.0")

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

set(LIB_NAME "${NAME}")
set(LIB_VERSION ${PROJECT_VERSION})
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION}")
set(LIB_FILENAME "${LIB_NAME_VERSIONED}.tar.gz")
set(LIB_URL "https://sourceforge.net/projects/${LIB_NAME}/files/${LIB_NAME}/${LIB_VERSION}/${LIB_FILENAME}/download")
set(LIB_SHA_256_HASH "8c7bff24d14068fcf1857710c9eb331e9d7ca1b74e718705228e9da9eac8e6ac")
download_and_patch(
        ${LIB_NAME} ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH} ""
        ""
)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/asap.c
        src/main.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}

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
