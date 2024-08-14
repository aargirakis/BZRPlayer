include(FetchContent)
include(ExternalProject)

function(patch_sources target_name external_source_dir patch_sources_dir)
    if (NOT patch_sources_dir)
        return()
    endif ()

    message(STATUS "Patching '${target_name}'")

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

function(download_patch_and_make target_name target_name_versioned target_filename target_url sha_256_hash patch_sources_dir make_args)
    message(STATUS "Downloading '${target_name_versioned}' at '${target_url}'")

    set(external_source_dir "${CMAKE_BINARY_DIR}/_deps/${target_name_versioned}")
    set(downloaded_file "${external_source_dir}/${target_filename}")

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
            COMMAND ${CMAKE_COMMAND} -E tar xjf ${downloaded_file}
            WORKING_DIRECTORY ${external_source_dir}/..
            RESULT_VARIABLE UNPACK_RESULT
    )

    if (NOT UNPACK_RESULT EQUAL 0)
        message(FATAL_ERROR "Error unpacking '${downloaded_file}'")
    endif ()

    patch_sources("${target_name_versioned}" "${external_source_dir}" "${patch_sources_dir}")

    if (make_args)
        if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
            set(make_command
                    ${CMAKE_PREFIX_PATH}/../../Tools/mingw810_32/bin/mingw32-make.exe
                    AR=${CMAKE_PREFIX_PATH}/../../Tools/mingw810_32/bin/ar.exe
            )
        else ()
            set(make_command make)
        endif ()

        set(make_command ${make_command} -j -l10 ${make_args}) #TODO -j -l10?
    endif ()

    ExternalProject_Add(  #TODO add update step & patch step with ExternalProject_Add
            ${target_name}_ExternalProject_Add
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

function(download_and_patch target_name target_name_versioned target_filename target_url sha_256_hash patch_sources_dir)
    download_patch_and_make(
            "${target_name}" "${target_name_versioned}" "${target_filename}" "${target_url}" "${sha_256_hash}"
            "${patch_sources_dir}" ""
    )

    set(EXTERNAL_SOURCE_DIR ${EXTERNAL_SOURCE_DIR} PARENT_SCOPE)
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

#TODO remove (unused & deprecated)
function(clone_and_patch repo_name repo_url repo_tag is_tag patch_sources_dir)
    if (is_tag)
        set(clone_at tag)
    else ()
        set(clone_at commit)
    endif ()

    message(STATUS "Cloning repository '${repo_name}' ${repo_url} at ${clone_at} '${repo_tag}'")

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

    patch_sources("${repo_name}" "${external_source_dir}" "${patch_sources_dir}")

    set(EXTERNAL_SOURCE_DIR ${external_source_dir} PARENT_SCOPE)
endfunction()