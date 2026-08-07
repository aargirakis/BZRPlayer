cmake_minimum_required(VERSION 3.28)

set(NAME "vgmstream")
set(VERSION "2117")
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "${NAME}" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION_SHORT}" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(NAME_VERSIONED "${NAME}-${VERSION}")
set(TAG "r${VERSION}")
set(NAME_TAGGED "${NAME}-${TAG}")
set(FILENAME "${NAME_TAGGED}.tar.gz")
set(FILENAME_URL "${TAG}.tar.gz")
set(URL "https://github.com/${NAME}/${NAME}/archive/refs/tags/${FILENAME_URL}")
set(SHA_256_HASH "b9ad0ffabb5919e1c9ec2912416e3ce37916417a8c410196822ce6c610dcde66")
set(UNPACKED_DIR "${NAME_TAGGED}")
download_patch_and_add(${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} true ${UNPACKED_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/patches)

add_subdirectory(deps/ffmpeg)
add_subdirectory(deps/libcue)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/src/*.c
        ${EXTERNAL_SOURCE_DIR}/src/base/*.c
        ${EXTERNAL_SOURCE_DIR}/src/coding/*.c
        ${EXTERNAL_SOURCE_DIR}/src/coding/libs/*.c
        ${EXTERNAL_SOURCE_DIR}/src/layout/*.c
        ${EXTERNAL_SOURCE_DIR}/src/meta/*.c
        ${EXTERNAL_SOURCE_DIR}/src/util/*.c

        # some extra sources are included here in order to prevent linking failure
        # since vgmstream lacks of some exports needed by our custom ffmpeg build
        ${EXTERNAL_SOURCE_DIR_ffmpeg}/libavcodec/adts_parser.c
        ${EXTERNAL_SOURCE_DIR_ffmpeg}/libavformat/demux.c
        ${EXTERNAL_SOURCE_DIR_ffmpeg}/libavformat/oggparsedirac.c
        ${EXTERNAL_SOURCE_DIR_ffmpeg}/libavformat/utils.c

        src/main.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/src
        ${EXTERNAL_SOURCE_DIR}/src/base
        ${EXTERNAL_SOURCE_DIR}/src/coding
        ${EXTERNAL_SOURCE_DIR}/src/coding/libs
        ${EXTERNAL_SOURCE_DIR}/src/layout
        ${EXTERNAL_SOURCE_DIR}/src/meta
        ${EXTERNAL_SOURCE_DIR}/src/util
        ${EXTERNAL_SOURCE_DIR_ffmpeg} # needed by the included extra sources
        ${EXTERNAL_SOURCE_DIR_libcue}

        ${CMAKE_SOURCE_DIR}/src/app
        ${LOGGER_API_DIR}
        ${FMOD_API_DIR})

set(CONFIG_FILENAME "${PROJECT_NAME}.cfg")
set(PLUGIN_${NAME}_CONFIG_FILENAME "${CONFIG_FILENAME}" PARENT_SCOPE)
target_compile_definitions(${PROJECT_NAME} PRIVATE CONFIG_FILENAME="${CONFIG_FILENAME}")
target_compile_definitions(${PROJECT_NAME} PRIVATE PLUGIN_CONFIGS_DIR="${PLUGIN_CONFIGS_DIR}")
target_compile_definitions(${PROJECT_NAME} PRIVATE VGM_USE_FFMPEG)

foreach (FFMPEG_STATIC_LIB ${FFMPEG_STATIC_LIBS})
    add_library(${FFMPEG_STATIC_LIB} STATIC IMPORTED GLOBAL)
    add_dependencies(${FFMPEG_STATIC_LIB} ${FFMPEG_TARGET})
    set_target_properties(${FFMPEG_STATIC_LIB} PROPERTIES IMPORTED_LOCATION ${EXTERNAL_SOURCE_DIR_ffmpeg}/out/lib/${FFMPEG_STATIC_LIB}.a)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${FFMPEG_STATIC_LIB})
endforeach ()

target_link_libraries(${PROJECT_NAME} PRIVATE libcue logger)

if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE -lbcrypt)
endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_PATH}/${CMAKE_INSTALL_LIBDIR}${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_PATH}${DATA_DIR}${PLUGINS_DIR})

if (NOT WIN32)
    install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}${PLUGINS_DIR})
endif ()