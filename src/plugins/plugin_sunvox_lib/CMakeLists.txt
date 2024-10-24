cmake_minimum_required(VERSION 3.28)

set(NAME "sunvox_lib")
set(VERSION "2.1.2b")
project(plugin_${NAME})
set(PLUGIN_${NAME}_DLL "${PROJECT_NAME}.dll" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "SunVox Library" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(LIB_NAME "${NAME}")
set(LIB_VERSION "${VERSION}")
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION}")
set(LIB_FILENAME "${LIB_NAME_VERSIONED}.zip")
set(LIB_URL "https://warmplace.ru/soft/sunvox/${LIB_FILENAME}")
set(LIB_SHA_256_HASH "dc8ed3e7869eefe77a691bf4af6e20091691245649546f05c5893a1f38d9e34c")
set(LIB_UNPACKED_DIR "${LIB_NAME}")
download_patch_and_add(
        ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH} true ${LIB_UNPACKED_DIR} ""
)

add_compile_definitions(SUNVOX_MAIN)
add_compile_definitions(DATA_PLUGINS_DIR="${DATA_PLUGINS_DIR}")

file(GLOB SOURCES
        src/main.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/${LIB_NAME}/headers

        ${CMAKE_SOURCE_DIR}/src/app
        ${EXTERNAL_SOURCE_DIR_fmod}/api/core/inc
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DATA_PLUGINS_DIR}
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${EXTERNAL_SOURCE_DIR}/${LIB_NAME}/windows/lib_x86_64/sunvox.dll
        ${OUTPUT_DATA_PLUGINS_DIR}
        VERBATIM
)