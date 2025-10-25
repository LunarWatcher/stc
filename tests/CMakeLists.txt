add_executable(pseudoecho src/_meta/Echo.cpp)
add_executable(tests
    src/ColourTests.cpp
    src/EnvironmentTests.cpp
    src/LockTests.cpp
    src/StdFixTests.cpp
    src/StringUtilTests.cpp

    # This test does not contain any tests, but is a test through just existing.
    # It includes everything, which forces everything to be evaluated, which means 
    # I coudl've detected .starts_with() being used when that's a C++20 function
    # being used in a C++17 library :facepaw:
    src/compiling/EverythingCompilesTest.cpp

    src/TestRunner.cpp
)
Include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.8.1
)

FetchContent_MakeAvailable(Catch2)

target_include_directories(tests PUBLIC ./src/)
target_link_libraries(tests Catch2::Catch2 stc)
add_dependencies(tests pseudoecho)
