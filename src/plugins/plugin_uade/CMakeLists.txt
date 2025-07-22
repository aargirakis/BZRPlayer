cmake_minimum_required(VERSION 3.28)

set(NAME "uade")
set(VERSION "99326f2a11543969ccba4acf5094586ce594bc45")
string(SUBSTRING ${VERSION} 0 12 VERSION_SHORT)
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "UADE (Matti)" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(NAME_VERSIONED "${NAME}-${VERSION_SHORT}")
set(FILENAME "${NAME}-${VERSION}.tar.gz")
set(URL "https://gitlab.com/mvtiaine/${NAME}/-/archive/${VERSION}/${FILENAME}")
set(SHA_256_HASH "d6cf842016bc5e20a1db0ae0dbecec57f6ce7b6361daa74d60c81fd56baca9f0")
set(UNPACKED_DIR "${NAME}-${VERSION}")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CFLAGS "CFLAGS=\"-DDISABLE_BENCODETOOLS -DDISABLE_ZAKALWE\"")
else ()
    set(CFLAGS "CFLAGS=\"-DDISABLE_BENCODETOOLS -DDISABLE_ZAKALWE -DNDEBUG -s\"")
endif ()

set(CONFIGURE_COMMAND "configure ${CFLAGS} \
--without-avx2 --without-uade123 --without-uadefs --without-uadesimple --without-write-audio --without-vasm \
--bencode-tools-prefix=/dev/null --libzakalwe-prefix=/dev/null")
set(BUILD_BYPRODUCTS "src/frontends/libuade/libuade.a")
download_patch_and_make(
        ${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} true ${UNPACKED_DIR} ""
        ${CONFIGURE_COMMAND} "" ${BUILD_BYPRODUCTS}
)

set(SONG_CONF "song.conf")
set(AUDACIOUS_UADE_VERSION "0.13.2")
set(SONG_CONF_URL "https://raw.githubusercontent.com/mvtiaine/audacious-uade/refs/tags/${AUDACIOUS_UADE_VERSION}/conf/${SONG_CONF}")
set(DESTINATION_PATH "${EXTERNAL_SOURCE_DIR}")
download_to(${SONG_CONF} ${SONG_CONF_URL} "ce2ae365913d55a157114bb40818a3ba062c64b2008e02614fefda859f961505" ${DESTINATION_PATH} true "")

patch_sources(${NAME_VERSIONED} ${CMAKE_CURRENT_SOURCE_DIR}/patches ${EXTERNAL_SOURCE_DIR})

file(READ "${EXTERNAL_SOURCE_DIR}/version" UADE_VERSION)
string(REPLACE "\r" "" UADE_VERSION "${UADE_VERSION}")
string(REPLACE "\n" "" UADE_VERSION "${UADE_VERSION}")
string(STRIP UADE_VERSION ${UADE_VERSION})
set(REGEX "^[1-9][0-9]*\.[0-9]+")
string(REGEX MATCH ${REGEX} MATCH_RESULT "${UADE_VERSION}")

if (NOT MATCH_RESULT OR NOT MATCH_RESULT STREQUAL UADE_VERSION)
    message(FATAL_ERROR "Invalid ${NAME} version string")
endif ()

set(PLUGIN_${NAME}_VERSION "${UADE_VERSION} (${VERSION_SHORT})" PARENT_SCOPE)

#TODO disable vasm compilation

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
target_compile_definitions(${PROJECT_NAME} PRIVATE UADE_CORE_PATH="${UADE_DATA_DIR}/${UADE_CORE}")
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

#TODO check https://github.com/mvtiaine/audacious-uade/commit/6445062c77dceb5182f550d057e64724349e4e41
#TODO put uadecore in lib folder
#TODO eagleplayer.conf.patch
#TODO compare players & eagleplayer.conf:

#[[bzr2:  DariusZendeh prefixes=dz,dzp,mkiio
matti: DariusZendeh prefixes=dz,mkiio

bzr2:  MarkII prefixes=mk2,mkii,mii
matti: MarkII prefixes=mk2,mkii

bzr2:  PTK-Prowiz prefixes=mod_ntkamp epopt=type:m&k. always_ends
matti: NoiseTracker prefixes=mod_ntkamp

(snd is also for ZoundMonitor)
bzr2:  RichardJoseph prefixes=rjp,sng
matti: RichardJoseph prefixes=rj

bzr2:  Quartet_ST prefixes=4v
matti: Quartet_ST prefixes=qts

bzr2:  ZoundMonitor prefixes=zm,sng
matti: ZoundMonitor prefixes=sng

bzr2:
JochenHippel-CoSo prefixes=hipc,soc
JochenHippel-CoSo prefixes=hipc
JochenHippel prefixes=hip,mcmd_disabled detect_format_by_name
Jochen_Hippel_ST prefixes=sog
Hippel-ST_note prefixes=soc,hst

matti:
JochenHippel-CoSo_UADE prefixes=hipc
JochenHippelCOSO prefixes=soc
JochenHippel prefixes=hip,mcmd,sog
Jochen_Hippel_ST prefixes=hst,sdc

bzr2:  SoundTracker-Pro-I prefixes=stp
matti: not found

bzr2:  SymphoniePro prefixes=sym
matti: not found

bzr2:  Stonetracker prefixes=spm
matti: not found]]

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
        ${EXTERNAL_SOURCE_DIR}/${SONG_CONF}
        ${EXTERNAL_SOURCE_DIR}/uade.conf
        ${EXTERNAL_SOURCE_DIR}/src/${UADE_CORE}
        ${EXTERNAL_SOURCE_DIR}/uaerc
        ${CMAKE_CURRENT_SOURCE_DIR}/dist/${UADE_SONGLENGTHS}
        ${UADE_DATA_PATH}
        VERBATIM)
