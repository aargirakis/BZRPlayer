cmake_minimum_required(VERSION 3.28)

set(NAME "sndh-player")
set(VERSION "1.04")
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "SNDH-Archive-Player" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(NAME_VERSIONED "${NAME}-${VERSION}")
set(TAG "AtariAudio_${VERSION}")
set(NAME_TAGGED "${NAME}-${TAG}")
set(FILENAME "${NAME_TAGGED}.tar.gz")
set(FILENAME_URL "${TAG}.tar.gz")
set(URL "https://github.com/arnaud-carre/${NAME}/archive/refs/tags/${FILENAME_URL}")
set(SHA_256_HASH "d2bc90e039aa4a5ed0d73e19cc39ab711848a9d61c17db866b8d1a12d839990a")
set(UNPACKED_DIR "${NAME_TAGGED}")
download_patch_and_add(${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} true ${UNPACKED_DIR} "")

set(TIMEDB_FILENAME "timedb.inc.h")
set(TIMEDB_VERSION "713")
set(TIMEDB_FILE_URL "https://sourceforge.net/p/sc68/code/${TIMEDB_VERSION}/tree/file68/src/${TIMEDB_FILENAME}?format=raw")
set(TIMEDB_DESTINATION_PATH "${EXTERNAL_SOURCE_DIR}")
download_to(${TIMEDB_FILENAME} ${TIMEDB_FILE_URL} "" ${TIMEDB_DESTINATION_PATH} true "")

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/AtariAudio/*.cpp
        ${EXTERNAL_SOURCE_DIR}/AtariAudio/external/ice_24.c
        ${EXTERNAL_SOURCE_DIR}/AtariAudio/external/Musashi/m68kcpu.c
        ${EXTERNAL_SOURCE_DIR}/AtariAudio/external/Musashi/m68kops.c
        src/main.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}
        ${EXTERNAL_SOURCE_DIR}/AtariAudio

        ${CMAKE_SOURCE_DIR}/src/app
        ${LOGGER_API_DIR}
        ${FMOD_API_DIR})

set(CONFIG_FILENAME "${PROJECT_NAME}.cfg")
set(PLUGIN_${NAME}_CONFIG_FILENAME "${CONFIG_FILENAME}" PARENT_SCOPE)
target_compile_definitions(${PROJECT_NAME} PRIVATE CONFIG_FILENAME="${CONFIG_FILENAME}")
target_compile_definitions(${PROJECT_NAME} PRIVATE PLUGIN_CONFIGS_DIR="${PLUGIN_CONFIGS_DIR}")
target_compile_definitions(${PROJECT_NAME} PRIVATE _strdup=strdup)

target_link_libraries(${PROJECT_NAME} PRIVATE logger)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_PATH}/${CMAKE_INSTALL_LIBDIR}${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_PATH}${DATA_DIR}${PLUGINS_DIR})

if (NOT WIN32)
    install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}${PLUGINS_DIR})
endif ()