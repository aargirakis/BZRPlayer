cmake_minimum_required(VERSION 3.28)

set(NAME "protrekkr")
project(plugin_${NAME})

set(CMAKE_C_STANDARD 17)

if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(CMAKE_CXX_STANDARD 17)
else ()
    set(CMAKE_CXX_STANDARD 14)
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__REPLAYER__ -D__STAND_ALONE__ -D__WINAMP__")

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

        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/external/FMOD/api/core/inc
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        -L${CMAKE_SOURCE_DIR}/src/external/zlib -lzdll -lmsacm32
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "codec_${NAME}"
        PREFIX ""
        SUFFIX ".dll"
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR_PARENT}/data/plugin
        COMPILE_FLAGS "-m32"
        LINK_FLAGS "-m32"
)
