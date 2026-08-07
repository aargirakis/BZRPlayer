cmake_minimum_required(VERSION 3.28)

set(NAME "libpac")
set(VERSION "0.9.0")
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "${NAME}" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(NAME_VERSIONED "${NAME}-${VERSION}")
set(FILENAME "${NAME_VERSIONED}.tgz")
set(URL "http://prdownloads.sourceforge.net/${NAME}/${FILENAME}")
set(SHA_256_HASH "17516a4a9fd75c4186c6d85a8b0d46cbff98fbc2042bccf0ce4062b6f68e3e49")
download_patch_and_add(${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} false ""
        ${CMAKE_CURRENT_SOURCE_DIR}/patches)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/src/*.c
        src/main.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/src

        ${CMAKE_SOURCE_DIR}/src/app
        ${LOGGER_API_DIR}
        ${FMOD_API_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE logger)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_PATH}/${CMAKE_INSTALL_LIBDIR}${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_PATH}${DATA_DIR}${PLUGINS_DIR})

if (NOT WIN32)
    install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}${PLUGINS_DIR})
endif ()