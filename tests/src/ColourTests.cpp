#include <catch2/catch_test_macros.hpp>

#include <sstream>
#include <stc/Colour.hpp>
using namespace stc;

TEST_CASE("Forcing allows std::stringstream to contain ANSI") {
    std::stringstream ss;
    SECTION("Baseline: std::stringstream contains no ANSI") {
        ss << colour::bg<0>
            << "Text "
            << colour::reset
            << "Normal text";

        REQUIRE(ss.str() == "Text Normal text");
    }
    SECTION("Forced std::stringstream contains ANSI") {
        ss << colour::force
            << colour::bg<0>
            << "Text "
            << colour::reset
            << "Normal text";

        REQUIRE(ss.str() != "Text Normal text");
    }
    SECTION("Forced output is togglable") {
        ss << colour::force
            << colour::bg<0>
            << "Text "
            << colour::force<false>
            << "Normal text";

        // The ANSI stuff will prevent Text from technically being the first part of the string
        REQUIRE(!ss.str().starts_with("Text "));
        REQUIRE(ss.str().ends_with("Normal text"));
    }
}
