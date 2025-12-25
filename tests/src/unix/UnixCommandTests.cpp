#if !defined(_WIN32) && !defined(__APPLE__)

#include <format>
#include <filesystem>
#include <stc/StringUtil.hpp>
#include <util/FreeEnv.hpp>
#include "_meta/Constants.hpp"
#include <stc/unix/Process.hpp>
#include <catch2/catch_test_macros.hpp>
#include <stc/test/CaptureStream.hpp>

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

TEST_CASE("Clearing buffers should work", "[Process]") {
    stc::Unix::Process p({
        "/usr/bin/env", "bash", "-i"
    }, stc::Unix::Pipes::separate(true));

}

TEST_CASE("Not stopping a process shouldn't cause a segfault", "[Process]") {
    std::shared_ptr<stc::Unix::Process> p;
    std::vector<std::string> command = {
        "/usr/bin/env", "bash", "-c", "sleep 5"
    };
    SECTION("Pipes") {
        p = std::make_shared<stc::Unix::Process>(
            command,
            stc::Unix::Pipes::separate()
        );
    }
    SECTION("PTY") {
        p = std::make_shared<stc::Unix::Process>(
            command,
            stc::Unix::createPTY()
        );
    }
    SECTION("Nocapture") {
        p = std::make_shared<stc::Unix::Process>(
            command
        );
    }

    REQUIRE(p != nullptr); // safeguard
    p.reset();
    REQUIRE(p == nullptr); // safeguard

    // This observationally seems to work, but not entirely sure if it's 100% reliable. 
    // For some reason, using stop() instead of sigkill() seems to propagate a SIGTERM out of waitpid()
    // Future me: this seems to also happen with sigkill in certain cases. This _is_ catch2 spotting the signal and
    // treating it as an error. Not sure how I can prevent this.
    SUCCEED("If a SIGABRT is triggered, this fails");
}

TEST_CASE("Chdir changing should work as expected", "[Process]") {
    // TODO: handling output like this is awkward
    auto currDir = std::filesystem::current_path().string();

    SECTION("Current working directory is current") {
        stc::Unix::Process p({
            "/usr/bin/env", "bash", "-c", "pwd"
        }, stc::Unix::Pipes::separate(false));
        REQUIRE(p.block() == 0);
        REQUIRE(p.getStdoutBuffer() == currDir + "\n");
    }

    SECTION("Invalid working directory should throw") {
        REQUIRE_THROWS(
            stc::Unix::Process({
                "/usr/bin/env", "bash", "-c", "pwd"
            }, 
                stc::Unix::Pipes::separate(false),
                stc::Unix::Environment {
                    .workingDirectory = "/gjfdhfdhjfdhjfdhjgkfdjkfghhgjkfdurjkfdj"
                }
            )
        );
    }
    SECTION("Valid working directory is valid") {
        stc::Unix::Process p({
            "/usr/bin/env", "bash", "-c", "pwd"
        }, 
            stc::Unix::Pipes::separate(false),
            stc::Unix::Environment {
                .workingDirectory = "/usr/bin"
            }
        );
        INFO(
            "You appear to be running this test in a completely fucked environment, or added Windows support without "
            "updating this test"
        );
        REQUIRE(std::filesystem::exists("/usr/bin"));
        REQUIRE(p.block() == 0);
        REQUIRE(p.getStdoutBuffer() == "/usr/bin\n");
    }

    INFO(
        "This asserts that the current path for the parent process hasn't changed. The working directory change should "
        "only affect the child process."
    );
    REQUIRE(std::filesystem::current_path().string() == currDir);
}

TEST_CASE("When enabled, Process should output its command") {
    stc::testutil::CaptureStandardStreams capt;
    stc::Unix::Process p({
        "/usr/bin/env", "bash", "-c", "echo", "hi"
    }, stc::Unix::Pipes::shared(false), std::nullopt, {
        .verboseUserOutput = true
    });


    auto result = p.block();
    INFO(p.getStderrBuffer() << "\n" << p.getStdoutBuffer());
    REQUIRE(result == 0);
    REQUIRE(capt.cerr.content.str() == "");
    INFO("|" << capt.cout.content.str() << "|");
    REQUIRE(capt.cout.content.str() == R"(Exec: "/usr/bin/env" "bash" "-c" "echo" "hi"
)"); // This linebreak is loadbearing
}

#endif
