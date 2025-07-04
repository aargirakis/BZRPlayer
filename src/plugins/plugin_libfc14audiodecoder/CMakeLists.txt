cmake_minimum_required(VERSION 3.28)

set(NAME "libfc14audiodecoder")
set(VERSION "1.0.6")
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "libfc14audiodecoder" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(NAME_VERSIONED "${NAME}-${VERSION}")
set(FILENAME "${NAME_VERSIONED}.tar.bz2")
set(URL "https://github.com/mschwendt/${NAME}/releases/download/${VERSION}/${FILENAME}")
set(SHA_256_HASH "61dca3499f9b4c231fb89a57a652957f96ce375eb965425f3f5a1ec4064e6a45")
download_patch_and_add(
        ${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} false "" "")

configure_file(
        ${EXTERNAL_SOURCE_DIR}/src/Config.h.in
        ${EXTERNAL_SOURCE_DIR}/src/Config.h
        @ONLY NEWLINE_STYLE UNIX
)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/src/fc14audiodecoder.cpp
        ${EXTERNAL_SOURCE_DIR}/src/FC.cpp
        ${EXTERNAL_SOURCE_DIR}/src/FC_Data.cpp
        ${EXTERNAL_SOURCE_DIR}/src/LamePaulaMixer.cpp
        ${EXTERNAL_SOURCE_DIR}/src/LamePaulaVoice.cpp
        ${EXTERNAL_SOURCE_DIR}/src/Paula.cpp
        src/main.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/src

        ${CMAKE_SOURCE_DIR}/src/app
        ${FMOD_API_DIR})

target_compile_definitions(${PROJECT_NAME} PRIVATE FC_API_EXT_1)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
)
