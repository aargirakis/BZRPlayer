cmake_minimum_required(VERSION 3.28)

set(NAME "libxmp")
set(VERSION "4.6.3")
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "Libxmp" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(NAME_VERSIONED "${NAME}-${VERSION}")
set(FILENAME "${NAME_VERSIONED}.tar.gz")
set(URL "https://github.com/${NAME}/${NAME}/releases/download/${NAME_VERSIONED}/${FILENAME}")
set(SHA_256_HASH "b189a2ff3f3eef0008512e0fb27c2cdc27480bc1066b82590a84d02548fab96d")
download_patch_and_add(
        ${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} false ""
        ${CMAKE_CURRENT_SOURCE_DIR}/patches
)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/src/*.c
        ${EXTERNAL_SOURCE_DIR}/src/depackers/*.c
        ${EXTERNAL_SOURCE_DIR}/src/depackers/lhasa/*.c
        ${EXTERNAL_SOURCE_DIR}/src/loaders/*.c
        ${EXTERNAL_SOURCE_DIR}/src/loaders/prowizard/*.c
        src/queue.c
        src/main.cpp

        ${CMAKE_SOURCE_DIR}/src/app/BaseRow.cpp)

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
        src

        ${CMAKE_SOURCE_DIR}/src/app
        ${FMOD_API_DIR})

if (WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE BUILDING_DLL)
endif ()

target_compile_definitions(${PROJECT_NAME} PRIVATE PLUGINS_CONFIG_DIR="${PLUGINS_CONFIG_DIR}")

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
)
