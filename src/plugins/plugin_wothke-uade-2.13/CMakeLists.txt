cmake_minimum_required(VERSION 3.28)

set(NAME "wothke-uade-2.13")
set(VERSION "6e96e16c30d794f8e739921da5b23f722dc53b2f")
string(SUBSTRING ${VERSION} 0 12 VERSION_SHORT)
project(plugin_${NAME})
set(PLUGIN_${NAME}_DLL "${PROJECT_NAME}.dll" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "WebUADE+" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION_SHORT}" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION GREATER_EQUAL 10)
    add_compile_options($<$<COMPILE_LANGUAGE:C>:-fcommon>)
endif ()

if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION GREATER_EQUAL 14)
    add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-implicit-function-declaration>)
    add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-int-conversion>)
endif ()

set(LIB_NAME "${NAME}")
set(LIB_VERSION "${VERSION}")
set(LIB_VERSION_SHORT "${VERSION_SHORT}")
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION_SHORT}")
set(LIB_FILENAME "${LIB_NAME_VERSIONED}.zip")
set(LIB_FILENAME_URL "${LIB_VERSION}.zip")
set(LIB_URL "https://bitbucket.org/wothke/uade-2.13/get/${LIB_FILENAME_URL}")
set(LIB_SHA_256_HASH "1f0df2bc7ecb93212ca9c6ca4e76e93c266858dc04233a8d343b90f7b191c0ba")
download_patch_and_add(
        ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH} false ""
        ${CMAKE_CURRENT_SOURCE_DIR}/patches
)

set(UADE_DATA_PATH "/data/plugin/uade")
add_compile_definitions(UADE_DATA_PATH="${UADE_DATA_PATH}")
add_compile_definitions(EMSCRIPTEN=1)
add_compile_definitions(_UADE_STRSEP_H_) #TODO needed?

set(FILENAME "uade.md5")
set(FILE_URL "http://bzrplayer.blazer.nu/${FILENAME}")
set(DESTINATION_PATH "${OUTPUT_DIR}${UADE_DATA_PATH}")
download_to(${FILENAME} ${FILE_URL} "" ${DESTINATION_PATH} true "")

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
        ${EXTERNAL_SOURCE_DIR}/src/frontends/common/md5.c
        ${EXTERNAL_SOURCE_DIR}/src/frontends/common/songdb.c
        ${EXTERNAL_SOURCE_DIR}/src/frontends/common/songinfo.c
        ${EXTERNAL_SOURCE_DIR}/src/frontends/common/support.c
        ${EXTERNAL_SOURCE_DIR}/src/frontends/common/uadeconf.c
        ${EXTERNAL_SOURCE_DIR}/src/frontends/common/vplist.c
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
        src/flod/*.cpp #TODO
        src/md5/*.c    #TODO all src/md5/*.* files (apart types.h) comes from xmms-sid (version from 0.8.96 to 0.8.99.2) that seem untraceable on the web: better upgrade to 0.9.2
        src/blazer.c
        src/main.cpp

        ${CMAKE_SOURCE_DIR}/src/app/BaseRow.cpp
)

list(REMOVE_ITEM SOURCES ${EXTERNAL_SOURCE_DIR}/compat/stdinttest.c)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/compat
        ${EXTERNAL_SOURCE_DIR}/src
        ${EXTERNAL_SOURCE_DIR}/src/frontends/common
        ${EXTERNAL_SOURCE_DIR}/src/include
        ${EXTERNAL_SOURCE_DIR}/src/machdep
        src/flod  #TODO
        src/md5  #TODO

        ${CMAKE_SOURCE_DIR}/src/app
        ${EXTERNAL_SOURCE_DIR_fmod}/api/core/inc
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        -L"${EXTERNAL_SOURCE_DIR_fmod}"/api/core/lib/x86 -lfmod -lws2_32
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/data/plugin
        COMPILE_FLAGS "-m32"
        LINK_FLAGS "-m32"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        ${EXTERNAL_SOURCE_DIR}/players
        ${OUTPUT_DIR}${UADE_DATA_PATH}/players

        #TODO where they come from?
        #TODO why MusicMaker-4V, MusicMaker-8V & onEscapee should overwrite the uade bundled ones?
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/dist/players
        ${OUTPUT_DIR}${UADE_DATA_PATH}/players

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${EXTERNAL_SOURCE_DIR}/amigasrc/score/score
        ${OUTPUT_DIR}${UADE_DATA_PATH}/amigasrc/score/score

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${EXTERNAL_SOURCE_DIR}/eagleplayer.conf
        ${EXTERNAL_SOURCE_DIR}/uade.conf
        ${EXTERNAL_SOURCE_DIR}/uaerc
        ${OUTPUT_DIR}${UADE_DATA_PATH}

        VERBATIM
)
