#define CATCH_CONFIG_RUNNER
#include "catch2/catch_session.hpp"

#include "stc/Environment.hpp"
#include <exception>
#include <iostream>

int main(int argc, const char* argv[]) {
    stc::syscommand("timeout /T 2 /NOBREAK >nul");
    return Catch::Session().run(argc, argv);
}
