cmake_minimum_required(VERSION 3.28)

project(app VERSION "${PROJECT_VERSION}")

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
set(QT_VERSION_MAJOR "6")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(QT_MODULES Core Gui Network OpenGL OpenGLWidgets Svg Widgets Xml)

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS ${QT_MODULES} REQUIRED)

configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/BZRPlayer.exe.manifest.in
        ${CMAKE_CURRENT_SOURCE_DIR}/BZRPlayer.exe.manifest
        @ONLY NEWLINE_STYLE UNIX
)

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
        BZRPlayer.exe.manifest)

if (NOT WIN32)
    #TODO workaround
    if (CMAKE_BUILD_TYPE STREQUAL "Release")
        set(CMAKE_BUILD_RPATH "${LIB_DIR}")
        #set(CMAKE_INSTALL_RPATH "${LIB_DIR}")
    else ()
        set(CMAKE_BUILD_RPATH "${OUTPUT_DIR}${LIB_DIR}")
    endif ()
endif ()

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_PREFIX_PATH}/include/qtadvanceddocking-qt${QT_VERSION_MAJOR}
        ${FMOD_API_DIR})

target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_VERSION="${PROJECT_VERSION}")

if (NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(${PROJECT_NAME} PRIVATE OUTPUT_DIR="${OUTPUT_DIR}")
endif ()

target_compile_definitions(${PROJECT_NAME} PRIVATE DATA_DIR="${DATA_DIR}")
target_compile_definitions(${PROJECT_NAME} PRIVATE LIB_DIR="${LIB_DIR}")
target_compile_definitions(${PROJECT_NAME} PRIVATE USER_DIR="${USER_DIR}")
target_compile_definitions(${PROJECT_NAME} PRIVATE PLUGINS_FMOD_DIR="${PLUGINS_FMOD_DIR}")
target_compile_definitions(${PROJECT_NAME} PRIVATE RESOURCES_DIR="${RESOURCES_DIR}")
target_compile_definitions(${PROJECT_NAME} PRIVATE LAYOUTS_DIR="${LAYOUTS_DIR}")
target_compile_definitions(${PROJECT_NAME} PRIVATE PLAYLISTS_DIR="${PLAYLISTS_DIR}")
target_compile_definitions(${PROJECT_NAME} PRIVATE PLUGINS_DIR="${PLUGINS_DIR}")
target_compile_definitions(${PROJECT_NAME} PRIVATE PLUGINS_CONFIG_DIR="${PLUGINS_CONFIG_DIR}")

foreach (QT_MODULE ${QT_MODULES})
    target_link_libraries(${PROJECT_NAME} PRIVATE "Qt${QT_VERSION_MAJOR}::${QT_MODULE}")
endforeach ()

if (WIN32)
    set(QT_ADS "${CMAKE_PREFIX_PATH}/bin/libqtadvanceddocking-qt${QT_VERSION_MAJOR}.${LIB_EXT}")
else ()
    set(QT_ADS "${CMAKE_PREFIX_PATH}/${CMAKE_INSTALL_LIBDIR}/libqtadvanceddocking-qt${QT_VERSION_MAJOR}.${LIB_EXT}")
endif ()

target_link_libraries(${PROJECT_NAME} PRIVATE
        ${QT_ADS}
        -L"${FMOD_DIR}" -lfmod
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME ${EXE_NAME}
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}${LIB_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}${BIN_DIR}
        #ARCHIVE_OUTPUT_DIRECTORY DESTINATION ${OUTPUT_DIR}${LIB_DIR} #TODO
)

if (WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE true)
endif ()

if (WIN32)
    foreach (QT_MODULE ${QT_MODULES})
        list(APPEND RUNTIME_LIBS ${CMAKE_PREFIX_PATH}/bin/Qt${QT_VERSION_MAJOR}${QT_MODULE}.dll)
    endforeach ()
endif ()

if (WIN32)
    list(APPEND RUNTIME_LIBS
            ${QT_ADS}
            ${CMAKE_PREFIX_PATH}/bin/libbrotlicommon.dll
            ${CMAKE_PREFIX_PATH}/bin/libbrotlidec.dll
            ${CMAKE_PREFIX_PATH}/bin/libb2-1.dll
            ${CMAKE_PREFIX_PATH}/bin/libbz2-1.dll
            ${CMAKE_PREFIX_PATH}/bin/libcrypto-3-x64.dll
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
            ${CMAKE_PREFIX_PATH}/bin/libssl-3-x64.dll
            ${CMAKE_PREFIX_PATH}/bin/libstdc++-6.dll
            ${CMAKE_PREFIX_PATH}/bin/libwinpthread-1.dll
            ${CMAKE_PREFIX_PATH}/bin/libzstd.dll
            ${CMAKE_PREFIX_PATH}/bin/zlib1.dll
    )

    if (WIN32)
        list(APPEND RUNTIME_LIBS
                ${CMAKE_PREFIX_PATH}/bin/libdouble-conversion.dll
                ${CMAKE_PREFIX_PATH}/bin/libmd4c.dll
        )
        file(GLOB RUNTIME_LIBS_VERSIONED
                ${CMAKE_PREFIX_PATH}/bin/libicudt*.dll
                ${CMAKE_PREFIX_PATH}/bin/libicuin*.dll
                ${CMAKE_PREFIX_PATH}/bin/libicuuc*.dll
        )
        list(APPEND RUNTIME_LIBS ${RUNTIME_LIBS_VERSIONED})
    endif ()

    foreach (LIB ${RUNTIME_LIBS})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${LIB} ${OUTPUT_DIR}
                VERBATIM)
    endforeach ()
endif ()

set(RUNTIME_LIBS ${RUNTIME_LIBS} PARENT_SCOPE)

if (WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/static/data ${OUTPUT_DIR}/${DATA_DIR}
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/LICENSE ${OUTPUT_DIR}/LICENSE.txt
            VERBATIM)

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/static/BZRPlayer.VisualElementsManifest.xml ${OUTPUT_DIR}
            VERBATIM)

    set(QT_PLUGINS_DIR "${CMAKE_PREFIX_PATH}/share/qt${QT_VERSION_MAJOR}/plugins")

    cmake_path(SET OUTPUT_PLATFORMS_DIR NORMALIZE ${OUTPUT_DIR}/${PLATFORMS_DIR})

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUT_PLATFORMS_DIR}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${QT_PLUGINS_DIR}/platforms/qwindows.dll ${OUTPUT_PLATFORMS_DIR}
            VERBATIM)

    cmake_path(SET OUTPUT_TLS_DIR NORMALIZE ${OUTPUT_DIR}/${TLS_DIR})

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUT_TLS_DIR}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${QT_PLUGINS_DIR}/tls/qopensslbackend.dll ${OUTPUT_TLS_DIR}
            VERBATIM)
else ()
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/static/data ${OUTPUT_DIR}/${DATA_DIR}
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/LICENSE ${OUTPUT_DIR}
            VERBATIM)
endif ()