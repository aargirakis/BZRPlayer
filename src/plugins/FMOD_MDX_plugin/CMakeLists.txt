cmake_minimum_required(VERSION 3.28)

set(NAME mdx)
project(plugin_${NAME})

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

include(FetchContent)
set(GIT_REPO_NAME "mdxmini")
set(GIT_REPO "https://github.com/mistydemeo/${GIT_REPO_NAME}.git")
set(GIT_TAG "v2.0.0")
#set(PATCH_FILE git apply ${CMAKE_CURRENT_SOURCE_DIR}/patches/test.patch)
set(external_repo_name external_repo_${GIT_REPO_NAME})

FetchContent_Declare(
        ${external_repo_name}
        GIT_REPOSITORY ${GIT_REPO}
        GIT_TAG ${GIT_TAG}
        GIT_SHALLOW true
        # PATCH_COMMAND ${PATCH_FILE}
)

message(STATUS "Cloning repository '${GIT_REPO_NAME}' ${GIT_REPO} at '${GIT_TAG}'")

FetchContent_MakeAvailable(${external_repo_name})

message(STATUS "Patching cloned '${GIT_REPO_NAME}' repository")

execute_process(
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/patches ${${external_repo_name}_SOURCE_DIR}
        RESULT_VARIABLE copy_result
        ERROR_VARIABLE copy_error
)

if (NOT ${copy_result} EQUAL 0)
    message(FATAL_ERROR "Error patching cloned '${GIT_REPO_NAME}' repository: ${copy_error}")
endif ()

file(GLOB SOURCES
        ${${external_repo_name}_SOURCE_DIR}/src/*.c
        src/main.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${${external_repo_name}_SOURCE_DIR}/src

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
