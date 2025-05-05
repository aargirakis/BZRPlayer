cmake_minimum_required(VERSION 3.28)

set(NAME "sc68")
set(VERSION "a3eac9974c8d261e2e8cd8bdc0872e05269f8659")
string(SUBSTRING ${VERSION} 0 12 VERSION_SHORT)
set(REVISION "r106") # original sourceforge svn revision number
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "${NAME}" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION_SHORT} (${REVISION})" PARENT_SCOPE)

find_package(ZLIB REQUIRED)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(NAME_VERSIONED "${NAME}-${VERSION_SHORT}-${REVISION}")
set(FILENAME "${NAME}-${VERSION}.zip")
set(FILENAME_URL "${VERSION}.zip")
# since sourceforge (https://sourceforge.net/p/sc68/code/HEAD) doesn't provide a stable/permanent tarball url,
# an (outdated) github mirror it is used here
set(URL "https://github.com/Zeinok/${NAME}/archive/${FILENAME_URL}")
set(SHA_256_HASH "67a7bed2b4bff56eae543bb90c4f44bb08544ef0e5752b6eb3f1eb569ccf2104")
set(UNPACKED_DIR "${NAME}-${VERSION}")
download_patch_and_add(
        ${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} true ${UNPACKED_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/patches
)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/file68/src/*.c
        ${EXTERNAL_SOURCE_DIR}/libsc68/*.c
        ${EXTERNAL_SOURCE_DIR}/libsc68/emu68/*.c
        ${EXTERNAL_SOURCE_DIR}/libsc68/io68/*.c
        ${EXTERNAL_SOURCE_DIR}/unice68/*.c
        src/main.cpp)

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
        ${FMOD_API_DIR})

set(SC68_DATA_DIR "${PLUGINS_DIR}/sc68")
target_compile_definitions(${PROJECT_NAME} PRIVATE SC68_DATA_DIR="${SC68_DATA_DIR}")
target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_FREE)
target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_MALLOC)
target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_STDINT_H)
target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_STRING_H)
target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_ZLIB_H)
target_compile_definitions(${PROJECT_NAME} PRIVATE PACKAGE_BUGREPORT="")
target_compile_definitions(${PROJECT_NAME} PRIVATE PACKAGE_STRING="")
target_compile_definitions(${PROJECT_NAME} PRIVATE PACKAGE_URL="")
target_compile_definitions(${PROJECT_NAME} PRIVATE PACKAGE_VERNUM)

target_link_libraries(${PROJECT_NAME} PRIVATE ZLIB::ZLIB)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${DATA_DIR}/${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${DATA_DIR}/${PLUGINS_DIR}
)

cmake_path(SET SC68_DATA_PATH NORMALIZE ${OUTPUT_DIR}/${DATA_DIR}/${SC68_DATA_DIR})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        ${EXTERNAL_SOURCE_DIR}/file68/data68/Replay ${SC68_DATA_PATH}/Replay
        VERBATIM)