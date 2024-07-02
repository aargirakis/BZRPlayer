cmake_minimum_required(VERSION 3.28)

project(bzr2 VERSION "2.0.73")
set(PROJECT_VERSION_PARENT ${PROJECT_VERSION})

#TODO remove
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")

set(OUTPUT_DIR_PARENT "${CMAKE_BINARY_DIR}/output")

set(SUBDIRECTORIES_BY_MODULES
        app "src"                                                                 #TODO songlength download doesnt work
        plugin_aac "src/plugins/FMOD_aac_plugin"
        plugin_adplug "src/plugins/FMOD_adplug_plugin"
        plugin_ahx "src/plugins/FMOD_AHX_plugin"                                  #TODO crash when opening tracker view during playback #check cmake TODO
        plugin_asap "src/plugins/FMOD_ASAP_plugin"
        plugin_audiofile "src/plugins/FMOD_audiofile_plugin"
        plugin_fc "src/plugins/FMOD_FC_plugin"
        plugin_flod "src/plugins/FMOD_Flod_plugin"
        plugin_furnace "src/plugins/FMOD_furnace_plugin"
        plugin_game "src/plugins/FMOD_Gamemusic_plugin"
        plugin_highly_experimental "src/plugins/FMOD_Highly_Experimental_plugin"  #check cmake TODO
        plugin_highly_quixotic "src/plugins/FMOD_Highly_Quixotic_plugin"          #check cmake TODO
        plugin_highly_theoretical "src/plugins/FMOD_Highly_Theoretical_plugin"    #check cmake TODO
        plugin_jaytrax "src/plugins/FMOD_jaytrax_plugin"
        plugin_kdm "src/plugins/FMOD_kdm_plugin"
        plugin_klystrack "src/plugins/FMOD_klystrack_plugin"                      #TODO never worked
        plugin_libopenmpt "src/plugins/FMOD_libopenmpt_plugin"                    #TODO crash when opening tracker view during playback
        plugin_libsidplayfp "src/plugins/FMOD_libsidplayfp_plugin"                #check cmake TODO
        plugin_libxmp "src/plugins/FMOD_libxmp_plugin"                            #TODO might crash when opening tracker view during playback
        plugin_mdx "src/plugins/FMOD_MDX_plugin"
        plugin_org "src/plugins/FMOD_org_plugin"                                   #TODO never worked ?
        plugin_pac "src/plugins/FMOD_pac_plugin"
)

set(MODULES)
while (SUBDIRECTORIES_BY_MODULES)
    list(POP_FRONT SUBDIRECTORIES_BY_MODULES MODULE SUBDIRECTORY)
    list(APPEND MODULES ${MODULE})
    add_subdirectory(${SUBDIRECTORY})
endwhile ()

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CPACK_GENERATOR "ZIP")
    set(CPACK_PACKAGE_NAME "BZR-Player")
    set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
    set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
    set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
    set(CPACK_OUTPUT_FILE_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/release_zip)
    #   set(CPACK_PACKAGE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/release_zip)


    #install(TARGETS app
    #        RUNTIME DESTINATION bin
    #        LIBRARY DESTINATION lib
    #        ARCHIVE DESTINATION lib
    #)

    install(
            DIRECTORY
            ${OUTPUT_DIR_PARENT}/data
            ${OUTPUT_DIR_PARENT}/platforms
            DESTINATION .
    )

    install(
            DIRECTORY
            ${OUTPUT_DIR_PARENT}/user/plugin/sid
            DESTINATION user/plugin
            FILES_MATCHING PATTERN "Songlengths.md5"
    )

    install(
            FILES
            ${OUTPUT_DIR_PARENT}/LICENSE.txt
            ${OUTPUT_DIR_PARENT}/BZRPlayer.exe
            ${OUTPUT_DIR_PARENT}/BZRPlayerTest.exe
            ${OUTPUT_DIR_PARENT}/BZRPlayer.VisualElementsManifest.xml
            ${OUTPUT_DIR_PARENT}/fmod.dll
            ${OUTPUT_DIR_PARENT}/ksnd.dll
            ${OUTPUT_DIR_PARENT}/libgcc_s_dw2-1.dll
            ${OUTPUT_DIR_PARENT}/libopenmpt.dll
            ${OUTPUT_DIR_PARENT}/libstdc++-6.dll
            ${OUTPUT_DIR_PARENT}/libwinpthread-1.dll
            ${OUTPUT_DIR_PARENT}/LICENSE.txt
            ${OUTPUT_DIR_PARENT}/Qt5Core.dll
            ${OUTPUT_DIR_PARENT}/Qt5Gui.dll
            ${OUTPUT_DIR_PARENT}/Qt5Network.dll

            #TODO not needed?
            ${OUTPUT_DIR_PARENT}/Qt5OpenGL.dll

            ${OUTPUT_DIR_PARENT}/Qt5Svg.dll
            ${OUTPUT_DIR_PARENT}/Qt5Widgets.dll
            ${OUTPUT_DIR_PARENT}/Qt5Xml.dll
            ${OUTPUT_DIR_PARENT}/qtadvanceddocking.dll
            ${OUTPUT_DIR_PARENT}/sunvox.dll
            ${OUTPUT_DIR_PARENT}/zlib1.dll
            DESTINATION .
    )

    include(CPack)

    add_custom_target(run_cpack ALL
            COMMAND ${CPACK_EXECUTABLE}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            DEPENDS ${MODULES}
    )
endif ()

#TODO delete output dir (at the right stage) #file(REMOVE_RECURSE ${OUTPUT_DIR_PARENT})
#TODO why compiled output on win is bigger?
#TODO manage plugin versions in settingswindow.cpp