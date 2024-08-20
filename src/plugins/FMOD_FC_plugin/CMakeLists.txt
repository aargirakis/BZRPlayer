cmake_minimum_required(VERSION 3.28)

set(NAME "fc")
project(plugin_${NAME} VERSION "1.0.3")

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

set(LIB_NAME "libfc14audiodecoder")
set(LIB_VERSION ${PROJECT_VERSION})
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION}")
set(LIB_FILENAME "${LIB_NAME_VERSIONED}.tar.bz2")
set(LIB_URL "https://src.fedoraproject.org/repo/pkgs/${LIB_NAME}/${LIB_FILENAME}/4df18b3f7eac9ec4ff8e53f7c9a017c3/${LIB_FILENAME}")
set(LIB_SHA_256_HASH "0b8c87fe933158c63dfb6943f88f65be2b0c63c959fb30f0b2821388840168e6")
download_and_patch(
        ${LIB_NAME} ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH} ""
        ${CMAKE_CURRENT_SOURCE_DIR}/patches
)

set(SIZEOF_CHAR 1)
set(SIZEOF_INT 4)
set(SIZEOF_LONG_INT 4)
set(SIZEOF_SHORT_INT 2)
configure_file(
        ${EXTERNAL_SOURCE_DIR}/src/Config.h.in
        ${EXTERNAL_SOURCE_DIR}/src/Config.h
        @ONLY NEWLINE_STYLE UNIX
)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/src/fc14audiodecoder.cpp
        ${EXTERNAL_SOURCE_DIR}/src/FC.cpp
        ${EXTERNAL_SOURCE_DIR}/src/FC_Data.cpp
        ${EXTERNAL_SOURCE_DIR}/src/LamePaulaMixer.cpp
        ${EXTERNAL_SOURCE_DIR}/src/LamePaulaVoice.cpp
        ${EXTERNAL_SOURCE_DIR}/src/Paula.cpp
        src/main.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/src

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
