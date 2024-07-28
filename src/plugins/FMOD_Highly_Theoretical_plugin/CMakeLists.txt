cmake_minimum_required(VERSION 3.28)

set(NAME "highly_theoretical")
project(plugin_${NAME})

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DEMU_COMPILE -DEMU_LITTLE_ENDIAN -DLSB_FIRST -DC68K_NO_JUMP_TABLE -DNO_DYNAREC")

set(GIT_REPO_NAME ${NAME})
set(GIT_REPO_URL "https://gitlab.com/kode54/${GIT_REPO_NAME}.git")
set(GIT_CLONE_AT "0e4c18c5b757b04dbcb68c572c5a4f6fd803283c")
set(GIT_IS_TAG false)
clone_and_patch(
        ${GIT_REPO_NAME} ${GIT_REPO_URL} ${GIT_CLONE_AT} ${GIT_IS_TAG}
        ""
        ""
)

set(EXTERNAL_SOURCE_DIR_${NAME} "${EXTERNAL_SOURCE_DIR}")

include(${CMAKE_SOURCE_DIR}/src/external/psflib/psflib.cmake)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR_${NAME}}/Core/*.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/Core/c68k/*.c
        ${EXTERNAL_SOURCE_DIR_psflib}/psf2fs.c #TODO needed?
        ${EXTERNAL_SOURCE_DIR_psflib}/psflib.c
        src/main.cpp        #TODO compilation strangely slow
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR_${NAME}}/Core
        ${EXTERNAL_SOURCE_DIR_${NAME}}/Core/c68k
        ${EXTERNAL_SOURCE_DIR_psflib}

        ${CMAKE_SOURCE_DIR}/src/app
        ${CMAKE_SOURCE_DIR}/src/external/FMOD/api/core/inc
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        -L${CMAKE_SOURCE_DIR}/src/external/zlib -lzdll -lshlwapi
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "codec_${NAME}"
        PREFIX ""
        SUFFIX ".dll"
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR_PARENT}/data/plugin
        COMPILE_FLAGS "-m32"
        LINK_FLAGS "-m32"
)
