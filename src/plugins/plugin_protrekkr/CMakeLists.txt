cmake_minimum_required(VERSION 3.28)

set(NAME "protrekkr")
set(VERSION "2.8.1")
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "ProTrekkr" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

find_package(ZLIB REQUIRED)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 14)

set(NAME_VERSIONED "${NAME}-${VERSION}")
set(FILENAME "${NAME_VERSIONED}.tar.gz")
set(FILENAME_URL "v${VERSION}.tar.gz")
set(URL "https://github.com/hitchhikr/${NAME}/archive/refs/tags/${FILENAME_URL}")
set(SHA_256_HASH "40219b51997d4368ff931dada0832bacd99558d088d3aebe424e5d5e33f53b8d")
download_patch_and_add(
        ${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} false ""
        ${CMAKE_CURRENT_SOURCE_DIR}/patches
)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/release/distrib/replay/lib/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/editors/patterns_blocks.cpp
        ${EXTERNAL_SOURCE_DIR}/src/files/303s.cpp
        ${EXTERNAL_SOURCE_DIR}/src/files/files.cpp
        ${EXTERNAL_SOURCE_DIR}/src/files/reverbs.cpp
        ${EXTERNAL_SOURCE_DIR}/src/files/synths.cpp
        src/main.cpp)

if (WIN32)
    list(REMOVE_ITEM SOURCES ${EXTERNAL_SOURCE_DIR}/release/distrib/replay/lib/endianness.cpp)
else ()
    list(REMOVE_ITEM SOURCES ${EXTERNAL_SOURCE_DIR}/release/distrib/replay/lib/samples_unpack.cpp)
endif ()

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/release/distrib/replay/lib
        ${EXTERNAL_SOURCE_DIR}/release/distrib/replay/lib/include
        ${EXTERNAL_SOURCE_DIR}/release/distrib/replay/lib/sounddriver/include
        ${EXTERNAL_SOURCE_DIR}/src/editors/include
        ${EXTERNAL_SOURCE_DIR}/src/files/include
        ${EXTERNAL_SOURCE_DIR}/src/include
        src

        ${CMAKE_SOURCE_DIR}/src/app
        ${FMOD_API_DIR})

target_compile_definitions(${PROJECT_NAME} PRIVATE BZR2 __STAND_ALONE__ __WINAMP__)

if (WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE __WIN32__)
else ()
    target_compile_definitions(${PROJECT_NAME} PRIVATE __GCC__ __LINUX__)
endif ()

target_link_libraries(${PROJECT_NAME} PRIVATE ZLIB::ZLIB)

if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE -lmsacm32)
endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
)
