cmake_minimum_required(VERSION 3.28)

set(NAME audiofile)
project(plugin_${NAME})

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")

file(GLOB SOURCES
        src/libaudiofile0.3.6/*.c
        src/libaudiofile0.3.6/*.cpp
        src/libaudiofile0.3.6/alac/*.c
        src/libaudiofile0.3.6/alac/*.cpp
        src/libaudiofile0.3.6/modules/*.cpp
        src/main.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        src/libaudiofile0.3.6
        src/libaudiofile0.3.6/alac
        src/libaudiofile0.3.6/modules

        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/external/FMOD/api/core/inc
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "codec_${NAME}"
        PREFIX ""
        SUFFIX ".dll"
        #LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR_PARENT}/data/plugin
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR_PARENT}/data/plugin
        COMPILE_FLAGS "-m32"
        LINK_FLAGS "-m32"
)
