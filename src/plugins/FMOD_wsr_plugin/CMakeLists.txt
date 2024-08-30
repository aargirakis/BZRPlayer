cmake_minimum_required(VERSION 3.28)

set(NAME "wsr")
project(plugin_${NAME})

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION GREATER_EQUAL 10)
    add_compile_options($<$<COMPILE_LANGUAGE:C>:-fcommon>)
endif ()

set(LIB_NAME "audiodecoder.wsr")
set(LIB_VERSION "1.1.0")
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION}")
set(LIB_FILENAME "${LIB_NAME_VERSIONED}.tar.gz")
set(LIB_FILENAME2 "v${LIB_VERSION}.tar.gz")
set(LIB_URL "https://github.com/xbmc/${LIB_NAME}/archive/refs/tags/${LIB_FILENAME2}")
set(LIB_SHA_256_HASH "c84c49696939f950650ac05fbe6c0b9c6889003cea0cfb223d8045480ceb0a1a")
download_and_patch(
        ${LIB_NAME} ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH} ""
        ""
)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/lib/in_wsr/*.c
        ${EXTERNAL_SOURCE_DIR}/lib/in_wsr/nec/*.c
        src/main.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/lib/in_wsr

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
