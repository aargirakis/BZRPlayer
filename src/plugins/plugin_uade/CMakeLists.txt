cmake_minimum_required(VERSION 3.28)

set(NAME "uade")
set(VERSION "6cb14781463b949ace24c16b0d511461b7454d0e")
string(SUBSTRING ${VERSION} 0 12 VERSION_SHORT)
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "UADE (Matti)" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION_SHORT}" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(NAME_VERSIONED "${NAME}-${VERSION_SHORT}")
set(FILENAME "${NAME}-dev.tar.gz")
set(URL "https://gitlab.com/mvtiaine/${NAME}/-/archive/dev/${NAME}-dev.tar.gz")
set(SHA_256_HASH "99bcd3f2d4b5876d916c22a377c71d852551a45a7dccb50bab5c02a5dcda9f37")
set(UNPACKED_DIR "${NAME}-dev")
set(CONFIGURE_COMMAND "configure CFLAGS=\"-DDISABLE_BENCODETOOLS -DDISABLE_ZAKALWE\" \
--without-uade123 --without-uadefs --without-uadesimple --without-write-audio --without-avx2 \
--bencode-tools-prefix=/dev/null --libzakalwe-prefix=/dev/null")
set(BUILD_BYPRODUCTS "src/frontends/libuade/libuade.a")
download_patch_and_make(
        ${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} true ${UNPACKED_DIR} "" ${CONFIGURE_COMMAND} "" ${BUILD_BYPRODUCTS}
)

#TODO disable vasm compilation
#TODO case insensitivity on linux?

file(GLOB SOURCES
        src/flod/*.cpp #TODO
        src/main.cpp

        ${CMAKE_SOURCE_DIR}/src/app/BaseRow.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

add_dependencies(${PROJECT_NAME} ${NAME_VERSIONED})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/src/frontends/include

        src/flod  #TODO

        ${CMAKE_SOURCE_DIR}/src/app
        ${FMOD_API_DIR})

set(UADE_DATA_DIR "${PLUGINS_DIR}/uade")
set(PLUGIN_${NAME}_DIR "${UADE_DATA_DIR}" PARENT_SCOPE)
set(UADE_SONGLENGTHS "uade.md5")

if (WIN32)
    set(UADE_CORE "uadecore.exe")
else ()
    set(UADE_CORE "uadecore")
endif ()

target_compile_definitions(${PROJECT_NAME} PRIVATE UADE_DATA_DIR="${UADE_DATA_DIR}")
target_compile_definitions(${PROJECT_NAME} PRIVATE UADE_CORE="${UADE_CORE}")
target_compile_definitions(${PROJECT_NAME} PRIVATE PLUGINS_CONFIG_DIR="${PLUGINS_CONFIG_DIR}")
target_compile_definitions(${PROJECT_NAME} PRIVATE EMSCRIPTEN)

add_library(LIBUADE_STATIC STATIC IMPORTED GLOBAL)

set_target_properties(LIBUADE_STATIC PROPERTIES
        IMPORTED_LOCATION "${EXTERNAL_SOURCE_DIR}/${BUILD_BYPRODUCTS}"
)

target_link_libraries(${PROJECT_NAME} PRIVATE LIBUADE_STATIC -L"${FMOD_DIR}" -lfmod)

if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE -lws2_32)
endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
)

cmake_path(SET UADE_DATA_PATH NORMALIZE ${OUTPUT_DIR}/${DATA_DIR}/${UADE_DATA_DIR})

#TODO song.conf
#TODO uadecore from uade is 5mb from audacious-uade is 1mb
#TODO put uadecore in lib folder
#TODO compare players & eagleplayer.conf

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        ${EXTERNAL_SOURCE_DIR}/players ${UADE_DATA_PATH}/players
        #
        #        #TODO where they come from?
        #        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        #        ${CMAKE_CURRENT_SOURCE_DIR}/dist/players ${UADE_DATA_PATH}/players
        #

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${EXTERNAL_SOURCE_DIR}/amigasrc/score/score
        ${EXTERNAL_SOURCE_DIR}/eagleplayer.conf
        ${EXTERNAL_SOURCE_DIR}/uade.conf
        ${EXTERNAL_SOURCE_DIR}/src/${UADE_CORE}
        ${EXTERNAL_SOURCE_DIR}/uaerc
        ${CMAKE_CURRENT_SOURCE_DIR}/dist/${UADE_SONGLENGTHS}
        ${UADE_DATA_PATH}
        VERBATIM)
