cmake_minimum_required(VERSION 3.28)

set(NAME "atari-audio-library")
set(VERSION "1.06")
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "AtariAudio Library" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(NAME_VERSIONED "${NAME}-${VERSION}")
set(NAME2 "AtariAudio")
set(FILENAME "${NAME2}-${VERSION}.tar.gz")
set(FILENAME_URL "v${VERSION}.tar.gz")
set(URL "https://github.com/arnaud-carre/${NAME2}/archive/refs/tags/${FILENAME_URL}")
set(SHA_256_HASH "cd2795974c860e8fd4fc7f25f1b1c0e198f13fe20966ce3f415c7087002f3004")
set(UNPACKED_DIR "${NAME2}-${VERSION}")
download_patch_and_add(${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} true ${UNPACKED_DIR} "")

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/src/*.cpp
        ${EXTERNAL_SOURCE_DIR}/src/external/ice_24.c
        ${EXTERNAL_SOURCE_DIR}/src/external/Musashi/*.c
        src/main.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/src

        ${CMAKE_SOURCE_DIR}/src/app
        ${LOGGER_API_DIR}
        ${FMOD_API_DIR})

set(CONFIG_FILENAME "${PROJECT_NAME}.cfg")
set(PLUGIN_${NAME}_CONFIG_FILENAME "${CONFIG_FILENAME}" PARENT_SCOPE)
target_compile_definitions(${PROJECT_NAME} PRIVATE CONFIG_FILENAME="${CONFIG_FILENAME}")
target_compile_definitions(${PROJECT_NAME} PRIVATE PLUGIN_CONFIGS_DIR="${PLUGIN_CONFIGS_DIR}")

target_link_libraries(${PROJECT_NAME} PRIVATE logger)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_PATH}/${CMAKE_INSTALL_LIBDIR}${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_PATH}${DATA_DIR}${PLUGINS_DIR})

if (NOT WIN32)
    install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}${PLUGINS_DIR})
endif ()