cmake_minimum_required(VERSION 3.28)

include(ExternalProject)

set(DEPENDENCIES_DIR ${CMAKE_BINARY_DIR}/_deps)

function(unpack file_to_unpack unpack_to unpack_to_parent_dir)
    if (NOT unpack_to_parent_dir)
        set(unpack_to ${unpack_to}/..)
    endif ()

    file(ARCHIVE_EXTRACT INPUT "${file_to_unpack}" DESTINATION "${unpack_to}")
    set(EXTERNAL_SOURCE_DIR ${unpack_to} PARENT_SCOPE)
endfunction()

function(patch_sources target_name patches_dir external_source_dir)
    if (NOT patches_dir)
        return()
    endif ()

    message(STATUS "Patching '${target_name}'")

    if (patches_dir)
        execute_process(
                COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
                ${patches_dir} ${external_source_dir}
                RESULT_VARIABLE copy_result
                ERROR_VARIABLE copy_error
        )

        if (NOT ${copy_result} EQUAL 0)
            message(FATAL_ERROR "Error patching '${target_name}': ${copy_error}")
        endif ()
    endif ()
endfunction()

function(unpack_and_patch file_to_unpack target_name_versioned unpack_to_parent_dir target_unpacked_dir patches_dir)
    unpack("${file_to_unpack}" "${DEPENDENCIES_DIR}/${target_name_versioned}" ${unpack_to_parent_dir})

    set(external_source_dir "${DEPENDENCIES_DIR}/${target_name_versioned}/${target_unpacked_dir}")

    patch_sources("${target_name_versioned}" "${patches_dir}" "${external_source_dir}")

    set(external_source_dir "${DEPENDENCIES_DIR}/${target_name_versioned}/${target_unpacked_dir}" PARENT_SCOPE)
    set(EXTERNAL_SOURCE_DIR ${EXTERNAL_SOURCE_DIR} PARENT_SCOPE)
endfunction()

function(download_patch_and_make target_name target_name_versioned target_filename target_url sha_256_hash unpack_to_parent_dir target_unpacked_dir patches_dir make_args)
    message(STATUS "Downloading '${target_name_versioned}' at '${target_url}'")

    set(downloaded_file "${DEPENDENCIES_DIR}/${target_name_versioned}/${target_filename}")

    file(
            DOWNLOAD ${target_url} ${downloaded_file}
            EXPECTED_HASH SHA256=${sha_256_hash}
    )

    unpack_and_patch("${downloaded_file}" "${target_name_versioned}" "${unpack_to_parent_dir}" "${target_unpacked_dir}" "${patches_dir}")

    if (make_args)
        if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
            set(make_command
                    ${CMAKE_PREFIX_PATH}/../usr/bin/make.exe
                    AR=${CMAKE_PREFIX_PATH}/bin/ar.exe
            )
        else ()
            set(make_command make)
        endif ()

        set(make_command ${make_command} -j -l10 ${make_args}) #TODO -j -l10?
    endif ()

    ExternalProject_Add(  #TODO add update step & patch step with ExternalProject_Add
            ${target_name}_ExtProj
            #URL ${target_url}
            #URL_HASH SHA256=${sha_256_hash}
            SOURCE_DIR ${external_source_dir}
            DOWNLOAD_COMMAND ""
            #PATCH_COMMAND ""
            CONFIGURE_COMMAND ""
            BUILD_COMMAND "${make_command}"
            INSTALL_COMMAND ""
            WORKING_DIRECTORY ${external_source_dir}
            #DOWNLOAD_NO_PROGRESS true
            #CONFIGURE_HANDLED_BY_BUILD true
            BUILD_IN_SOURCE 1
            #BUILD_ALWAYS OFF
            #TODO skip download and extraction when not needed
    )

    set(EXTERNAL_SOURCE_DIR ${external_source_dir} PARENT_SCOPE)
endfunction()

function(download_and_patch target_name target_name_versioned target_filename target_url sha_256_hash unpack_to_parent_dir target_unpacked_dir patches_dir)
    download_patch_and_make(
            "${target_name}" "${target_name_versioned}" "${target_filename}" "${target_url}" "${sha_256_hash}" "${unpack_to_parent_dir}" "${target_unpacked_dir}"
            "${patches_dir}" ""
    )

    set(EXTERNAL_SOURCE_DIR ${EXTERNAL_SOURCE_DIR} PARENT_SCOPE)
endfunction()

function(download_to target_filename target_url sha_256_hash destination_path)
    message(STATUS "Downloading '${target_filename}' at '${target_url}' to '${destination_path}'")

    if ("${sha_256_hash}" STREQUAL "")
        file(
                DOWNLOAD ${target_url} ${destination_path}/${target_filename}
        )
    else ()
        file(
                DOWNLOAD ${target_url} ${destination_path}/${target_filename}
                EXPECTED_HASH SHA256=${sha_256_hash}
        )
    endif ()
endfunction()
