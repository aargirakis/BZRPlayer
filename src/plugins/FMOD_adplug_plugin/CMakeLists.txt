cmake_minimum_required(VERSION 3.28)

set(NAME "adplug")
project(plugin_${NAME} VERSION "2.3.3")

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

set(GIT1_REPO_NAME "${NAME}")
set(GIT1_REPO_URL "https://github.com/${GIT1_REPO_NAME}/${GIT1_REPO_NAME}.git")
set(GIT1_CLONE_AT "${GIT1_REPO_NAME}-${PROJECT_VERSION}")
set(GIT1_IS_TAG true)
clone_and_patch(
        ${GIT1_REPO_NAME} ${GIT1_REPO_URL} ${GIT1_CLONE_AT} ${GIT1_IS_TAG}
        ""
        ""
)

set(EXTERNAL_SOURCE_DIR_1 "${EXTERNAL_SOURCE_DIR}")

set(GIT2_REPO_NAME "libbinio")
set(GIT2_REPO_URL "https://github.com/${GIT1_REPO_NAME}/${GIT2_REPO_NAME}.git")
set(GIT2_CLONE_AT "${GIT2_REPO_NAME}-1.5")
set(GIT2_IS_TAG true)
clone_and_patch(
        ${GIT2_REPO_NAME} ${GIT2_REPO_URL} ${GIT2_CLONE_AT} ${GIT2_IS_TAG}
        ""
        ""
)

set(EXTERNAL_SOURCE_DIR_2 "${EXTERNAL_SOURCE_DIR}")

set(VERSION "${PROJECT_VERSION}")
configure_file(
        ${EXTERNAL_SOURCE_DIR_1}/src/version.h.in
        ${EXTERNAL_SOURCE_DIR_1}/src/version.h
        @ONLY NEWLINE_STYLE UNIX
)

set(ENABLE_STRING 1)
set(ENABLE_IOSTREAM 1)
set(ISO_STDLIB 1)
set(WITH_MATH 1)
set(TYPE_INT "long long")
set(TYPE_FLOAT "long double")
configure_file(
        ${EXTERNAL_SOURCE_DIR_2}/src/binio.h.in
        ${EXTERNAL_SOURCE_DIR_2}/src/binio.h
        @ONLY NEWLINE_STYLE UNIX
)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR_1}/src/*.c
        ${EXTERNAL_SOURCE_DIR_1}/src/*.cpp
        ${EXTERNAL_SOURCE_DIR_2}/src/*.cpp
        src/main.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR_1}/src
        ${EXTERNAL_SOURCE_DIR_2}/src

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
