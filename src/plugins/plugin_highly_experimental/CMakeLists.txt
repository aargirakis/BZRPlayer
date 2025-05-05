cmake_minimum_required(VERSION 3.28)

set(NAME "highly_experimental")
set(VERSION "0fa96d186e3c0437951732d0c50bf1da4e32970e")
string(SUBSTRING ${VERSION} 0 12 VERSION_SHORT)
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "Highly Experimental" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION_SHORT}" PARENT_SCOPE)

include(${CMAKE_SOURCE_DIR}/src/external/psflib/psflib.cmake)

find_package(ZLIB REQUIRED)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(NAME_VERSIONED "${NAME}-${VERSION_SHORT}")
set(FILENAME "${NAME}-${VERSION}.zip")
set(URL "https://gitlab.com/kode54/${NAME}/-/archive/${VERSION}/${FILENAME}")
set(SHA_256_HASH "ed290bb4b1229b6c0c58408164d394aa38b7b666a4ac773c2ba151dde42de128")
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

        ${CMAKE_SOURCE_DIR}/src/app
        ${FMOD_API_DIR})

target_compile_definitions(${PROJECT_NAME} PRIVATE EMU_COMPILE EMU_LITTLE_ENDIAN)

target_compile_options(${PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:C>:-Wno-implicit-function-declaration>)

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
