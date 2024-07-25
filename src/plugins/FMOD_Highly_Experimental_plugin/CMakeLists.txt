cmake_minimum_required(VERSION 3.28)

set(NAME "highly_experimental")
project(plugin_${NAME})

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DEMU_COMPILE -DEMU_LITTLE_ENDIAN")

set(GIT1_REPO_NAME ${NAME})
set(GIT1_REPO_URL "https://gitlab.com/kode54/${GIT1_REPO_NAME}.git")
set(GIT1_CLONE_AT "0fa96d186e3c0437951732d0c50bf1da4e32970e")
set(GIT1_IS_TAG false)
clone_and_patch(
        ${GIT1_REPO_NAME} ${GIT1_REPO_URL} ${GIT1_CLONE_AT} ${GIT1_IS_TAG}
        ""
        ${CMAKE_CURRENT_SOURCE_DIR}/patches
)

set(EXTERNAL_SOURCE_DIR_1 "${EXTERNAL_SOURCE_DIR}")

set(GIT2_REPO_NAME "psflib")
set(GIT2_REPO_URL "https://gitlab.com/kode54/${GIT2_REPO_NAME}.git")
set(GIT2_CLONE_AT "3bea757c8b45c5e68da1b5a7b736ad960a06a124")
set(GIT2_IS_TAG false)
clone_and_patch(
        ${GIT2_REPO_NAME} ${GIT2_REPO_URL} ${GIT2_CLONE_AT} ${GIT2_IS_TAG}
        ""
        ""
)

set(EXTERNAL_SOURCE_DIR_2 "${EXTERNAL_SOURCE_DIR}")

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR_1}/Core/*.c
        ${EXTERNAL_SOURCE_DIR_2}/psf2fs.c #TODO needed?
        ${EXTERNAL_SOURCE_DIR_2}/psflib.c
        src/main.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR_1}/Core
        ${EXTERNAL_SOURCE_DIR_2}

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
