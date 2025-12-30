cmake_minimum_required(VERSION 3.28)

project(logger)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 23)

file(GLOB SOURCES src/logger.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

if (WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
            OUTPUT_NAME "${PROJECT_NAME}"
            PREFIX ""
            LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}
            RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
else ()
    set_target_properties(${PROJECT_NAME} PROPERTIES
            OUTPUT_NAME "${PROJECT_NAME}"
            PREFIX "lib"
            LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR}
            RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/${LIB_DIR})
endif ()

set(LOGGER_API_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src" PARENT_SCOPE)
