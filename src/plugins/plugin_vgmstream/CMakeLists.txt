cmake_minimum_required(VERSION 3.28)

set(NAME "vgmstream")
set(VERSION "r1831")
project(plugin_${NAME})
set(PLUGIN_${NAME}_DLL "${PROJECT_NAME}.dll" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "${NAME}" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(LIB_NAME "${NAME}")
set(LIB_VERSION "${VERSION}")
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION}")
set(LIB_FILENAME "${LIB_NAME_VERSIONED}.tar.gz")
set(LIB_FILENAME_URL "${LIB_VERSION}.tar.gz")
set(LIB_URL "https://github.com/${LIB_NAME}/${LIB_NAME}/archive/refs/tags/${LIB_FILENAME_URL}")
set(LIB_SHA_256_HASH "d3485abc1094f104244e8367b352fef7e17e7fc1e6ade19a78bbd09cbf3daea9")
download_patch_and_add(
        ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH} false "" ""
)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/src/*.c
        ${EXTERNAL_SOURCE_DIR}/src/coding/*.c
        ${EXTERNAL_SOURCE_DIR}/src/layout/*.c
        ${EXTERNAL_SOURCE_DIR}/src/meta/*.c
        ${EXTERNAL_SOURCE_DIR}/src/util/*.c
        src/main.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/src
        ${EXTERNAL_SOURCE_DIR}/src/coding
        ${EXTERNAL_SOURCE_DIR}/src/layout
        ${EXTERNAL_SOURCE_DIR}/src/meta
        ${EXTERNAL_SOURCE_DIR}/src/util

        ${CMAKE_SOURCE_DIR}/src/app
        ${EXTERNAL_SOURCE_DIR_fmod}/api/core/inc
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/data/plugin
        COMPILE_FLAGS "-m32"
        LINK_FLAGS "-m32"
)
