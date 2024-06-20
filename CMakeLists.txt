cmake_minimum_required(VERSION 3.28)

project(bzr2 VERSION "2.0.73")

configure_file(${CMAKE_SOURCE_DIR}/src/version.h.in ${CMAKE_SOURCE_DIR}/src/version.h @ONLY NEWLINE_STYLE UNIX)
configure_file(${CMAKE_SOURCE_DIR}/src/BZRPlayer.exe.manifest.in ${CMAKE_SOURCE_DIR}/src/BZRPlayer.exe.manifest @ONLY NEWLINE_STYLE UNIX)

set(OUTPUT_DIR_PARENT "${CMAKE_BINARY_DIR}/output")

add_subdirectory(src)
add_subdirectory(src/plugins/FMOD_aac_plugin)
add_subdirectory(src/plugins/FMOD_adplug_plugin)

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
            DEPENDS app plugin_aac plugin_adplug
            COMMAND ${CPACK_EXECUTABLE}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

endif ()

#TODO delete output dir (at the right stage) #file(REMOVE_RECURSE ${OUTPUT_DIR_PARENT})