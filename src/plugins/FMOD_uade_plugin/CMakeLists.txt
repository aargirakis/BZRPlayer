cmake_minimum_required(VERSION 3.28)

set(NAME "uade")
project(plugin_${NAME})

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION GREATER_EQUAL 10)
    add_compile_options($<$<COMPILE_LANGUAGE:C>:-fcommon>)
endif ()

if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION GREATER_EQUAL 14)
    add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-implicit-function-declaration>)
    add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-int-conversion>)
endif ()

if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    add_compile_options($<$<CONFIG:DEBUG>:$<$<COMPILE_LANGUAGE:C>:-O1>>) #TODO why dll linking on windows with debug build type fails with (default) -O0 flag ?
endif ()

set(LIB_NAME "wothke-uade-2.13")
set(LIB_VERSION "6e96e16c30d7")
set(LIB_COMMIT "${LIB_VERSION}94f8e739921da5b23f722dc53b2f")
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION}")
set(LIB_FILENAME "${LIB_NAME_VERSIONED}.zip")
set(LIB_FILENAME2 "${LIB_COMMIT}.zip")
set(LIB_URL "https://bitbucket.org/wothke/uade-2.13/get/${LIB_FILENAME2}")
set(LIB_SHA_256_HASH "1f0df2bc7ecb93212ca9c6ca4e76e93c266858dc04233a8d343b90f7b191c0ba")
download_and_patch(
        ${LIB_NAME} ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH} ""
        ${CMAKE_CURRENT_SOURCE_DIR}/patches
)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/compat/memmemrep.c
        ${EXTERNAL_SOURCE_DIR}/compat/strlrep.c
        ${EXTERNAL_SOURCE_DIR}/src/audio.c
        ${EXTERNAL_SOURCE_DIR}/src/cfgfile.c
        ${EXTERNAL_SOURCE_DIR}/src/cia.c
        ${EXTERNAL_SOURCE_DIR}/src/compiler.c
        ${EXTERNAL_SOURCE_DIR}/src/cpudefs.c
        ${EXTERNAL_SOURCE_DIR}/src/cpuemu1.c
        ${EXTERNAL_SOURCE_DIR}/src/cpuemu2.c
        ${EXTERNAL_SOURCE_DIR}/src/cpuemu3.c
        ${EXTERNAL_SOURCE_DIR}/src/cpuemu4.c
        ${EXTERNAL_SOURCE_DIR}/src/cpuemu5.c
        ${EXTERNAL_SOURCE_DIR}/src/cpuemu6.c
        ${EXTERNAL_SOURCE_DIR}/src/cpuemu7.c
        ${EXTERNAL_SOURCE_DIR}/src/cpuemu8.c
        ${EXTERNAL_SOURCE_DIR}/src/cpustbl.c
        ${EXTERNAL_SOURCE_DIR}/src/custom.c
        ${EXTERNAL_SOURCE_DIR}/src/fpp.c
        ${EXTERNAL_SOURCE_DIR}/src/frontends/common/amifilemagic.c
        ${EXTERNAL_SOURCE_DIR}/src/frontends/common/eagleplayer.c
        ${EXTERNAL_SOURCE_DIR}/src/frontends/common/effects.c
        ${EXTERNAL_SOURCE_DIR}/src/frontends/common/songdb.c
        ${EXTERNAL_SOURCE_DIR}/src/frontends/common/songinfo.c
        ${EXTERNAL_SOURCE_DIR}/src/frontends/common/support.c
        ${EXTERNAL_SOURCE_DIR}/src/frontends/common/uadeconf.c
        ${EXTERNAL_SOURCE_DIR}/src/md-support.c
        ${EXTERNAL_SOURCE_DIR}/src/memory.c
        ${EXTERNAL_SOURCE_DIR}/src/missing.c
        ${EXTERNAL_SOURCE_DIR}/src/newcpu.c
        ${EXTERNAL_SOURCE_DIR}/src/ossupport.c
        ${EXTERNAL_SOURCE_DIR}/src/readcpu.c
        ${EXTERNAL_SOURCE_DIR}/src/sd-sound.c
        ${EXTERNAL_SOURCE_DIR}/src/sinctable.c
        ${EXTERNAL_SOURCE_DIR}/src/text_scope.c
        ${EXTERNAL_SOURCE_DIR}/src/uade.c
        ${EXTERNAL_SOURCE_DIR}/src/uademain.c
        ${EXTERNAL_SOURCE_DIR}/src/unixatomic.c
        src/flod/*.cpp
        src/md5/*.c
        src/blazer.c
        src/main.cpp
)

list(REMOVE_ITEM SOURCES ${EXTERNAL_SOURCE_DIR}/compat/stdinttest.c)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/compat
        ${EXTERNAL_SOURCE_DIR}/src
        ${EXTERNAL_SOURCE_DIR}/src/frontends/common
        ${EXTERNAL_SOURCE_DIR}/src/include
        ${EXTERNAL_SOURCE_DIR}/src/machdep
        src/flod
        src/md5
        src

        ${CMAKE_SOURCE_DIR}/src/app
        ${EXTERNAL_SOURCE_DIR_fmod}/api/core/inc
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        -L"${EXTERNAL_SOURCE_DIR_fmod}"/api/core/lib/x86 -lfmod -lws2_32
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "codec_${NAME}"
        PREFIX ""
        SUFFIX ".dll"
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR_PARENT}/data/plugin
        COMPILE_FLAGS "-m32"
        LINK_FLAGS "-m32"
)
