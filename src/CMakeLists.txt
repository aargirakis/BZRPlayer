cmake_minimum_required(VERSION 3.28)
project(app)

set(CMAKE_CXX_STANDARD 14)

#TODO original qmake flags to check: -fexceptions -mthreads -DUNICODE -D_UNICODE -DMINGW_HAS_SECURE_API=1

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(PROJECT_VERSION ${PROJECT_VERSION_PARENT})

configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/version.h.in
        ${CMAKE_CURRENT_SOURCE_DIR}/version.h
        @ONLY NEWLINE_STYLE UNIX
)
configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/BZRPlayer.exe.manifest.in
        ${CMAKE_CURRENT_SOURCE_DIR}/BZRPlayer.exe.manifest
        @ONLY NEWLINE_STYLE UNIX
)

set(QT_VERSION_MAJOR "5")
set(QT_MODULES Core Gui Network OpenGL Svg Widgets Xml) #TODO Angle?

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS ${QT_MODULES} REQUIRED)

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
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_PREFIX_PATH}/bin/Qt${QT_VERSION_MAJOR}${QT_MODULE}.dll"
            ${OUTPUT_DIR_PARENT}
            VERBATIM
    )
endforeach ()

if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(QT_PLATFORMS_DIR "${CMAKE_PREFIX_PATH}/plugins/platforms")
else ()
    set(QT_PLATFORMS_DIR "${CMAKE_PREFIX_PATH}/lib/qt/plugins/platforms")
endif ()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUT_DIR_PARENT}/platforms"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${QT_PLATFORMS_DIR}/qwindows.dll"
        "${OUTPUT_DIR_PARENT}/platforms"
        VERBATIM
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_PREFIX_PATH}/bin/libgcc_s_dw2-1.dll
        ${CMAKE_PREFIX_PATH}/bin/libstdc++-6.dll
        ${CMAKE_PREFIX_PATH}/bin/libwinpthread-1.dll
        ${OUTPUT_DIR_PARENT}
        VERBATIM
)

if (NOT CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_PREFIX_PATH}/bin/libbrotlicommon.dll
            ${CMAKE_PREFIX_PATH}/bin/libbrotlidec.dll
            ${CMAKE_PREFIX_PATH}/bin/libbz2-1.dll
            ${CMAKE_PREFIX_PATH}/bin/libfreetype-6.dll
            ${CMAKE_PREFIX_PATH}/bin/libglib-2.0-0.dll
            ${CMAKE_PREFIX_PATH}/bin/libgraphite2.dll
            ${CMAKE_PREFIX_PATH}/bin/libharfbuzz-0.dll
            ${CMAKE_PREFIX_PATH}/bin/libiconv-2.dll
            ${CMAKE_PREFIX_PATH}/bin/libintl-8.dll
            ${CMAKE_PREFIX_PATH}/bin/libpcre2-8-0.dll
            ${CMAKE_PREFIX_PATH}/bin/libpcre2-16-0.dll
            ${CMAKE_PREFIX_PATH}/bin/libpng16-16.dll
            ${CMAKE_PREFIX_PATH}/bin/libssp-0.dll
            ${CMAKE_PREFIX_PATH}/bin/libzstd.dll
            ${OUTPUT_DIR_PARENT}
            VERBATIM
    )
endif ()

if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    #TODO
    set(ZLIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/static")
    set(LIBCRYPTO "${CMAKE_CURRENT_SOURCE_DIR}/static/libcrypto-1_1.dll") #TODO needed?
    set(LIBSSL "${CMAKE_CURRENT_SOURCE_DIR}/static/libssl-1_1.dll")
else ()
    set(ZLIB_DIR "${CMAKE_PREFIX_PATH}/bin")
    set(LIBCRYPTO "${CMAKE_PREFIX_PATH}/bin/libcrypto-3.dll") #TODO needed?
    set(LIBSSL "${CMAKE_PREFIX_PATH}/bin/libssl-3.dll")
endif ()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/LICENSE
        ${OUTPUT_DIR_PARENT}/LICENSE.txt

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/external/FMOD/api/core/lib/x86/fmod.dll
        ${OUTPUT_DIR_PARENT}

        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/static/data
        ${OUTPUT_DIR_PARENT}/data

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/static/user/plugin/sid/Songlengths.md5
        ${OUTPUT_DIR_PARENT}/user/plugin/sid/Songlengths.md5

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/static/BZRPlayerTest.exe
        ${CMAKE_CURRENT_SOURCE_DIR}/static/BZRPlayer.VisualElementsManifest.xml
        ${LIBCRYPTO}   #TODO needed?
        ${LIBSSL}
        ${CMAKE_CURRENT_SOURCE_DIR}/static/qtadvanceddocking.dll
        ${ZLIB_DIR}/zlib1.dll  #TODO move to the right place
        ${CMAKE_CURRENT_SOURCE_DIR}/plugins/FMOD_klystrack_plugin/src/x86/release/ksnd.dll
        ${CMAKE_CURRENT_SOURCE_DIR}/plugins/FMOD_libopenmpt_plugin/src/libopenmpt-0.7.6+release/bin/libopenmpt.dll
        ${CMAKE_CURRENT_SOURCE_DIR}/plugins/FMOD_sunvox_plugin/sunvox_lib/windows/lib_x86/sunvox.dll
        ${OUTPUT_DIR_PARENT}

        VERBATIM
)


#TODO preserve date when copy files
#TODO add BZRPlayerTest compilation
