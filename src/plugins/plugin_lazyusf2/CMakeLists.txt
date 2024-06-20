cmake_minimum_required(VERSION 3.28)

set(NAME "lazyusf2")
set(VERSION "421f00bcaa1988b8e1825e91780129f24fbd1aa0")
string(SUBSTRING ${VERSION} 0 12 VERSION_SHORT)
project(plugin_${NAME})
set(PLUGIN_${NAME}_DLL "${PROJECT_NAME}.dll" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "${NAME}" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION_SHORT}" PARENT_SCOPE)

find_package(ZLIB REQUIRED)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

#if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION GREATER_EQUAL 10) #TODO for DYNAREC #TODO 10?
#    add_compile_options($<$<COMPILE_LANGUAGE:C>:-fcommon>)
#endif ()

if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION GREATER_EQUAL 14)
    add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-return-mismatch>)
endif ()

if (CMAKE_COMPILER_IS_GNUCC)
    add_compile_options(-mfpmath=sse -msse2) # not needed for 64bit (already set by default)
endif ()

set(LIB_NAME "${NAME}")
set(LIB_VERSION "${VERSION}")
set(LIB_VERSION_SHORT "${VERSION_SHORT}")
set(LIB_NAME_VERSIONED "${LIB_NAME}-${LIB_VERSION_SHORT}")
set(LIB_FILENAME "${LIB_NAME}-${LIB_VERSION}.tar.bz2")
set(LIB_URL "https://gitlab.com/kode54/${LIB_NAME}/-/archive/${LIB_VERSION}/${LIB_FILENAME}")
set(LIB_SHA_256_HASH "b4ff5df45572e2c5ac5ecc98a5421701330a655b9971e5c012fe9f42bd12c51f")
set(LIB_UNPACKED_DIR "${LIB_NAME}-${LIB_VERSION}")
download_patch_and_add(
        ${LIB_NAME_VERSIONED} ${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH} true ${LIB_UNPACKED_DIR} ""
)

set(EXTERNAL_SOURCE_DIR_${NAME} "${EXTERNAL_SOURCE_DIR}")

#TODO get OVERLOAD feature from https://github.com/joshware/lazyusf2/commit/f32bff34e35e23185456781ef1041003309dbf8f

add_compile_definitions(EMU_COMPILE ARCH_MIN_SSE2)
#add_compile_definitions(DYNAREC)   #TODO
#add_compile_definitions(SP_STATUS_TASKDONE=0x200)   #TODO

include(${CMAKE_SOURCE_DIR}/src/external/psflib/psflib.cmake)

file(GLOB SOURCES                                                        #TODO remove unnecessary

        #    ${EXTERNAL_SOURCE_DIR_${NAME}}/r4300/x86/*.c #TODO for DYNAREC

        ${EXTERNAL_SOURCE_DIR_${NAME}}/ai/*.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/api/*.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/debugger/dbg_decoder.c  #TODO
        ${EXTERNAL_SOURCE_DIR_${NAME}}/main/*.c               #TODO
        ${EXTERNAL_SOURCE_DIR_${NAME}}/memory/*.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/pi/*.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/r4300/*.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/rdp/*.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/ri/*.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/rsp/*.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/rsp_hle/*.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/rsp_lle/rsp.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/si/*.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/usf/*.c
        ${EXTERNAL_SOURCE_DIR_${NAME}}/vi/*.c
        ${EXTERNAL_SOURCE_DIR_psflib}/psf2fs.c #TODO needed?
        ${EXTERNAL_SOURCE_DIR_psflib}/psflib.c
        src/main.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE #TODO remove unnecessary

        #    ${EXTERNAL_SOURCE_DIR_${NAME}}/r4300/x86 #TODO for DYNAREC

        ${EXTERNAL_SOURCE_DIR_${NAME}}
        ${EXTERNAL_SOURCE_DIR_${NAME}}/ai
        ${EXTERNAL_SOURCE_DIR_${NAME}}/api
        ${EXTERNAL_SOURCE_DIR_${NAME}}/debugger
        ${EXTERNAL_SOURCE_DIR_${NAME}}/main
        ${EXTERNAL_SOURCE_DIR_${NAME}}/memory
        ${EXTERNAL_SOURCE_DIR_${NAME}}/osal
        ${EXTERNAL_SOURCE_DIR_${NAME}}/pi
        ${EXTERNAL_SOURCE_DIR_${NAME}}/r4300
        ${EXTERNAL_SOURCE_DIR_${NAME}}/rdp
        ${EXTERNAL_SOURCE_DIR_${NAME}}/ri
        ${EXTERNAL_SOURCE_DIR_${NAME}}/rsp
        ${EXTERNAL_SOURCE_DIR_${NAME}}/rsp_hle
        ${EXTERNAL_SOURCE_DIR_${NAME}}/rsp_lle
        ${EXTERNAL_SOURCE_DIR_${NAME}}/si
        ${EXTERNAL_SOURCE_DIR_${NAME}}/usf
        ${EXTERNAL_SOURCE_DIR_${NAME}}/vi
        ${EXTERNAL_SOURCE_DIR_psflib}

        ${CMAKE_SOURCE_DIR}/src/app
        ${EXTERNAL_SOURCE_DIR_fmod}/api/core/inc
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        ZLIB::ZLIB
        # -lshlwapi   #TODO
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/data/plugin
        COMPILE_FLAGS "-m32"
        LINK_FLAGS "-m32"
)
