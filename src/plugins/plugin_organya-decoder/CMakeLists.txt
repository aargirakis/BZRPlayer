cmake_minimum_required(VERSION 3.28)

set(NAME "organya-decoder")
project(plugin_${NAME} VERSION "1.11")
set(PLUGIN_DLL_${NAME} "${PROJECT_NAME}.dll" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(LIB_NAME "${NAME}")
set(LIB_VERSION ${PROJECT_VERSION})
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION}")
set(LIB_UNPACKED_DIR "losnoco-foo_input_org-d713df1b6c8a")
set(LIB_FILENAME "${LIB_UNPACKED_DIR}.zip")
set(LIB_URL "https://www.cavestory.org/downloads/${LIB_FILENAME}")
set(LIB_SHA_256_HASH "15d3f0851104c3b8dcb2d0fd600dac86c6afe98b7a3a16033d70dc2fcca9c8c8")
download_and_patch(
        ${LIB_NAME} ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH} true ${LIB_UNPACKED_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/patches
)

set(org_data_path "/data/plugin/orgsamples")
add_compile_definitions(ORG_DATA_PATH="${org_data_path}")

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/liborganya/*.c
        ${EXTERNAL_SOURCE_DIR}/liborganya/*.cpp
        src/main.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/liborganya

        ${CMAKE_SOURCE_DIR}/src/app
        ${EXTERNAL_SOURCE_DIR_fmod}/api/core/inc
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        SUFFIX ".dll"
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR_PARENT}/data/plugin
        COMPILE_FLAGS "-m32"
        LINK_FLAGS "-m32"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        ${EXTERNAL_SOURCE_DIR}/liborganya/samples
        ${OUTPUT_DIR_PARENT}${org_data_path}

        VERBATIM
)