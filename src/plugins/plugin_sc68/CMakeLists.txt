cmake_minimum_required(VERSION 3.28)

set(NAME "sc68")
set(VERSION "83cc8653e06cb4ba8c17b0c3e24b9cce4e461cfd")
string(SUBSTRING ${VERSION} 0 12 VERSION_SHORT)
set(REVISION "r100") # original sourceforge svn revision number
project(plugin_${NAME})
set(PLUGIN_${NAME}_DLL "${PROJECT_NAME}.dll" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "${NAME}" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION_SHORT} (${REVISION})" PARENT_SCOPE)

find_package(ZLIB REQUIRED)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION GREATER_EQUAL 14)
    add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-implicit-function-declaration>)
endif ()

set(LIB_NAME "${NAME}")
set(LIB_VERSION "${VERSION}")
set(LIB_VERSION_SHORT "${VERSION_SHORT}")
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION_SHORT}-${REVISION}")
set(LIB_FILENAME "${LIB_NAME}-${LIB_VERSION}.zip")
set(LIB_FILENAME_URL "${LIB_VERSION}.zip")
# since sourceforge (https://sourceforge.net/p/sc68/code/HEAD) doesn't provide a stable/permanent tarball url,
# an (outdated) github mirror it is used here
set(LIB_URL "https://github.com/Zeinok/${LIB_NAME}/archive/${LIB_FILENAME_URL}")
set(LIB_SHA_256_HASH "29c6eddc372624871c8b6ff63acaf4ef774ff248545c769d706c1bad036afda0")
set(LIB_UNPACKED_DIR "${LIB_NAME}-${LIB_VERSION}")
download_patch_and_add(
        ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH} true ${LIB_UNPACKED_DIR} ""
)

set(SC68_DATA_PATH "/data/plugin/sc68")
add_compile_definitions(SC68_DATA_PATH="${SC68_DATA_PATH}")
add_compile_definitions(HAVE_STDINT_H)
add_compile_definitions(HAVE_ZLIB_H)
add_compile_definitions(PACKAGE_BUGREPORT="")
add_compile_definitions(PACKAGE_STRING="")
add_compile_definitions(PACKAGE_URL="")
add_compile_definitions(PACKAGE_VERNUM)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/file68/*.c
        ${EXTERNAL_SOURCE_DIR}/libsc68/*.c
        ${EXTERNAL_SOURCE_DIR}/libsc68/emu68/*.c
        ${EXTERNAL_SOURCE_DIR}/libsc68/io68/*.c
        ${EXTERNAL_SOURCE_DIR}/unice68/*.c
        src/main.cpp
)

list(REMOVE_ITEM SOURCES
        ${EXTERNAL_SOURCE_DIR}/libsc68/io68/ym_atarist_table.c
        ${EXTERNAL_SOURCE_DIR}/libsc68/io68/ym_linear_table.c
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}
        ${EXTERNAL_SOURCE_DIR}/file68
        ${EXTERNAL_SOURCE_DIR}/file68/sc68
        ${EXTERNAL_SOURCE_DIR}/libsc68
        ${EXTERNAL_SOURCE_DIR}/libsc68/emu68
        ${EXTERNAL_SOURCE_DIR}/libsc68/io68
        ${EXTERNAL_SOURCE_DIR}/libsc68/sc68
        ${EXTERNAL_SOURCE_DIR}/unice68

        ${CMAKE_SOURCE_DIR}/src/app
        ${EXTERNAL_SOURCE_DIR_fmod}/api/core/inc
)

target_link_libraries(${PROJECT_NAME} PRIVATE ZLIB::ZLIB)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/data/plugin
        COMPILE_FLAGS "-m32"
        LINK_FLAGS "-m32"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        ${EXTERNAL_SOURCE_DIR}/file68/data68/Replay
        ${OUTPUT_DIR}${SC68_DATA_PATH}/Replay
        VERBATIM
)