cmake_minimum_required(VERSION 3.28)

project(bzr2 VERSION "2.0.75")

if (NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(WARNING "Only x64 build is supported")
    return()
endif ()

if (NOT DEFINED CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "" OR CMAKE_BUILD_TYPE STREQUAL "None")
    message(STATUS "CMAKE_BUILD_TYPE is 'None' or not set: forcing to 'Debug'")
    set(CMAKE_BUILD_TYPE "Debug")
endif ()

set(DATA_DIR "/data")
set(PLATFORMS_DIR "/platforms")
set(USER_DIR "/user")
set(DATA_PLUGINS_DIR "${DATA_DIR}/plugin")
set(DATA_PLUGINS_FMOD_DIR "${DATA_PLUGINS_DIR}/fmod")
set(USER_LAYOUTS_DIR "${USER_DIR}/layouts")
set(USER_PLAYLISTS_DIR "${USER_DIR}/playlists")
set(USER_PLUGINS_DIR "${USER_DIR}/plugin")
set(USER_PLUGINS_CONFIG_DIR "${USER_PLUGINS_DIR}/config")
set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/output")
set(OUTPUT_DATA_DIR "${OUTPUT_DIR}${DATA_DIR}")
set(OUTPUT_DATA_PLUGINS_DIR "${OUTPUT_DIR}${DATA_PLUGINS_DIR}")
set(OUTPUT_PLATFORMS_DIR "${OUTPUT_DIR}${PLATFORMS_DIR}")

include(${CMAKE_SOURCE_DIR}/utils.cmake)
include(${CMAKE_SOURCE_DIR}/src/external/fmod/fmod.cmake)

add_compile_options(-w) #TODO remove

set(SUBDIRECTORIES_BY_MODULES
        app "src/app"
        plugin_adplug "src/plugins/plugin_adplug"
        plugin_asap "src/plugins/plugin_asap"
        plugin_audiodecoder.wsr "src/plugins/plugin_audiodecoder.wsr"
        plugin_audiofile "src/plugins/plugin_audiofile"
        plugin_faad2 "src/plugins/plugin_faad2"
        plugin_flod "src/plugins/plugin_flod"
        plugin_furnace "src/plugins/plugin_furnace"
        plugin_game-music-emu "src/plugins/plugin_game-music-emu"
        plugin_highly_experimental "src/plugins/plugin_highly_experimental"  #check cmake TODO
        plugin_highly_quixotic "src/plugins/plugin_highly_quixotic"          #check cmake TODO
        plugin_highly_theoretical "src/plugins/plugin_highly_theoretical"    #check cmake TODO
        plugin_hivelytracker "src/plugins/plugin_hivelytracker"
        plugin_jaytrax "src/plugins/plugin_jaytrax"
        plugin_kdm "src/plugins/plugin_kdm"
        plugin_klystron "src/plugins/plugin_klystron"
        plugin_lazyusf2 "src/plugins/plugin_lazyusf2"
        plugin_libfc14audiodecoder "src/plugins/plugin_libfc14audiodecoder"
        plugin_libopenmpt "src/plugins/plugin_libopenmpt"
        plugin_libpac "src/plugins/plugin_libpac"
        plugin_libsidplayfp "src/plugins/plugin_libsidplayfp"
        plugin_libstsound "src/plugins/plugin_libstsound"
        plugin_libxmp "src/plugins/plugin_libxmp"
        plugin_mdxmini "src/plugins/plugin_mdxmini"
        plugin_organya-decoder "src/plugins/plugin_organya-decoder"
        plugin_protrekkr "src/plugins/plugin_protrekkr"
        plugin_sc68 "src/plugins/plugin_sc68"                                #TODO seeking it is now supported (should be implemented into main.cpp) #TODO also track length seems supported (see DisplayInfo function in sc68.c)
        plugin_sndh-player "src/plugins/plugin_sndh-player"                  #check cmake TODO
        plugin_sunvox_lib "src/plugins/plugin_sunvox_lib"                    #TODO add sv_unload_dll() (see sunvox_lib README)
        plugin_tunes98_plug "src/plugins/plugin_tunes98_plug"                #check cmake TODO
        plugin_v2m-player "src/plugins/plugin_v2m-player"                    #TODO seeking it is now supported (should be implemented into main.cpp). #TODO add v2mz support (see sources/v2m dir for example files)
        plugin_vgmplay-legacy "src/plugins/plugin_vgmplay-legacy"            #check cmake TODO
        plugin_vgmstream "src/plugins/plugin_vgmstream"
        plugin_vio2sf "src/plugins/plugin_vio2sf"                            #check cmake TODO
        plugin_webuade "src/plugins/plugin_webuade"        #TODO unfinished
        plugin_zxtune "src/plugins/plugin_zxtune"
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
    if (NOT DEFINED CPACK_EXECUTABLE)
        set(CPACK_EXECUTABLE "${CMAKE_PREFIX_PATH}/bin/cpack")
    endif ()
    set(CPACK_GENERATOR "ZIP")
    set(CPACK_PACKAGE_NAME "BZR-Player")
    set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
    set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
    set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
    set(CPACK_OUTPUT_FILE_PREFIX "${OUTPUT_DIR}_release")

    install(
            FILES ${RUNTIME_DLLS}
            DESTINATION .
    )

    install(
            DIRECTORY
            ${OUTPUT_DATA_DIR}
            ${OUTPUT_DIR}/platforms
            DESTINATION .
    )

    install(
            FILES
            ${OUTPUT_DIR}/BZRPlayer.exe
            ${OUTPUT_DIR}/BZRPlayer.VisualElementsManifest.xml
            ${OUTPUT_DIR}/fmod.dll
            ${LIB_QT_ADVANCED_DOCKING_SYSTEM}
            ${OUTPUT_DIR}/LICENSE.txt
            DESTINATION .
    )

    include(CPack)

    add_custom_target(
            cpack ALL
            COMMAND ${CPACK_EXECUTABLE}
            DEPENDS ${MODULES}
    )
endif ()

#TODO queue.h & queue.c (in hivelytracker & libxmp plugins) should be replaced by c++ std::queue (as in plugin_libopenmpt's main.cpp)
