cmake_minimum_required(VERSION 3.28)

set(NAME "kdm")
set(VERSION "-") #TODO use file timestamp date
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "Ken's Digital Music" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION}" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

#TODO test offline mode
#TODO unneded dirs in _deps kdm-*

set(FILENAME "kdmsongs.zip")
set(FILENAME2 "kdmsrc.zip")
set(URL "https://www.advsys.net/ken/${FILENAME}")
set(SHA_256_HASH "38b42f309a5007a101890db3531e61d46abb1c76e8760d47a9fa979df2bb1703")
set(DESTINATION_PATH "${DEPENDENCIES_DIR}/${NAME}")
download_to(${FILENAME} ${URL} ${SHA_256_HASH} ${DESTINATION_PATH} false ${NAME})
unpack(${DESTINATION_PATH}/${FILENAME} ${DESTINATION_PATH}/${NAME} false)
unpack(${DESTINATION_PATH}/${FILENAME2} ${DESTINATION_PATH}/${NAME} false)

set(EXTERNAL_SOURCE_DIR "${DESTINATION_PATH}/kdmwin64")
patch_sources(${NAME} ${CMAKE_CURRENT_SOURCE_DIR}/patches ${EXTERNAL_SOURCE_DIR})

FetchContent_Declare(
        ${NAME}
        SOURCE_DIR "${EXTERNAL_SOURCE_DIR}"
        EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(${NAME})

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/kdmeng.c
        src/main.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        src

        ${CMAKE_SOURCE_DIR}/src/app
        ${FMOD_API_DIR})

#TODO
target_compile_options(${PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:C>:-Wno-implicit-function-declaration>)

#TODO needed?
#target_link_libraries(${PROJECT_NAME} PRIVATE -lwinmm)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
)
