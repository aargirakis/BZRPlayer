cmake_minimum_required(VERSION 3.28)

set(NAME "hivelytracker")
set(VERSION "1.9")
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "HivelyTracker" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

string(REPLACE "." "_" TAG "${VERSION}")
set(NAME_VERSIONED "${NAME}-${VERSION}")
set(NAME_TAGGED "${NAME}-${TAG}")
set(FILENAME "${NAME_TAGGED}.zip")
set(URL "https://github.com/pete-gordon/${NAME}/archive/refs/tags/V${TAG}.zip")
set(SHA_256_HASH "22e5f98bf92502443510cacd9c3c1b8163063bf4108c215dce6d20a41d003e75")
set(UNPACKED_DIR "${NAME_TAGGED}")
download_patch_and_add(
        ${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} true ${UNPACKED_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/patches
)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/Replayer_Windows/hvl_replay.c
        ${EXTERNAL_SOURCE_DIR}/Replayer_Windows/hvl_tables.c
        src/queue.c
        src/main.cpp

        ${CMAKE_SOURCE_DIR}/src/app/BaseRow.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/Replayer_Windows
        src

        ${CMAKE_SOURCE_DIR}/src/app
        ${FMOD_API_DIR})

target_compile_definitions(${PROJECT_NAME} PRIVATE PLUGINS_CONFIG_DIR="${PLUGINS_CONFIG_DIR}")

target_compile_options(${PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:C>:-fcommon>)
target_compile_options(${PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:C>:-Wno-int-conversion>)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${DATA_DIR}/${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${DATA_DIR}/${PLUGINS_DIR}
)
