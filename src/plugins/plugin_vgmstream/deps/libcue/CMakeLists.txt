cmake_minimum_required(VERSION 3.28)

set(NAME "libcue")
project(${NAME})

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(VERSION "f35fd675703d8984c2ea26cca7cb403493368fd5")
string(SUBSTRING ${VERSION} 0 12 VERSION_SHORT)
set(NAME_VERSIONED "${NAME}-${VERSION_SHORT}")
set(FILENAME "${NAME}-${VERSION}.tar.gz")
set(FILENAME_URL "${VERSION}.tar.gz")
set(URL "https://github.com/lipnitsk/${NAME}/archive/${FILENAME_URL}")
set(SHA_256_HASH "2f1b58fb1e315d976be2d8441b9dc3d336ce9dfde7ec7a8f263ade905749525d")
set(UNPACKED_DIR "${NAME}-${VERSION}")
download_patch_and_add(
        ${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} true ${UNPACKED_DIR} ""
)

set(EXTERNAL_SOURCE_DIR_${NAME} "${EXTERNAL_SOURCE_DIR}" PARENT_SCOPE)

bison_target(CueParser ${EXTERNAL_SOURCE_DIR}/cue_parser.y ${EXTERNAL_SOURCE_DIR}/cue_parser.c)
flex_target(CueScanner ${EXTERNAL_SOURCE_DIR}/cue_scanner.l ${EXTERNAL_SOURCE_DIR}/cue_scanner.c)
add_flex_bison_dependency(CueScanner CueParser)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/cd.c
        ${EXTERNAL_SOURCE_DIR}/cdtext.c
        ${EXTERNAL_SOURCE_DIR}/rem.c
        ${EXTERNAL_SOURCE_DIR}/time.c)

add_library(${PROJECT_NAME} OBJECT ${SOURCES}
        ${BISON_CueParser_OUTPUTS}
        ${FLEX_CueScanner_OUTPUTS})

target_include_directories(${PROJECT_NAME} PUBLIC ${EXTERNAL_SOURCE_DIR}/libcue.h)

set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
