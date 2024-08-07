cmake_minimum_required(VERSION 3.28)

set(NAME "audiofile")
project(plugin_${NAME})

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")

set(GIT_REPO_NAME "${NAME}")
set(GIT_REPO_URL "https://github.com/mpruett/${GIT_REPO_NAME}.git")
set(GIT_CLONE_AT "d9363a5d16af4ce55eb35c5aad9ca19bb9c53cbe")
set(GIT_IS_TAG false)
clone_and_patch(
        ${GIT_REPO_NAME} ${GIT_REPO_URL} ${GIT_CLONE_AT} ${GIT_IS_TAG}
        libaudiofile/modules/UT_RebufferModule.cpp
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
