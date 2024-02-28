cmake_minimum_required(VERSION 3.24)

set(NAME AZEmulator)
set(VERSION 1.0)

project(${NAME})

if (NOT DEFINED CMAKE_BUILD_TYPE)
    message(FATAL_ERROR "CMAKE_BUILD_TYPE not defined")
endif ()

set(CMAKE_CXX_STANDARD 20)
set(BUILD_SHARED_LIBS ON)

if (NOT DEFINED APP_OUTPUT_DIR)
    set(APP_OUTPUT_DIR ${CMAKE_BINARY_DIR}/OUT)
endif ()

add_subdirectory(sources/libraries)

add_subdirectory(sources/main)
