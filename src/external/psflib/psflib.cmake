cmake_minimum_required(VERSION 3.28)

set(LIB_NAME "psflib")
set(LIB_VERSION "3bea757c8b45c5e68da1b5a7b736ad960a06a124")
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION}")
set(LIB_FILENAME "${LIB_NAME_VERSIONED}.zip")
set(LIB_URL "https://gitlab.com/kode54/${LIB_NAME}/-/archive/${LIB_VERSION}/${LIB_FILENAME}")
set(LIB_SHA_256_HASH "16cf2ffc3113c84edcb04e2980087626c70e1915c9597eeb0a5f70b36d30d1fd")
if (NOT TARGET "${LIB_NAME}_ExternalProject_Add")
    download_and_patch(
            ${LIB_NAME} ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH}
            ""
            ""
    )

    set(EXTERNAL_SOURCE_DIR_${LIB_NAME} "${EXTERNAL_SOURCE_DIR}")
    set(EXTERNAL_SOURCE_DIR_${LIB_NAME} "${EXTERNAL_SOURCE_DIR}" PARENT_SCOPE)
endif ()