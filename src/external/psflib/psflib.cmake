cmake_minimum_required(VERSION 3.28)

set(LIB_NAME "psflib")
set(LIB_VERSION "3bea757c8b45c5e68da1b5a7b736ad960a06a124")
string(SUBSTRING ${LIB_VERSION} 0 12 VERSION_SHORT)
set(LIB_NAME_VERSIONED "${LIB_NAME}-${VERSION_SHORT}")
set(LIB_FILENAME "${LIB_NAME}-${LIB_VERSION}.zip")
set(LIB_URL "https://gitlab.com/kode54/${LIB_NAME}/-/archive/${LIB_VERSION}/${LIB_FILENAME}")
set(LIB_SHA_256_HASH "90fd3cdf2e4309fa548dcbecf82f2d37f77d674f8427e2162ac5065698343093")
set(LIB_UNPACKED_DIR "${LIB_NAME}-${LIB_VERSION}")
if (NOT TARGET "${LIB_NAME_VERSIONED}")
    download_patch_and_add(
            ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH} true ${LIB_UNPACKED_DIR} ""
    )

    set(EXTERNAL_SOURCE_DIR_${LIB_NAME} "${EXTERNAL_SOURCE_DIR}")
    set(EXTERNAL_SOURCE_DIR_${LIB_NAME} "${EXTERNAL_SOURCE_DIR}" PARENT_SCOPE)
endif ()