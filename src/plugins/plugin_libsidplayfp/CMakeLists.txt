cmake_minimum_required(VERSION 3.28)

set(NAME "libsidplayfp")
set(VERSION "2.12.0")
project(plugin_${NAME})
set(PLUGIN_${NAME}_DLL "${PROJECT_NAME}.dll" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "${NAME}" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 14)

set(LIB1_NAME "${NAME}")
set(LIB1_VERSION "${VERSION}")
set(LIB1_NAME_VERSIONED "${LIB1_NAME}-${LIB1_VERSION}")
set(LIB1_FILENAME "${LIB1_NAME_VERSIONED}.tar.gz")
set(LIB1_URL "https://github.com/${LIB1_NAME}/${LIB1_NAME}/releases/download/v${LIB1_VERSION}/${LIB1_FILENAME}")
set(LIB1_SHA_256_HASH "bc4f4fa203dcf0736fe48c23dce9aa0db825370e5941e7595e4851efe6937cdc")
download_patch_and_add(
        ${LIB1_NAME_VERSIONED} ${LIB1_FILENAME} ${LIB1_URL} ${LIB1_SHA_256_HASH} false ""
        ${CMAKE_CURRENT_SOURCE_DIR}/patches/${LIB1_NAME}
)

set(EXTERNAL_SOURCE_DIR_1 "${EXTERNAL_SOURCE_DIR}")

add_compile_definitions(USER_PLUGINS_CONFIG_DIR="${USER_PLUGINS_CONFIG_DIR}")

add_compile_definitions(HAVE_CONFIG_H)
add_compile_definitions(HAVE_CXX14)
set(PACKAGE ${GIT_REPO_NAME})
set(VERSION ${LIB1_VERSION})
set(PACKAGE_URL "https://github.com/${GIT_REPO_NAME}/${GIT_REPO_NAME}/")
configure_file(
        src/config.h.in
        ${EXTERNAL_SOURCE_DIR_1}/src/config.h
        @ONLY NEWLINE_STYLE UNIX
)

set(PACKAGE_VERSION "${LIB1_VERSION}")
set(HAVE_BUILTIN_EXPECT 1)
set(RESID_BRANCH_HINTS 1)
set(RESID_INLINING 1)
set(RESID_INLINE "inline")
configure_file(
        ${EXTERNAL_SOURCE_DIR_1}/src/builders/residfp-builder/residfp/siddefs-fp.h.in
        ${EXTERNAL_SOURCE_DIR_1}/src/builders/residfp-builder/residfp/siddefs-fp.h
        @ONLY NEWLINE_STYLE UNIX
)

string(REPLACE "." ";" LIB1_VERSION_LIST ${LIB1_VERSION})
list(GET LIB1_VERSION_LIST 0 LIB1_VERSION_MAJOR)
list(GET LIB1_VERSION_LIST 1 LIB1_VERSION_MINOR)
list(GET LIB1_VERSION_LIST 2 LIB1_VERSION_PATCH)
set(LIB_MAJOR "${LIB1_VERSION_MAJOR}")
set(LIB_MINOR "${LIB1_VERSION_MINOR}")
set(LIB_LEVEL "${LIB1_VERSION_PATCH}")
configure_file(
        ${EXTERNAL_SOURCE_DIR_1}/src/sidplayfp/sidversion.h.in
        ${EXTERNAL_SOURCE_DIR_1}/src/sidplayfp/sidversion.h
        @ONLY NEWLINE_STYLE UNIX
)

set(LIB2_NAME "sidid")
set(LIB2_VERSION "56ba876ad30cbdd4ea7e330df7f9bb19ec899ed4")
string(SUBSTRING ${LIB2_VERSION} 0 12 LIB2_VERSION_SHORT)
set(LIB2_NAME_VERSIONED "${LIB2_NAME}-${LIB2_VERSION_SHORT}")
set(LIB2_FILENAME "${LIB2_NAME}-${LIB2_VERSION}.zip")
set(LIB2_FILENAME_URL "${LIB2_VERSION}.zip")
set(LIB2_URL "https://github.com/cadaver/${LIB2_NAME}/archive/${LIB2_FILENAME_URL}")
set(LIB2_SHA_256_HASH "c2ede25e575623dd55f3ab8da771127e7e51f784db604915ef80ea049c9cf710")
set(LIB2_UNPACKED_DIR "${LIB2_NAME}-${LIB2_VERSION}")
download_patch_and_add(
        ${LIB2_NAME_VERSIONED} ${LIB2_FILENAME} ${LIB2_URL} ${LIB2_SHA_256_HASH} true ${LIB2_UNPACKED_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/patches/${LIB2_NAME}
)

set(EXTERNAL_SOURCE_DIR_2 "${EXTERNAL_SOURCE_DIR}")

set(LIB3_NAME "vice")
set(LIB3_VERSION "3.8")
set(LIB3_NAME_VERSIONED "${LIB3_NAME}-${LIB3_VERSION}")
set(LIB3_FILENAME "${LIB3_NAME_VERSIONED}.tar.gz")
set(LIB3_URL "https://sourceforge.net/projects/${LIB3_NAME}-emu/files/releases/${LIB3_FILENAME}")
set(LIB3_SHA_256_HASH "1d7dc4d0f2bbcc2a871bb954ff4a5df63048dea9c16f5f1e9bc8260fa41a1004")
download_patch_and_add(
        ${LIB3_NAME_VERSIONED} ${LIB3_FILENAME} ${LIB3_URL} ${LIB3_SHA_256_HASH} false "" ""
)

set(EXTERNAL_SOURCE_DIR_3 "${EXTERNAL_SOURCE_DIR}")

set(HVSC_SONGLENGTHS_FILENAME "Songlengths.md5")
set(PLUGIN_DATA_DIR "${DATA_PLUGINS_DIR}/sid")
set(PLUGIN_USER_DIR "${USER_PLUGINS_DIR}/sid")
set(HVSC_SONGLENGTHS_DATA_PATH "${PLUGIN_DATA_DIR}/${HVSC_SONGLENGTHS_FILENAME}")
set(HVSC_SONGLENGTHS_USER_PATH "${PLUGIN_USER_DIR}/${HVSC_SONGLENGTHS_FILENAME}")
set(PLUGIN_${NAME}_USER_DIR "${PLUGIN_USER_DIR}" PARENT_SCOPE)
set(PLUGIN_${NAME}_HVSC_SONGLENGTHS_DATA_PATH "${HVSC_SONGLENGTHS_DATA_PATH}" PARENT_SCOPE)
set(PLUGIN_${NAME}_HVSC_SONGLENGTHS_USER_PATH "${HVSC_SONGLENGTHS_USER_PATH}" PARENT_SCOPE)
add_compile_definitions(PLUGIN_DATA_DIR="${PLUGIN_DATA_DIR}")
add_compile_definitions(HVSC_SONGLENGTHS_DATA_PATH="${HVSC_SONGLENGTHS_DATA_PATH}")
add_compile_definitions(HVSC_SONGLENGTHS_USER_PATH="${HVSC_SONGLENGTHS_USER_PATH}")

set(HVSC_SONGLENGTHS_URL "https://www.hvsc.c64.org/download/C64Music/DOCUMENTS/${HVSC_SONGLENGTHS_FILENAME}")
set(PLUGIN_${NAME}_HVSC_SONGLENGTHS_URL "${HVSC_SONGLENGTHS_URL}" PARENT_SCOPE)
set(DESTINATION_PATH "${OUTPUT_DIR}${PLUGIN_DATA_DIR}")
download_to(${HVSC_SONGLENGTHS_FILENAME} ${HVSC_SONGLENGTHS_URL} "" ${DESTINATION_PATH} true "")

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR_1}/src/*.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/builders/residfp-builder/*.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/builders/residfp-builder/residfp/*.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/builders/residfp-builder/residfp/resample/*.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/c64/*.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/c64/CIA/*.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/c64/CPU/*.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/c64/VIC_II/*.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/sidplayfp/*.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/sidtune/*.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/utils/*.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/utils/MD5/*.cpp
        ${EXTERNAL_SOURCE_DIR_1}/src/utils/STILview/*.cpp
        ${EXTERNAL_SOURCE_DIR_2}/sidid.c
        src/main.cpp
)

list(REMOVE_ITEM SOURCES
        ${EXTERNAL_SOURCE_DIR_1}/src/builders/residfp-builder/residfp/resample/test.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR_1}/src
        ${EXTERNAL_SOURCE_DIR_1}/src/builders/residfp-builder
        ${EXTERNAL_SOURCE_DIR_1}/src/builders/residfp-builder/residfp
        ${EXTERNAL_SOURCE_DIR_1}/src/c64
        ${EXTERNAL_SOURCE_DIR_1}/src/c64/Banks
        ${EXTERNAL_SOURCE_DIR_1}/src/c64/CIA
        ${EXTERNAL_SOURCE_DIR_1}/src/c64/CPU
        ${EXTERNAL_SOURCE_DIR_1}/src/c64/VIC_II
        ${EXTERNAL_SOURCE_DIR_1}/src/sidplayfp
        ${EXTERNAL_SOURCE_DIR_1}/src/sidtune
        ${EXTERNAL_SOURCE_DIR_1}/src/utils
        ${EXTERNAL_SOURCE_DIR_1}/src/utils/MD5
        ${EXTERNAL_SOURCE_DIR_1}/src/utils/STILview
        src

        ${CMAKE_SOURCE_DIR}/src/app
        ${EXTERNAL_SOURCE_DIR_fmod}/api/core/inc
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DATA_PLUGINS_DIR}
)

set(SIDID_CFG_DATA_PATH "${DATA_PLUGINS_DIR}/sid/sidid.cfg")
set(BASIC_BIN_DATA_PATH "${DATA_PLUGINS_DIR}/sid/basic.bin")
set(CHARACTERS_BIN_DATA_PATH "${DATA_PLUGINS_DIR}/sid/characters.bin")
set(KERNAL_BIN_DATA_PATH "${DATA_PLUGINS_DIR}/sid/kernal.bin")
add_compile_definitions(SIDID_CFG_DATA_PATH="${SIDID_CFG_DATA_PATH}")
add_compile_definitions(BASIC_BIN_DATA_PATH="${BASIC_BIN_DATA_PATH}")
add_compile_definitions(CHARACTERS_BIN_DATA_PATH="${CHARACTERS_BIN_DATA_PATH}")
add_compile_definitions(KERNAL_BIN_DATA_PATH="${KERNAL_BIN_DATA_PATH}")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${EXTERNAL_SOURCE_DIR_2}/sidid.cfg
        ${OUTPUT_DIR}${SIDID_CFG_DATA_PATH}

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${EXTERNAL_SOURCE_DIR_3}/data/C64/basic-901226-01.bin
        ${OUTPUT_DIR}${BASIC_BIN_DATA_PATH}

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${EXTERNAL_SOURCE_DIR_3}/data/C64/chargen-901225-01.bin
        ${OUTPUT_DIR}${CHARACTERS_BIN_DATA_PATH}

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${EXTERNAL_SOURCE_DIR_3}/data/C64/kernal-901227-03.bin
        ${OUTPUT_DIR}${KERNAL_BIN_DATA_PATH}

        VERBATIM
)