#if !defined(_WIN32) && !defined(__APPLE__)

#include "_meta/Constants.hpp"
#include <stc/unix/Process.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Process should work with pipes", "[Process]") {
    stc::Unix::Process p(
        {
            ECHO_CMD,
            "Look at me, I'm a moving target",
            "Then suddenly, catgirls"
        }, {
            .stdoutPipe = std::make_shared<stc::Unix::Pipe>()
        }
    );
    REQUIRE(p.block() == 0);
    REQUIRE(p.getStdoutBuffer() == R"(Argument: ./bin/pseudoecho
Argument: Look at me, I'm a moving target
Argument: Then suddenly, catgirls
)"); // This linebreak is loadbearing for the test to work

    INFO("The stdout buffer should not be cleared after the last call");
    REQUIRE(p.getStdoutBuffer(true) != "");

    INFO("The stdout buffer should be cleared after the last call");
    REQUIRE(p.getStdoutBuffer() == "");

    REQUIRE(p.getStderrBuffer() == "");
} 

TEST_CASE("Process should work without pipes", "[Process]") {
    stc::Unix::Process p({
        "/usr/bin/env", "bash", "-c", "exit 69"
    });
    REQUIRE(p.block() == 69);
}

TEST_CASE("Process should work with PTY mode", "[Process]") {
    stc::Unix::Process p({
        "/usr/bin/env", "bash", "-"
    }, std::make_shared<stc::Unix::PTY>());

    REQUIRE(p.writeToStdin("echo 'hi'\n") == 10);
    REQUIRE(p.writeToStdin("exit 69\n") == 8);

    // TODO: figure out if there's a way to use catch2 to set timeouts for stuff. This will block indefinitely if
    // something has gone wrong in the implementation
    REQUIRE(p.block() == 69);
    REQUIRE(p.getStderrBuffer().empty());
    REQUIRE_FALSE(p.getStdoutBuffer().empty());

    INFO(p.getStdoutBuffer());
    REQUIRE(p.getStdoutBuffer().find("$ echo 'hi'") != std::string::npos);
}

#endif
