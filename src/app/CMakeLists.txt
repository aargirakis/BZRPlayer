cmake_minimum_required(VERSION 3.28)

cmake_policy(SET CMP0177 OLD) #TODO fix

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

add_compile_definitions(PROJECT_VERSION="${PROJECT_VERSION}")

option(USE_DEV_PATHS "Ensure BZR2 will use paths relative to `output` directory avoiding the system ones" 0)

if (USE_DEV_PATHS)
    add_compile_definitions(USE_DEV_PATHS="${USE_DEV_PATHS}")
    add_compile_definitions(OUTPUT_DIR="${OUTPUT_DIR}")
endif ()

add_compile_definitions(DATA_DIR="${DATA_DIR}")
add_compile_definitions(USER_DIR="${USER_DIR}")
add_compile_definitions(PLUGINS_FMOD_DIR="${PLUGINS_FMOD_DIR}")
add_compile_definitions(RESOURCES_DIR="${RESOURCES_DIR}")
add_compile_definitions(LAYOUTS_DIR="${LAYOUTS_DIR}")
add_compile_definitions(PLAYLISTS_DIR="${PLAYLISTS_DIR}")
add_compile_definitions(PLUGINS_DIR="${PLUGINS_DIR}")
add_compile_definitions(PLUGINS_CONFIG_DIR="${PLUGINS_CONFIG_DIR}")

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

if (NOT WIN32 AND USE_DEV_PATHS EQUAL 1)
    #TODO temp workaround
    set(CMAKE_BUILD_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")
    #set(CMAKE_INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}") #used when invoking install(TARGETS ,,,)
endif ()

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_PREFIX_PATH}/include/qtadvanceddocking-qt${QT_VERSION_MAJOR}
        ${LIB_FMOD_API_DIR}
)

foreach (QT_MODULE ${QT_MODULES})
    target_link_libraries(${PROJECT_NAME} PRIVATE "Qt${QT_VERSION_MAJOR}::${QT_MODULE}")
endforeach ()

if (WIN32)
    set(LIB_QT_ADS "${CMAKE_PREFIX_PATH}/bin/libqtadvanceddocking-qt${QT_VERSION_MAJOR}.${LIB_EXT}")
else ()
    set(LIB_QT_ADS "${CMAKE_PREFIX_PATH}/lib/libqtadvanceddocking-qt${QT_VERSION_MAJOR}.${LIB_EXT}")
endif ()

target_link_libraries(${PROJECT_NAME} PRIVATE
        ${LIB_QT_ADS}
        -L"${LIB_FMOD_DIR}" -lfmod
)

IF (WIN32)
    set(EXE_NAME "BZRPlayer")
else ()
    set(EXE_NAME "bzr-player")
endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME ${EXE_NAME}
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${BIN_DIR}
        #ARCHIVE_OUTPUT_DIRECTORY DESTINATION ${OUTPUT_DIR}/${LIB_DIR} #TODO
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
            ${LIB_QT_ADS}
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
        install(FILES ${LIB}
                DESTINATION ${OUTPUT_DIR})
    endforeach ()
endif ()

set(RUNTIME_LIBS ${RUNTIME_LIBS} PARENT_SCOPE)

if (WIN32)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/static/data
            DESTINATION ${OUTPUT_DIR})

    install(FILES ${CMAKE_SOURCE_DIR}/LICENSE
            DESTINATION ${OUTPUT_DIR}
            RENAME LICENSE.txt
    )

    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/static/BZRPlayer.VisualElementsManifest.xml
            DESTINATION ${OUTPUT_DIR})

    set(QT_PLUGINS_DIR "${CMAKE_PREFIX_PATH}/share/qt${QT_VERSION_MAJOR}/plugins")

    install(FILES ${QT_PLUGINS_DIR}/platforms/qwindows.dll
            DESTINATION ${OUTPUT_DIR}/${PLATFORMS_DIR})

    install(FILES ${QT_PLUGINS_DIR}/tls/qopensslbackend.dll
            DESTINATION ${OUTPUT_DIR}/${TLS_DIR})
else ()
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/static/data
            DESTINATION ${OUTPUT_DIR}/${SHARE_DIR})

    install(FILES ${CMAKE_SOURCE_DIR}/LICENSE
            DESTINATION ${OUTPUT_DIR})
endif ()