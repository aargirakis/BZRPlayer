cmake_minimum_required(VERSION 3.28)

set(NAME "emu8950")
project(${NAME})

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(VERSION "28e2cb8765bdc3818afaf9f9f32a234f71af0ef5")
string(SUBSTRING ${VERSION} 0 12 VERSION_SHORT)
set(NAME_VERSIONED "${NAME}-${VERSION_SHORT}")
set(FILENAME "${NAME}-${VERSION}.tar.gz")
set(FILENAME_URL "${VERSION}.tar.gz")
set(URL "https://github.com/digital-sound-antiques/${NAME}/archive/${FILENAME_URL}")
set(SHA_256_HASH "36242131a19065148f9ff8eeaccf349622b64267718d332bf45d112026a3abfb")
set(UNPACKED_DIR "${NAME}-${VERSION}")
download_patch_and_add(${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} true ${UNPACKED_DIR} "")

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/emu8950.c
        ${EXTERNAL_SOURCE_DIR}/emuadpcm.c)

add_library(${PROJECT_NAME} OBJECT ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ${EXTERNAL_SOURCE_DIR})

set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
