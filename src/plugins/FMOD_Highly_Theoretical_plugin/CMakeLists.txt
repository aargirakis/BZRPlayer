cmake_minimum_required(VERSION 3.28)

set(NAME "highly_theoretical")
project(plugin_${NAME})

find_package(ZLIB)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION GREATER_EQUAL 14)
    add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-int-conversion>)
endif ()

set(LIB_NAME ${NAME})
set(LIB_VERSION "0e4c18c5b757b04dbcb68c572c5a4f6fd803283c")
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION}")
set(LIB_FILENAME "${LIB_NAME_VERSIONED}.zip")
set(LIB_URL "https://gitlab.com/kode54/${LIB_NAME}/-/archive/${LIB_VERSION}/${LIB_FILENAME}")
set(LIB_SHA_256_HASH "fe097680bb78d820139d1c1a10ece5724bd95d655ffc10302decf0b8d378d35b")
download_and_patch(
        ${LIB_NAME} ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH} ""
        ""
)

set(EXTERNAL_SOURCE_DIR_${NAME} "${EXTERNAL_SOURCE_DIR}")

add_compile_definitions(
        EMU_COMPILE EMU_LITTLE_ENDIAN
        LSB_FIRST C68K_NO_JUMP_TABLE NO_DYNAREC
)

include(${CMAKE_SOURCE_DIR}/src/external/psflib/psflib.cmake)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR_${NAME}}/Core/*.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/Core/c68k/*.c
        ${EXTERNAL_SOURCE_DIR_psflib}/psf2fs.c #TODO needed?
        ${EXTERNAL_SOURCE_DIR_psflib}/psflib.c
        src/main.cpp        #TODO compilation strangely slow
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR_${NAME}}/Core
        ${EXTERNAL_SOURCE_DIR_${NAME}}/Core/c68k
        ${EXTERNAL_SOURCE_DIR_psflib}

        ${CMAKE_SOURCE_DIR}/src/app
        ${EXTERNAL_SOURCE_DIR_fmod}/api/core/inc
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        ZLIB::ZLIB
        -lshlwapi
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "codec_${NAME}"
        PREFIX ""
        SUFFIX ".dll"
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR_PARENT}/data/plugin
        COMPILE_FLAGS "-m32"
        LINK_FLAGS "-m32"
)
