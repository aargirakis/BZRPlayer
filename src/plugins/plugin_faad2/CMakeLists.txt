cmake_minimum_required(VERSION 3.28)

set(NAME "faad2")
set(VERSION "2.11.2")
project(plugin_${NAME})
set(PLUGIN_${NAME}_DLL "${PROJECT_NAME}.dll" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "FAAD2" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION GREATER_EQUAL 14)
    add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-implicit-function-declaration>)
endif ()

set(LIB_NAME "${NAME}")
set(LIB_VERSION "${VERSION}")
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION}")
set(LIB_FILENAME "${LIB_NAME_VERSIONED}.tar.gz")
set(LIB_FILENAME_URL "${LIB_VERSION}.tar.gz")
set(LIB_URL "https://github.com/knik0/${LIB_NAME}/archive/refs/tags/${LIB_FILENAME_URL}")
set(LIB_SHA_256_HASH "3fcbd305e4abd34768c62050e18ca0986f7d9c5eca343fb98275418013065c0e")
download_patch_and_add(
        ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH} false "" ""
)

add_compile_definitions(HAVE_CONFIG_H=1)
set(HAVE_MEMCPY 1)
configure_file(
        src/config.h.in
        ${EXTERNAL_SOURCE_DIR}/config.h
        @ONLY NEWLINE_STYLE UNIX
)

add_compile_definitions(PACKAGE_VERSION="${LIB_VERSION}")

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/libfaad/*.c
        src/main.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/include
        ${EXTERNAL_SOURCE_DIR}/libfaad

        ${CMAKE_SOURCE_DIR}/src/app
        ${EXTERNAL_SOURCE_DIR_fmod}/api/core/inc
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DATA_PLUGINS_DIR}
)
