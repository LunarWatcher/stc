add_executable(pseudoecho src/_meta/Echo.cpp)
add_executable(tests
    src/Main.cpp

    src/ColourTests.cpp
    src/EnvironmentTests.cpp
    src/LockTests.cpp
    src/StdFixTests.cpp
    src/StringUtilTests.cpp

    src/extra-modules/test/CaptureStreamTests.cpp

    src/unix/UnixCommandTests.cpp

    # Test utils and test util tests
    src/util/FreeEnvTests.cpp

    # This test does not contain any tests, but is a test through just existing.
    # It includes everything, which forces everything to be evaluated, which means 
    # I coudl've detected .starts_with() being used when that's a C++20 function
    # being used in a C++17 library :facepaw:
    src/compiling/EverythingCompilesTest.cpp
)
Include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.11.0
)

FetchContent_MakeAvailable(Catch2)

target_include_directories(tests PUBLIC ./src/)
target_link_libraries(
    tests 
    Catch2::Catch2 
    stc::stc
    stc::testutil
)
add_dependencies(tests pseudoecho)
