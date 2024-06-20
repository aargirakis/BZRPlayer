cmake_minimum_required(VERSION 3.28)

set(NAME "libopenmpt")
set(VERSION "0.7.6+release")
project(plugin_${NAME})
set(PLUGIN_${NAME}_DLL "${PROJECT_NAME}.dll" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "${NAME}" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

find_package(ZLIB REQUIRED)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(LIB_NAME "${NAME}")
set(LIB_VERSION "${VERSION}")
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION}")
set(LIB_FILENAME "${LIB_NAME_VERSIONED}.makefile.tar.gz")
set(LIB_URL "https://lib.openmpt.org/files/${NAME}/src/${LIB_FILENAME}")
set(LIB_SHA_256_HASH "36643c01ee48b43eabff54c5cffb395987edf71adcddf89a6511cb9c81431fb3")
set(LIB_MAKE_ARGS CONFIG=mingw-w64 WINDOWS_ARCH=x86 OPENMPT123=0 TEST=0 EXAMPLES=0 SHARED_LIB=0 STATIC_LIB=1)
set(LIB_BUILD_BYPRODUCTS "bin/libopenmpt.a")
download_patch_and_make(
        ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH} false ""
        ${CMAKE_CURRENT_SOURCE_DIR}/patches "${LIB_MAKE_ARGS}" ${LIB_BUILD_BYPRODUCTS}
)

file(GLOB SOURCES
        src/queue.c #TODO remove
        src/main.cpp

        ${CMAKE_SOURCE_DIR}/src/app/BaseRow.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/libopenmpt

        ${CMAKE_SOURCE_DIR}/src/app
        ${EXTERNAL_SOURCE_DIR_fmod}/api/core/inc
)

add_library(LIBOPENMPT_STATIC STATIC IMPORTED GLOBAL)

set_target_properties(LIBOPENMPT_STATIC PROPERTIES
        IMPORTED_LOCATION "${EXTERNAL_SOURCE_DIR}/${LIB_BUILD_BYPRODUCTS}"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        LIBOPENMPT_STATIC
        ZLIB::ZLIB
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/data/plugin
        COMPILE_FLAGS "-m32"
        LINK_FLAGS "-m32"
)
