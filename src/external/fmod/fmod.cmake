cmake_minimum_required(VERSION 3.28)

set(NAME "fmod")
set(VERSION "2.02.27")
set(PLUGIN_${NAME}_NAME "FMOD")
set(PLUGIN_${NAME}_VERSION "${VERSION}")

set(NAME_VERSIONED "${NAME}-${VERSION}")

if (WIN32)
    set(FILENAME "FMOD SoundSystem ${VERSION}.zip")
    unpack_and_patch(
            ${CMAKE_CURRENT_LIST_DIR}/dist/${FILENAME} ${NAME_VERSIONED} true ""
            ${CMAKE_CURRENT_LIST_DIR}/patches/win
    )

    set(EXTERNAL_SOURCE_DIR_${NAME} "${EXTERNAL_SOURCE_DIR}/FMOD SoundSystem/FMOD Studio API Windows")
    set(FMOD_DIR "${EXTERNAL_SOURCE_DIR_${NAME}}/api/core/lib/x64")
    set(FMOD_LIB "fmod.dll")

    add_custom_target(copy-lib-${NAME} ALL
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${FMOD_DIR}/${FMOD_LIB} ${OUTPUT_DIR}
            VERBATIM)
else ()
    string(REPLACE "." "" UNPACKED_DIR "fmodstudioapi${VERSION}linux")
    set(FILENAME "${UNPACKED_DIR}.tar.gz")
    unpack_and_patch(
            ${CMAKE_CURRENT_LIST_DIR}/dist/${FILENAME} ${NAME_VERSIONED} true ${UNPACKED_DIR}
            ${CMAKE_CURRENT_LIST_DIR}/patches/linux
    )

    set(EXTERNAL_SOURCE_DIR_${NAME} "${EXTERNAL_SOURCE_DIR}")
    set(FMOD_DIR "${EXTERNAL_SOURCE_DIR_${NAME}}/api/core/lib/x86_64")
    set(FMOD_SONAME "libfmod.so.13")
    set(FMOD_REALNAME "${FMOD_SONAME}.27")

    cmake_path(SET FMOD_LIB_DIR NORMALIZE ${OUTPUT_DIR}${LIB_DIR})

    add_custom_target(copy-lib-${NAME} ALL
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${FMOD_DIR}/${FMOD_REALNAME} ${FMOD_LIB_DIR}/${FMOD_SONAME}
            VERBATIM)
endif ()

set(FMOD_API_DIR "${EXTERNAL_SOURCE_DIR_${NAME}}/api/core/inc")
