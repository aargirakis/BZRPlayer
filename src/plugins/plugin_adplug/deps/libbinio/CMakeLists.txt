cmake_minimum_required(VERSION 3.28)

set(NAME "libbinio")
project(${NAME})

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(VERSION "1.5")
set(NAME_VERSIONED "${NAME}-${VERSION}")
set(NAME_VERSIONED2 "${NAME}-${NAME}-${VERSION}")
set(FILENAME "${NAME_VERSIONED2}.zip")
set(FILENAME_URL "${NAME_VERSIONED}.zip")
set(URL "https://github.com/adplug/${NAME}/archive/refs/tags/${FILENAME_URL}")
set(SHA_256_HASH "fd00c22d9ad3d4b5c90c4428d49719d0549f6a15cf5c612ffd5c9ba6c10689b9")
set(UNPACKED_DIR "${NAME_VERSIONED2}")
download_patch_and_add(
        ${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} true ${UNPACKED_DIR} ""
)

set(EXTERNAL_SOURCE_DIR_${NAME} "${EXTERNAL_SOURCE_DIR}" PARENT_SCOPE)

set(ENABLE_STRING 1)
set(ENABLE_IOSTREAM 1)
set(ISO_STDLIB 1)
set(WITH_MATH 1)
set(TYPE_INT "long long")
set(TYPE_FLOAT "long double")
configure_file(
        ${EXTERNAL_SOURCE_DIR}/src/binio.h.in
        ${EXTERNAL_SOURCE_DIR}/src/binio.h
        @ONLY NEWLINE_STYLE UNIX
)

file(GLOB SOURCES ${EXTERNAL_SOURCE_DIR}/src/*.cpp)

add_library(${PROJECT_NAME} OBJECT ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ${EXTERNAL_SOURCE_DIR}/src)

set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
