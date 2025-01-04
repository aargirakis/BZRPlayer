cmake_minimum_required(VERSION 3.28)

set(NAME "webken")
set(VERSION "e698e5a25db45a3377a82278206d3e1d43498758")
string(SUBSTRING ${VERSION} 0 12 VERSION_SHORT)
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "webKen" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION_SHORT}" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(VERSION_SHORT "${VERSION_SHORT}")
set(NAME_VERSIONED "${NAME}-${VERSION_SHORT}")
set(NAME_VERSIONED2 "wothke-${NAME}-${VERSION_SHORT}")
set(FILENAME "${NAME_VERSIONED2}.zip")
set(FILENAME_URL "${VERSION}.zip")
set(URL "https://bitbucket.org/wothke/${NAME}/get/${FILENAME_URL}")
set(SHA_256_HASH "6650834a7119b93b103f25a9a968bf77b31f6bf1c471457fd7cefd3277280c7c")
set(UNPACKED_DIR "${NAME_VERSIONED2}")
download_patch_and_add(
        ${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} true ${UNPACKED_DIR} ""
)

set(EXTERNAL_SOURCE_DIR_${NAME} "${EXTERNAL_SOURCE_DIR}")

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/ken/*.c
        src/main.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/src/app
        ${FMOD_API_DIR})

target_compile_definitions(${PROJECT_NAME} PRIVATE EMSCRIPTEN)

target_compile_options(${PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:C>:-Wno-implicit-function-declaration>)
target_compile_options(${PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:C>:-Wno-int-conversion>)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
)
