cmake_minimum_required(VERSION 3.28)

set(NAME libsidplayfp)
project(plugin_${NAME} VERSION "2.7.1")

set(CMAKE_C_STANDARD 17)

if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(CMAKE_CXX_STANDARD 17)
else ()
    set(CMAKE_CXX_STANDARD 14)
endif ()

#TODO the libsidplayfp utilized sourcecode (taken from the zip file) is wrong: it lacks of mandatory files because the presence of ".gitignore" file.
#TODO see https://github.com/libsidplayfp/libsidplayfp/issues/135 for the right source code to utilize.
#TODO these files are: sidplayer1.bin, sidplayer2.bin, config.h and others

set(PACKAGE_VERSION ${PROJECT_VERSION})
set(RESID_BRANCH_HINTS 1)
set(HAVE_BUILTIN_EXPECT 1) #TODO
set(RESID_INLINING 1)
set(RESID_INLINE inline)
configure_file(
        src/libsidplayfp-${PROJECT_VERSION}/src/builders/residfp-builder/residfp/siddefs-fp.h.in
        ${CMAKE_CURRENT_SOURCE_DIR}/src/libsidplayfp-${PROJECT_VERSION}/src/builders/residfp-builder/residfp/siddefs-fp.h
        @ONLY NEWLINE_STYLE UNIX
)

#set(LIB_MAJOR ${PROJECT_VERSION_MAJOR})
#set(LIB_MINOR ${PROJECT_VERSION_MINOR})
#set(LIB_LEVEL ${PROJECT_VERSION_PATCH})
#configure_file(
#        src/libsidplayfp-${PROJECT_VERSION}/src/sidplayfp/sidversion.h.in
#        ${CMAKE_CURRENT_SOURCE_DIR}/src/libsidplayfp-${PROJECT_VERSION}/src/sidplayfp/sidversion.h
#        @ONLY NEWLINE_STYLE UNIX
#)

file(GLOB SOURCES
        src/libsidplayfp-${PROJECT_VERSION}/src/*.cpp
        src/libsidplayfp-${PROJECT_VERSION}/src/builders/residfp-builder/*.cpp
        src/libsidplayfp-${PROJECT_VERSION}/src/builders/residfp-builder/residfp/*.cpp
        src/libsidplayfp-${PROJECT_VERSION}/src/builders/residfp-builder/residfp/resample/*.cpp
        src/libsidplayfp-${PROJECT_VERSION}/src/c64/*.cpp
        src/libsidplayfp-${PROJECT_VERSION}/src/c64/CIA/*.cpp
        src/libsidplayfp-${PROJECT_VERSION}/src/c64/CPU/*.cpp
        src/libsidplayfp-${PROJECT_VERSION}/src/c64/VIC_II/*.cpp
        src/libsidplayfp-${PROJECT_VERSION}/src/sidplayfp/*.cpp
        src/libsidplayfp-${PROJECT_VERSION}/src/sidtune/*.cpp
        src/libsidplayfp-${PROJECT_VERSION}/src/utils/*.cpp
        src/libsidplayfp-${PROJECT_VERSION}/src/utils/MD5/*.cpp
        src/libsidplayfp-${PROJECT_VERSION}/src/utils/STILview/*.cpp
        src/sidid.c
        src/main.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        src
        src/libsidplayfp-${PROJECT_VERSION}/src
        src/libsidplayfp-${PROJECT_VERSION}/src/builders/residfp-builder
        src/libsidplayfp-${PROJECT_VERSION}/src/builders/residfp-builder/residfp
        src/libsidplayfp-${PROJECT_VERSION}/src/c64
        src/libsidplayfp-${PROJECT_VERSION}/src/c64/Banks
        src/libsidplayfp-${PROJECT_VERSION}/src/c64/CIA
        src/libsidplayfp-${PROJECT_VERSION}/src/c64/CPU
        src/libsidplayfp-${PROJECT_VERSION}/src/c64/VIC_II
        src/libsidplayfp-${PROJECT_VERSION}/src/sidplayfp
        src/libsidplayfp-${PROJECT_VERSION}/src/sidtune
        src/libsidplayfp-${PROJECT_VERSION}/src/utils
        src/libsidplayfp-${PROJECT_VERSION}/src/utils/MD5
        src/libsidplayfp-${PROJECT_VERSION}/src/utils/STILview

        ${CMAKE_SOURCE_DIR}/src
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
