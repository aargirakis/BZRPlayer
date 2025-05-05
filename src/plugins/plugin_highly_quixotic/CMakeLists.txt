cmake_minimum_required(VERSION 3.28)

set(NAME "highly_quixotic")
set(VERSION "d730174d436e8cabca90fabdb3ad4ddf614d31cc")
string(SUBSTRING ${VERSION} 0 12 VERSION_SHORT)
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "Highly Quixotic" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION_SHORT}" PARENT_SCOPE)

include(${CMAKE_SOURCE_DIR}/src/external/psflib/psflib.cmake)

find_package(ZLIB REQUIRED)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(NAME_VERSIONED "${NAME}-${VERSION_SHORT}")
set(FILENAME "${NAME}-${VERSION}.zip")
set(URL "https://gitlab.com/kode54/${NAME}/-/archive/${VERSION}/${FILENAME}")
set(SHA_256_HASH "6e25e219fe7877dfd871a500a6e03e95f15e6cf873289a275fa246eb7f58f918")
set(UNPACKED_DIR "${NAME}-${VERSION}")
download_patch_and_add(
        ${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} true ${UNPACKED_DIR} ""
)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/Core/*.c
        ${EXTERNAL_SOURCE_DIR_psflib}/psf2fs.c #TODO needed?
        ${EXTERNAL_SOURCE_DIR_psflib}/psflib.c
        src/main.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/Core
        ${EXTERNAL_SOURCE_DIR_psflib}
        src

        ${CMAKE_SOURCE_DIR}/src/app
        ${FMOD_API_DIR})

target_compile_definitions(${PROJECT_NAME} PRIVATE EMU_COMPILE EMU_LITTLE_ENDIAN)

target_compile_options(${PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fpermissive>)

target_link_libraries(${PROJECT_NAME} PRIVATE ZLIB::ZLIB)

if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE -lshlwapi)
endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${DATA_DIR}/${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${DATA_DIR}/${PLUGINS_DIR}
)
