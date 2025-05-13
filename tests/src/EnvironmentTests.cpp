#include "_meta/Constants.hpp"
#include "stc/Environment.hpp"
#include "stc/StringUtil.hpp"

#include "catch2/catch_test_macros.hpp"

#ifndef _WIN32
// Would be great to test this on Windows as well, but windows is so unreliable
// that this test will not work.
//
// sleep isn't a thing, timeout is.
//
// Timeout prints a lot of unnecessary text, so it's dropped.
//
// Attempting to redirect it to nul errors out and immediately exits.
//
// Good fucking job, microsoft. Y'all suck at making software
TEST_CASE("Syscommand should deal with sleeping", "[Environment][syscommand]") {
    std::string a, b;
    for (int i = 0; i < 2000; ++i) {
        a += ('a' + (i % 26));
        b += ('A' + (i % 26));
    }
    auto res = stc::syscommand("echo " + a + "&& sleep 4 && echo " + b);
    INFO(stc::string::getByteString(res));
    INFO(stc::string::getByteString(a));
    INFO(stc::string::getByteString(b));

    REQUIRE(res == a + "\n" + b + "\n");

}
#endif

TEST_CASE("Syscommand should handle return codes", "[Environment][syscommand]") {
    int exitCode = 0;
    auto res = stc::syscommand("exit 69", &exitCode);

    REQUIRE(res == "");
    REQUIRE(exitCode == 69);

    REQUIRE_NOTHROW(res = stc::syscommand("gjdjgsdjgkfdsjkglfdjkglsfdjklgøsjklgøsjklgødjklgøsfdjklgø", &exitCode));
    REQUIRE(exitCode != 0);
}

TEST_CASE("Verify hostname return value", "[Environment][getHostname]") {
    auto control = stc::syscommand("hostname");
    if (auto eol = control.find('\n'); eol >= 0) {
        control.replace(eol, control.size() - eol, "");
    }

    REQUIRE(control != "");

    auto hostName = stc::getHostname();

    REQUIRE(hostName.has_value());
    REQUIRE(hostName->size() != 0);
    REQUIRE(hostName->size() == control.size());
    INFO(*hostName);
    REQUIRE(hostName == control);
}

TEST_CASE("Vectorised syscommand should deal with output", "[Environment][syscommand2]") {
    int statusCode;
    auto output = stc::syscommand(std::vector {
        ECHO_CMD,
        "hello"
    }, &statusCode);

    REQUIRE(statusCode == 0);
    REQUIRE(output == ("Argument: " ECHO_CMD "\nArgument: hello\n"));
}

TEST_CASE("Vectorised syscommand should handle errors", "[Environment][syscommand2]") {
    int code;
    REQUIRE_NOTHROW([&]() {
        stc::syscommand(std::vector {
            "fhdjohgjkfdshgjfkdslhgjfkdlshjgkfldshjkgfdsjlhkgfd"
        }, &code);
    }());
    REQUIRE(code != 0);
}
