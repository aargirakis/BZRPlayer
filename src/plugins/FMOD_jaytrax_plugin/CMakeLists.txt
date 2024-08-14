cmake_minimum_required(VERSION 3.28)

set(NAME "jaytrax")
project(plugin_${NAME})

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

set(LIB_NAME "${NAME}")
set(LIB_VERSION "1c2a999014058ef04973c4f3fac8bdf61a3fa9a4")
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION}")
set(LIB_FILENAME "${LIB_VERSION}.zip")
set(LIB_URL "https://github.com/pachuco/${LIB_NAME}/archive/${LIB_FILENAME}")
set(LIB_SHA_256_HASH "f65a4b33a060acca1294d35a57a8bbfda39af452daff76edcddb76d85b9468e3")
download_and_patch(
        ${LIB_NAME} ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH}
        ""
)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/lib_oldjaytrax/*.c
        src/main.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/lib_oldjaytrax

        ${CMAKE_SOURCE_DIR}/src/app
        ${CMAKE_SOURCE_DIR}/src/external/FMOD/api/core/inc
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "codec_${NAME}"
        PREFIX ""
        SUFFIX ".dll"
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR_PARENT}/data/plugin
        COMPILE_FLAGS "-m32"
        LINK_FLAGS "-m32"
)
