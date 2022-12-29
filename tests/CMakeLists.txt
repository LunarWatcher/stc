add_executable(tests
    src/StdFixTests.cpp
    src/LockTests.cpp
    src/StringUtilTests.cpp
)
Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.1.0
)

FetchContent_MakeAvailable(Catch2)

target_include_directories(tests PUBLIC ../src/ ./src/)
target_link_libraries(tests Catch2::Catch2WithMain)
