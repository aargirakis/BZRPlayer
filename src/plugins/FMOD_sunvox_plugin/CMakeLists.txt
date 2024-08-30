cmake_minimum_required(VERSION 3.28)

set(NAME "sunvox")
project(plugin_${NAME})

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

set(LIB_NAME "${NAME}")
set(LIB_VERSION "2.1c")
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION}")
set(LIB_FILENAME "${LIB_NAME}_lib-${LIB_VERSION}.zip")
set(LIB_URL "https://web.archive.org/web/20230329102810/https://warmplace.ru/soft/${LIB_NAME}/${LIB_FILENAME}")
set(LIB_SHA_256_HASH "5856adc13bb0105b5d5b0422d2cbb5fb5c74831fd7f52412d922ed35cd17bbca")
set(LIB_UNPACKED_DIR "sunvox_lib")
download_and_patch(
        ${LIB_NAME} ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH} ${LIB_UNPACKED_DIR}
        ""
)

file(GLOB SOURCES
        src/main.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/headers

        ${CMAKE_SOURCE_DIR}/src/app
        ${EXTERNAL_SOURCE_DIR_fmod}/api/core/inc
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "codec_${NAME}"
        PREFIX ""
        SUFFIX ".dll"
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR_PARENT}/data/plugin
        COMPILE_FLAGS "-m32"
        LINK_FLAGS "-m32"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${EXTERNAL_SOURCE_DIR}/windows/lib_x86/sunvox.dll
        ${OUTPUT_DIR_PARENT}/data/plugin
        VERBATIM
)