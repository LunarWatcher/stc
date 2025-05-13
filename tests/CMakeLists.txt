add_executable(tests
    src/StdFixTests.cpp
    src/LockTests.cpp
    src/StringUtilTests.cpp
    src/EnvironmentTests.cpp

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
