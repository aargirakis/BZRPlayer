cmake_minimum_required(VERSION 3.28)

set(NAME "audiodecoder.wsr")
set(VERSION "1.1.0")
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "${NAME}" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(NAME_VERSIONED "${NAME}-${VERSION}")
set(FILENAME "${NAME_VERSIONED}.tar.gz")
set(FILENAME_URL "v${VERSION}.tar.gz")
set(URL "https://github.com/xbmc/${NAME}/archive/refs/tags/${FILENAME_URL}")
set(SHA_256_HASH "c84c49696939f950650ac05fbe6c0b9c6889003cea0cfb223d8045480ceb0a1a")
download_patch_and_add(${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} false "" "")

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/lib/in_wsr/*.c
        ${EXTERNAL_SOURCE_DIR}/lib/in_wsr/nec/*.c
        src/main.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/lib/in_wsr

        ${CMAKE_SOURCE_DIR}/src/app
        ${LOGGER_API_DIR}
        ${FMOD_API_DIR})

target_compile_options(${PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:C>:-fcommon>)

target_link_libraries(${PROJECT_NAME} PRIVATE logger)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_PATH}/${CMAKE_INSTALL_LIBDIR}${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_PATH}${DATA_DIR}${PLUGINS_DIR})

if (NOT WIN32)
    install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}${PLUGINS_DIR})
endif ()