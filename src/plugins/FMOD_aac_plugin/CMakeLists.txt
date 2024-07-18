cmake_minimum_required(VERSION 3.28)

set(NAME aac)
project(plugin_${NAME} VERSION "2.10.0")

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

set(GIT_REPO_NAME "faad2")
set(GIT_REPO_URL "https://github.com/knik0/${GIT_REPO_NAME}.git")
string(REPLACE "." "_" GIT_CLONE_AT "${PROJECT_VERSION}")
set(GIT_IS_TAG true)

clone_and_patch(
        ${GIT_REPO_NAME} ${GIT_REPO_URL} ${GIT_CLONE_AT} ${GIT_IS_TAG}
        ""
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

        ${CMAKE_SOURCE_DIR}/src
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
