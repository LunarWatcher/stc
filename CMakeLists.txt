cmake_minimum_required(VERSION 3.10)
project(stc)

add_library(stc INTERFACE)
target_include_directories(stc INTERFACE src/)

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    message (STATUS "stc: Running standalone, enabling test targets.")

    set(OUTPUT ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT})

    option(STC_LINT "Whether or not to enable clang-tidy checks" OFF)
    option(STC_LINT_WARNINGS_ARE_ERRORS "Whether or not to set -warnings-as-errors" OFF)


    if (NOT CMAKE_BUILD_TYPE)
        # Make explicit flags redundant for release builds
        set(CMAKE_BUILD_TYPE Release)
    endif()

    set (CMAKE_CXX_STANDARD 20)

    if (NOT WIN32)
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpedantic -Wall -Wextra -fsanitize=undefined")
    endif()

    add_subdirectory(tests)
    add_subdirectory(demos)
    add_custom_target(test
        COMMAND tests ${ALUI_TEST_OPTIONS}
        DEPENDS tests
        COMMENT "Compile and run tests")

    find_package(Doxygen)
    if (DOXYGEN_FOUND)
        set(DOXYGEN_GENERATE_HTML YES)
        set(DOXYGEN_GENERATE_MAN NO)
        set(DOXYGEN_EXCLUDE_SYMBOLS "std::*")
        set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "README.md")
        set(DOXYGEN_RECURSIVE YES)
        set(DOXYGEN_EXTRACT_STATIC YES)
        set(DOXYGEN_EXTRACT_ALL YES)
        list(
            APPEND DOXYGEN_EXCLUDE_PATTERNS
            "*/build/*"
            "*/_deps/*"
            "*/old_docs/*"
        )

        doxygen_add_docs(
            doxygen
            ${PROJECT_SOURCE_DIR}
            COMMENT "Generate documentation"
        )
    else()
        message(WARNING "Not generating documentation: doxygen missing")
    endif()

    if (STC_LINT)
        find_program(
            CLANGTIDY
            NAMES "clang-tidy" "clang-tidy.exe"
            REQUIRED
            )

        set (CLANG_TIDY_COMMAND "${CLANGTIDY}" "--config-file=${CMAKE_CURRENT_SOURCE_DIR}/.clang-tidy")
        if (STC_LINT_WARNINGS_ARE_ERRORS)
            list (APPEND CLANG_TIDY_COMMAND  -warnings-as-errors=*)
        endif()

        if (WIN32)
            list (APPEND CLANG_TIDY_COMMAND --extra-arg=/EHsc)
        endif()

        message(STATUS "Using CLANG_TIDY_COMMAND=${CLANG_TIDY_COMMAND}")
        set_target_properties(stc PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}")
    endif()

endif()
