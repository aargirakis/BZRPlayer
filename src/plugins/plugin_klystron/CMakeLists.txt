cmake_minimum_required(VERSION 3.28)

set(NAME "klystron")
project(plugin_${NAME})
set(PLUGIN_DLL_${NAME} "${PROJECT_NAME}.dll" PARENT_SCOPE)

find_package(SDL2 REQUIRED)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION GREATER_EQUAL 14)
    add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-incompatible-pointer-types>)
endif ()

set(LIB_NAME "${NAME}")
set(LIB_VERSION "8655115f58cfe83a5d792c6a703f7bd46cbb2167")
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION}")
set(LIB_FILENAME "${LIB_NAME_VERSIONED}.zip")
set(LIB_FILENAME_URL "${LIB_VERSION}.zip")
set(LIB_URL "https://github.com/kometbomb/${LIB_NAME}/archive/${LIB_FILENAME_URL}")
set(LIB_SHA_256_HASH "32744f21bb452ea5f9388d522a56c9f977659b05273544280eed0bb1d73ac3f3")
download_and_patch(
        ${LIB_NAME} ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH} false ""
        ${CMAKE_CURRENT_SOURCE_DIR}/patches
)

add_compile_definitions(NOSDL_MIXER)
add_compile_definitions(STEREOOUTPUT)
add_compile_definitions(USENATIVEAPIS)
add_compile_definitions(USESDLMUTEXES)
add_compile_definitions(WIN32)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/src/lib/*.c
        ${EXTERNAL_SOURCE_DIR}/src/snd/*.c
        src/main.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/src
        ${SDL2_INCLUDE_DIR}

        ${CMAKE_SOURCE_DIR}/src/app
        ${EXTERNAL_SOURCE_DIR_fmod}/api/core/inc
)

target_link_libraries(${PROJECT_NAME} PRIVATE -lwinmm)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        SUFFIX ".dll"
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR_PARENT}/data/plugin
        COMPILE_FLAGS "-m32"
        LINK_FLAGS "-m32"
)
