cmake_minimum_required(VERSION 3.28)

set(LIB_NAME "qt-ads")
set(LIB_NAME2 "Qt-Advanced-Docking-System")
string(TOLOWER ${LIB_NAME2} LIB_NAME3)
set(LIB_VERSION "4.3.1")
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION}")
set(LIB_NAME_VERSIONED2 "${LIB_NAME2}-${LIB_VERSION}")
set(LIB_FILENAME "${LIB_NAME_VERSIONED2}.zip")
set(LIB_FILENAME_URL "${LIB_VERSION}.zip")
set(LIB_URL "https://github.com/githubuser0xFFFF/${LIB_NAME3}/archive/refs/tags/${LIB_FILENAME_URL}")
set(LIB_SHA_256_HASH "40d37227eb88d4f88f0eb918cdba60959d6cb9cb52796fe156565a2d7cc2a146")
set(LIB_UNPACKED_DIR "${LIB_NAME_VERSIONED2}")
set(ADS_VERSION ${LIB_VERSION})
set(CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}")
set(CMAKE_SIZEOF_VOID_P "4")
download_patch_and_cmake(
        ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH} true ${LIB_UNPACKED_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/patches
)

set_target_properties(qt5advanceddocking PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ""
        LIBRARY_OUTPUT_DIRECTORY ""
        RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}"
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(LIB_QT_ADVANCED_DOCKING_SYSTEM "${OUTPUT_DIR}/libqt5advanceddockingd.dll")
else ()
    set(LIB_QT_ADVANCED_DOCKING_SYSTEM "${OUTPUT_DIR}/libqt5advanceddocking.dll")
endif ()

set(EXTERNAL_SOURCE_DIR_${LIB_NAME} "${EXTERNAL_SOURCE_DIR}")
