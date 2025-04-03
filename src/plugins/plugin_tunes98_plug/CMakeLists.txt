cmake_minimum_required(VERSION 3.28)

set(NAME "tunes98_plug")
set(VERSION "1.2")
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "TN_S98.tnplug" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 14)

if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION GREATER_EQUAL 14)
    add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-incompatible-pointer-types>)
endif ()

#TODO tunes98_plug is based on m_s98.kpi v1.0r8. here m_s98.kpi v1.0r10: https://www.purose.net/befis/download/kmp/kpis98-1.0r10s.7z
#TODO files formats that should be supported in both tunes98_plug/m_s98 (apart disabled vgm/vgz):
#TODO *.s98 (already working, but s98 vz variant is skipped since missing from main.cpp magic detection, or because missing zlib support: check m_s98.cpp.patch)
#TODO *.cym (already working if not skipped by main.cpp magic detection)
#TODO *.mym and *.x1f (still not tried these): sample files here: http://ftp.kameli.net/pub/mym/
#TODO check if file format infos are already loaded into fc->s98 instead of doing a manual detection in main.cpp)

set(LIB_NAME "${NAME}")
set(LIB_VERSION "${VERSION}")
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION}")
set(LIB_FILENAME "${LIB_NAME}.lha")
set(LIB_URL "https://www.a500.org/downloads/audio/tunenet/${LIB_FILENAME}")
set(LIB_SHA_256_HASH "b93958b4546dcc1609015ae724d5fc1267152635489ce23724629d0561fe3828")
download_patch_and_add(
        ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH} true ""
        ${CMAKE_CURRENT_SOURCE_DIR}/patches
)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/s98/*.cpp
        ${EXTERNAL_SOURCE_DIR}/s98/device/*.c
        ${EXTERNAL_SOURCE_DIR}/s98/device/*.cpp
        ${EXTERNAL_SOURCE_DIR}/s98/device/emu2413/*.c
        ${EXTERNAL_SOURCE_DIR}/s98/device/fmgen/*.cpp
        src/main.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/include/windows.h
        ${EXTERNAL_SOURCE_DIR}/s98
        ${EXTERNAL_SOURCE_DIR}/s98/device
        ${EXTERNAL_SOURCE_DIR}/s98/device/emu2413
        ${EXTERNAL_SOURCE_DIR}/s98/device/fmgen

        ${CMAKE_SOURCE_DIR}/src/app
        ${LIB_FMOD_API_DIR}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${DATA_DIR}/${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${DATA_DIR}/${PLUGINS_DIR}
)
