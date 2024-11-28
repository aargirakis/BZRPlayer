cmake_minimum_required(VERSION 3.28)

set(NAME "libvgm")
set(VERSION "34c368cde98f33c42455fbbfbec07073ba79bf5c")
string(SUBSTRING ${VERSION} 0 12 VERSION_SHORT)
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "libvgm" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION_SHORT}" PARENT_SCOPE)

find_package(ZLIB REQUIRED)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(NAME_VERSIONED "${NAME}-${VERSION_SHORT}")
set(FILENAME "${NAME}-${VERSION}.zip")
set(FILENAME_URL "${VERSION}.zip")
set(URL "https://github.com/ValleyBell/${NAME}/archive/${FILENAME_URL}")
set(SHA_256_HASH "b9139240830e17273a7c6e74c3410a0b6956dfea8e988729dc50d5148ed900eb")
set(UNPACKED_DIR "${NAME}-${VERSION}")
download_patch_and_add(
        ${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} true "${UNPACKED_DIR}" ""
)

#TODO be sure *.s98 & *.gym are played by libvgm instead of tunes98

file(GLOB SOURCES
        #${EXTERNAL_SOURCE_DIR}/audio/AudDrv_DSound.cpp
        #${EXTERNAL_SOURCE_DIR}/audio/AudDrv_WinMM.cpp
        #${EXTERNAL_SOURCE_DIR}/audio/AudDrv_WASAPI.cpp
        #${EXTERNAL_SOURCE_DIR}/audio/AudioStream.c
        ${EXTERNAL_SOURCE_DIR}/emu/*.c
        ${EXTERNAL_SOURCE_DIR}/emu/cores/*.c
        ${EXTERNAL_SOURCE_DIR}/utils/OSMutex_Win.c

        src/main.cpp)

list(REMOVE_ITEM SOURCES
        ${EXTERNAL_SOURCE_DIR}/emu/cores/adlibemu_opl_inc.c
        ${EXTERNAL_SOURCE_DIR}/emu/cores/scsplfo.c)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}
        ${EXTERNAL_SOURCE_DIR}/audio #TODO
        ${EXTERNAL_SOURCE_DIR}/utils #TODO

        ${CMAKE_SOURCE_DIR}/src/app #TODO
        ${FMOD_API_DIR})

#target_compile_definitions(${PROJECT_NAME} PRIVATE VGM_LITTLE_ENDIAN) #TODO
#target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_STDINT_H) #TODO
#target_compile_definitions(${PROJECT_NAME} PRIVATE _WIN32_WINNT=0x500) #TODO

target_compile_options(${PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:C>:-Wno-implicit-function-declaration>)

target_link_libraries(${PROJECT_NAME} PRIVATE
        ZLIB::ZLIB
        #TODO
        -ldsound)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
)
