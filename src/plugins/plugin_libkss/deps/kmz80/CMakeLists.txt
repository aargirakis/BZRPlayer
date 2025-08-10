cmake_minimum_required(VERSION 3.28)

set(NAME "kmz80")
project(${NAME})

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(VERSION "daf94dee27c7471d1e3415d44a30f850d32c529d")
string(SUBSTRING ${VERSION} 0 12 VERSION_SHORT)
set(NAME_VERSIONED "${NAME}-${VERSION_SHORT}")
set(FILENAME "${NAME}-${VERSION}.tar.gz")
set(FILENAME_URL "${VERSION}.tar.gz")
set(URL "https://github.com/digital-sound-antiques/${NAME}/archive/${FILENAME_URL}")
set(SHA_256_HASH "3695aa0e02235a1d018147c218821b0df43258ea5b4aaf730c51761d82853638")
set(UNPACKED_DIR "${NAME}-${VERSION}")
download_patch_and_add(
        ${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} true ${UNPACKED_DIR} ""
)

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
