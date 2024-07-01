cmake_minimum_required(VERSION 3.28)

set(NAME adplug)
project(plugin_${NAME} VERSION "2.3.3")

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

set(VERSION ${PROJECT_VERSION})
configure_file(
        src/adplug-adplug-${PROJECT_VERSION}/src/version.h.in
        ${CMAKE_CURRENT_SOURCE_DIR}/src/adplug-adplug-${PROJECT_VERSION}/src/version.h
        @ONLY NEWLINE_STYLE UNIX
)

file(GLOB SOURCES
        src/adplug-adplug-${PROJECT_VERSION}/src/*.c
        src/adplug-adplug-${PROJECT_VERSION}/src/*.cpp
        src/adplug/libbinio/*.cpp
        src/main.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        src/adplug-adplug-${PROJECT_VERSION}/src
        src/adplug/libbinio

        ${CMAKE_SOURCE_DIR}/src
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
