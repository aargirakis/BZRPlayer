cmake_minimum_required(VERSION 3.28)

set(NAME "game-music-emu")
set(VERSION "0.6.5")
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "Game_Music_Emu" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(NAME_VERSIONED "${NAME}-${VERSION}")
set(FILENAME "${NAME_VERSIONED}.tar.gz")
set(FILENAME_URL "${VERSION}.tar.gz")
set(URL "https://github.com/libgme/${NAME}/archive/refs/tags/${FILENAME_URL}")
set(SHA_256_HASH "8531678502451c2cf04248cda45c8b4645e19fcfb63e6a7ec2549641c47bb392")
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
        ${LOGGER_API_DIR}
        ${FMOD_API_DIR})

set(CONFIG_FILENAME "${PROJECT_NAME}.cfg")
set(PLUGIN_${NAME}_CONFIG_FILENAME "${CONFIG_FILENAME}" PARENT_SCOPE)
target_compile_definitions(${PROJECT_NAME} PRIVATE CONFIG_FILENAME="${CONFIG_FILENAME}")
target_compile_definitions(${PROJECT_NAME} PRIVATE PLUGIN_CONFIGS_DIR="${PLUGIN_CONFIGS_DIR}")
target_compile_definitions(${PROJECT_NAME} PRIVATE BLARGG_LITTLE_ENDIAN)
target_compile_definitions(${PROJECT_NAME} PRIVATE VGM_YM2612_MAME)

if (WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE BLARGG_BUILD_DLL)
endif ()

target_link_libraries(${PROJECT_NAME} PRIVATE logger)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
)
