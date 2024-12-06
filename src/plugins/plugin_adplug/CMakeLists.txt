cmake_minimum_required(VERSION 3.28)

set(NAME "adplug")
set(VERSION "2.3.3")
project(plugin_${NAME})
set(PLUGIN_${NAME}_DLL "${PROJECT_NAME}.dll" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "AdPlug" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(LIB1_NAME "${NAME}")
set(LIB1_VERSION "${VERSION}")
set(LIB1_NAME_VERSIONED "${LIB1_NAME}-${LIB1_VERSION}")
set(LIB1_FILENAME "${LIB1_NAME_VERSIONED}.tar.bz2")
set(LIB1_URL "https://github.com/${LIB1_NAME}/${LIB1_NAME}/releases/download/${LIB1_NAME_VERSIONED}/${LIB1_FILENAME}")
set(LIB1_SHA_256_HASH "a0f3c1b18fb49dea7ac3e8f820e091a663afa5410d3443612bf416cff29fa928")
download_patch_and_add(
        ${LIB1_NAME_VERSIONED} ${LIB1_FILENAME} ${LIB1_URL} ${LIB1_SHA_256_HASH} false "" ""
)

set(EXTERNAL_SOURCE_DIR_1 "${EXTERNAL_SOURCE_DIR}")

set(LIB2_NAME "libbinio")
set(LIB2_VERSION "1.5")
set(LIB2_NAME_VERSIONED "${LIB2_NAME}-${LIB2_VERSION}")
set(LIB2_NAME_VERSIONED2 "${LIB2_NAME}-${LIB2_NAME}-${LIB2_VERSION}")
set(LIB2_FILENAME "${LIB2_NAME_VERSIONED2}.zip")
set(LIB2_FILENAME_URL "${LIB2_NAME_VERSIONED}.zip")
set(LIB2_URL "https://github.com/${LIB1_NAME}/${LIB2_NAME}/archive/refs/tags/${LIB2_FILENAME_URL}")
set(LIB2_SHA_256_HASH "fd00c22d9ad3d4b5c90c4428d49719d0549f6a15cf5c612ffd5c9ba6c10689b9")
set(LIB2_UNPACKED_DIR "${LIB2_NAME_VERSIONED2}")
download_patch_and_add(
        ${LIB2_NAME_VERSIONED} ${LIB2_FILENAME} ${LIB2_URL} ${LIB2_SHA_256_HASH} true ${LIB2_UNPACKED_DIR} ""
)

set(EXTERNAL_SOURCE_DIR_2 "${EXTERNAL_SOURCE_DIR}")

set(VERSION "${PROJECT_VERSION}")
configure_file(
        ${EXTERNAL_SOURCE_DIR_1}/src/version.h.in
        ${EXTERNAL_SOURCE_DIR_1}/src/version.h
        @ONLY NEWLINE_STYLE UNIX
)

set(ENABLE_STRING 1)
set(ENABLE_IOSTREAM 1)
set(ISO_STDLIB 1)
set(WITH_MATH 1)
set(TYPE_INT "long long")
set(TYPE_FLOAT "long double")
configure_file(
        ${EXTERNAL_SOURCE_DIR_2}/src/binio.h.in
        ${EXTERNAL_SOURCE_DIR_2}/src/binio.h
        @ONLY NEWLINE_STYLE UNIX
)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR_1}/src/*.c
        ${EXTERNAL_SOURCE_DIR_1}/src/*.cpp
        ${EXTERNAL_SOURCE_DIR_2}/src/*.cpp
        src/main.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR_1}/src
        ${EXTERNAL_SOURCE_DIR_2}/src

        ${CMAKE_SOURCE_DIR}/src/app
        ${EXTERNAL_SOURCE_DIR_fmod}/api/core/inc
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DATA_PLUGINS_DIR}
        COMPILE_FLAGS "-m32"
        LINK_FLAGS "-m32"
)
