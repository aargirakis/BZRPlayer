cmake_minimum_required(VERSION 3.28)

set(NAME aac)
project(plugin_${NAME} VERSION "2.10.0")

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

set(LIB_NAME "faad2")
string(REPLACE "." "_" LIB_VERSION "${PROJECT_VERSION}")
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION}")
set(LIB_FILENAME "${LIB_NAME_VERSIONED}.zip")
set(LIB_URL "https://github.com/knik0/${LIB_NAME}/archive/refs/tags/${LIB_VERSION}.zip")
set(LIB_SHA_256_HASH "841841dd2ccd8cac295b15420ad20cc69ca4951a3de642ed2bdba8d634bfcf8a")
download_and_patch(
        ${LIB_NAME} ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH}
        ""
)

add_compile_definitions(HAVE_CONFIG_H=1)
set(HAVE_MEMCPY 1)
configure_file(
        src/config.h.in
        ${EXTERNAL_SOURCE_DIR}/config.h
        @ONLY NEWLINE_STYLE UNIX
)

configure_file(
        src/win32_ver.h.in
        ${EXTERNAL_SOURCE_DIR}/libfaad/win32_ver.h
        @ONLY NEWLINE_STYLE UNIX
)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/libfaad/*.c
        src/main.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/include
        ${EXTERNAL_SOURCE_DIR}/libfaad

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
