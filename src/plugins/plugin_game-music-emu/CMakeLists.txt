cmake_minimum_required(VERSION 3.28)

set(NAME "game-music-emu")
set(VERSION "0.6.3")
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "Game_Music_Emu" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

set(NAME_VERSIONED "${NAME}-${VERSION}")
set(FILENAME "${NAME_VERSIONED}.zip")
set(FILENAME_URL "${VERSION}.zip")
set(URL "https://github.com/libgme/${NAME}/archive/refs/tags/${FILENAME_URL}")
set(SHA_256_HASH "2fe64e09ab228aa9b055cd69997a2b49179011e091127cf801161fb5bf5ae8cf")
download_patch_and_add(
        ${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} false "" ""
)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/gme/*.cpp
        src/main.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/gme

        ${CMAKE_SOURCE_DIR}/src/app
        ${FMOD_API_DIR})

target_compile_definitions(${PROJECT_NAME} PRIVATE BLARGG_BUILD_DLL)
target_compile_definitions(${PROJECT_NAME} PRIVATE VGM_YM2612_MAME)
target_compile_definitions(${PROJECT_NAME} PRIVATE PLUGINS_CONFIG_DIR="${PLUGINS_CONFIG_DIR}")

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${DATA_DIR}/${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${DATA_DIR}/${PLUGINS_DIR}
)
