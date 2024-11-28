cmake_minimum_required(VERSION 3.28)

set(NAME "libvgm")
set(VERSION "34c368cde98f33c42455fbbfbec07073ba79bf5c")
project(plugin_${NAME})
set(PLUGIN_${NAME}_DLL "${PROJECT_NAME}.dll" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "libvgm" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

find_package(ZLIB REQUIRED)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION GREATER_EQUAL 14)
    add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-implicit-function-declaration>)
endif ()

set(LIB_NAME "${NAME}")
set(LIB_VERSION "${VERSION}")
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION}")
set(LIB_FILENAME "${LIB_NAME_VERSIONED}.zip")
set(LIB_FILENAME_URL "${LIB_VERSION}.zip")
set(LIB_URL "https://github.com/ValleyBell/${LIB_NAME}/archive/${LIB_FILENAME_URL}")
set(LIB_SHA_256_HASH "b9139240830e17273a7c6e74c3410a0b6956dfea8e988729dc50d5148ed900eb")
download_patch_and_add(
        ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH} false "" ""
)

#TODO be sure *.s98 & *.gym are played by libvgm instead of tunes98

#add_compile_definitions(VGM_LITTLE_ENDIAN) #TODO
#add_compile_definitions(HAVE_STDINT_H) #TODO
#add_compile_definitions(_WIN32_WINNT=0x500) #TODO

file(GLOB SOURCES
        #${EXTERNAL_SOURCE_DIR}/audio/AudDrv_DSound.cpp
        #${EXTERNAL_SOURCE_DIR}/audio/AudDrv_WinMM.cpp
        #${EXTERNAL_SOURCE_DIR}/audio/AudDrv_WASAPI.cpp
        #${EXTERNAL_SOURCE_DIR}/audio/AudioStream.c
        ${EXTERNAL_SOURCE_DIR}/emu/*.c
        ${EXTERNAL_SOURCE_DIR}/emu/cores/*.c
        ${EXTERNAL_SOURCE_DIR}/utils/OSMutex_Win.c

        src/main.cpp
)

list(REMOVE_ITEM SOURCES
        ${EXTERNAL_SOURCE_DIR}/emu/cores/adlibemu_opl_inc.c
        ${EXTERNAL_SOURCE_DIR}/emu/cores/scsplfo.c
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/audio #TODO
        ${EXTERNAL_SOURCE_DIR}/utils #TODO

        ${CMAKE_SOURCE_DIR}/src/app #TODO
        ${EXTERNAL_SOURCE_DIR_fmod}/api/core/inc
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        ZLIB::ZLIB
        #     -ldsound    #TODO
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DATA_PLUGINS_DIR}
)
