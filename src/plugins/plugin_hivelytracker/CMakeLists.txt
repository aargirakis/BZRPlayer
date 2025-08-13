cmake_minimum_required(VERSION 3.28)

set(NAME "hivelytracker")
set(VERSION "f393ca7c6416f00bcb574b334a7e8b57dcb19eb2")
string(SUBSTRING ${VERSION} 0 12 VERSION_SHORT)
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "HivelyTracker" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION_SHORT}" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(NAME_VERSIONED "${NAME}-${VERSION_SHORT}")
set(FILENAME "${NAME}-${VERSION}.tar.gz")
set(FILENAME_URL "${VERSION_SHORT}.tar.gz")
set(URL "https://github.com/pete-gordon/${NAME}/archive/${FILENAME_URL}")
set(SHA_256_HASH "523de29ec7da41cd1e308e08297ec18f5545cc59d6e90cc61b0015ad01d5baba")
set(UNPACKED_DIR "${NAME}-${VERSION}")
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

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
)
