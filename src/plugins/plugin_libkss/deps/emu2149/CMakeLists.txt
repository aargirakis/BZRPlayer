cmake_minimum_required(VERSION 3.28)

set(NAME "emu2149")
project(${NAME})

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(VERSION "bf68e0f59083c6e32490554aa6a5f5f6cd4abf7e")
string(SUBSTRING ${VERSION} 0 12 VERSION_SHORT)
set(NAME_VERSIONED "${NAME}-${VERSION_SHORT}")
set(FILENAME "${NAME}-${VERSION}.tar.gz")
set(FILENAME_URL "${VERSION}.tar.gz")
set(URL "https://github.com/digital-sound-antiques/${NAME}/archive/${FILENAME_URL}")
set(SHA_256_HASH "91f9b2fc1f98d17cf33ee1ff42dbaa424c561cb4b47b822ffa1d9b378a5395db")
set(UNPACKED_DIR "${NAME}-${VERSION}")
download_patch_and_add(${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} true ${UNPACKED_DIR} "")

file(GLOB SOURCES ${EXTERNAL_SOURCE_DIR}/emu2149.c)

add_library(${PROJECT_NAME} OBJECT ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ${EXTERNAL_SOURCE_DIR})

set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
