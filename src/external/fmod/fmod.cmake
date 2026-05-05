cmake_minimum_required(VERSION 3.28)

set(NAME "fmod")
set(VERSION "2.03.09")
set(PLUGIN_${NAME}_NAME "FMOD")
set(PLUGIN_${NAME}_VERSION "${VERSION}")

set(NAME_VERSIONED "${NAME}-${VERSION}")

if (WIN32)
    set(OS "win")
    set(ARCH "x64")
else ()
    set(OS "linux")
    set(ARCH "x86_64")
endif ()

string(REPLACE "." "" UNPACKED_DIR "fmodstudioapi${VERSION}${OS}")
set(FILENAME "${UNPACKED_DIR}.tar.gz")
unpack_and_patch(
        ${CMAKE_CURRENT_LIST_DIR}/dist/${FILENAME} ${NAME_VERSIONED} true ${UNPACKED_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/patches
)

set(EXTERNAL_SOURCE_DIR_${NAME} "${EXTERNAL_SOURCE_DIR}")
set(FMOD_DIR "${EXTERNAL_SOURCE_DIR_${NAME}}/api/core/lib/${ARCH}")
set(FMOD_API_DIR "${EXTERNAL_SOURCE_DIR_${NAME}}/api/core/inc")

if (WIN32)
    set(FMOD_LIB "fmod.dll")

    add_custom_target(copy-lib-${NAME} ALL
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${FMOD_DIR}/${FMOD_LIB} ${OUTPUT_DIR}
            VERBATIM)
else ()
    set(FMOD_SONAME "libfmod.so.14")
    set(FMOD_REALNAME "${FMOD_SONAME}.9")

    cmake_path(SET FMOD_LIB_DIR NORMALIZE ${OUTPUT_DIR}${LIB_DIR})

    add_custom_target(copy-lib-${NAME} ALL
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${FMOD_DIR}/${FMOD_REALNAME} ${FMOD_LIB_DIR}/${FMOD_SONAME}
            VERBATIM)
endif ()