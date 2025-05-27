cmake_minimum_required(VERSION 3.28)

set(NAME "game-music-emu")
set(VERSION "0.6.4")
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "Game_Music_Emu" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

set(NAME_VERSIONED "${NAME}-${VERSION}")
set(FILENAME "${NAME_VERSIONED}.tar.gz")
set(FILENAME_URL "${VERSION}.tar.gz")
set(URL "https://github.com/libgme/${NAME}/archive/refs/tags/${FILENAME_URL}")
set(SHA_256_HASH "f2360feb5a32ace226c583df4faf6eff74145c81264aaea11e17a1af2f6f101a")
download_patch_and_add(
        ${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} false "" ""
)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/gme/*.cpp
        ${EXTERNAL_SOURCE_DIR}/gme/ext/*.c
        src/main.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/gme
        ${EXTERNAL_SOURCE_DIR}/gme/ext

        ${CMAKE_SOURCE_DIR}/src/app
        ${FMOD_API_DIR})

target_compile_definitions(${PROJECT_NAME} PRIVATE BLARGG_LITTLE_ENDIAN)
target_compile_definitions(${PROJECT_NAME} PRIVATE VGM_YM2612_MAME)
target_compile_definitions(${PROJECT_NAME} PRIVATE PLUGINS_CONFIG_DIR="${PLUGINS_CONFIG_DIR}")

if (WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE BLARGG_BUILD_DLL)
endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
)
