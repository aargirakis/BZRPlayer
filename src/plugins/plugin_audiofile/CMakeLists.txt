cmake_minimum_required(VERSION 3.28)

set(NAME "audiofile")
set(VERSION "d9363a5d16af4ce55eb35c5aad9ca19bb9c53cbe")
string(SUBSTRING ${VERSION} 0 12 VERSION_SHORT)
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "Audio File Library" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION_SHORT}" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(LIB_NAME "${NAME}")
set(LIB_VERSION "${VERSION}")
set(LIB_VERSION_SHORT "${VERSION_SHORT}")
set(LIB_NAME_VERSIONED "${LIB_NAME}-${VERSION_SHORT}")
set(LIB_FILENAME "${LIB_NAME}-${LIB_VERSION}.zip")
set(LIB_FILENAME_URL "${LIB_VERSION}.zip")
set(LIB_URL "https://github.com/mpruett/${LIB_NAME}/archive/${LIB_FILENAME_URL}")
set(LIB_SHA_256_HASH "83835f1705a0cec378c1c4fc5d7d6385737be6dcfbc480049bd82e3657069194")
set(LIB_UNPACKED_DIR "${LIB_NAME}-${LIB_VERSION}")
download_patch_and_add(
        ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH} true ${LIB_UNPACKED_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/patches
)

configure_file(
        src/config.h.in
        ${EXTERNAL_SOURCE_DIR}/libaudiofile/config.h
        @ONLY NEWLINE_STYLE UNIX
)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/libaudiofile/*.c
        ${EXTERNAL_SOURCE_DIR}/libaudiofile/*.cpp
        ${EXTERNAL_SOURCE_DIR}/libaudiofile/alac/*.c
        ${EXTERNAL_SOURCE_DIR}/libaudiofile/alac/*.cpp
        ${EXTERNAL_SOURCE_DIR}/libaudiofile/modules/*.cpp
        src/main.cpp
)

list(REMOVE_ITEM SOURCES ${EXTERNAL_SOURCE_DIR}/libaudiofile/modules/UT_RebufferModule.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/libaudiofile
        ${EXTERNAL_SOURCE_DIR}/libaudiofile/alac
        ${EXTERNAL_SOURCE_DIR}/libaudiofile/modules

        ${CMAKE_SOURCE_DIR}/src/app
        ${LIB_FMOD_API_DIR}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${DATA_DIR}/${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${DATA_DIR}/${PLUGINS_DIR}
)
