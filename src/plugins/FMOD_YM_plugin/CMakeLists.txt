cmake_minimum_required(VERSION 3.28)

set(NAME "ym")
project(plugin_${NAME})

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

set(LIB_NAME "libstsound")
set(LIB_VERSION "ecae51265621ed841c6c0d7b22a4ada104fa3322")
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION}")
set(LIB_FILENAME "${LIB_NAME_VERSIONED}.zip")
set(LIB_FILENAME2 "${LIB_VERSION}.tar.gz")
set(LIB_URL "https://github.com/cpcsdk/${LIB_NAME}/archive/${LIB_FILENAME2}")
set(LIB_SHA_256_HASH "97aed95140622236c8d596b7b45313f9f14ffe0d1f47b8de8496eba2802ef0d6")
download_and_patch(
        ${LIB_NAME} ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH} ""
        ""
)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/*.cpp
        ${EXTERNAL_SOURCE_DIR}/LZH/LzhLib.cpp
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
