cmake_minimum_required(VERSION 3.28)

set(NAME "sndh-player")
set(VERSION "0.79")
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "SNDH-Archive-Player" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(LIB_NAME "${NAME}")
set(LIB_VERSION "${VERSION}")
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION}")
set(LIB_FILENAME "${LIB_NAME_VERSIONED}.tar.gz")
set(LIB_FILENAME_URL "v${LIB_VERSION}.tar.gz")
set(LIB_URL "https://github.com/arnaud-carre/${LIB_NAME}/archive/refs/tags/${LIB_FILENAME_URL}")
set(LIB_SHA_256_HASH "c5e66cc2a724c8f4e88683c904d54805ff1c0e4b22cf3d45697e681cdcab6845")
download_patch_and_add(
        ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH} false ""
        ${CMAKE_CURRENT_SOURCE_DIR}/patches
)

set(TIMEDB_FILENAME "timedb.inc.h")
set(TIMEDB_VERSION "713")
set(TIMEDB_FILE_URL "https://sourceforge.net/p/sc68/code/${TIMEDB_VERSION}/tree/file68/src/${TIMEDB_FILENAME}?format=raw")
set(TIMEDB_DESTINATION_PATH "${EXTERNAL_SOURCE_DIR}")
download_to(${TIMEDB_FILENAME} ${TIMEDB_FILE_URL} "" ${TIMEDB_DESTINATION_PATH} true "")

add_compile_definitions(_strdup=strdup)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/AtariAudio/*.cpp
        ${EXTERNAL_SOURCE_DIR}/AtariAudio/external/ice_24.c
        ${EXTERNAL_SOURCE_DIR}/AtariAudio/external/Musashi/m68kcpu.c
        ${EXTERNAL_SOURCE_DIR}/AtariAudio/external/Musashi/m68kops.c
        src/main.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}
        ${EXTERNAL_SOURCE_DIR}/AtariAudio
        # ${EXTERNAL_SOURCE_DIR}/AtariAudio/external            #TODO remove?
        # ${EXTERNAL_SOURCE_DIR}/AtariAudio/external/Musashi    #TODO remove?

        ${CMAKE_SOURCE_DIR}/src/app
        ${LIB_FMOD_API_DIR}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${DATA_DIR}/${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${DATA_DIR}/${PLUGINS_DIR}
)
