cmake_minimum_required(VERSION 3.28)
project(plugin_aac)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE PLUGIN_SOURCES src/faad2-2_10_0/libfaad/*.c)

add_library(${PROJECT_NAME} SHARED
        ${PLUGIN_SOURCES}
        src/main.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
        src/faad2-2_10_0/include
        src/faad2-2_10_0/libfaad

        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/external/FMOD/api/core/inc
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "codec_aac"
        PREFIX ""
        SUFFIX ".dll"
        #LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR_PARENT}/data/plugin
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR_PARENT}/data/plugin
        COMPILE_FLAGS "-m32"
        LINK_FLAGS "-m32"
)
