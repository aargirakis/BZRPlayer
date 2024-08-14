cmake_minimum_required(VERSION 3.28)

set(NAME "libxmp")
project(plugin_${NAME} VERSION "4.5.0")

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

set(LIB_NAME "${NAME}")
set(LIB_VERSION ${PROJECT_VERSION})
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION}")
set(LIB_FILENAME "${LIB_NAME_VERSIONED}.tar.gz")
set(LIB_URL "https://github.com/${LIB_NAME}/${LIB_NAME}/releases/download/${LIB_NAME_VERSIONED}/${LIB_FILENAME}")
set(LIB_SHA_256_HASH "7847d262112d14e8442f44e5ac6ed9ddbca54c251284720b563c852b31f26e75")
download_and_patch(
        ${LIB_NAME} ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH}
        ${CMAKE_CURRENT_SOURCE_DIR}/patches
)

add_compile_definitions(BUILDING_DLL=1)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/src/*.c
        ${EXTERNAL_SOURCE_DIR}/src/depackers/*.c
        ${EXTERNAL_SOURCE_DIR}/src/loaders/*.c
        ${EXTERNAL_SOURCE_DIR}/src/loaders/prowizard/*.c
        ${EXTERNAL_SOURCE_DIR}/src/win32/*.c
        src/main.cpp

        ${CMAKE_SOURCE_DIR}/src/plugins/FMOD_Flod_plugin/src/flod/BaseRow.cpp
)

list(REMOVE_ITEM SOURCES
        ${EXTERNAL_SOURCE_DIR}/src/loaders/prowizard/pm.c
        ${EXTERNAL_SOURCE_DIR}/src/loaders/prowizard/pm20.c
        ${EXTERNAL_SOURCE_DIR}/src/loaders/prowizard/pm40.c
        ${EXTERNAL_SOURCE_DIR}/src/loaders/prowizard/pp30.c
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/include
        ${EXTERNAL_SOURCE_DIR}/src
        ${EXTERNAL_SOURCE_DIR}/src/depackers
        ${EXTERNAL_SOURCE_DIR}/src/loaders
        ${EXTERNAL_SOURCE_DIR}/src/loaders/prowizard
        ${EXTERNAL_SOURCE_DIR}/src/win32
        ${EXTERNAL_SOURCE_DIR}/src/win32/sys

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
