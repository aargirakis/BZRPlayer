cmake_minimum_required(VERSION 3.28)

project(bzr2 VERSION "2.0.79")

include(GNUInstallDirs)

if (NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(WARNING "Only x64 build is supported")
    return()
endif ()

if (NOT DEFINED CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "" OR CMAKE_BUILD_TYPE STREQUAL "None")
    message(STATUS "CMAKE_BUILD_TYPE is 'None' or not set: forcing to 'Debug'")
    set(CMAKE_BUILD_TYPE "Debug")
endif ()

set(CMAKE_INSTALL_MESSAGE NEVER)
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/output")
cmake_path(SET OUTPUT_DIR NORMALIZE "${CMAKE_INSTALL_PREFIX}")

if (WIN32)
    set(EXE_NAME "BZRPlayer")
    set(PLATFORMS_DIR "/platforms")
    set(TLS_DIR "/tls")
    set(DATA_DIR "/data")
    set(USER_DIR "/user")
else ()
    set(EXE_NAME "bzr-player")
    set(PREFIX_DIR "${CMAKE_PREFIX_PATH}")
    set(BIN_DIR "${PREFIX_DIR}/${CMAKE_INSTALL_BINDIR}")
    set(LIB_DIR "${PREFIX_DIR}/${CMAKE_INSTALL_LIBDIR}/bzr-player")
    set(DATA_DIR "${PREFIX_DIR}/share/bzr-player")
    set(USER_DIR "bzr-player")
endif ()

set(RESOURCES_DIR "/resources")
set(LAYOUTS_DIR "/layouts")
set(PLAYLISTS_DIR "/playlists")
set(PLUGINS_DIR "/plugin")
set(PLUGINS_FMOD_DIR "${PLUGINS_DIR}/fmod")
set(PLUGINS_CONFIG_DIR "${PLUGINS_DIR}/config")

include(${CMAKE_SOURCE_DIR}/utils.cmake)
include(${CMAKE_SOURCE_DIR}/src/external/fmod/fmod.cmake)

set_platform_lib_ext()

add_compile_options(-w) #TODO remove

set(SUBDIRECTORIES_BY_MODULES
        app "src/app"

        plugin_vgmstream "src/plugins/plugin_vgmstream"

)

set(MODULES)
while (SUBDIRECTORIES_BY_MODULES)
    list(POP_FRONT SUBDIRECTORIES_BY_MODULES MODULE SUBDIRECTORY)
    list(APPEND MODULES ${MODULE})
    add_subdirectory(${SUBDIRECTORY})
endwhile ()

configure_file(
        ${CMAKE_SOURCE_DIR}/src/app/plugins.h.in
        ${CMAKE_SOURCE_DIR}/src/app/plugins.h
        @ONLY NEWLINE_STYLE UNIX
)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    if (WIN32)
        #TODO needed?
        if (NOT DEFINED CPACK_EXECUTABLE)
            set(CPACK_EXECUTABLE "${CMAKE_PREFIX_PATH}/bin/cpack")
        endif ()

        set(CPACK_PACKAGE_NAME "BZR-Player")
        set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
        set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-win64")
        set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
        set(CPACK_OUTPUT_FILE_PREFIX "${OUTPUT_DIR}_release")
        set(CPACK_GENERATOR ZIP)

        install(DIRECTORY
                ${OUTPUT_DIR}${DATA_DIR}
                ${OUTPUT_DIR}/${PLATFORMS_DIR}
                ${OUTPUT_DIR}/${TLS_DIR}
                DESTINATION .
        )

        install(FILES
                ${OUTPUT_DIR}/${EXE_NAME}.exe
                ${OUTPUT_DIR}/BZRPlayer.VisualElementsManifest.xml
                ${OUTPUT_DIR}/LICENSE.txt
                ${OUTPUT_DIR}/${FMOD_LIB}
                ${RUNTIME_LIBS}
                DESTINATION .
        )

        include(CPack)

        add_custom_target(generate_win_zip ALL
                COMMAND ${CPACK_EXECUTABLE}
                DEPENDS ${MODULES})
    endif ()
endif ()
