cmake_minimum_required(VERSION 3.28)

set(NAME "klystron")
set(VERSION "8655115f58cfe83a5d792c6a703f7bd46cbb2167")
string(SUBSTRING ${VERSION} 0 12 VERSION_SHORT)
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "${NAME}" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION_SHORT}" PARENT_SCOPE)

find_package(SDL2 REQUIRED)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(NAME_VERSIONED "${NAME}-${VERSION_SHORT}")
set(FILENAME "${NAME}-${VERSION}.zip")
set(FILENAME_URL "${VERSION}.zip")
set(URL "https://github.com/kometbomb/${NAME}/archive/${FILENAME_URL}")
set(SHA_256_HASH "32744f21bb452ea5f9388d522a56c9f977659b05273544280eed0bb1d73ac3f3")
set(UNPACKED_DIR "${NAME}-${VERSION}")
download_patch_and_add(
        ${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} true ${UNPACKED_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/patches
)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/src/lib/*.c
        ${EXTERNAL_SOURCE_DIR}/src/snd/*.c
        src/main.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/src
        ${SDL2_INCLUDE_DIR}

        ${CMAKE_SOURCE_DIR}/src/app
        ${FMOD_API_DIR})

target_compile_definitions(${PROJECT_NAME} PRIVATE NOSDL_MIXER)
target_compile_definitions(${PROJECT_NAME} PRIVATE STEREOOUTPUT)

if (WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE USENATIVEAPIS)
    target_compile_definitions(${PROJECT_NAME} PRIVATE USESDLMUTEXES)
endif ()

target_compile_options(${PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:C>:-Wno-incompatible-pointer-types>)

if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE -lwinmm)
endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
)
