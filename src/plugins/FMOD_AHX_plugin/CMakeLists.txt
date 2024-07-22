cmake_minimum_required(VERSION 3.28)

set(NAME "ahx")
project(plugin_${NAME} VERSION "1.9")

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fcommon") #TODO why needed for linux cross-compilation only?

set(GIT_REPO_NAME "hivelytracker")
set(GIT_REPO_URL "https://github.com/pete-gordon/${GIT_REPO_NAME}.git")
string(REPLACE "." "_" GIT_CLONE_AT "V${PROJECT_VERSION}")
set(GIT_IS_TAG true)
clone_and_patch(
        ${GIT_REPO_NAME} ${GIT_REPO_URL} ${GIT_CLONE_AT} ${GIT_IS_TAG}
        ""
        ${CMAKE_CURRENT_SOURCE_DIR}/patches
)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/Replayer_Windows/hvl_replay.c
        ${EXTERNAL_SOURCE_DIR}/Replayer_Windows/hvl_tables.c
        src/queue.c
        src/main.cpp

        ${CMAKE_SOURCE_DIR}/src/plugins/FMOD_Flod_plugin/src/flod/BaseRow.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/Replayer_Windows
        src

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
