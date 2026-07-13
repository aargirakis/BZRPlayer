cmake_minimum_required(VERSION 3.28)

set(NAME "emu76489")
project(${NAME})

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(VERSION "829a4b7e917f2c2ccd07d40cb4bced7e37bbe3e2")
string(SUBSTRING ${VERSION} 0 12 VERSION_SHORT)
set(NAME_VERSIONED "${NAME}-${VERSION_SHORT}")
set(FILENAME "${NAME}-${VERSION}.tar.gz")
set(FILENAME_URL "${VERSION}.tar.gz")
set(URL "https://github.com/digital-sound-antiques/${NAME}/archive/${FILENAME_URL}")
set(SHA_256_HASH "28035a4c5cc278203beeaf7f679f78c1502794fce94dd0189a06bdabadf58e2b")
set(UNPACKED_DIR "${NAME}-${VERSION}")
download_patch_and_add(${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} true ${UNPACKED_DIR} "")

file(GLOB SOURCES ${EXTERNAL_SOURCE_DIR}/emu76489.c)

add_library(${PROJECT_NAME} OBJECT ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ${EXTERNAL_SOURCE_DIR})

set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
