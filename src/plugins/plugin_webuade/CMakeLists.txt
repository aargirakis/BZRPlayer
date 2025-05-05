cmake_minimum_required(VERSION 3.28)

set(NAME "webuade")
set(VERSION "8abcb3e6841d11269527db6e7085386b0c983014")
string(SUBSTRING ${VERSION} 0 12 VERSION_SHORT)
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "WebUADE+" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION_SHORT}" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(NAME_VERSIONED "${NAME}-${VERSION_SHORT}")
set(NAME_VERSIONED2 "wothke-uade-2.13-${VERSION_SHORT}")
set(FILENAME "${NAME_VERSIONED2}.zip")
set(FILENAME_URL "${VERSION}.zip")
set(URL "https://bitbucket.org/wothke/uade-2.13/get/${FILENAME_URL}")
set(SHA_256_HASH "c47bf0148057eb5f345b54030ff81ab73f84cf83632c740b02fbf8e0c011d0ee")
set(UNPACKED_DIR "${NAME_VERSIONED2}")
download_patch_and_add(
        ${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} true ${UNPACKED_DIR}
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
        src/main.cpp

        ${CMAKE_SOURCE_DIR}/src/app/BaseRow.cpp)

if (WIN32)
    list(APPEND SOURCES src/strsep.c)
endif ()

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/compat
        ${EXTERNAL_SOURCE_DIR}/src
        ${EXTERNAL_SOURCE_DIR}/src/frontends/common
        ${EXTERNAL_SOURCE_DIR}/src/include
        ${EXTERNAL_SOURCE_DIR}/src/machdep
        src
        src/flod  #TODO
        src/md5  #TODO

        ${CMAKE_SOURCE_DIR}/src/app
        ${FMOD_API_DIR})

set(UADE_DATA_DIR "${PLUGINS_DIR}/uade")
set(PLUGIN_${NAME}_DIR "${UADE_DATA_DIR}" PARENT_SCOPE)
set(UADE_SONGLENGTHS "uade.md5")
target_compile_definitions(${PROJECT_NAME} PRIVATE UADE_DATA_DIR="${UADE_DATA_DIR}")
target_compile_definitions(${PROJECT_NAME} PRIVATE PLUGINS_CONFIG_DIR="${PLUGINS_CONFIG_DIR}")
target_compile_definitions(${PROJECT_NAME} PRIVATE EMSCRIPTEN)

if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE
            -L"${FMOD_DIR}" -lfmod -lws2_32
    )
else ()
    target_link_libraries(${PROJECT_NAME} PRIVATE
            -L"${FMOD_DIR}" -lfmod
    )
endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${DATA_DIR}/${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${DATA_DIR}/${PLUGINS_DIR}
)


cmake_path(SET UADE_DATA_PATH NORMALIZE ${OUTPUT_DIR}/${DATA_DIR}/${UADE_DATA_DIR})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        ${EXTERNAL_SOURCE_DIR}/players ${UADE_DATA_PATH}/players

        #TODO where they come from?
        #TODO why MusicMaker-4V, MusicMaker-8V & onEscapee should overwrite the uade bundled ones?
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/dist/players ${UADE_DATA_PATH}/players

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${EXTERNAL_SOURCE_DIR}/amigasrc/score/score ${UADE_DATA_PATH}/amigasrc/score/score
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${EXTERNAL_SOURCE_DIR}/eagleplayer.conf
        ${EXTERNAL_SOURCE_DIR}/uade.conf
        ${EXTERNAL_SOURCE_DIR}/uaerc
        ${CMAKE_CURRENT_SOURCE_DIR}/dist/${UADE_SONGLENGTHS}
        ${UADE_DATA_PATH}
        VERBATIM)
