cmake_minimum_required(VERSION 3.28)

set(NAME "vio2sf")
project(plugin_${NAME})

find_package(ZLIB)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

add_compile_options($<$<COMPILE_LANGUAGE:C>:-msse4.1>)

set(LIB_NAME "${NAME}")
set(LIB_VERSION "1d68801f5fd370c3275affd51505353e2b366a7e")
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION}")
set(LIB_FILENAME "${LIB_NAME_VERSIONED}.tar.bz2")
set(LIB_URL "https://gitlab.com/kode54/${LIB_NAME}/-/archive/${LIB_VERSION}/${LIB_FILENAME}")
set(LIB_SHA_256_HASH "471cf5cd1c54f2dfd8c863a6e241a80bcbde7b7a9d419f3dae6ebc20e35b7176")
download_and_patch(
        ${LIB_NAME} ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH} ""
        ""
)

set(EXTERNAL_SOURCE_DIR_${NAME} "${EXTERNAL_SOURCE_DIR}")

add_compile_definitions(BARRAY_DECORATE RESAMPLER_DECORATE)

include(${CMAKE_SOURCE_DIR}/src/external/psflib/psflib.cmake)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/vio2sf/desmume/*.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/vio2sf/desmume/*.cpp
        ${EXTERNAL_SOURCE_DIR_psflib}/psf2fs.c  #TODO needed?
        ${EXTERNAL_SOURCE_DIR_psflib}/psflib.c
        src/main.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/vio2sf/desmume
        ${EXTERNAL_SOURCE_DIR_psflib}

        ${CMAKE_SOURCE_DIR}/src/app
        ${EXTERNAL_SOURCE_DIR_fmod}/api/core/inc
)

target_link_libraries(${PROJECT_NAME} PRIVATE ZLIB::ZLIB)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "codec_${NAME}"
        PREFIX ""
        SUFFIX ".dll"
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR_PARENT}/data/plugin
        COMPILE_FLAGS "-m32"
        LINK_FLAGS "-m32"
)
