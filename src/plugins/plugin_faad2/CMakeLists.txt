cmake_minimum_required(VERSION 3.28)

set(NAME "faad2")
set(VERSION "2.10.0")
project(plugin_${NAME})
set(PLUGIN_${NAME}_DLL "${PROJECT_NAME}.dll" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "FAAD2" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION GREATER_EQUAL 14)
    add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-implicit-function-declaration>)
endif ()

set(LIB_NAME "${NAME}")
set(LIB_VERSION "${VERSION}")
string(REPLACE "." "_" LIB_TAG "${LIB_VERSION}")
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION}")
set(LIB_NAME_TAGGED "${LIB_NAME}-${LIB_TAG}")
set(LIB_FILENAME "${LIB_NAME_TAGGED}.zip")
set(LIB_FILENAME_URL "${LIB_TAG}.zip")
set(LIB_URL "https://github.com/knik0/${LIB_NAME}/archive/refs/tags/${LIB_FILENAME_URL}")
set(LIB_SHA_256_HASH "841841dd2ccd8cac295b15420ad20cc69ca4951a3de642ed2bdba8d634bfcf8a")
set(LIB_UNPACKED_DIR "${LIB_NAME_TAGGED}")
download_patch_and_add(
        ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH} true ${LIB_UNPACKED_DIR} ""
)

add_compile_definitions(HAVE_CONFIG_H=1)
set(HAVE_MEMCPY 1)
configure_file(
        src/config.h.in
        ${EXTERNAL_SOURCE_DIR}/config.h
        @ONLY NEWLINE_STYLE UNIX
)

configure_file(
        src/win32_ver.h.in
        ${EXTERNAL_SOURCE_DIR}/libfaad/win32_ver.h
        @ONLY NEWLINE_STYLE UNIX
)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/libfaad/*.c
        src/main.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/include
        ${EXTERNAL_SOURCE_DIR}/libfaad

        ${CMAKE_SOURCE_DIR}/src/app
        ${EXTERNAL_SOURCE_DIR_fmod}/api/core/inc
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/data/plugin
        COMPILE_FLAGS "-m32"
        LINK_FLAGS "-m32"
)
