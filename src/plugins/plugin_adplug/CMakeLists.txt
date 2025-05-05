cmake_minimum_required(VERSION 3.28)

set(NAME "adplug")
set(VERSION "2.3.3")
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "AdPlug" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

add_subdirectory(deps/libbinio)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(NAME_VERSIONED "${NAME}-${VERSION}")
set(FILENAME "${NAME_VERSIONED}.tar.bz2")
set(URL "https://github.com/${NAME}/${NAME}/releases/download/${NAME_VERSIONED}/${FILENAME}")
set(SHA_256_HASH "a0f3c1b18fb49dea7ac3e8f820e091a663afa5410d3443612bf416cff29fa928")
download_patch_and_add(
        ${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} false "" ""
)

configure_file(
        ${EXTERNAL_SOURCE_DIR}/src/version.h.in
        ${EXTERNAL_SOURCE_DIR}/src/version.h
        @ONLY NEWLINE_STYLE UNIX
)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/src/*.c
        ${EXTERNAL_SOURCE_DIR}/src/*.cpp
        src/main.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/src

        ${CMAKE_SOURCE_DIR}/src/app
        ${FMOD_API_DIR})

target_compile_definitions(${PROJECT_NAME} PRIVATE stricmp=strcasecmp)
target_compile_definitions(${PROJECT_NAME} PRIVATE PLUGINS_CONFIG_DIR="${PLUGINS_CONFIG_DIR}")

target_link_libraries(${PROJECT_NAME} PRIVATE libbinio)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${DATA_DIR}/${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${DATA_DIR}/${PLUGINS_DIR}
)
