cmake_minimum_required(VERSION 3.28)

set(NAME "organya-decoder")
set(VERSION "1.11")
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "Organya decoder" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(LIB_NAME "${NAME}")
set(LIB_VERSION "${VERSION}")
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION}")
set(LIB_UNPACKED_DIR "losnoco-foo_input_org-d713df1b6c8a")
set(LIB_FILENAME "${LIB_UNPACKED_DIR}.zip")
set(LIB_URL "https://www.cavestory.org/downloads/${LIB_FILENAME}")
set(LIB_SHA_256_HASH "15d3f0851104c3b8dcb2d0fd600dac86c6afe98b7a3a16033d70dc2fcca9c8c8")
download_patch_and_add(
        ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH} true ${LIB_UNPACKED_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/patches
)

set(ORGANYA_DATA_DIR "${PLUGINS_DIR}/organya")
add_compile_definitions(ORGANYA_DATA_DIR="${ORGANYA_DATA_DIR}")

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/liborganya/*.c
        ${EXTERNAL_SOURCE_DIR}/liborganya/*.cpp
        src/main.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/liborganya

        ${CMAKE_SOURCE_DIR}/src/app
        ${LIB_FMOD_API_DIR}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${DATA_DIR}/${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${DATA_DIR}/${PLUGINS_DIR}
)

cmake_path(SET ORGANYA_DATA_PATH NORMALIZE ${OUTPUT_DIR}/${DATA_DIR}/${ORGANYA_DATA_DIR})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        ${EXTERNAL_SOURCE_DIR}/liborganya/samples ${ORGANYA_DATA_PATH}
        VERBATIM)