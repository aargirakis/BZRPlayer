cmake_minimum_required(VERSION 3.28)

set(NAME "jaytrax")
set(VERSION "1c2a999014058ef04973c4f3fac8bdf61a3fa9a4")
string(SUBSTRING ${VERSION} 0 12 VERSION_SHORT)
project(plugin_${NAME})
set(PLUGIN_${NAME}_LIB "${PROJECT_NAME}.${LIB_EXT}" PARENT_SCOPE)
set(PLUGIN_${NAME}_NAME "Jaytrax" PARENT_SCOPE)
set(PLUGIN_${NAME}_VERSION "${VERSION_SHORT}" PARENT_SCOPE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(NAME_VERSIONED "${NAME}-${VERSION_SHORT}")
set(FILENAME "${NAME}-${VERSION}.zip")
set(FILENAME_URL "${VERSION}.zip")
set(URL "https://github.com/pachuco/${NAME}/archive/${FILENAME_URL}")
set(SHA_256_HASH "f65a4b33a060acca1294d35a57a8bbfda39af452daff76edcddb76d85b9468e3")
set(UNPACKED_DIR "${NAME}-${VERSION}")
download_patch_and_add(
        ${NAME_VERSIONED} ${FILENAME} ${URL} ${SHA_256_HASH} true ${UNPACKED_DIR} ""
)

file(GLOB SOURCES
        ${EXTERNAL_SOURCE_DIR}/lib_oldjaytrax/*.c
        src/main.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${EXTERNAL_SOURCE_DIR}/lib_oldjaytrax

        ${CMAKE_SOURCE_DIR}/src/app
        ${FMOD_API_DIR})

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}/${PLUGINS_DIR}
)
