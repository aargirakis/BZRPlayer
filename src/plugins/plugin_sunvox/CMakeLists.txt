cmake_minimum_required(VERSION 3.28)

set(NAME "sunvox")
set(VERSION "2.1.1c")
project(plugin_${NAME})
set(PLUGIN_${NAME}_DLL "${PROJECT_NAME}.dll" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "SunVox" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(LIB_NAME "${NAME}")
set(LIB_VERSION "${VERSION}")
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION}")
set(LIB_FILENAME "${LIB_NAME}_lib-${LIB_VERSION}.zip")
set(LIB_URL "https://warmplace.ru/soft/${LIB_NAME}/${LIB_FILENAME}")
set(LIB_SHA_256_HASH "9c57bb37b47182520995e9c154d5bc65bf0e02fdba03111602e01b7928879cfb")
set(LIB_UNPACKED_DIR "sunvox_lib")
download_patch_and_add(
        ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH} true ${LIB_UNPACKED_DIR} ""
)

add_compile_definitions(SUNVOX_MAIN)

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
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/data/plugin
        COMPILE_FLAGS "-m32"
        LINK_FLAGS "-m32"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${EXTERNAL_SOURCE_DIR}/windows/lib_x86/sunvox.dll
        ${OUTPUT_DIR}/data/plugin
        VERBATIM
)