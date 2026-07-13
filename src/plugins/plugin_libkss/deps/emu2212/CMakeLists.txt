cmake_minimum_required(VERSION 3.28)

set(NAME "emu2212")
project(${NAME})

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(VERSION "b21783bd7a021b2fdc7f946c7836a1110aad2ddd")
string(SUBSTRING ${VERSION} 0 12 VERSION_SHORT)
set(NAME_VERSIONED "${NAME}-${VERSION_SHORT}")
set(FILENAME "${NAME}-${VERSION}.tar.gz")
set(FILENAME_URL "${VERSION}.tar.gz")
set(URL "https://github.com/digital-sound-antiques/${NAME}/archive/${FILENAME_URL}")
set(SHA_256_HASH "5dcbe1bd7cd01c06c0f95679540c1d55535aa5aba811e468722d49adc857e040")
set(UNPACKED_DIR "${NAME}-${VERSION}")
download_patch_and_add(${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} true ${UNPACKED_DIR} "")

file(GLOB SOURCES ${EXTERNAL_SOURCE_DIR}/emu2212.c)

add_library(${PROJECT_NAME} OBJECT ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ${EXTERNAL_SOURCE_DIR})

set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
