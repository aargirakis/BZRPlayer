cmake_minimum_required(VERSION 3.28)

set(NAME "libsidplayfp")
set(VERSION "2.16.0")
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "${NAME}" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

add_subdirectory(deps/sidid)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(NAME_VERSIONED "${NAME}-${VERSION}")
set(FILENAME "${NAME_VERSIONED}.tar.gz")
set(URL "https://github.com/${NAME}/${NAME}/releases/download/v${VERSION}/${FILENAME}")
set(SHA_256_HASH "81bfd58ccaa3a0ef28c903b841b4fa3014811901fd12fbb9a80b7b5d5ec2b151")
download_patch_and_add(
        ${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} false "" ""
)

set(PACKAGE_VERSION "${VERSION}")
set(HAVE_BUILTIN_EXPECT 1)
set(RESID_BRANCH_HINTS 1)
set(RESID_INLINING 1)
set(RESID_INLINE "inline")
configure_file(
        ${EXTERNAL_SOURCE_DIR}/src/builders/residfp-builder/residfp/siddefs-fp.h.in
        ${EXTERNAL_SOURCE_DIR}/src/builders/residfp-builder/residfp/siddefs-fp.h
        @ONLY NEWLINE_STYLE UNIX
)

string(REPLACE "." ";" VERSION_LIST ${VERSION})
list(GET VERSION_LIST 0 VERSION_MAJOR)
list(GET VERSION_LIST 1 VERSION_MINOR)
list(GET VERSION_LIST 2 VERSION_PATCH)
set(MAJOR "${VERSION_MAJOR}")
set(MINOR "${VERSION_MINOR}")
set(LEVEL "${VERSION_PATCH}")
configure_file(
        ${EXTERNAL_SOURCE_DIR}/src/sidplayfp/sidversion.h.in
        ${EXTERNAL_SOURCE_DIR}/src/sidplayfp/sidversion.h
        @ONLY NEWLINE_STYLE UNIX
)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/src/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/builders/residfp-builder/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/builders/residfp-builder/residfp/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/builders/residfp-builder/residfp/resample/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/c64/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/c64/CIA/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/c64/CPU/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/c64/VIC_II/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/sidplayfp/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/sidtune/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/utils/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/utils/MD5/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/utils/STILview/*.cpp
        src/main.cpp)

list(REMOVE_ITEM SOURCES
        ${EXTERNAL_SOURCE_DIR}/src/builders/residfp-builder/residfp/resample/test.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/src
        ${EXTERNAL_SOURCE_DIR}/src/builders/residfp-builder
        ${EXTERNAL_SOURCE_DIR}/src/builders/residfp-builder/residfp
        ${EXTERNAL_SOURCE_DIR}/src/c64
        ${EXTERNAL_SOURCE_DIR}/src/c64/Banks
        ${EXTERNAL_SOURCE_DIR}/src/c64/CIA
        ${EXTERNAL_SOURCE_DIR}/src/c64/CPU
        ${EXTERNAL_SOURCE_DIR}/src/c64/VIC_II
        ${EXTERNAL_SOURCE_DIR}/src/sidplayfp
        ${EXTERNAL_SOURCE_DIR}/src/sidtune
        ${EXTERNAL_SOURCE_DIR}/src/utils
        ${EXTERNAL_SOURCE_DIR}/src/utils/MD5
        ${EXTERNAL_SOURCE_DIR}/src/utils/STILview

        deps/sidid/src

        ${CMAKE_SOURCE_DIR}/src/app
        ${FMOD_API_DIR})

target_compile_definitions(${PROJECT_NAME} PRIVATE RUNTIME_DISPATCH)

set(HVSC_SONGLENGTHS_FILENAME "Songlengths.md5")
set(SID_DATA_DIR "${PLUGINS_DIR}/sid")
set(HVSC_SONGLENGTHS_PATH "${SID_DATA_DIR}/${HVSC_SONGLENGTHS_FILENAME}")
set(PLUGIN_${NAME}_DIR "${SID_DATA_DIR}" PARENT_SCOPE)
set(PLUGIN_${NAME}_HVSC_SONGLENGTHS_PATH "${HVSC_SONGLENGTHS_PATH}" PARENT_SCOPE)
set(SIDID_CFG "sidid.cfg")
set(BASIC_BIN "basic-901226-01.bin")
set(CHARACTERS_BIN "chargen-901225-01.bin")
set(KERNAL_BIN "kernal-901227-03.bin")
set(BASIC_BIN_DATA_PATH "${SID_DATA_DIR}/${BASIC_BIN}")
set(CHARACTERS_BIN_DATA_PATH "${SID_DATA_DIR}/${CHARACTERS_BIN}")
set(KERNAL_BIN_DATA_PATH "${SID_DATA_DIR}/${KERNAL_BIN}")
set(SIDID_CFG_DATA_PATH "${SID_DATA_DIR}/${SIDID_CFG}")
target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_CXX20)
target_compile_definitions(${PROJECT_NAME} PRIVATE PACKAGE="${NAME}")
target_compile_definitions(${PROJECT_NAME} PRIVATE PACKAGE_URL="${URL}")
target_compile_definitions(${PROJECT_NAME} PRIVATE VERSION="${VERSION}")
target_compile_definitions(${PROJECT_NAME} PRIVATE BASIC_BIN_DATA_PATH="${BASIC_BIN_DATA_PATH}")
target_compile_definitions(${PROJECT_NAME} PRIVATE CHARACTERS_BIN_DATA_PATH="${CHARACTERS_BIN_DATA_PATH}")
target_compile_definitions(${PROJECT_NAME} PRIVATE HVSC_SONGLENGTHS_PATH="${HVSC_SONGLENGTHS_PATH}")
target_compile_definitions(${PROJECT_NAME} PRIVATE KERNAL_BIN_DATA_PATH="${KERNAL_BIN_DATA_PATH}")
target_compile_definitions(${PROJECT_NAME} PRIVATE PLUGINS_CONFIG_DIR="${PLUGINS_CONFIG_DIR}")
target_compile_definitions(${PROJECT_NAME} PRIVATE SID_DATA_DIR="${SID_DATA_DIR}")
target_compile_definitions(${PROJECT_NAME} PRIVATE SIDID_CFG_DATA_PATH="${SIDID_CFG_DATA_PATH}")

target_link_libraries(${PROJECT_NAME} PRIVATE sidid)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
)

cmake_path(SET SID_DATA_PATH NORMALIZE ${OUTPUT_DIR}${DATA_DIR}${SID_DATA_DIR})

set(BASIC_BIN_URL "https://sourceforge.net/p/vice-emu/code/HEAD/tree/tags/v3.10/vice/data/C64/${BASIC_BIN}?format=raw")
set(DESTINATION_PATH "${SID_DATA_PATH}")
download_to(${BASIC_BIN} ${BASIC_BIN_URL} "89878cea0a268734696de11c4bae593eaaa506465d2029d619c0e0cbccdfa62d" ${DESTINATION_PATH} true "")

set(CHARACTERS_BIN_URL "https://sourceforge.net/p/vice-emu/code/HEAD/tree/tags/v3.10/vice/data/C64/${CHARACTERS_BIN}?format=raw")
set(DESTINATION_PATH "${SID_DATA_PATH}")
download_to(${CHARACTERS_BIN} ${CHARACTERS_BIN_URL} "fd0d53b8480e86163ac98998976c72cc58d5dd8eb824ed7b829774e74213b420" ${DESTINATION_PATH} true "")

set(KERNAL_BIN_URL "https://sourceforge.net/p/vice-emu/code/HEAD/tree/tags/v3.10/vice/data/C64/${KERNAL_BIN}?format=raw")
set(DESTINATION_PATH "${SID_DATA_PATH}")
download_to(${KERNAL_BIN} ${KERNAL_BIN_URL} "83c60d47047d7beab8e5b7bf6f67f80daa088b7a6a27de0d7e016f6484042721" ${DESTINATION_PATH} true "")

set(HVSC_SONGLENGTHS_URL "https://www.hvsc.c64.org/download/C64Music/DOCUMENTS/${HVSC_SONGLENGTHS_FILENAME}")
set(PLUGIN_${NAME}_HVSC_SONGLENGTHS_URL "${HVSC_SONGLENGTHS_URL}" PARENT_SCOPE)
set(DESTINATION_PATH "${SID_DATA_PATH}")
download_to(${HVSC_SONGLENGTHS_FILENAME} ${HVSC_SONGLENGTHS_URL} "" ${DESTINATION_PATH} true "")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${EXTERNAL_SOURCE_DIR_sidid}/${SIDID_CFG} ${SID_DATA_PATH}
        VERBATIM)
