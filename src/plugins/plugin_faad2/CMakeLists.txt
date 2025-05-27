cmake_minimum_required(VERSION 3.28)

set(NAME "faad2")
set(VERSION "2.11.2")
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "FAAD2" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(NAME_VERSIONED "${NAME}-${VERSION}")
set(FILENAME "${NAME_VERSIONED}.tar.gz")
set(FILENAME_URL "${VERSION}.tar.gz")
set(URL "https://github.com/knik0/${NAME}/archive/refs/tags/${FILENAME_URL}")
set(SHA_256_HASH "3fcbd305e4abd34768c62050e18ca0986f7d9c5eca343fb98275418013065c0e")
download_patch_and_add(
        ${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} false "" ""
)

set(HAVE_MEMCPY 1)
set(HAVE_STDINT_H 1)
configure_file(
        src/config.h.in
        ${EXTERNAL_SOURCE_DIR}/config.h
        @ONLY NEWLINE_STYLE UNIX
)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/libfaad/*.c
        src/main.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/include
        ${EXTERNAL_SOURCE_DIR}/libfaad

        ${CMAKE_SOURCE_DIR}/src/app
        ${FMOD_API_DIR})

target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_CONFIG_H=1)
target_compile_definitions(${PROJECT_NAME} PRIVATE PACKAGE_VERSION="${VERSION}")

target_compile_options(${PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:C>:-Wno-implicit-function-declaration>)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
)
