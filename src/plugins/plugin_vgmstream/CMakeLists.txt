cmake_minimum_required(VERSION 3.28)

set(NAME "vgmstream")
set(VERSION "r2023")
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "${NAME}" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(NAME_VERSIONED "${NAME}-${VERSION}")
set(FILENAME "${NAME_VERSIONED}.tar.gz")
set(FILENAME_URL "${VERSION}.tar.gz")
set(URL "https://github.com/${NAME}/${NAME}/archive/refs/tags/${FILENAME_URL}")
set(SHA_256_HASH "4f4288f177cb12ecee7c2280a719044061195629947e08571e5c4484901b73bc")
download_patch_and_add(
        ${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} false "" ""
)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/src/*.c
        ${EXTERNAL_SOURCE_DIR}/src/base/*.c
        ${EXTERNAL_SOURCE_DIR}/src/coding/*.c
        ${EXTERNAL_SOURCE_DIR}/src/coding/libs/*.c
        ${EXTERNAL_SOURCE_DIR}/src/layout/*.c
        ${EXTERNAL_SOURCE_DIR}/src/meta/*.c
        ${EXTERNAL_SOURCE_DIR}/src/util/*.c
        src/main.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/src
        ${EXTERNAL_SOURCE_DIR}/src/base
        ${EXTERNAL_SOURCE_DIR}/src/coding
        ${EXTERNAL_SOURCE_DIR}/src/coding/libs
        ${EXTERNAL_SOURCE_DIR}/src/layout
        ${EXTERNAL_SOURCE_DIR}/src/meta
        ${EXTERNAL_SOURCE_DIR}/src/util

        ${CMAKE_SOURCE_DIR}/src/app
        ${FMOD_API_DIR})

target_compile_definitions(${PROJECT_NAME} PRIVATE _VORBIS_CUSTOM_DECODER_H_)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${DATA_DIR}/${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${DATA_DIR}/${PLUGINS_DIR}
)
