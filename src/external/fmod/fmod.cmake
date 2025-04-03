cmake_minimum_required(VERSION 3.28)

set(NAME "fmod")
set(VERSION "2.02.27")
set(PLUGIN_${NAME}_NAME "FMOD")
set(PLUGIN_${NAME}_VERSION "${VERSION}")

set(LIB_NAME "${NAME}")
set(LIB_VERSION "${VERSION}")
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION}")

if (WIN32)
    set(LIB_FILENAME "FMOD SoundSystem ${LIB_VERSION}.zip")
    unpack_and_patch(
            ${CMAKE_CURRENT_LIST_DIR}/dist/${LIB_FILENAME} ${LIB_NAME_VERSIONED} true ""
            ${CMAKE_CURRENT_LIST_DIR}/patches/win
    )

    set(EXTERNAL_SOURCE_DIR_${LIB_NAME} "${EXTERNAL_SOURCE_DIR}/FMOD SoundSystem/FMOD Studio API Windows")
    set(LIB_FMOD_DIR "${EXTERNAL_SOURCE_DIR_${LIB_NAME}}/api/core/lib/x64")
    set(LIB_FMOD "fmod.dll")

    add_custom_target(copy-lib-${LIB_NAME} ALL
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${LIB_FMOD_DIR}/${LIB_FMOD} ${OUTPUT_DIR}
            VERBATIM)
else ()
    string(REPLACE "." "" LIB_UNPACKED_DIR "fmodstudioapi${LIB_VERSION}linux")
    set(LIB_FILENAME "${LIB_UNPACKED_DIR}.tar.gz")
    unpack_and_patch(
            ${CMAKE_CURRENT_LIST_DIR}/dist/${LIB_FILENAME} ${LIB_NAME_VERSIONED} true ${LIB_UNPACKED_DIR}
            ${CMAKE_CURRENT_LIST_DIR}/patches/linux
    )

    set(EXTERNAL_SOURCE_DIR_${LIB_NAME} "${EXTERNAL_SOURCE_DIR}")
    set(LIB_FMOD_DIR "${EXTERNAL_SOURCE_DIR_${LIB_NAME}}/api/core/lib/x86_64")
    set(LIB_FMOD_SONAME "libfmod.so.13")
    set(LIB_FMOD_REALNAME "${LIB_FMOD_SONAME}.27")

    cmake_path(SET FMOD_LIB_DIR NORMALIZE ${OUTPUT_DIR}${LIB_DIR})

    add_custom_target(copy-lib-${LIB_NAME} ALL
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${LIB_FMOD_DIR}/${LIB_FMOD_REALNAME} ${FMOD_LIB_DIR}/${LIB_FMOD_SONAME}
            VERBATIM)
endif ()

set(LIB_FMOD_API_DIR "${EXTERNAL_SOURCE_DIR_${LIB_NAME}}/api/core/inc")
