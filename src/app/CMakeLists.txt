cmake_minimum_required(VERSION 3.28)

project(app VERSION "${BZR2_VERSION}")

include(${CMAKE_SOURCE_DIR}/src/external/qt-ads/qt-ads.cmake)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

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
        ${EXTERNAL_SOURCE_DIR_qt-ads}/src
)

foreach (QT_MODULE ${QT_MODULES})
    target_link_libraries(${PROJECT_NAME} PRIVATE "Qt${QT_VERSION_MAJOR}::${QT_MODULE}")
endforeach ()

target_link_libraries(${PROJECT_NAME} PRIVATE
        -L"${EXTERNAL_SOURCE_DIR_fmod}"/api/core/lib/x86 -lfmod
        ${LIB_QT_ADVANCED_DOCKING_SYSTEM}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "BZRPlayer"
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}
        COMPILE_FLAGS "-m32"
        LINK_FLAGS "-m32"
        WIN32_EXECUTABLE true
)

foreach (QT_MODULE ${QT_MODULES})
    list(APPEND RUNTIME_DLLS ${CMAKE_PREFIX_PATH}/bin/Qt${QT_VERSION_MAJOR}${QT_MODULE}.dll)
endforeach ()

list(APPEND RUNTIME_DLLS
        ${CMAKE_PREFIX_PATH}/bin/libbrotlicommon.dll
        ${CMAKE_PREFIX_PATH}/bin/libbrotlidec.dll
        ${CMAKE_PREFIX_PATH}/bin/libbz2-1.dll
        ${CMAKE_PREFIX_PATH}/bin/libcrypto-3.dll
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
    list(APPEND RUNTIME_DLLS
            ${CMAKE_PREFIX_PATH}/bin/libdouble-conversion.dll
            ${CMAKE_PREFIX_PATH}/bin/libmd4c.dll
    )
    file(GLOB RUNTIME_DLLS_VERSIONED
            ${CMAKE_PREFIX_PATH}/bin/libicudt*.dll
            ${CMAKE_PREFIX_PATH}/bin/libicuin*.dll
            ${CMAKE_PREFIX_PATH}/bin/libicuuc*.dll
    )
    list(APPEND RUNTIME_DLLS ${RUNTIME_DLLS_VERSIONED})
else ()
    list(APPEND RUNTIME_DLLS
            ${CMAKE_PREFIX_PATH}/bin/libssp-0.dll
    )
endif ()

foreach (DLL ${RUNTIME_DLLS})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${DLL}"
            ${OUTPUT_DIR}
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
        COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUT_DIR}/platforms"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${QT_PLATFORMS_DIR}/qwindows.dll"
        "${OUTPUT_DIR}/platforms"
        VERBATIM
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/LICENSE
        ${OUTPUT_DIR}/LICENSE.txt

        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/static/data
        ${OUTPUT_DATA_DIR}

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/static/BZRPlayer.VisualElementsManifest.xml

        ${OUTPUT_DIR}

        VERBATIM
)
