cmake_minimum_required(VERSION 3.28)

set(NAME "libxmp")
set(VERSION "4.6.2")
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "Libxmp" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(LIB_NAME "${NAME}")
set(LIB_VERSION "${VERSION}")
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION}")
set(LIB_FILENAME "${LIB_NAME_VERSIONED}.tar.gz")
set(LIB_URL "https://github.com/${LIB_NAME}/${LIB_NAME}/releases/download/${LIB_NAME_VERSIONED}/${LIB_FILENAME}")
set(LIB_SHA_256_HASH "acac1705be2c4fb4d2d70dc05759853ba6aab747a83de576b082784d46f5a4b9")
download_patch_and_add(
        ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH} false ""
        ${CMAKE_CURRENT_SOURCE_DIR}/patches
)

if (WIN32)
    add_compile_definitions(BUILDING_DLL)
endif ()

add_compile_definitions(PLUGINS_CONFIG_DIR="${PLUGINS_CONFIG_DIR}")

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/src/*.c
        ${EXTERNAL_SOURCE_DIR}/src/depackers/*.c
        ${EXTERNAL_SOURCE_DIR}/src/depackers/lhasa/*.c
        ${EXTERNAL_SOURCE_DIR}/src/loaders/*.c
        ${EXTERNAL_SOURCE_DIR}/src/loaders/prowizard/*.c
        src/queue.c #TODO remove (and the queue.h include in common.h.patch)
        src/main.cpp

        ${CMAKE_SOURCE_DIR}/src/app/BaseRow.cpp
)

list(REMOVE_ITEM SOURCES
        ${EXTERNAL_SOURCE_DIR}/src/depackers/lhasa/bit_stream_reader.c
        ${EXTERNAL_SOURCE_DIR}/src/depackers/lhasa/lh_new_decoder.c
        ${EXTERNAL_SOURCE_DIR}/src/depackers/lhasa/pma_common.c
        ${EXTERNAL_SOURCE_DIR}/src/depackers/lhasa/tree_decode.c
        ${EXTERNAL_SOURCE_DIR}/src/loaders/prowizard/pm.c
        ${EXTERNAL_SOURCE_DIR}/src/loaders/prowizard/pm20.c
        ${EXTERNAL_SOURCE_DIR}/src/loaders/prowizard/pm40.c
        ${EXTERNAL_SOURCE_DIR}/src/loaders/prowizard/pp30.c
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/include
        ${EXTERNAL_SOURCE_DIR}/src
        ${EXTERNAL_SOURCE_DIR}/src/depackers
        ${EXTERNAL_SOURCE_DIR}/src/depackers/lhasa
        ${EXTERNAL_SOURCE_DIR}/src/loaders
        ${EXTERNAL_SOURCE_DIR}/src/loaders/prowizard
        src #TODO remove

        ${CMAKE_SOURCE_DIR}/src/app
        ${LIB_FMOD_API_DIR}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${DATA_DIR}/${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${DATA_DIR}/${PLUGINS_DIR}
)
