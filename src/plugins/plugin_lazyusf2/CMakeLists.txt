cmake_minimum_required(VERSION 3.28)

set(NAME "lazyusf2")
set(VERSION "421f00bcaa1988b8e1825e91780129f24fbd1aa0")
string(SUBSTRING ${VERSION} 0 12 VERSION_SHORT)
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "${NAME}" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION_SHORT}" PARENT_SCOPE)

include(${CMAKE_SOURCE_DIR}/src/external/psflib/psflib.cmake)

find_package(ZLIB REQUIRED)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(NAME_VERSIONED "${NAME}-${VERSION_SHORT}")
set(FILENAME "${NAME}-${VERSION}.tar.bz2")
set(URL "https://gitlab.com/kode54/${NAME}/-/archive/${VERSION}/${FILENAME}")
set(SHA_256_HASH "b4ff5df45572e2c5ac5ecc98a5421701330a655b9971e5c012fe9f42bd12c51f")
set(UNPACKED_DIR "${NAME}-${VERSION}")
download_patch_and_add(
        ${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} true ${UNPACKED_DIR} ""
)

#TODO get OVERLOAD feature from https://github.com/joshware/lazyusf2/commit/f32bff34e35e23185456781ef1041003309dbf8f

file(GLOB SOURCES                                                        #TODO remove unnecessary

        #    ${EXTERNAL_SOURCE_DIR}/r4300/x86_64/*.c #TODO for DYNAREC

        ${EXTERNAL_SOURCE_DIR}/ai/*.c
        ${EXTERNAL_SOURCE_DIR}/api/*.c
        ${EXTERNAL_SOURCE_DIR}/debugger/dbg_decoder.c  #TODO
        ${EXTERNAL_SOURCE_DIR}/main/*.c               #TODO
        ${EXTERNAL_SOURCE_DIR}/memory/*.c
        ${EXTERNAL_SOURCE_DIR}/pi/*.c
        ${EXTERNAL_SOURCE_DIR}/r4300/*.c
        ${EXTERNAL_SOURCE_DIR}/rdp/*.c
        ${EXTERNAL_SOURCE_DIR}/ri/*.c
        ${EXTERNAL_SOURCE_DIR}/rsp/*.c
        ${EXTERNAL_SOURCE_DIR}/rsp_hle/*.c
        ${EXTERNAL_SOURCE_DIR}/rsp_lle/rsp.c
        ${EXTERNAL_SOURCE_DIR}/si/*.c
        ${EXTERNAL_SOURCE_DIR}/usf/*.c
        ${EXTERNAL_SOURCE_DIR}/vi/*.c
        ${EXTERNAL_SOURCE_DIR_psflib}/psf2fs.c #TODO needed?
        ${EXTERNAL_SOURCE_DIR_psflib}/psflib.c
        src/main.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE #TODO remove unnecessary

        #    ${EXTERNAL_SOURCE_DIR}/r4300/x86_64 #TODO for DYNAREC

        ${EXTERNAL_SOURCE_DIR}
        ${EXTERNAL_SOURCE_DIR}/ai
        ${EXTERNAL_SOURCE_DIR}/api
        ${EXTERNAL_SOURCE_DIR}/debugger
        ${EXTERNAL_SOURCE_DIR}/main
        ${EXTERNAL_SOURCE_DIR}/memory
        ${EXTERNAL_SOURCE_DIR}/osal
        ${EXTERNAL_SOURCE_DIR}/pi
        ${EXTERNAL_SOURCE_DIR}/r4300
        ${EXTERNAL_SOURCE_DIR}/rdp
        ${EXTERNAL_SOURCE_DIR}/ri
        ${EXTERNAL_SOURCE_DIR}/rsp
        ${EXTERNAL_SOURCE_DIR}/rsp_hle
        ${EXTERNAL_SOURCE_DIR}/rsp_lle
        ${EXTERNAL_SOURCE_DIR}/si
        ${EXTERNAL_SOURCE_DIR}/usf
        ${EXTERNAL_SOURCE_DIR}/vi
        ${EXTERNAL_SOURCE_DIR_psflib}

        ${CMAKE_SOURCE_DIR}/src/app
        ${FMOD_API_DIR})

target_compile_definitions(${PROJECT_NAME} PRIVATE EMU_COMPILE ARCH_MIN_SSE2)
#target_compile_definitions(${PROJECT_NAME} PRIVATE DYNAREC)   #TODO
#target_compile_definitions(${PROJECT_NAME} PRIVATE SP_STATUS_TASKDONE=0x200)   #TODO

#target_compile_options(${PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:C>:-fcommon>) #TODO for DYNAREC
target_compile_options(${PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:C>:-Wno-return-mismatch>)

target_link_libraries(${PROJECT_NAME} PRIVATE ZLIB::ZLIB)

#if (WIN32) TODO
#    target_link_libraries(${PROJECT_NAME} PRIVATE -lshlwapi)
#endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${DATA_DIR}/${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${DATA_DIR}/${PLUGINS_DIR}
)
