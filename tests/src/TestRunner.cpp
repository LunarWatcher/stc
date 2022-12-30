#define CATCH_CONFIG_RUNNER
#include "catch2/catch_session.hpp"

#include "stc/Environment.hpp"
#include <exception>
#include <iostream>

class NotARealShellException : public std::exception {
public:
    const char* what() { return "PowerShell isn't a real shell"; }
};

int main(int argc, const char* argv[], char** envp) {
    return Catch::Session().run(argc, argv);
}
