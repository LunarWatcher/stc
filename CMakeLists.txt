cmake_minimum_required(VERSION 3.10)
project(stc)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if (NOT CMAKE_BUILD_TYPE)
    # Make explicit flags redundant for release builds
    set(CMAKE_BUILD_TYPE Release)
endif()

add_subdirectory(tests)


# CMake is exclusively used for tests. `add_subdirectory()` with this file will cause stc tests to be built
# ... at least it will when they're added :)
