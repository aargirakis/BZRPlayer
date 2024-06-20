cmake_minimum_required(VERSION 3.28)

set(NAME "protrekkr")
set(VERSION "2.6.4")
project(plugin_${NAME})
set(PLUGIN_${NAME}_DLL "${PROJECT_NAME}.dll" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "ProTrekkr" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

find_package(ZLIB REQUIRED)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 14)

set(LIB_NAME "${NAME}")
set(LIB_VERSION "${VERSION}")
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION}")
set(LIB_FILENAME "${LIB_NAME_VERSIONED}.tar.gz")
set(LIB_FILENAME_URL "v${LIB_VERSION}.tar.gz")
set(LIB_URL "https://github.com/hitchhikr/${LIB_NAME}/archive/refs/tags/${LIB_FILENAME_URL}")
set(LIB_SHA_256_HASH "801deb0ac1781fbd0b6608b8c319b5dc7d36400e15401abeb8a559e3127914e2")
download_patch_and_add(
        ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH} false ""
        ${CMAKE_CURRENT_SOURCE_DIR}/patches
)

add_compile_definitions(BZR2 __STAND_ALONE__ __WINAMP__)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/release/distrib/replay/lib/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/editors/patterns_blocks.cpp
        ${EXTERNAL_SOURCE_DIR}/src/files/303s.cpp
        ${EXTERNAL_SOURCE_DIR}/src/files/files.cpp
        ${EXTERNAL_SOURCE_DIR}/src/files/reverbs.cpp
        ${EXTERNAL_SOURCE_DIR}/src/files/synths.cpp
        src/main.cpp
)

list(REMOVE_ITEM SOURCES
        ${EXTERNAL_SOURCE_DIR}/release/distrib/replay/lib/endianness.cpp
)

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
        ${EXTERNAL_SOURCE_DIR_fmod}/api/core/inc
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        ZLIB::ZLIB
        -lmsacm32
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/data/plugin
        COMPILE_FLAGS "-m32"
        LINK_FLAGS "-m32"
)
