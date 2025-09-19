cmake_minimum_required(VERSION 3.28)

set(NAME "ffmpeg")
project(${NAME})

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(VERSION "7.1.1")

file(READ ${EXTERNAL_SOURCE_DIR}/cmake/dependencies/ffmpeg.cmake file_content)
string(REGEX MATCH "GIT_TAG n([0-9.]*)" MATCH_RESULT ${file_content})

if (NOT MATCH_RESULT)
    message(FATAL_ERROR "Unable to check ${NAME} dependency version needed")
endif ()

if (NOT ${CMAKE_MATCH_1} STREQUAL ${VERSION})
    message(FATAL_ERROR "${NAME} needs to be updated (and possibly adapted) to version ${CMAKE_MATCH_1}")
endif ()

execute_process(
        COMMAND sed
        -e "/^\#/d" ${EXTERNAL_SOURCE_DIR}/ext_libs/extra/ffmpeg_options.txt
        OUTPUT_VARIABLE FFMPEG_OPTIONS
        RESULT_VARIABLE SED_RESULT
        ERROR_VARIABLE SED_ERROR)

if (NOT SED_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to run sed command with error:\n${SED_ERROR}")
endif ()

set(NAME_VERSIONED "${NAME}-${VERSION}")
set(NAME2 "FFmpeg")
set(TAG "n${VERSION}")
set(NAME_TAGGED "${NAME2}-${TAG}")
set(FILENAME "${NAME_TAGGED}.tar.gz")
set(FILENAME_URL "${TAG}.tar.gz")
set(URL "https://github.com/${NAME2}/${NAME2}/archive/refs/tags/${FILENAME_URL}")
set(SHA_256_HASH "f117507dc501f2a6c11f9241d8d0c3213846cfad91764361af37befd6b6c523d")
set(UNPACKED_DIR "${NAME_TAGGED}")

#TODO "--extra-libs=-static --extra-cflags=--static --pkg-config-flags=--static" needed?
set(CONFIGURE_COMMAND "configure ${FFMPEG_OPTIONS} --prefix=./out --disable-x86asm --arch=x86_64")

if (WIN32)
    set(CONFIGURE_COMMAND "${CONFIGURE_COMMAND} --target-os=mingw64")
else ()
    set(CONFIGURE_COMMAND "${CONFIGURE_COMMAND} --target-os=linux")
endif ()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CONFIGURE_COMMAND "${CONFIGURE_COMMAND} --extra-cflags=\"-g -DDEBUG\"")
else ()
    set(CONFIGURE_COMMAND "${CONFIGURE_COMMAND} --disable-debug")
endif ()

list(APPEND FFMPEG_STATIC_LIBS libavcodec)
list(APPEND FFMPEG_STATIC_LIBS libavformat)
list(APPEND FFMPEG_STATIC_LIBS libavutil)
list(APPEND FFMPEG_STATIC_LIBS libswresample)

foreach (FFMPEG_STATIC_LIB ${FFMPEG_STATIC_LIBS})
    list(APPEND BUILD_BYPRODUCTS "out/lib/${FFMPEG_STATIC_LIB}.a")
endforeach ()

download_patch_and_make(
        ${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} true ${UNPACKED_DIR} ""
        ${CONFIGURE_COMMAND} "" true "${BUILD_BYPRODUCTS}"
)

set(EXTERNAL_SOURCE_DIR_${NAME} "${EXTERNAL_SOURCE_DIR}" PARENT_SCOPE)
set(FFMPEG_TARGET "${NAME_VERSIONED}" PARENT_SCOPE)
set(FFMPEG_STATIC_LIBS "${FFMPEG_STATIC_LIBS}" PARENT_SCOPE)
