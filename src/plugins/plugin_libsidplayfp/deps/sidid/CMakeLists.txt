cmake_minimum_required(VERSION 3.28)

set(NAME "sidid")
project(${NAME})

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(VERSION "56ba876ad30cbdd4ea7e330df7f9bb19ec899ed4")
string(SUBSTRING ${VERSION} 0 12 VERSION_SHORT)
set(NAME_VERSIONED "${NAME}-${VERSION_SHORT}")
set(FILENAME "${NAME}-${VERSION}.tar.gz")
set(FILENAME_URL "${VERSION}.tar.gz")
set(URL "https://github.com/cadaver/${NAME}/archive/${FILENAME_URL}")
set(SHA_256_HASH "893e4e375279b9b9efeb21580b4b907286282fa77b338dff1038639e3fa27d47")
set(UNPACKED_DIR "${NAME}-${VERSION}")
download_patch_and_add(
        ${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} true ${UNPACKED_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/patches
)

set(EXTERNAL_SOURCE_DIR_${NAME} "${EXTERNAL_SOURCE_DIR}" PARENT_SCOPE)

file(GLOB SOURCES ${EXTERNAL_SOURCE_DIR}/sidid.c)

add_library(${PROJECT_NAME} OBJECT ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE src)

set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
