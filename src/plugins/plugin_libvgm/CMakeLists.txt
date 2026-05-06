cmake_minimum_required(VERSION 3.28)

set(NAME "libvgm")
set(VERSION "d1151884997ba4769b070b7c7aaedeffc60a25fb")
string(SUBSTRING ${VERSION} 0 12 VERSION_SHORT)
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "${NAME}" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION_SHORT}" PARENT_SCOPE)

find_package(ZLIB REQUIRED)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(NAME_VERSIONED "${NAME}-${VERSION_SHORT}")
set(FILENAME "${NAME}-${VERSION}.tar.gz")
set(FILENAME_URL "${VERSION}.tar.gz")
set(URL "https://github.com/ValleyBell/${NAME}/archive/${FILENAME_URL}")
set(SHA_256_HASH "7af2e58bc1bef990c0ee759cdef7bdf5e1e1ebc9d16cde489932b0370c0e7fbc")
set(UNPACKED_DIR "${NAME}-${VERSION}")
download_patch_and_add(
        ${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} true ${UNPACKED_DIR} ""
)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/emu/*.c
        ${EXTERNAL_SOURCE_DIR}/emu/cores/*.c
        ${EXTERNAL_SOURCE_DIR}/emu/cores/*.cpp
        ${EXTERNAL_SOURCE_DIR}/emu/cores/ymfm/*.cpp
        ${EXTERNAL_SOURCE_DIR}/player/*.c
        ${EXTERNAL_SOURCE_DIR}/player/*.cpp
        ${EXTERNAL_SOURCE_DIR}/utils/DataLoader.c
        ${EXTERNAL_SOURCE_DIR}/utils/MemoryLoader.c
        src/main.cpp)

if (WIN32)
    list(APPEND SOURCES ${EXTERNAL_SOURCE_DIR}/utils/StrUtils-CPConv_Win.c)
else ()
    list(APPEND SOURCES ${EXTERNAL_SOURCE_DIR}/utils/StrUtils-CPConv_IConv.c)
endif ()

list(REMOVE_ITEM SOURCES
        ${EXTERNAL_SOURCE_DIR}/emu/cores/adlibemu_opl_inc.c
        ${EXTERNAL_SOURCE_DIR}/emu/cores/scsplfo.c)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}

        ${CMAKE_SOURCE_DIR}/src/app
        ${FMOD_API_DIR})

target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_STDINT_H)
target_compile_definitions(${PROJECT_NAME} PRIVATE VGM_LITTLE_ENDIAN)
target_compile_definitions(${PROJECT_NAME} PRIVATE PLUGINS_CONFIG_DIR="${PLUGINS_CONFIG_DIR}")

target_link_libraries(${PROJECT_NAME} PRIVATE ZLIB::ZLIB)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
)
