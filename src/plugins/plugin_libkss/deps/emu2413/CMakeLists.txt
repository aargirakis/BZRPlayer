cmake_minimum_required(VERSION 3.28)

set(NAME "emu2413")
project(${NAME})

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(VERSION "104bc22e85bfdeb1c157c4beee3a75df89701028")
string(SUBSTRING ${VERSION} 0 12 VERSION_SHORT)
set(NAME_VERSIONED "${NAME}-${VERSION_SHORT}")
set(FILENAME "${NAME}-${VERSION}.tar.gz")
set(FILENAME_URL "${VERSION}.tar.gz")
set(URL "https://github.com/digital-sound-antiques/${NAME}/archive/${FILENAME_URL}")
set(SHA_256_HASH "dc926b86b2d7a5bd3d45e0e3d9479381a19fb6632551f03a85f263b0973e47bd")
set(UNPACKED_DIR "${NAME}-${VERSION}")
download_patch_and_add(${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} true ${UNPACKED_DIR} "")

file(GLOB SOURCES ${EXTERNAL_SOURCE_DIR}/emu2413.c)

add_library(${PROJECT_NAME} OBJECT ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ${EXTERNAL_SOURCE_DIR})

set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
