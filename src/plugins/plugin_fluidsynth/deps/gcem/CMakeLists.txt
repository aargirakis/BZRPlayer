cmake_minimum_required(VERSION 3.28)

set(NAME "gcem")
set(VERSION "012ae73c6d0a2cb09ffe86475f5c6fba3926e200")
file(READ ${EXTERNAL_SOURCE_DIR}cmake_admin/FindGCEM.cmake file_content)
string(REGEX MATCH "set\\(GCEM_REVISION \"([^\"]+)\"\\)" MATCH_RESULT ${file_content})

if (NOT MATCH_RESULT)
    message(FATAL_ERROR "Unable to check ${NAME} dependency version needed")
endif ()

if (NOT ${CMAKE_MATCH_1} STREQUAL ${VERSION})
    message(FATAL_ERROR "${NAME} needs to be updated (and possibly adapted) to version ${CMAKE_MATCH_1}")
endif ()

string(SUBSTRING ${VERSION} 0 12 VERSION_SHORT)
set(NAME_VERSIONED "${NAME}-${VERSION_SHORT}")
set(FILENAME "${NAME}-${VERSION}.tar.gz")
set(FILENAME_URL "${VERSION}.tar.gz")
set(URL "https://github.com/kthohr/${NAME}/archive/${FILENAME_URL}")
set(SHA_256_HASH "34ab0ee87a9eb26d3087fa9b49c2572ea8ee03db0c9705b83648301a3a3fc172")
set(UNPACKED_DIR "${NAME}-${VERSION}")
download_patch_and_add(
        ${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} true ${UNPACKED_DIR} ""
)

set(EXTERNAL_SOURCE_DIR_${NAME} "${EXTERNAL_SOURCE_DIR}" PARENT_SCOPE)
