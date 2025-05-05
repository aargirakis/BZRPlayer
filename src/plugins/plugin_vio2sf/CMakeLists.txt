cmake_minimum_required(VERSION 3.28)

set(NAME "vio2sf")
set(VERSION "1d68801f5fd370c3275affd51505353e2b366a7e")
string(SUBSTRING ${VERSION} 0 12 VERSION_SHORT)
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "${NAME}" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION_SHORT}" PARENT_SCOPE)

include(${CMAKE_SOURCE_DIR}/src/external/psflib/psflib.cmake)

find_package(ZLIB REQUIRED)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(NAME_VERSIONED "${NAME}-${VERSION_SHORT}")
set(FILENAME "${NAME}-${VERSION}.tar.bz2")
set(URL "https://gitlab.com/kode54/${NAME}/-/archive/${VERSION}/${FILENAME}")
set(SHA_256_HASH "471cf5cd1c54f2dfd8c863a6e241a80bcbde7b7a9d419f3dae6ebc20e35b7176")
set(UNPACKED_DIR "${NAME}-${VERSION}")
download_patch_and_add(
        ${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} true ${UNPACKED_DIR} ""
)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/src/vio2sf/desmume/*.c
        ${EXTERNAL_SOURCE_DIR}/src/vio2sf/desmume/*.cpp
        ${EXTERNAL_SOURCE_DIR_psflib}/psf2fs.c  #TODO needed?
        ${EXTERNAL_SOURCE_DIR_psflib}/psflib.c
        src/main.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/src/vio2sf/desmume
        ${EXTERNAL_SOURCE_DIR_psflib}

        ${CMAKE_SOURCE_DIR}/src/app
        ${FMOD_API_DIR})

target_compile_definitions(${PROJECT_NAME} PRIVATE BARRAY_DECORATE RESAMPLER_DECORATE)

target_compile_options(${PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:C>:-msse4.1>)

target_link_libraries(${PROJECT_NAME} PRIVATE ZLIB::ZLIB)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${DATA_DIR}/${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${DATA_DIR}/${PLUGINS_DIR}
)
