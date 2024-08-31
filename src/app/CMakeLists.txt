cmake_minimum_required(VERSION 3.28)

project(app)

set(CMAKE_CXX_STANDARD 14)

#TODO original qmake flags to check: -fexceptions -mthreads -DUNICODE -D_UNICODE -DMINGW_HAS_SECURE_API=1

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(PROJECT_VERSION "${PROJECT_VERSION_PARENT}")
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
set(QT_MODULES Core Gui Network OpenGL Svg Widgets Xml)

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
        ${EXTERNAL_SOURCE_DIR_fmod}/api/core/inc
        ${CMAKE_SOURCE_DIR}/src/external/Qt-Advanced-Docking-System-4.0.0/src
)

foreach (QT_MODULE ${QT_MODULES})
    target_link_libraries(${PROJECT_NAME} PRIVATE "Qt${QT_VERSION_MAJOR}::${QT_MODULE}")
endforeach ()

target_link_libraries(${PROJECT_NAME} PRIVATE
        -L"${EXTERNAL_SOURCE_DIR_fmod}"/api/core/lib/x86 -lfmod
        -L${CMAKE_SOURCE_DIR}/src/external/Qt-Advanced-Docking-System-4.0.0/bin -lqtadvanceddocking
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "BZRPlayer"
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR_PARENT}
        COMPILE_FLAGS "-m32"
        LINK_FLAGS "-m32"
        WIN32_EXECUTABLE true
)

foreach (QT_MODULE ${QT_MODULES})
    List(APPEND RUNTIME_DLLS ${CMAKE_PREFIX_PATH}/bin/Qt${QT_VERSION_MAJOR}${QT_MODULE}.dll)
endforeach ()

List(APPEND RUNTIME_DLLS
        ${CMAKE_PREFIX_PATH}/bin/libbrotlicommon.dll
        ${CMAKE_PREFIX_PATH}/bin/libbrotlidec.dll
        ${CMAKE_PREFIX_PATH}/bin/libbz2-1.dll
        ${CMAKE_PREFIX_PATH}/bin/libcrypto-3.dll        #TODO needed?
        ${CMAKE_PREFIX_PATH}/bin/libfreetype-6.dll
        ${CMAKE_PREFIX_PATH}/bin/libgcc_s_dw2-1.dll
        ${CMAKE_PREFIX_PATH}/bin/libglib-2.0-0.dll
        ${CMAKE_PREFIX_PATH}/bin/libgraphite2.dll
        ${CMAKE_PREFIX_PATH}/bin/libharfbuzz-0.dll
        ${CMAKE_PREFIX_PATH}/bin/libiconv-2.dll
        ${CMAKE_PREFIX_PATH}/bin/libintl-8.dll
        ${CMAKE_PREFIX_PATH}/bin/libpcre2-8-0.dll
        ${CMAKE_PREFIX_PATH}/bin/libpcre2-16-0.dll
        ${CMAKE_PREFIX_PATH}/bin/libpng16-16.dll
        ${CMAKE_PREFIX_PATH}/bin/libssl-3.dll
        ${CMAKE_PREFIX_PATH}/bin/libstdc++-6.dll
        ${CMAKE_PREFIX_PATH}/bin/libwinpthread-1.dll
        ${CMAKE_PREFIX_PATH}/bin/libzstd.dll
        ${CMAKE_PREFIX_PATH}/bin/zlib1.dll
)

if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    List(APPEND RUNTIME_DLLS
            ${CMAKE_PREFIX_PATH}/bin/libdouble-conversion.dll
            ${CMAKE_PREFIX_PATH}/bin/libicudt75.dll
            ${CMAKE_PREFIX_PATH}/bin/libicuin75.dll
            ${CMAKE_PREFIX_PATH}/bin/libicuuc75.dll
            ${CMAKE_PREFIX_PATH}/bin/libmd4c.dll
    )
else ()
    List(APPEND RUNTIME_DLLS
            ${CMAKE_PREFIX_PATH}/bin/libssp-0.dll
    )
endif ()

foreach (DLL ${RUNTIME_DLLS})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${DLL}"
            ${OUTPUT_DIR_PARENT}
            VERBATIM
    )
endforeach ()

set(RUNTIME_DLLS ${RUNTIME_DLLS} PARENT_SCOPE)

if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(QT_PLATFORMS_DIR "${CMAKE_PREFIX_PATH}/share/qt${QT_VERSION_MAJOR}/plugins/platforms")
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
        ${CMAKE_SOURCE_DIR}/LICENSE
        ${OUTPUT_DIR_PARENT}/LICENSE.txt

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${EXTERNAL_SOURCE_DIR_fmod}/api/core/lib/x86/fmod.dll
        ${OUTPUT_DIR_PARENT}

        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/static/data
        ${OUTPUT_DIR_PARENT}/data

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/static/BZRPlayerTest.exe
        ${CMAKE_CURRENT_SOURCE_DIR}/static/BZRPlayer.VisualElementsManifest.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/static/qtadvanceddocking.dll

        #TODO are this in the right place?
        ${CMAKE_SOURCE_DIR}/src/plugins/FMOD_klystrack_plugin/src/x86/release/ksnd.dll
        ${OUTPUT_DIR_PARENT}

        VERBATIM
)

#TODO add BZRPlayerTest compilation
