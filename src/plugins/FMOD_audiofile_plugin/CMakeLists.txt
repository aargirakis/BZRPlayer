cmake_minimum_required(VERSION 3.28)

set(NAME audiofile)
project(plugin_${NAME})

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")

include(FetchContent)
set(GIT_REPO_NAME "audiofile")
set(GIT_REPO "https://github.com/mpruett/audiofile.git")
set(GIT_TAG "d9363a5d16af4ce55eb35c5aad9ca19bb9c53cbe")
#set(PATCH_FILE git apply ${CMAKE_CURRENT_SOURCE_DIR}/patches/test.patch)
set(external_repo_name external_repo_${GIT_REPO_NAME})

FetchContent_Declare(
        ${external_repo_name}
        GIT_REPOSITORY ${GIT_REPO}
        GIT_TAG ${GIT_TAG}
        GIT_SHALLOW false
        # PATCH_COMMAND ${PATCH_FILE}
)

message(STATUS "Cloning repository '${GIT_REPO_NAME}' ${GIT_REPO} at '${GIT_TAG}'")

FetchContent_MakeAvailable(${external_repo_name})

message(STATUS "Patching cloned '${GIT_REPO_NAME}' repository")

execute_process(
        COMMAND ${CMAKE_COMMAND} -E rm -f
        ${${external_repo_name}_SOURCE_DIR}/libaudiofile/modules/UT_RebufferModule.cpp
        #RESULT_VARIABLE delete_result
)

execute_process(
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/patches ${${external_repo_name}_SOURCE_DIR}
        RESULT_VARIABLE copy_result
        ERROR_VARIABLE copy_error
)

if (NOT ${copy_result} EQUAL 0)
    message(FATAL_ERROR "Error patching cloned '${GIT_REPO_NAME}' repository: ${copy_error}")
endif ()

configure_file(
        src/config.h.in
        ${${external_repo_name}_SOURCE_DIR}/libaudiofile/config.h
        @ONLY NEWLINE_STYLE UNIX
)

file(GLOB SOURCES
        ${${external_repo_name}_SOURCE_DIR}/libaudiofile/*.c
        ${${external_repo_name}_SOURCE_DIR}/libaudiofile/*.cpp
        ${${external_repo_name}_SOURCE_DIR}/libaudiofile/alac/*.c
        ${${external_repo_name}_SOURCE_DIR}/libaudiofile/alac/*.cpp
        ${${external_repo_name}_SOURCE_DIR}/libaudiofile/modules/*.cpp
        src/main.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${${external_repo_name}_SOURCE_DIR}/libaudiofile
        ${${external_repo_name}_SOURCE_DIR}/libaudiofile/alac
        ${${external_repo_name}_SOURCE_DIR}/libaudiofile/modules

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
