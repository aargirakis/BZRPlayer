cmake_minimum_required(VERSION 3.28)

set(NAME "fmt")
project(${NAME})

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(VERSION "e57ca2e3685b160617d3d95fcd9e789c4e06ca88")
string(SUBSTRING ${VERSION} 0 12 VERSION_SHORT)
set(NAME_VERSIONED "${NAME}-${VERSION_SHORT}")
set(FILENAME "${NAME}-${VERSION}.zip")
set(FILENAME_URL "${VERSION}.zip")
set(URL "https://github.com/fmtlib/${NAME}/archive/${FILENAME_URL}")
set(SHA_256_HASH "38004c88c72900a935cbeb01d7668af86ec1b9b10273555b02967f8fd574afc1")
set(UNPACKED_DIR "${NAME}-${VERSION}")
download_patch_and_add(
        ${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} true ${UNPACKED_DIR} ""
)

set(EXTERNAL_SOURCE_DIR_${NAME} "${EXTERNAL_SOURCE_DIR}" PARENT_SCOPE)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/src/format.cc
        ${EXTERNAL_SOURCE_DIR}/src/os.cc)

add_library(${PROJECT_NAME} OBJECT ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ${EXTERNAL_SOURCE_DIR}/include)

set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
