cmake_minimum_required(VERSION 3.28)

set(NAME "jaytrax")
project(plugin_${NAME})

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

set(GIT_REPO_NAME ${NAME})
set(GIT_REPO_URL "https://github.com/pachuco/${GIT_REPO_NAME}.git")
set(GIT_CLONE_AT "1c2a999014058ef04973c4f3fac8bdf61a3fa9a4")
set(GIT_IS_TAG false)
clone_and_patch(
        ${GIT_REPO_NAME} ${GIT_REPO_URL} ${GIT_CLONE_AT} ${GIT_IS_TAG}
        ""
        ""
)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/lib_oldjaytrax/*.c
        src/main.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/lib_oldjaytrax

        ${CMAKE_SOURCE_DIR}/src/app
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
