cmake_minimum_required(VERSION 3.28)

set(NAME "flod")
project(plugin_${NAME})

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

file(GLOB SOURCES
        src/flod/*.cpp
        src/flod/brooke/*.cpp
        src/flod/daglish/*.cpp
        src/flod/delta1/*.cpp
        src/flod/delta2/*.cpp
        src/flod/digitalmugician/*.cpp
        src/flod/fred/*.cpp
        src/flod/hippel/*.cpp
        src/flod/hubbard/*.cpp
        src/flod/infogrames/*.cpp
        src/flod/joseph/*.cpp
        src/flod/sidmon/*.cpp
        src/flod/soundfx/*.cpp
        src/flod/soundmon/*.cpp
        src/flod/trackers/PTPlayer.cpp
        src/flod/trackers/PTVoice.cpp
        src/flod/whittaker/*.cpp
        src/main.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        src
        src/flod
        src/flod/brooke
        src/flod/daglish
        src/flod/delta1
        src/flod/delta2
        src/flod/digitalmugician
        src/flod/fred
        src/flod/hippel
        src/flod/hubbard
        src/flod/infogrames
        src/flod/joseph
        src/flod/sidmon
        src/flod/soundfx
        src/flod/soundmon
        src/flod/trackers
        src/flod/whittaker

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
