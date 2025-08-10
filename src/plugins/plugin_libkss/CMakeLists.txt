cmake_minimum_required(VERSION 3.28)

set(NAME "libkss")
set(VERSION "1.2.1")
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "LIBKSS" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

add_subdirectory(deps/emu2149)
add_subdirectory(deps/emu2212)
add_subdirectory(deps/emu2413)
add_subdirectory(deps/emu8950)
add_subdirectory(deps/emu76489)
add_subdirectory(deps/kmz80)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(NAME_VERSIONED "${NAME}-${VERSION}")
set(FILENAME "${NAME_VERSIONED}.tar.gz")
set(FILENAME_URL "${VERSION}.tar.gz")
set(URL "https://github.com/digital-sound-antiques/${NAME}/archive/refs/tags/${FILENAME_URL}")
set(SHA_256_HASH "303a0948350a137630a69658d658e9c11b42832588768b9f60c19b8586348c76")
download_patch_and_add(
        ${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} false ""
        ${CMAKE_CURRENT_SOURCE_DIR}/patches
)

set(EXTERNAL_SOURCE_DIR_${NAME} "${EXTERNAL_SOURCE_DIR}")

set(DRV_NAME "kss-drivers")
set(DRV_VERSION "e8032da38ea8439caca699954fce82b5050bf86f")
string(SUBSTRING ${DRV_VERSION} 0 12 DRV_VERSION_SHORT)
set(DRV_NAME_VERSIONED "${DRV_NAME}-${DRV_VERSION_SHORT}")
set(DRV_FILENAME "${DRV_NAME}-${DRV_VERSION}.tar.gz")
set(DRV_FILENAME_URL "${DRV_VERSION}.tar.gz")
set(DRV_URL "https://github.com/digital-sound-antiques/${DRV_NAME}/archive/${DRV_FILENAME_URL}")
set(DRV_SHA_256_HASH "533cc994c0b4b85bcecedd98351a981204779833d89d49577e619d3d5685d47e")
set(DRV_UNPACKED_DIR "${DRV_NAME}-${DRV_VERSION}")
download_patch_and_add(
        ${DRV_NAME_VERSIONED} ${DRV_FILENAME} ${DRV_URL} ${DRV_SHA_256_HASH} true ${DRV_UNPACKED_DIR} ""
)

set(EXTERNAL_SOURCE_DIR_${DRV_NAME} "${EXTERNAL_SOURCE_DIR}")

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/*.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/**/*.c
        src/main.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR_${DRV_NAME}}
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src
        ${EXTERNAL_SOURCE_DIR_${NAME}}/src/kss

        ${CMAKE_SOURCE_DIR}/src/app
        ${FMOD_API_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE
        emu2149 emu2212 emu2413 emu8950 emu76489 kmz80)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
)
