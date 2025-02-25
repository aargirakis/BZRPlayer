cmake_minimum_required(VERSION 3.28)

set(NAME "fmod")
set(VERSION "2.02.27")
set(PLUGIN_${NAME}_NAME "FMOD")
set(PLUGIN_${NAME}_VERSION "${VERSION}")

set(LIB_NAME "${NAME}")
set(LIB_VERSION "${VERSION}")
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION}")
set(LIB_FILENAME "FMOD SoundSystem ${LIB_VERSION}.zip")
unpack_and_patch(
        ${CMAKE_CURRENT_LIST_DIR}/dist/${LIB_FILENAME} ${LIB_NAME_VERSIONED} true "" ${CMAKE_CURRENT_LIST_DIR}/patches
)

set(EXTERNAL_SOURCE_DIR_${LIB_NAME} "${EXTERNAL_SOURCE_DIR}/FMOD SoundSystem/FMOD Studio API Windows")

add_custom_target(
        copy-lib-${LIB_NAME} ALL
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${EXTERNAL_SOURCE_DIR_${LIB_NAME}}/api/core/lib/x86/fmod.dll
        ${OUTPUT_DIR}
        VERBATIM
)
