cmake_minimum_required(VERSION 3.28)

set(NAME "fmt")
project(${NAME})

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(VERSION "e57ca2e3685b160617d3d95fcd9e789c4e06ca88")
string(SUBSTRING ${VERSION} 0 12 VERSION_SHORT)
set(NAME_VERSIONED "${NAME}-${VERSION_SHORT}")
set(FILENAME "${NAME}-${VERSION}.tar.gz")
set(FILENAME_URL "${VERSION}.tar.gz")
set(URL "https://github.com/fmtlib/${NAME}/archive/${FILENAME_URL}")
set(SHA_256_HASH "2d18d7b1c393791a180c8321cedd702093b5527625be955178bef8af2b1cf6db")
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
