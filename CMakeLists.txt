cmake_minimum_required(VERSION 3.10)
project(stc)

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    message (STATUS "stc: Running standalone.")

    set(OUTPUT ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT})

    if (NOT CMAKE_BUILD_TYPE)
        # Make explicit flags redundant for release builds
        set(CMAKE_BUILD_TYPE Release)
    endif()

    set (CMAKE_CXX_STANDARD 17)

    if (NOT WIN32)
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpedantic -Wall -Wextra")
    endif()

    add_subdirectory(tests)

else()
    message (STATUS "stc: Submodule detected. Enabling interface target")

    add_library(stc INTERFACE)
    target_include_directories(stc INTERFACE src/)
endif()
