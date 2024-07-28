cmake_minimum_required(VERSION 3.28)

set(NAME "libopenmpt")
project(plugin_${NAME} VERSION "0.7.6")

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

set(LIB_NAME "${NAME}")
set(LIB_VERSION ${PROJECT_VERSION}+release)
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION}")
set(LIB_FILENAME "${LIB_NAME_VERSIONED}.makefile.tar.gz")
set(LIB_URL "https://lib.openmpt.org/files/${NAME}/src/${LIB_FILENAME}")
set(LIB_SHA_256_HASH "36643c01ee48b43eabff54c5cffb395987edf71adcddf89a6511cb9c81431fb3")
download_and_patch(
        ${LIB_NAME} ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH}
        ""
        ${CMAKE_CURRENT_SOURCE_DIR}/patches
)

set(MAKE_ARGS
        -j -l10 CONFIG=mingw-w64
        WINDOWS_ARCH=x86 OPENMPT123=0 TEST=0 EXAMPLES=0
        SHARED_LIB=0 STATIC_LIB=1
)

if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(MAKE ${CMAKE_PREFIX_PATH}/../../Tools/mingw810_32/bin/mingw32-make.exe)
    set(MAKE_ARGS "AR=${CMAKE_PREFIX_PATH}/../../Tools/mingw810_32/bin/ar.exe ${MAKE_ARGS}")
else ()
    set(MAKE ${CMAKE_PREFIX_PATH}/../bin/make)
endif ()

set(LIBOPENMPT_STATIC_FILENAME "libopenmpt.a")
set(LIBOPENMPT_STATIC "${EXTERNAL_SOURCE_DIR}/bin/${LIBOPENMPT_STATIC_FILENAME}")
set(LIBOPENMPT_STATIC_FINAL "${LIBOPENMPT_STATIC}.final")

add_custom_target(
        make_libopenmpt ALL
        COMMENT "Starting libopenmpt make"
        COMMAND ${MAKE} ${MAKE_ARGS}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${LIBOPENMPT_STATIC}" "${LIBOPENMPT_STATIC_FINAL}"
        WORKING_DIRECTORY ${EXTERNAL_SOURCE_DIR}
)

add_library(libopenmpt STATIC IMPORTED)

add_dependencies(libopenmpt make_libopenmpt)

file(WRITE ${LIBOPENMPT_STATIC_FINAL} "")

set_target_properties(libopenmpt PROPERTIES
        IMPORTED_LOCATION "${LIBOPENMPT_STATIC_FINAL}"
)

file(GLOB SOURCES
        src/queue.c
        src/main.cpp

        ${CMAKE_SOURCE_DIR}/src/plugins/FMOD_Flod_plugin/src/flod/BaseRow.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE libopenmpt)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/libopenmpt

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
