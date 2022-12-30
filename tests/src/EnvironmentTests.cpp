#include "stc/Environment.hpp"
#include "catch2/catch_test_macros.hpp"

TEST_CASE("Syscommand should deal with sleeping", "[Environment][syscommand]") {
    std::string a, b;
    for (int i = 0; i < 2000; ++i) {
        a += ('a' + (i % 26));
        b += ('A' + (i % 26));
    }
    auto res = stc::syscommand("echo " + a + " && sleep 2 && echo " + b);

    REQUIRE(res == a + "\n" + b + "\n");

}

TEST_CASE("Syscommand should handle return codes", "[Environment][syscommand]") {
    int exitCode = 0;
    auto res = stc::syscommand("exit 69", &exitCode);

    REQUIRE(res == "");
    REQUIRE(exitCode == 69);

    REQUIRE_NOTHROW(res = stc::syscommand("gjdjgsdjgkfdsjkglfdjkglsfdjklgøsjklgøsjklgødjklgøsfdjklgø", &exitCode));
    REQUIRE(exitCode != 0);
}
