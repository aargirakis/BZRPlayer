cmake_minimum_required(VERSION 3.28)

set(NAME "asap")
set(VERSION "8.0.0")
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "ASAP" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(NAME_VERSIONED "${NAME}-${VERSION}")
set(FILENAME "${NAME_VERSIONED}.tar.gz")
set(URL "https://sourceforge.net/projects/${NAME}/files/${NAME}/${VERSION}/${FILENAME}")
set(SHA_256_HASH "062d7db2a0747bf9200560141452f8fa2289909b147ac40a5bb5b60d5275a14f")
download_patch_and_add(
        ${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} false "" ""
)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/asap.c
        src/main.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}

        ${CMAKE_SOURCE_DIR}/src/app
        ${FMOD_API_DIR})

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
)
