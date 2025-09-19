cmake_minimum_required(VERSION 3.28)

set(NAME "vgmstream")
set(VERSION "2023")
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "${NAME}" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(NAME_VERSIONED "${NAME}-${VERSION}")
set(TAG "r${VERSION}")
set(NAME_TAGGED "${NAME}-${TAG}")
set(FILENAME "${NAME_TAGGED}.tar.gz")
set(FILENAME_URL "${TAG}.tar.gz")
set(URL "https://github.com/${NAME}/${NAME}/archive/refs/tags/${FILENAME_URL}")
set(SHA_256_HASH "4f4288f177cb12ecee7c2280a719044061195629947e08571e5c4484901b73bc")
set(UNPACKED_DIR "${NAME_TAGGED}")
download_patch_and_add(
        ${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} true ${UNPACKED_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/patches
)

add_subdirectory(deps/ffmpeg)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/src/*.c
        ${EXTERNAL_SOURCE_DIR}/src/base/*.c
        ${EXTERNAL_SOURCE_DIR}/src/coding/*.c
        ${EXTERNAL_SOURCE_DIR}/src/coding/libs/*.c
        ${EXTERNAL_SOURCE_DIR}/src/layout/*.c
        ${EXTERNAL_SOURCE_DIR}/src/meta/*.c
        ${EXTERNAL_SOURCE_DIR}/src/util/*.c

        # some extra sources are included here
        # in order to prevent linking failure on win and runtime failure on linux
        # since vgmstream lacks of some exports needed by our custom ffmpeg build
        ${EXTERNAL_SOURCE_DIR_ffmpeg}/libavcodec/adts_parser.c
        ${EXTERNAL_SOURCE_DIR_ffmpeg}/libavformat/demux.c
        ${EXTERNAL_SOURCE_DIR_ffmpeg}/libavformat/oggparsedirac.c
        ${EXTERNAL_SOURCE_DIR_ffmpeg}/libavformat/utils.c

        src/main.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/src
        ${EXTERNAL_SOURCE_DIR}/src/base
        ${EXTERNAL_SOURCE_DIR}/src/coding
        ${EXTERNAL_SOURCE_DIR}/src/coding/libs
        ${EXTERNAL_SOURCE_DIR}/src/layout
        ${EXTERNAL_SOURCE_DIR}/src/meta
        ${EXTERNAL_SOURCE_DIR}/src/util
        ${EXTERNAL_SOURCE_DIR_ffmpeg} # needed by the included extra sources
        ${EXTERNAL_SOURCE_DIR_ffmpeg}/out/include

        ${CMAKE_SOURCE_DIR}/src/app
        ${FMOD_API_DIR})

target_compile_definitions(${PROJECT_NAME} PRIVATE VGM_USE_FFMPEG)

foreach (FFMPEG_STATIC_LIB ${FFMPEG_STATIC_LIBS})
    add_library(${FFMPEG_STATIC_LIB} STATIC IMPORTED GLOBAL)
    add_dependencies(${FFMPEG_STATIC_LIB} ${FFMPEG_TARGET})
    set_target_properties(${FFMPEG_STATIC_LIB} PROPERTIES IMPORTED_LOCATION ${EXTERNAL_SOURCE_DIR_ffmpeg}/out/lib/${FFMPEG_STATIC_LIB}.a)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${FFMPEG_STATIC_LIB})
endforeach ()

if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE -lbcrypt)
endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
)
