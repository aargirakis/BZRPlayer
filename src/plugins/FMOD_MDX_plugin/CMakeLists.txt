cmake_minimum_required(VERSION 3.28)

set(NAME mdx)
project(plugin_${NAME} VERSION "2.0.0")

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

set(GIT_REPO_NAME "mdxmini")
set(GIT_REPO_URL "https://github.com/mistydemeo/${GIT_REPO_NAME}.git")
set(GIT_CLONE_AT "v${PROJECT_VERSION}")
set(GIT_IS_TAG true)

clone_and_patch(
        ${GIT_REPO_NAME} ${GIT_REPO_URL} ${GIT_CLONE_AT} ${GIT_IS_TAG}
        ""
        ${CMAKE_CURRENT_SOURCE_DIR}/patches
)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/src/*.c
        src/main.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/src

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
