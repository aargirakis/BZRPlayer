cmake_minimum_required(VERSION 3.28)

set(NAME "libsidplayfp")
project(plugin_${NAME} VERSION "2.7.1")

set(CMAKE_C_STANDARD 17)

if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(CMAKE_CXX_STANDARD 17)
else ()
    set(CMAKE_CXX_STANDARD 14)
endif ()

set(LIB_NAME "${NAME}")
set(LIB_VERSION ${PROJECT_VERSION})
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION}")
set(LIB_FILENAME "${LIB_NAME_VERSIONED}.tar.gz")
set(LIB_URL "https://github.com/${LIB_NAME}/${LIB_NAME}/releases/download/v${PROJECT_VERSION}/${LIB_FILENAME}")
set(LIB_SHA_256_HASH "416df16c09b0ab058c21adc1792abfac8524a27b0dac0034afc040057f6346ba")
download_and_patch(
        ${LIB_NAME} ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH}
        ""
        ${CMAKE_CURRENT_SOURCE_DIR}/patches/${LIB_NAME}
)

set(EXTERNAL_SOURCE_DIR_1 "${EXTERNAL_SOURCE_DIR}")

add_compile_definitions(HAVE_CONFIG_H=1)
add_compile_definitions(HAVE_CXX17=1)
set(PACKAGE ${GIT_REPO_NAME})
set(VERSION ${PROJECT_VERSION})
set(PACKAGE_URL "https://github.com/${GIT_REPO_NAME}/${GIT_REPO_NAME}/")
configure_file(
        src/config.h.in
        ${EXTERNAL_SOURCE_DIR_1}/src/config.h
        @ONLY NEWLINE_STYLE UNIX
)

set(PACKAGE_VERSION "${PROJECT_VERSION}")
set(HAVE_BUILTIN_EXPECT 1)
set(RESID_BRANCH_HINTS 1)
set(RESID_INLINING 1)
set(RESID_INLINE "inline")
configure_file(
        ${EXTERNAL_SOURCE_DIR_1}/src/builders/residfp-builder/residfp/siddefs-fp.h.in
        ${EXTERNAL_SOURCE_DIR_1}/src/builders/residfp-builder/residfp/siddefs-fp.h
        @ONLY NEWLINE_STYLE UNIX
)

set(LIB_MAJOR "${PROJECT_VERSION_MAJOR}")
set(LIB_MINOR "${PROJECT_VERSION_MINOR}")
set(LIB_LEVEL "${PROJECT_VERSION_PATCH}")
configure_file(
        ${EXTERNAL_SOURCE_DIR_1}/src/sidplayfp/sidversion.h.in
        ${EXTERNAL_SOURCE_DIR_1}/src/sidplayfp/sidversion.h
        @ONLY NEWLINE_STYLE UNIX
)

set(LIB_NAME "sidid")
set(LIB_VERSION "eba761121a3f1239d7cc16b59a64140dc7d483ff")
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION}")
set(LIB_FILENAME "${LIB_VERSION}.zip")
set(LIB_URL "https://github.com/cadaver/${LIB_NAME}/archive/${LIB_FILENAME}")
set(LIB_SHA_256_HASH "2ae065ef4c58c8b12210fd6d9e8543ebdc272ccd9996ab963ef3b464bc4b0ddc")
download_and_patch(
        ${LIB_NAME} ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH}
        ""
        ${CMAKE_CURRENT_SOURCE_DIR}/patches/${LIB_NAME}
)

set(EXTERNAL_SOURCE_DIR_2 "${EXTERNAL_SOURCE_DIR}")

set(LIB_NAME "vice")
set(LIB_VERSION "3.8")
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION}")
set(LIB_FILENAME "${LIB_NAME_VERSIONED}.tar.gz")
set(LIB_URL "https://sourceforge.net/projects/${LIB_NAME}-emu/files/releases/${LIB_FILENAME}/download")
set(LIB_SHA_256_HASH "1d7dc4d0f2bbcc2a871bb954ff4a5df63048dea9c16f5f1e9bc8260fa41a1004")
download_and_patch(
        ${LIB_NAME} ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH}
        ""
        ""
)

set(EXTERNAL_SOURCE_DIR_3 "${EXTERNAL_SOURCE_DIR}")

set(FILENAME "Songlengths.md5")
set(FILE_URL "https://www.hvsc.c64.org/download/C64Music/DOCUMENTS/${FILENAME}")
set(DESTINATION_PATH "${OUTPUT_DIR_PARENT}/user/plugin/sid")
download_to(${FILENAME} ${FILE_URL} "" ${DESTINATION_PATH})

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
        ${CMAKE_SOURCE_DIR}/src/external/FMOD/api/core/inc
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
        ${EXTERNAL_SOURCE_DIR_2}/sidid.cfg
        ${OUTPUT_DIR_PARENT}/data/plugin/sid/sidid.cfg

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${EXTERNAL_SOURCE_DIR_3}/data/C64/basic-901226-01.bin
        ${OUTPUT_DIR_PARENT}/data/plugin/sid/basic.bin

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${EXTERNAL_SOURCE_DIR_3}/data/C64/chargen-901225-01.bin
        ${OUTPUT_DIR_PARENT}/data/plugin/sid/characters.bin

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${EXTERNAL_SOURCE_DIR_3}/data/C64/kernal-901227-03.bin
        ${OUTPUT_DIR_PARENT}/data/plugin/sid/kernal.bin

        VERBATIM
)