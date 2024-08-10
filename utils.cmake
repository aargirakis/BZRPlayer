function(patch_sources target_name external_source_dir external_sources_to_remove patch_sources_dir)
    if (NOT external_sources_to_remove AND NOT patch_sources_dir)
        return()
    endif ()

    message(STATUS "Patching '${target_name}'")

    foreach (source_to_remove ${external_sources_to_remove})
        execute_process(
                COMMAND ${CMAKE_COMMAND} -E rm -f
                ${external_source_dir}/${source_to_remove}
                #RESULT_VARIABLE delete_result
        )
    endforeach ()

    if (patch_sources_dir)
        execute_process(
                COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
                ${patch_sources_dir} ${external_source_dir}
                RESULT_VARIABLE copy_result
                ERROR_VARIABLE copy_error
        )

        if (NOT ${copy_result} EQUAL 0)
            message(FATAL_ERROR "Error patching '${target_name}': ${copy_error}")
        endif ()
    endif ()
endfunction()

function(clone_and_patch repo_name repo_url repo_tag is_tag external_sources_to_remove patch_sources_dir)
    if (is_tag)
        set(clone_at tag)
    else ()
        set(clone_at commit)
    endif ()

    message(STATUS "Cloning repository '${repo_name}' ${repo_url} at ${clone_at} '${repo_tag}'")

    include(FetchContent)

    #set(PATCH_FILE git apply ${CMAKE_CURRENT_SOURCE_DIR}/patches/test.patch)
    FetchContent_Declare(
            ${repo_name}
            GIT_REPOSITORY ${repo_url}
            GIT_TAG ${repo_tag}
            GIT_SHALLOW ${is_tag}
            # PATCH_COMMAND ${PATCH_FILE}
    )

    FetchContent_Populate(${repo_name})

    set(external_source_dir ${${repo_name}_SOURCE_DIR})

    patch_sources("${repo_name}" "${external_source_dir}" "${external_sources_to_remove}" "${patch_sources_dir}")

    set(EXTERNAL_SOURCE_DIR ${external_source_dir} PARENT_SCOPE)
endfunction()

function(download_and_patch target_name target_name_versioned target_filename target_url sha_256_hash external_sources_to_remove patch_sources_dir)
    message(STATUS "Downloading '${target_name_versioned}' at '${target_url}'")

    set(external_source_dir "${CMAKE_BINARY_DIR}/${target_name_versioned}")

    set(downloaded_file "${CMAKE_BINARY_DIR}/${target_name_versioned}/${target_filename}")

    file(
            DOWNLOAD ${target_url} ${downloaded_file}
            EXPECTED_HASH SHA256=${sha_256_hash}
            STATUS DOWNLOAD_STATUS
    )

    list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
    list(GET DOWNLOAD_STATUS 1 ERROR_MESSAGE)

    if (NOT ${STATUS_CODE} EQUAL 0)
        message(FATAL_ERROR "Error downloading '${target_filename}': ${ERROR_MESSAGE}")
    endif ()

    execute_process(
            COMMAND ${CMAKE_COMMAND} -E tar xjf ${downloaded_file} #TODO? --directory ${UNTAR_DESTINATION}
            RESULT_VARIABLE UNPACK_RESULT
    )

    if (NOT UNPACK_RESULT EQUAL 0)
        message(FATAL_ERROR "Error unpacking '${downloaded_file}'")
    endif ()

    include(FetchContent)

    FetchContent_Declare(
            ${target_name}
            SOURCE_DIR ${external_source_dir}
    )

    FetchContent_Populate(${target_name})

    patch_sources("${target_name_versioned}" "${external_source_dir}" "${external_sources_to_remove}" "${patch_sources_dir}")

    set(EXTERNAL_SOURCE_DIR ${external_source_dir} PARENT_SCOPE)
endfunction()

function(download_to target_filename target_url sha_256_hash destination_path)
    message(STATUS "Downloading '${target_filename}' at '${target_url}' to '${destination_path}'")

    if ("${sha_256_hash}" STREQUAL "")
        file(
                DOWNLOAD ${target_url} ${destination_path}/${target_filename}
                STATUS DOWNLOAD_STATUS
        )
    else ()
        file(
                DOWNLOAD ${target_url} ${destination_path}/${target_filename}
                EXPECTED_HASH SHA256=${sha_256_hash}
                STATUS DOWNLOAD_STATUS
        )
    endif ()

    list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
    list(GET DOWNLOAD_STATUS 1 ERROR_MESSAGE)

    if (NOT ${STATUS_CODE} EQUAL 0)
        message(FATAL_ERROR "Error downloading '${target_filename}': ${ERROR_MESSAGE}")
    endif ()

endfunction()