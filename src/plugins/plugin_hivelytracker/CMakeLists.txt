cmake_minimum_required(VERSION 3.28)

set(NAME "hivelytracker")
set(VERSION "1.9")
project(plugin_${NAME})
set(PLUGIN_${NAME}_DLL "${PROJECT_NAME}.dll" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "HivelyTracker" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION GREATER_EQUAL 10)
    add_compile_options($<$<COMPILE_LANGUAGE:C>:-fcommon>)
endif ()

if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION GREATER_EQUAL 14)
    add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-int-conversion>)
endif ()

set(LIB_NAME "${NAME}")
set(LIB_VERSION "${VERSION}")
string(REPLACE "." "_" LIB_TAG "${LIB_VERSION}")
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION}")
set(LIB_NAME_TAGGED "${LIB_NAME}-${LIB_TAG}")
set(LIB_FILENAME "${LIB_NAME_TAGGED}.zip")
set(LIB_URL "https://github.com/pete-gordon/${LIB_NAME}/archive/refs/tags/V${LIB_TAG}.zip")
set(LIB_SHA_256_HASH "22e5f98bf92502443510cacd9c3c1b8163063bf4108c215dce6d20a41d003e75")
set(LIB_UNPACKED_DIR "${LIB_NAME_TAGGED}")
download_patch_and_add(
        ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH} true ${LIB_UNPACKED_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/patches
)

add_compile_definitions(USER_PLUGINS_CONFIG_DIR="${USER_PLUGINS_CONFIG_DIR}")

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/Replayer_Windows/hvl_replay.c
        ${EXTERNAL_SOURCE_DIR}/Replayer_Windows/hvl_tables.c
        src/queue.c #TODO remove
        src/main.cpp

        ${CMAKE_SOURCE_DIR}/src/app/BaseRow.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/Replayer_Windows
        src  #TODO remove

        ${CMAKE_SOURCE_DIR}/src/app
        ${EXTERNAL_SOURCE_DIR_fmod}/api/core/inc
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DATA_PLUGINS_DIR}
)
