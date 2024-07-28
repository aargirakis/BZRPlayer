cmake_minimum_required(VERSION 3.28)

set(GIT_REPO_NAME "psflib")
set(GIT_REPO_URL "https://gitlab.com/kode54/${GIT_REPO_NAME}.git")
set(GIT_CLONE_AT "3bea757c8b45c5e68da1b5a7b736ad960a06a124")
set(GIT_IS_TAG false)

FetchContent_GetProperties(${GIT_REPO_NAME})

if (NOT ${GIT_REPO_NAME}_POPULATED)
    clone_and_patch(
            ${GIT_REPO_NAME} ${GIT_REPO_URL} ${GIT_CLONE_AT} ${GIT_IS_TAG}
            ""
            ""
    )

    set(${GIT_REPO_NAME}_POPULATED true)
    set(EXTERNAL_SOURCE_DIR_${GIT_REPO_NAME} "${EXTERNAL_SOURCE_DIR}")
    set(EXTERNAL_SOURCE_DIR_${GIT_REPO_NAME} "${EXTERNAL_SOURCE_DIR}" PARENT_SCOPE)
endif ()