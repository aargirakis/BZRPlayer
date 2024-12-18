cmake_minimum_required(VERSION 3.28)

set(NAME "sndh-player")
set(VERSION "0.74")
project(plugin_${NAME})
set(PLUGIN_${NAME}_DLL "${PROJECT_NAME}.dll" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "SNDH-Archive-Player" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(LIB_NAME "${NAME}")
set(LIB_VERSION "${VERSION}")
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION}")
set(LIB_FILENAME "${LIB_NAME_VERSIONED}.zip")
set(LIB_FILENAME_URL "v${LIB_VERSION}.zip")
set(LIB_URL "https://github.com/arnaud-carre/${LIB_NAME}/archive/refs/tags/${LIB_FILENAME_URL}")
set(LIB_SHA_256_HASH "e54d569c364b6040c85af75902d49b8c724743597e2aeeba357d6aa9e52fc438")
download_patch_and_add(
        ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH} false ""
        ${CMAKE_CURRENT_SOURCE_DIR}/patches
)

set(FILENAME "timedb.inc.h")
#TODO https://sourceforge.net/p/sc68/code/710/tree/file68/src/timedb.inc.h?format=raw better using the versioned url?
set(FILE_URL "https://sourceforge.net/p/sc68/code/HEAD/tree/file68/src/${FILENAME}?format=raw")
set(DESTINATION_PATH "${EXTERNAL_SOURCE_DIR}")
download_to(${FILENAME} ${FILE_URL} "" ${DESTINATION_PATH} true "")

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
        ${EXTERNAL_SOURCE_DIR_fmod}/api/core/inc
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/data/plugin
        COMPILE_FLAGS "-m32"
        LINK_FLAGS "-m32"
)
