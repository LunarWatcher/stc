#define CATCH_CONFIG_RUNNER
#include "catch2/catch_session.hpp"

#include "stc/Environment.hpp"
#include <exception>
#include <iostream>

int main(int argc, const char* argv[], char** envp) {
    std::cout << "test: " << std::endl;
    system("timeout 2 >nul");
    std::cout << "End test" << std::endl;
    return Catch::Session().run(argc, argv);
}
