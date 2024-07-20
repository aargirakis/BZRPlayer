cmake_minimum_required(VERSION 3.28)

set(NAME "vio2sf")
project(plugin_${NAME})

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse4.1")

file(GLOB SOURCES
        src/desmume/*.c
        src/desmume/*.cpp
        src/psflib/psf2fs.c  #TODO needed?
        src/psflib/psflib.c
        src/main.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        src/desmume
        src/psflib

        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/external/FMOD/api/core/inc
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        -L${CMAKE_SOURCE_DIR}/src/external/zlib -lzdll
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "codec_${NAME}"
        PREFIX ""
        SUFFIX ".dll"
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR_PARENT}/data/plugin
        COMPILE_FLAGS "-m32"
        LINK_FLAGS "-m32"
)
