cmake_minimum_required(VERSION 3.28)

include(ExternalProject)
include(FetchContent)
include(ProcessorCount)

set(DEPENDENCIES_DIR ${CMAKE_BINARY_DIR}/_deps)

function(set_platform_lib_ext)
    if (WIN32)
        set(LIB_EXT "dll" PARENT_SCOPE)
    else ()
        set(LIB_EXT "so" PARENT_SCOPE)
    endif ()
endfunction()

function(download_to target_filename target_url sha_256_hash
        destination_path display_destination_path target_name)
    if (OFFLINE_MODE EQUAL 1 OR "${target_url}" STREQUAL "")
        file(COPY ${CMAKE_CURRENT_LIST_DIR}/dist/${target_filename} DESTINATION ${destination_path})
    else ()
        if (NOT target_name)
            set(target_name "${target_filename}")
        endif ()

        if (display_destination_path)
            message(STATUS "Downloading ${target_name} ${target_url} to ${destination_path}")
        else ()
            message(STATUS "Downloading ${target_name} ${target_url}")
        endif ()

        if ("${sha_256_hash}" STREQUAL "")
            file(DOWNLOAD ${target_url} ${destination_path}/${target_filename} STATUS status)
        else ()
            file(
                    DOWNLOAD ${target_url} ${destination_path}/${target_filename}
                    EXPECTED_HASH SHA256=${sha_256_hash} STATUS status
            )
        endif ()

        list(GET status 0 status_code)
        list(GET status 1 error_message)

        if (NOT status_code EQUAL 0)
            message(FATAL_ERROR "Error downloading ${target_url}: ${error_message}")
        endif ()
    endif ()
endfunction()

function(unpack file_to_unpack unpack_to unpack_to_parent_dir)
    if (NOT unpack_to_parent_dir)
        set(unpack_to ${unpack_to}/..)
    endif ()

    file(ARCHIVE_EXTRACT INPUT "${file_to_unpack}" DESTINATION "${unpack_to}")
    set(EXTERNAL_SOURCE_DIR ${unpack_to} PARENT_SCOPE)
endfunction()

function(patch_sources target_name patches_dir target_dir)
    if (NOT patches_dir)
        return()
    endif ()

    message(STATUS "Patching ${target_name}")

    file(GLOB PATCH_FILES "${patches_dir}/*.patch")

    foreach (PATCH_FILE ${PATCH_FILES})
        if (NOT WIN32)
            execute_process(
                    COMMAND lsdiff ${PATCH_FILE}
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    OUTPUT_VARIABLE FILE_TO_PATCH
            )
            execute_process(
                    COMMAND dos2unix ${target_dir}/${FILE_TO_PATCH}
                    ERROR_QUIET
            )
        endif ()

        execute_process(
                COMMAND patch
                -ul -d ${target_dir}
                -p0 -i ${PATCH_FILE}
                RESULT_VARIABLE PATCH_RESULT
                OUTPUT_VARIABLE PATCH_OUTPUT
                ERROR_VARIABLE PATCH_ERROR
        )
        if (NOT PATCH_RESULT EQUAL 0)
            message(FATAL_ERROR
                    "Failed to apply patch ${PATCH_FILE}: ${PATCH_OUTPUT}"
                    "${PATCH_ERROR}"
            )
        endif ()
    endforeach ()
endfunction()

function(unpack_and_patch file_to_unpack target_name unpack_to_parent_dir
        target_unpacked_dir patches_dir)
    unpack("${file_to_unpack}" "${DEPENDENCIES_DIR}/${target_name}" ${unpack_to_parent_dir})

    set(EXTERNAL_SOURCE_DIR "${DEPENDENCIES_DIR}/${target_name}/${target_unpacked_dir}")

    patch_sources("${target_name}" "${patches_dir}" "${EXTERNAL_SOURCE_DIR}")

    set(EXTERNAL_SOURCE_DIR ${EXTERNAL_SOURCE_DIR} PARENT_SCOPE)
endfunction()

function(download_and_patch target_name target_filename target_url
        sha_256_hash unpack_to_parent_dir target_unpacked_dir patches_dir)
    if (OFFLINE_MODE EQUAL 1 OR "${target_url}" STREQUAL "")
        unpack_and_patch("${CMAKE_CURRENT_LIST_DIR}/dist/${target_filename}" "${target_name}"
                "${unpack_to_parent_dir}" "${target_unpacked_dir}" "${patches_dir}")
    else ()
        download_to("${target_filename}" "${target_url}" "${sha_256_hash}" "${DEPENDENCIES_DIR}/${target_name}"
                false "${target_name}")
        unpack_and_patch("${DEPENDENCIES_DIR}/${target_name}/${target_filename}" "${target_name}"
                "${unpack_to_parent_dir}" "${target_unpacked_dir}" "${patches_dir}")
    endif ()

    set(EXTERNAL_SOURCE_DIR ${EXTERNAL_SOURCE_DIR} PARENT_SCOPE)
endfunction()

function(download_patch_and_add target_name target_filename target_url
        sha_256_hash unpack_to_parent_dir target_unpacked_dir patches_dir)
    download_and_patch(
            "${target_name}" "${target_filename}" "${target_url}" "${sha_256_hash}" "${unpack_to_parent_dir}"
            "${target_unpacked_dir}" "${patches_dir}"
    )

    ExternalProject_Add(
            ${target_name}
            SOURCE_DIR ${EXTERNAL_SOURCE_DIR}
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ""
            INSTALL_COMMAND ""
    )

    set(EXTERNAL_SOURCE_DIR ${EXTERNAL_SOURCE_DIR} PARENT_SCOPE)
endfunction()

function(download_patch_and_make target_name target_filename target_url sha_256_hash
        unpack_to_parent_dir target_unpacked_dir patches_dir configure_command
        make_args allow_install build_byproducts)
    download_and_patch(
            "${target_name}" "${target_filename}" "${target_url}" "${sha_256_hash}" "${unpack_to_parent_dir}"
            "${target_unpacked_dir}" "${patches_dir}"
    )

    if (WIN32)
        if (configure_command)
            separate_arguments(configure_command UNIX_COMMAND "bash ${configure_command}")
        else ()
            set(configure_command rem)
        endif ()
    else ()
        if (configure_command)
            separate_arguments(configure_command UNIX_COMMAND "./${configure_command}")
        else ()
            set(configure_command :)
        endif ()
    endif ()

    if (WIN32)
        set(make_command mingw32-make AR=ar)
    else ()
        set(make_command make)
    endif ()

    ProcessorCount(N)
    if (N EQUAL 0)
        set(N "")
    else ()
        math(EXPR N "${N}-1")
    endif ()

    set(make_command ${make_command} -j${N} ${make_args})

    if (allow_install)
        if (WIN32)
            set(install_command mingw32-make install)
        else ()
            set(install_command make install)
        endif ()
    endif ()

    foreach (build_byproduct ${build_byproducts})
        list(APPEND build_byproducts_with_full_path ${EXTERNAL_SOURCE_DIR}/${build_byproduct})
    endforeach ()

    ExternalProject_Add(
            ${target_name}
            SOURCE_DIR ${EXTERNAL_SOURCE_DIR}
            CONFIGURE_COMMAND ${configure_command}
            BUILD_COMMAND ${make_command}
            INSTALL_COMMAND "${install_command}"
            BUILD_IN_SOURCE 1
            BUILD_BYPRODUCTS ${build_byproducts_with_full_path}
    )

    set(EXTERNAL_SOURCE_DIR ${EXTERNAL_SOURCE_DIR} PARENT_SCOPE)
endfunction()

function(download_patch_and_cmake target_name target_filename target_url
        sha_256_hash unpack_to_parent_dir target_unpacked_dir patches_dir)
    download_and_patch(
            "${target_name}" "${target_filename}" "${target_url}" "${sha_256_hash}" "${unpack_to_parent_dir}"
            "${target_unpacked_dir}" "${patches_dir}"
    )

    set(FETCHCONTENT_BASE_DIR "${DEPENDENCIES_DIR}/${target_name}")

    FetchContent_Declare(
            ${target_name}
            SOURCE_DIR ${EXTERNAL_SOURCE_DIR}
            EXCLUDE_FROM_ALL
    )
    FetchContent_MakeAvailable(${target_name})

    set(EXTERNAL_SOURCE_DIR ${EXTERNAL_SOURCE_DIR} PARENT_SCOPE)
endfunction()
