cmake_minimum_required(VERSION 3.28)

set(NAME "libfc14audiodecoder")
set(VERSION "1.0.4")
project(plugin_${NAME})
set(PLUGIN_${NAME}_DLL "${PROJECT_NAME}.dll" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "libfc14audiodecoder" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(LIB_NAME "${NAME}")
set(LIB_VERSION "${VERSION}")
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION}")
set(LIB_FILENAME "${LIB_NAME_VERSIONED}.tar.bz2")
set(LIB_URL "https://github.com/mschwendt/${LIB_NAME}/releases/download/${LIB_VERSION}/${LIB_FILENAME}")
set(LIB_SHA_256_HASH "6b507d2f3c14657b0a1775c920474f4864ff9c651f8f7a4c1ab8faa3cc8c1a1e")
download_patch_and_add(
        ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH} false ""
        ${CMAKE_CURRENT_SOURCE_DIR}/patches
)

set(SIZEOF_CHAR 1)
set(SIZEOF_INT 4)
set(SIZEOF_LONG_INT 4)
set(SIZEOF_SHORT_INT 2)
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
        src/main.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/src

        ${CMAKE_SOURCE_DIR}/src/app
        ${EXTERNAL_SOURCE_DIR_fmod}/api/core/inc
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DATA_PLUGINS_DIR}
)
