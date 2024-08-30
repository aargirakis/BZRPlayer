cmake_minimum_required(VERSION 3.28)

set(NAME "protrekkr")
project(plugin_${NAME})

find_package(ZLIB)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 14)

add_compile_definitions(__REPLAYER__ __STAND_ALONE__ __WINAMP__)

file(GLOB SOURCES
        src/protrekkr/editors/patterns_blocks.cpp
        src/protrekkr/files/*.cpp
        src/protrekkr/replay/*.cpp
        src/main.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        src/protrekkr
        src/protrekkr/editors/include
        src/protrekkr/files
        src/protrekkr/include
        src/protrekkr/replay/include
        src/protrekkr/replay/samples/aiff/include
        src/protrekkr/replay/samples/wav/include

        ${CMAKE_SOURCE_DIR}/src/app
        ${EXTERNAL_SOURCE_DIR_fmod}/api/core/inc
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        ZLIB::ZLIB
        -lmsacm32
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "codec_${NAME}"
        PREFIX ""
        SUFFIX ".dll"
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR_PARENT}/data/plugin
        COMPILE_FLAGS "-m32"
        LINK_FLAGS "-m32"
)
