cmake_minimum_required(VERSION 3.28)

set(NAME "kmz80")
project(${NAME})

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(VERSION "86f9e79db4557360db42efe2f760a94b685f2ee9")
string(SUBSTRING ${VERSION} 0 12 VERSION_SHORT)
set(NAME_VERSIONED "${NAME}-${VERSION_SHORT}")
set(FILENAME "${NAME}-${VERSION}.tar.gz")
set(FILENAME_URL "${VERSION}.tar.gz")
set(URL "https://github.com/digital-sound-antiques/${NAME}/archive/${FILENAME_URL}")
set(SHA_256_HASH "5c4236d1a7e9aac24aa9294c33e8fc9055be27be2aee2b610330b57705fed7fd")
set(UNPACKED_DIR "${NAME}-${VERSION}")
download_patch_and_add(${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} true ${UNPACKED_DIR} "")

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/kmdmg.c
        ${EXTERNAL_SOURCE_DIR}/kmevent.c
        ${EXTERNAL_SOURCE_DIR}/kmr800.c
        ${EXTERNAL_SOURCE_DIR}/kmz80.c
        ${EXTERNAL_SOURCE_DIR}/kmz80c.c
        ${EXTERNAL_SOURCE_DIR}/kmz80t.c)

add_library(${PROJECT_NAME} OBJECT ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ${EXTERNAL_SOURCE_DIR})

set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
