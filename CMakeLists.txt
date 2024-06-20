cmake_minimum_required(VERSION 3.28)
project(bzr2)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(QT_PATH_ROOT "C:/Qt")
set(QT_VERSION "5.15.2")
set(QT_PATH_VERSIONED "${QT_PATH_ROOT}/${QT_VERSION}")
set(QT_MINGW_PATH "${QT_PATH_VERSIONED}/mingw81_32")
set(CMAKE_PREFIX_PATH "${QT_MINGW_PATH}")
#set(CMAKE_C_COMPILER "${QT_PATH_ROOT}/Tools/mingw810_32/bin/gcc.exe")
#set(CMAKE_CXX_COMPILER "${QT_PATH_ROOT}/Tools/mingw810_32/bin/g++.exe")
#set(CMAKE_LINKER "${QT_PATH_ROOT}/Tools/mingw810_32/bin/ld.exe")
#set(CMAKE_RC_COMPILER "${QT_PATH_ROOT}/Tools/mingw810_32/bin/windres.exe")
#set(CMAKE_MAKE_PROGRAM "${QT_PATH_ROOT}/Tools/Ninja/ninja.exe")

#TODO debug crashes
#set(CMAKE_C_FLAGS_DEBUG "-O3 -DNDEBUG")
#set(CMAKE_CXX_FLAGS_DEBUG "-O3 -DNDEBUG")
#message(WARNING "CMAKE_C_FLAGS_DEBUG is ${CMAKE_C_FLAGS_DEBUG}")
#message(WARNING "CMAKE_CXX_FLAGS_DEBUG is ${CMAKE_CXX_FLAGS_DEBUG}")
#message(WARNING "CMAKE_C_FLAGS_RELEASE is ${CMAKE_C_FLAGS_RELEASE}")
#message(WARNING "CMAKE_CXX_FLAGS_RELEASE is ${CMAKE_CXX_FLAGS_RELEASE}")

#SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
#SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")


#set(build_types Debug Release RelWithDebInfo MinSizeRel)
#if(NOT CMAKE_BUILD_TYPE)
#    message(STATUS "You can choose the type of build, options are:${build_types}")
#    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
#            "Options are ${build_types}"
#            FORCE
#    )
#    # Provide drop down menu options in cmake-gui
#    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${build_types})
#endif()
#message(STATUS "Doing a ${CMAKE_BUILD_TYPE} build")

find_package(Qt5 COMPONENTS
        Core
        Gui
        Network
        OpenGL
        Svg
        Xml
        REQUIRED)

add_executable(${PROJECT_NAME}
        src/about.cpp
        src/album.cpp
        src/albumgrid.cpp
        src/buttonoscilloscope.cpp
        src/channels.cpp
        src/dialogdeleteworkspace.cpp
        src/dialognewworkspace.cpp
        src/filedownloader.cpp
        src/fileinfoparser.cpp
        src/main.cpp
        src/mainwindow.cpp
        src/myitemdelegate.cpp
        src/noPageSlider.cpp
        src/patternview/AbstractPatternView.cpp
        src/patternview/AHXPatternView.cpp
        src/patternview/bitmapfont.cpp
        src/patternview/ChipTrackerPatternView.cpp
        src/patternview/Composer669PatternView.cpp
        src/patternview/DigiBooster17PatternView.cpp
        src/patternview/DigiBoosterProPatternView.cpp
        src/patternview/FastTracker1PatternView.cpp
        src/patternview/FastTracker2PatternView.cpp
        src/patternview/FastTracker24ChanPatternView.cpp
        src/patternview/FastTracker26ChanPatternView.cpp
        src/patternview/GameMusicCreatorPatternView.cpp
        src/patternview/GenericPatternView.cpp
        src/patternview/HivelyTrackerPatternView.cpp
        src/patternview/IceTrackerPatternView.cpp
        src/patternview/ImpulseTrackerPatternView.cpp
        src/patternview/MEDPatternView.cpp
        src/patternview/MultiTrackerPatternView.cpp
        src/patternview/NoiseTrackerPatternView.cpp
        src/patternview/OctaMED5ChanPatternView.cpp
        src/patternview/OctaMED44ChanPatternView.cpp
        src/patternview/OctaMED54ChanPatternView.cpp
        src/patternview/OctaMEDPatternView.cpp
        src/patternview/OctaMEDSoundstudioPatternView.cpp
        src/patternview/OktalyzerPatternView.cpp
        src/patternview/ProTracker1PatternView.cpp
        src/patternview/ProTracker36PatternView.cpp
        src/patternview/ScreamTracker2PatternView.cpp
        src/patternview/ScreamTracker3PatternView.cpp
        src/patternview/SoundFXPatternView.cpp
        src/patternview/SoundTracker26PatternView.cpp
        src/patternview/StarTrekker13PatternView.cpp
        src/patternview/UltimateSoundTrackerPatternView.cpp
        src/patternview/UltraTrackerPatternView.cpp
        src/playlist.cpp
        src/playlistitem.cpp
        src/playlistmodel.cpp
        src/qlistwidgetcustom.cpp
        src/QTableWidgetCustom.cpp
        src/settingswindow.cpp
        src/soundmanager.cpp
        src/trackerview.cpp
        src/various.cpp
        src/visualizer.cpp
        src/visualizers/effect.cpp
        src/visualizers/parallax.cpp
        src/visualizers/scroller.cpp
        src/visualizers/tracker.cpp
        src/visualizers/trackerfullscreen.cpp
        src/visualizers/visualizerfullscreen.cpp

        src/resource.qrc

        src/about.ui
        src/dialogdeleteworkspace.ui
        src/dialognewworkspace.ui
        src/mainwindow.ui
        src/settingswindow.ui

        src/BZRPlayer.rc
        src/BZRPlayer.exe.manifest
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "BZRPlayer"
        COMPILE_FLAGS "-m32"
        LINK_FLAGS "-m32"
        WIN32_EXECUTABLE true
)

target_include_directories(${PROJECT_NAME} PRIVATE
        external/FMOD/api/core/inc
        external/libmodplug/libmodplug
        external/Qt-Advanced-Docking-System-4.0.0/src
        src
        src/include
        src/patternview
     #   src/visualizers

        #TODO BZRPlayerTest sources are missing
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt5::Core
        Qt5::Gui
        Qt5::Network
        Qt5::OpenGL
        Qt5::Svg
        Qt5::Xml

        -L${CMAKE_SOURCE_DIR}/external/FMOD/api/core/lib/x86 -lfmod
        -L${CMAKE_SOURCE_DIR}/external/Qt-Advanced-Docking-System-4.0.0/bin -lqtadvanceddocking
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_NAME}_debug_output"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_NAME}_release_output"
)

set(WINDEPLOYQT_EXECUTABLE "${QT_MINGW_PATH}/bin/windeployqt.exe")

add_custom_command(TARGET ${PROJECT_NAME}
        COMMAND "${WINDEPLOYQT_EXECUTABLE}" \"$<TARGET_FILE:${PROJECT_NAME}>\"
        COMMENT "Deploy Qt libraries using ${WINDEPLOYQT_EXECUTABLE} for compilation target ${target}"
)

add_custom_command(TARGET ${PROJECT_NAME}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${QT_MINGW_PATH}/bin/libgcc_s_dw2-1.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${QT_MINGW_PATH}/bin/libstdc++-6.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${QT_MINGW_PATH}/bin/libwinpthread-1.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMENT "Deploy mingw runtime libraries from ${QT_MINGW_PATH}/bin"
)

#TODO miss: libcrypto-1_1.dll, libssl-1_1.dll, Qt5OpenGL.dll
#TODO check which ones are really needed
#TODO remove unwanted qt dlls
add_custom_command(TARGET ${PROJECT_NAME}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/LICENSE $<TARGET_FILE_DIR:${PROJECT_NAME}>/LICENSE.txt

        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/external/FMOD/api/core/lib/x86/fmod.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>

        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${CMAKE_SOURCE_DIR}/static/data $<TARGET_FILE_DIR:${PROJECT_NAME}>/data
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${CMAKE_SOURCE_DIR}/static/user $<TARGET_FILE_DIR:${PROJECT_NAME}>/user

        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/static/BZRPlayerTest.exe $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/static/BZRPlayer.VisualElementsManifest.xml $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/static/qtadvanceddocking.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/static/zlib1.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>

        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/src/plugins/FMOD_klystrack_plugin/src/x86/release/ksnd.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/src/plugins/FMOD_libopenmpt_plugin/src/libopenmpt-0.7.6+release/bin/libopenmpt.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/src/plugins/FMOD_sunvox_plugin/sunvox_lib/windows/lib_x86/sunvox.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>

        COMMENT "Deploy other files"
)

get_target_property(RELEASE_OUTPUT_DIR_NAME ${PROJECT_NAME} RUNTIME_OUTPUT_DIRECTORY_RELEASE)

#TODO exclude parent source dir from zip file
#TODO manage versions (and removal of old versioned zip)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/${RELEASE_OUTPUT_DIR_NAME})

        set(SOURCE_PATHS
                "${OUTPUT_DIR}/data " "${OUTPUT_DIR}/platforms " "${OUTPUT_DIR}/user/plugin/sid/Songlengths.md5 "
                "${OUTPUT_DIR}/BZRPlayer.exe " "${OUTPUT_DIR}/BZRPlayerTest.exe "
                "${OUTPUT_DIR}/BZRPlayer.VisualElementsManifest.xml "
                "${OUTPUT_DIR}/fmod.dll " "${OUTPUT_DIR}/ksnd.dll "
                "${OUTPUT_DIR}/libgcc_s_dw2-1.dll " "${OUTPUT_DIR}/libopenmpt.dll "
                "${OUTPUT_DIR}/libstdc++-6.dll " "${OUTPUT_DIR}/libwinpthread-1.dll "
                "${OUTPUT_DIR}/LICENSE.txt "
                "${OUTPUT_DIR}/Qt5Core.dll " "${OUTPUT_DIR}/Qt5Gui.dll "
                "${OUTPUT_DIR}/Qt5Network.dll " "${OUTPUT_DIR}/Qt5Svg.dll "
                "${OUTPUT_DIR}/Qt5Widgets.dll " "${OUTPUT_DIR}/Qt5Xml.dll "
                "${OUTPUT_DIR}/qtadvanceddocking.dll " "${OUTPUT_DIR}/sunvox.dll "
                "${OUTPUT_DIR}/zlib1.dll "
        )

        string (REPLACE ";" "$<SEMICOLON>" SOURCE_PATHS_JOINED "${SOURCE_PATHS}")

        #TODO
        file(REMOVE ${CMAKE_BINARY_DIR}/BZR-Player-ver.zip)

        #TODO sometimes zip it is not generated
        add_custom_command(TARGET ${PROJECT_NAME}
                POST_BUILD
                COMMAND ${CMAKE_COMMAND}
                -DSOURCE_PATHS="${SOURCE_PATHS_JOINED}"
                -DTARGET_PATH=${CMAKE_BINARY_DIR}/BZR-Player-ver.zip
                -DFORMAT="zip"
                -P ${CMAKE_CURRENT_LIST_DIR}/generate_release_zip.cmake
        )
endif()

#TODO preserve date when copy files
