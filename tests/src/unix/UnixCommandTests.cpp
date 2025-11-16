#if !defined(_WIN32) && !defined(__APPLE__)

#include <format>
#include <stc/StringUtil.hpp>
#include <util/FreeEnv.hpp>
#include "_meta/Constants.hpp"
#include <stc/unix/Process.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace std::literals;

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

TEST_CASE("Process should handle signals appropriately", "[Process]") {
    SECTION("PTY mode") {
        {
            stc::Unix::Process p({
                "/usr/bin/env", "bash", "-"
            }, std::make_shared<stc::Unix::PTY>());
            int signal;
            SECTION("SIGKILL") {
                p.sigkill();
                signal = 9;
            }
            SECTION("SIGTERM") {
                p.stop();
                signal = 15;
            }
            REQUIRE_NOTHROW(
                p.block() == signal
            );
            REQUIRE_FALSE(p.hasExitedNormally().value());
        }
    }
    SECTION("Pipe mode") {
        {
            stc::Unix::Process p({
                "/usr/bin/env", "bash", "-"
            }, stc::Unix::Pipes {});
            int signal;
            SECTION("SIGKILL") {
                p.sigkill();
                signal = 9;
            }
            SECTION("SIGTERM") {
                p.stop();
                signal = 15;
            }
            REQUIRE(
                p.block() == signal
            );
            REQUIRE_FALSE(p.hasExitedNormally().value());
        }
    }
}

TEST_CASE("exitedNormally should not be set by a self-exiting process", "[Process]") {
    stc::Unix::Process p({
        "/usr/bin/env", "bash", "-"
    }, stc::Unix::Pipes { .stdinPipe = stc::Unix::createPipe() });
    int code;
    SECTION("Non-zero code") {
        code = 69;
    }
    SECTION("Zero code") {
        code = 0;
    }
    std::string command = std::format("exit {}\n", code);
    REQUIRE(p.writeToStdin(command) == (ssize_t) command.length());
    REQUIRE(p.block() == code);
    REQUIRE(p.hasExitedNormally().has_value());
    REQUIRE(p.hasExitedNormally().value() == true);
}

// Note that the environment forwarding test-cases shouldn't 
TEST_CASE("Baseline: Environment should default to environ", "[Process]") {
    stc::Unix::Process p({
        "/usr/bin/env"
    }, stc::Unix::Pipes::shared(false));

    REQUIRE(p.block() == 0);
    auto out = p.getStdoutBuffer();
    auto envs = stc::string::split(out, "\n");

    size_t size = 0;
    for (char **env = environ; *env != nullptr; env++) {
        ++size;
    }

    INFO(out);
    INFO(
        "If this mismatches, it's possible a race condition bug has been reintroduced. "
        "This largely applies if envs.size() == 0. This results in flaky behaviour here"
    );
    REQUIRE(
        // +1 to account for _=/usr/bin/env, which is added by `env`
        envs.size() == size + 1
    );
    // Looks like order is guaranteed, which is neat
    for (size_t i = 0; i < size; ++i) {
        REQUIRE(
            strcmp(*(environ + i), envs.at(i).c_str()) == 0
        );
    }
}

TEST_CASE("stc::Unix::Environment should be able to ignore environ", "[Process]") {
    stc::Unix::Process p({
        "/usr/bin/env"
    }, stc::Unix::Pipes::shared(false), stc::Unix::Environment {
        {{"OwO", "x3 nuzzles pounces on you"}},
        false
    });

    REQUIRE(p.block() == 0);
    auto out = p.getStdoutBuffer();
    auto envs = stc::string::split(out, "\n");

    INFO(out);
    INFO(
        "If this mismatches, it's possible a race condition bug has been reintroduced. "
        "This largely applies if envs.size() == 0. This results in flaky behaviour here"
    );
    REQUIRE(
        // OwO + _=/usr/bin/env
        envs.size() == 2
    );
    REQUIRE(envs.at(0) == "OwO=x3 nuzzles pounces on you");
}

TEST_CASE("stc::Unix::Environment should merge properly with environ", "[Process]") {
    stc::Unix::Process p({
        "/usr/bin/env"
    }, stc::Unix::Pipes::shared(false), stc::Unix::Environment {
        {{"OwO", "x3 nuzzles pounces on you"}},
        true
    });

    REQUIRE(p.block() == 0);
    auto out = p.getStdoutBuffer();
    auto envs = stc::string::split(out, "\n");

    size_t size = 0;
    for (char **env = environ; *env != nullptr; env++) {
        ++size;
    }

    INFO(out);
    INFO(
        "If this mismatches, it's possible a race condition bug has been reintroduced. "
        "This largely applies if envs.size() == 0. This results in flaky behaviour here"
    );
    REQUIRE(
        // +2 to account for _=/usr/bin/env, which is added by `env`, and OwO added by the test
        envs.size() == size + 2
    );
    // Looks like order is guaranteed, which is neat
    for (size_t i = 0; i < size; ++i) {
        REQUIRE(
            strcmp(*(environ + i), envs.at(i).c_str()) == 0
        );
    }

    // at(size) is OwO, at(size + 1) is _=
    REQUIRE(envs.at(size) == "OwO=x3 nuzzles pounces on you");
}

TEST_CASE("stc::Unix::Environment defining existing variables shouldn't cause problems", "[Process]") {
    // Used to sneak stuff into environ()
    util::FreeEnv e("__PROCESS_TEST_CASE", "not_overridden");

    stc::Unix::Process p({
        "/usr/bin/env"
    }, stc::Unix::Pipes::shared(false), stc::Unix::Environment {
        {{"__PROCESS_TEST_CASE", "Trans rights are human rights"}},
        true
    });

    REQUIRE(p.block() == 0);
    auto out = p.getStdoutBuffer();
    auto envs = stc::string::split(out, "\n");

    size_t size = 0;
    for (char **env = environ; *env != nullptr; env++) {
        ++size;
    }

    INFO(out);
    INFO(
        "If this mismatches, it's possible a race condition bug has been reintroduced. "
        "This largely applies if envs.size() == 0. This results in flaky behaviour here"
    );
    REQUIRE(
        // +1 to account for _=/usr/bin/env, which is added by `env`
        // __PROCESS_TEST_CASE does not need to be registered, because it's added directly to environ by setEnv
        envs.size() == size + 1
    );
    // Looks like order is guaranteed, which is neat
    for (size_t i = 0; i < size; ++i) {
        if (envs.at(i).starts_with("__PROCESS_TEST_CASE")) {
            REQUIRE(envs.at(i) == "__PROCESS_TEST_CASE=Trans rights are human rights");
        }
    }
} 

#endif
