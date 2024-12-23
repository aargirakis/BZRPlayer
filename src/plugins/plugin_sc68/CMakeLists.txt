cmake_minimum_required(VERSION 3.28)

set(NAME "sc68")
set(VERSION "3.0.0b")
set(REVISION "r685") # original sourceforge svn revision number
project(plugin_${NAME})
set(PLUGIN_${NAME}_DLL "${PROJECT_NAME}.dll" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "${NAME}" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION} (${REVISION})" PARENT_SCOPE)

find_package(ZLIB REQUIRED)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(LIB_NAME "${NAME}")
set(LIB_VERSION "${VERSION}")
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION}-${REVISION}")
set(LIB_FILENAME "${LIB_NAME}-${LIB_VERSION}.tar.gz")
set(LIB_FILENAME_URL "${LIB_VERSION}.tar.gz")
# since sourceforge (https://sourceforge.net/p/sc68/code/HEAD) doesn't provide a stable/permanent tarball url,
# an (outdated) github mirror it is used here
set(LIB_URL "https://github.com/Zeinok/${LIB_NAME}/archive/refs/tags/${LIB_FILENAME_URL}")
set(LIB_SHA_256_HASH "468f78560dd8db8723182e6098f15b3faaed8c7f3637efdcdbef1186832ffb38")
set(LIB_UNPACKED_DIR "${LIB_NAME}-${LIB_VERSION}")
download_patch_and_add(
        ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH} true ${LIB_UNPACKED_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/patches #TODO api68.c.patch is needed?
)

set(SC68_DATA_PATH "${DATA_PLUGINS_DIR}/sc68")
add_compile_definitions(SC68_DATA_PATH="${SC68_DATA_PATH}")
add_compile_definitions(HAVE_ASSERT_H)
add_compile_definitions(HAVE_FCNTL_H)
add_compile_definitions(HAVE_FREE)
add_compile_definitions(HAVE_MALLOC)
add_compile_definitions(HAVE_STDINT_H)
add_compile_definitions(HAVE_STRING_H)
add_compile_definitions(HAVE_ZLIB_H)
add_compile_definitions(PACKAGE_BUGREPORT="")
add_compile_definitions(PACKAGE_STRING="")
add_compile_definitions(PACKAGE_URL="")
add_compile_definitions(PACKAGE_VERNUM)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/file68/src/*.c
        ${EXTERNAL_SOURCE_DIR}/libsc68/dial68/*.c
        ${EXTERNAL_SOURCE_DIR}/libsc68/emu68/*.c
        ${EXTERNAL_SOURCE_DIR}/libsc68/io68/*.c
        ${EXTERNAL_SOURCE_DIR}/libsc68/src/*.c
        src/main.cpp
)

list(REMOVE_ITEM SOURCES
        ${EXTERNAL_SOURCE_DIR}/libsc68/emu68/cc68.c
        ${EXTERNAL_SOURCE_DIR}/libsc68/emu68/lines68.c
        ${EXTERNAL_SOURCE_DIR}/libsc68/io68/ym_atarist_table.c
        ${EXTERNAL_SOURCE_DIR}/libsc68/io68/ym_linear_table.c
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}
        ${EXTERNAL_SOURCE_DIR}/file68
        ${EXTERNAL_SOURCE_DIR}/file68/sc68
        ${EXTERNAL_SOURCE_DIR}/libsc68
        ${EXTERNAL_SOURCE_DIR}/libsc68/dial68
        ${EXTERNAL_SOURCE_DIR}/libsc68/emu68
        ${EXTERNAL_SOURCE_DIR}/libsc68/io68
        ${EXTERNAL_SOURCE_DIR}/libsc68/sc68
        ${EXTERNAL_SOURCE_DIR}/sc68-libc
        ${EXTERNAL_SOURCE_DIR}/sc68-msvc
        ${EXTERNAL_SOURCE_DIR}/sc68-msvc/file68
        ${EXTERNAL_SOURCE_DIR}/sc68-msvc/sc68

        ${CMAKE_SOURCE_DIR}/src/app
        ${EXTERNAL_SOURCE_DIR_fmod}/api/core/inc
)

target_link_libraries(${PROJECT_NAME} PRIVATE ZLIB::ZLIB)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DATA_PLUGINS_DIR}
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        ${EXTERNAL_SOURCE_DIR}/file68/data68/Replay
        ${OUTPUT_DIR}${SC68_DATA_PATH}/Replay
        VERBATIM
)