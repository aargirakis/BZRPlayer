cmake_minimum_required(VERSION 3.28)

set(NAME "sc68")
set(VERSION "3.0.0b")
set(REVISION "r685") # original sourceforge svn revision number
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "${NAME}" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION} (${REVISION})" PARENT_SCOPE)

find_package(ZLIB REQUIRED)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(NAME_VERSIONED "${NAME}-${VERSION}-${REVISION}")
set(FILENAME "${NAME}-${VERSION}.tar.gz")
set(FILENAME_URL "${VERSION}.tar.gz")
# since sourceforge (https://sourceforge.net/p/sc68/code/HEAD) doesn't provide a stable/permanent tarball url,
# an (outdated) github mirror it is used here
set(URL "https://github.com/Zeinok/${NAME}/archive/refs/tags/${FILENAME_URL}")
set(SHA_256_HASH "468f78560dd8db8723182e6098f15b3faaed8c7f3637efdcdbef1186832ffb38")
set(UNPACKED_DIR "${NAME}-${VERSION}")
download_patch_and_add(
        ${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} true ${UNPACKED_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/patches #TODO api68.c.patch is needed?
)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/file68/src/*.c
        ${EXTERNAL_SOURCE_DIR}/libsc68/dial68/*.c
        ${EXTERNAL_SOURCE_DIR}/libsc68/emu68/*.c
        ${EXTERNAL_SOURCE_DIR}/libsc68/io68/*.c
        ${EXTERNAL_SOURCE_DIR}/libsc68/src/*.c
        src/main.cpp)

list(REMOVE_ITEM SOURCES
        ${EXTERNAL_SOURCE_DIR}/libsc68/emu68/cc68.c
        ${EXTERNAL_SOURCE_DIR}/libsc68/emu68/lines68.c
        ${EXTERNAL_SOURCE_DIR}/libsc68/io68/ym_atarist_table.c
        ${EXTERNAL_SOURCE_DIR}/libsc68/io68/ym_linear_table.c)

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
        ${FMOD_API_DIR})

set(SC68_DATA_DIR "${PLUGINS_DIR}/sc68")
target_compile_definitions(${PROJECT_NAME} PRIVATE SC68_DATA_DIR="${SC68_DATA_DIR}")
target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_ASSERT_H)
target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_FCNTL_H)
target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_FREE)
target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_MALLOC)
target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_STDINT_H)
target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_STRING_H)
target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_ZLIB_H)
target_compile_definitions(${PROJECT_NAME} PRIVATE PACKAGE_BUGREPORT="")
target_compile_definitions(${PROJECT_NAME} PRIVATE PACKAGE_STRING="")
target_compile_definitions(${PROJECT_NAME} PRIVATE PACKAGE_URL="")
target_compile_definitions(${PROJECT_NAME} PRIVATE PACKAGE_VERNUM)

if (NOT WIN32) #TODO
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:C>:-Wno-implicit-function-declaration>)
endif ()

target_link_libraries(${PROJECT_NAME} PRIVATE ZLIB::ZLIB)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
)

cmake_path(SET SC68_DATA_PATH NORMALIZE ${OUTPUT_DIR}/${DATA_DIR}/${SC68_DATA_DIR})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        ${EXTERNAL_SOURCE_DIR}/file68/data68/Replay ${SC68_DATA_PATH}/Replay
        VERBATIM)