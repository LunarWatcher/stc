#define CATCH_CONFIG_RUNNER
#include "catch2/catch_session.hpp"

#include "stc/Environment.hpp"
#include <exception>
#include <iostream>

int main(int argc, const char* argv[], char** envp) {
    return Catch::Session().run(argc, argv);
}
