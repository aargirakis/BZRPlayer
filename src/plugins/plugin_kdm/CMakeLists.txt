cmake_minimum_required(VERSION 3.28)

set(NAME "kdm")
set(VERSION "-") #TODO use file timestamp date
project(plugin_${NAME})
set(PLUGIN_${NAME}_DLL "${PROJECT_NAME}.dll" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "Ken's Digital Music" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

#TODO
if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION GREATER_EQUAL 14)
    add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-implicit-function-declaration>)
endif ()

#TODO test offline mode
#TODO unneded dirs in _deps kdm-*

set(LIB_NAME "${NAME}")
set(LIB_FILENAME "kdmsongs.zip")
set(LIB_FILENAME2 "kdmsrc.zip")
set(LIB_URL "https://www.advsys.net/ken/${LIB_FILENAME}")
set(LIB_SHA_256_HASH "38b42f309a5007a101890db3531e61d46abb1c76e8760d47a9fa979df2bb1703")
set(DESTINATION_PATH "${DEPENDENCIES_DIR}/${LIB_NAME}")
set(DESTINATION_PATH2 "${DEPENDENCIES_DIR}/${LIB_NAME2}")
download_to(${LIB_FILENAME} ${LIB_URL} ${LIB_SHA_256_HASH} ${DESTINATION_PATH} false ${LIB_NAME})
unpack(${DESTINATION_PATH}/${LIB_FILENAME} ${DESTINATION_PATH}/${LIB_NAME} false)
unpack(${DESTINATION_PATH}/${LIB_FILENAME2} ${DESTINATION_PATH}/${LIB_NAME} false)

set(EXTERNAL_SOURCE_DIR "${DESTINATION_PATH}/kdmwin64")
patch_sources(${LIB_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/patches ${EXTERNAL_SOURCE_DIR})

FetchContent_Declare(
        ${LIB_NAME}
        SOURCE_DIR "${EXTERNAL_SOURCE_DIR}"
        EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(${LIB_NAME})

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/kdmeng.c
        src/main.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        src

        ${CMAKE_SOURCE_DIR}/src/app
        ${EXTERNAL_SOURCE_DIR_fmod}/api/core/inc
)

target_link_libraries(${PROJECT_NAME} PRIVATE -lwinmm)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DATA_PLUGINS_DIR}
)
