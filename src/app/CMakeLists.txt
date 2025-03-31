cmake_minimum_required(VERSION 3.28)

project(app VERSION "${PROJECT_VERSION}")

set(QT_VERSION_MAJOR "6")
include(${CMAKE_SOURCE_DIR}/src/external/qt-ads/qt-ads.cmake)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/BZRPlayer.exe.manifest.in
        ${CMAKE_CURRENT_SOURCE_DIR}/BZRPlayer.exe.manifest
        @ONLY NEWLINE_STYLE UNIX
)

set(QT_MODULES Core Gui Network OpenGL OpenGLWidgets Svg Widgets Xml)

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS ${QT_MODULES} REQUIRED)
find_package(OpenSSL REQUIRED)

add_compile_definitions(PROJECT_VERSION="${PROJECT_VERSION}")
add_compile_definitions(DATA_DIR="${DATA_DIR}")
add_compile_definitions(USER_DIR="${USER_DIR}")
add_compile_definitions(DATA_PLUGINS_DIR="${DATA_PLUGINS_DIR}")
add_compile_definitions(DATA_PLUGINS_FMOD_DIR="${DATA_PLUGINS_FMOD_DIR}")
add_compile_definitions(USER_LAYOUTS_DIR="${USER_LAYOUTS_DIR}")
add_compile_definitions(USER_PLAYLISTS_DIR="${USER_PLAYLISTS_DIR}")
add_compile_definitions(USER_PLUGINS_DIR="${USER_PLUGINS_DIR}")
add_compile_definitions(USER_PLUGINS_CONFIG_DIR="${USER_PLUGINS_CONFIG_DIR}")

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
        -L"${EXTERNAL_SOURCE_DIR_fmod}"/api/core/lib/x64 -lfmod
        ${LIB_QT_ADVANCED_DOCKING_SYSTEM}
        OpenSSL::Crypto OpenSSL::SSL
        # OpenSSL::applink #TODO
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "BZRPlayer"
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}
        WIN32_EXECUTABLE true
)

foreach (QT_MODULE ${QT_MODULES})
    list(APPEND RUNTIME_DLLS ${CMAKE_PREFIX_PATH}/bin/Qt${QT_VERSION_MAJOR}${QT_MODULE}.dll)
endforeach ()

list(APPEND RUNTIME_DLLS
        ${CMAKE_PREFIX_PATH}/bin/libbrotlicommon.dll
        ${CMAKE_PREFIX_PATH}/bin/libbrotlidec.dll
        ${CMAKE_PREFIX_PATH}/bin/libb2-1.dll
        ${CMAKE_PREFIX_PATH}/bin/libbz2-1.dll
        ${CMAKE_PREFIX_PATH}/bin/libfreetype-6.dll
        ${CMAKE_PREFIX_PATH}/bin/libgcc_s_seh-1.dll
        ${CMAKE_PREFIX_PATH}/bin/libglib-2.0-0.dll
        ${CMAKE_PREFIX_PATH}/bin/libgraphite2.dll
        ${CMAKE_PREFIX_PATH}/bin/libharfbuzz-0.dll
        ${CMAKE_PREFIX_PATH}/bin/libiconv-2.dll
        ${CMAKE_PREFIX_PATH}/bin/libintl-8.dll
        ${CMAKE_PREFIX_PATH}/bin/libpcre2-8-0.dll
        ${CMAKE_PREFIX_PATH}/bin/libpcre2-16-0.dll
        ${CMAKE_PREFIX_PATH}/bin/libpng16-16.dll
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

#foreach (DLL ${RUNTIME_DLLS})
#    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#            COMMAND ${CMAKE_COMMAND} -E copy_if_different
#            "${DLL}"
#            ${OUTPUT_DIR}
#            VERBATIM
#    )
#endforeach ()

set(RUNTIME_DLLS ${RUNTIME_DLLS} PARENT_SCOPE)

if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(QT_PLUGINS_DIR "${CMAKE_PREFIX_PATH}/share/qt${QT_VERSION_MAJOR}/plugins")
else ()
    set(QT_PLUGINS_DIR "${CMAKE_PREFIX_PATH}/lib/qt${QT_VERSION_MAJOR}/plugins")
endif ()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUT_PLATFORMS_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${QT_PLUGINS_DIR}/platforms/qwindows.dll"
        "${OUTPUT_PLATFORMS_DIR}"
        VERBATIM
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUT_TLS_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${QT_PLUGINS_DIR}/tls/qopensslbackend.dll"
        "${OUTPUT_TLS_DIR}"
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


set(CMAKE_INSTALL_PREFIX "${OUTPUT_DIR}")
#set(CMAKE_TOOLCHAIN_FILE "${CMAKE_PREFIX_PATH}/scripts/buildsystems/MSYS2.cmake")

include(GNUInstallDirs)

#install(FILES ${OPENSSL_LIBRARIES}
#        DESTINATION ${OUTPUT_DIR})

install(TARGETS ${PROJECT_NAME}
        BUNDLE  DESTINATION .
        LIBRARY DESTINATION ${OUTPUT_DIR}
        RUNTIME DESTINATION ${OUTPUT_DIR}
)

install (TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${OUTPUT_DIR})

#install(TARGETS ${PROJECT_NAME} RUNTIME_DEPENDENCY_SET DESTINATION ${OUTPUT_DIR})
#install(TARGETS ${PROJECT_NAME} RUNTIME_DEPENDENCIES DESTINATION ${OUTPUT_DIR})

#install(FILES ${CMAKE_PREFIX_PATH}/bin/libcrypto-3-x64.dll DESTINATION ${OUTPUT_DIR})

add_custom_target(install_all ALL DEPENDS ${PROJECT_NAME})
add_custom_command(TARGET install_all POST_BUILD COMMAND ${CMAKE_COMMAND} --install ${CMAKE_CURRENT_BINARY_DIR}) #TODO ${OUTPUT_DIR}
