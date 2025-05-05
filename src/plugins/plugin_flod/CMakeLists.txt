cmake_minimum_required(VERSION 3.28)

set(NAME "flod")
set(VERSION "-")
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "Flod (C++ porting)" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

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

        ${CMAKE_SOURCE_DIR}/src/app/BaseRow.cpp)

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

        ${CMAKE_SOURCE_DIR}/src/app
        ${FMOD_API_DIR})

target_compile_definitions(${PROJECT_NAME} PRIVATE PLUGINS_CONFIG_DIR="${PLUGINS_CONFIG_DIR}")

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${DATA_DIR}/${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${DATA_DIR}/${PLUGINS_DIR}
)
