cmake_minimum_required(VERSION 3.28)
project(app)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(PROJECT_VERSION ${PROJECT_VERSION_PARENT})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/version.h @ONLY NEWLINE_STYLE UNIX)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/BZRPlayer.exe.manifest.in ${CMAKE_CURRENT_SOURCE_DIR}/BZRPlayer.exe.manifest @ONLY NEWLINE_STYLE UNIX)

set(QT_VERSION_MAJOR "5")

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

set(QT_MODULES Core Gui Network OpenGL Svg Widgets Xml)

find_package(Qt5 COMPONENTS ${QT_MODULES} REQUIRED)

file(GLOB SOURCES
        *.cpp
        patternview/*.cpp
        visualizers/*.cpp

        resource.qrc

        about.ui
        dialogdeleteworkspace.ui
        dialognewworkspace.ui
        mainwindow.ui
        settingswindow.ui

        BZRPlayer.rc
        BZRPlayer.exe.manifest
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        external/FMOD/api/core/inc
        external/Qt-Advanced-Docking-System-4.0.0/src
)

foreach (QT_MODULE ${QT_MODULES})
    target_link_libraries(${PROJECT_NAME} PRIVATE "Qt${QT_VERSION_MAJOR}::${QT_MODULE}")
endforeach ()

target_link_libraries(${PROJECT_NAME} PRIVATE
        -L${CMAKE_CURRENT_SOURCE_DIR}/external/FMOD/api/core/lib/x86 -lfmod
        -L${CMAKE_CURRENT_SOURCE_DIR}/external/Qt-Advanced-Docking-System-4.0.0/bin -lqtadvanceddocking
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "BZRPlayer"
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR_PARENT}
        COMPILE_FLAGS "-m32"
        LINK_FLAGS "-m32"
        WIN32_EXECUTABLE true
)

foreach (QT_MODULE ${QT_MODULES})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_PREFIX_PATH}/bin/Qt${QT_VERSION_MAJOR}${QT_MODULE}.dll" ${OUTPUT_DIR_PARENT}
            VERBATIM
    )
endforeach ()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUT_DIR_PARENT}/platforms"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_PREFIX_PATH}/plugins/platforms/qwindows.dll" "${OUTPUT_DIR_PARENT}/platforms"
        VERBATIM
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_PREFIX_PATH}/bin/libgcc_s_dw2-1.dll ${OUTPUT_DIR_PARENT}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_PREFIX_PATH}/bin/libstdc++-6.dll ${OUTPUT_DIR_PARENT}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_PREFIX_PATH}/bin/libwinpthread-1.dll ${OUTPUT_DIR_PARENT}
        VERBATIM
)

##TODO miss: libcrypto-1_1.dll, libssl-1_1.dll (really needed?)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/LICENSE ${OUTPUT_DIR_PARENT}/LICENSE.txt

        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/external/FMOD/api/core/lib/x86/fmod.dll ${OUTPUT_DIR_PARENT}

        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${CMAKE_CURRENT_SOURCE_DIR}/static/data ${OUTPUT_DIR_PARENT}/data
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/static/user/plugin/sid/Songlengths.md5 ${OUTPUT_DIR_PARENT}/user/plugin/sid/Songlengths.md5

        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/static/BZRPlayerTest.exe ${OUTPUT_DIR_PARENT}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/static/BZRPlayer.VisualElementsManifest.xml ${OUTPUT_DIR_PARENT}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/static/qtadvanceddocking.dll ${OUTPUT_DIR_PARENT}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/static/zlib1.dll ${OUTPUT_DIR_PARENT}

        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/plugins/FMOD_klystrack_plugin/src/x86/release/ksnd.dll ${OUTPUT_DIR_PARENT}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/plugins/FMOD_libopenmpt_plugin/src/libopenmpt-0.7.6+release/bin/libopenmpt.dll ${OUTPUT_DIR_PARENT}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/plugins/FMOD_sunvox_plugin/sunvox_lib/windows/lib_x86/sunvox.dll ${OUTPUT_DIR_PARENT}

        VERBATIM
)


#TODO preserve date when copy files
#TODO add BZRPlayerTest compilation
