cmake_minimum_required(VERSION 3.0.2)
project(slice99 LANGUAGES C)

# Fix the warnings about `DOWNLOAD_EXTRACT_TIMESTAMP` in newer CMake versions.
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
    cmake_policy(SET CMP0135 NEW)
endif()

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
