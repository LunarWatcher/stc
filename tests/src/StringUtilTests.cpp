#include "catch2/catch_test_macros.hpp"

#include "stc/StringUtil.hpp"

TEST_CASE("Ensure splitting works with single characters", "[Feat][String]") {
    auto res = stc::string::split("Hello,there", ',');
    REQUIRE(res.size() == 2);
    REQUIRE(res.at(0) == "Hello");
    REQUIRE(res.at(1) == "there");

    res = stc::string::split("A:B:C:D:E", ':', 2);
    REQUIRE(res.size() == 3);
    REQUIRE(res.at(0) == "A");
    REQUIRE(res.at(1) == "B");
    REQUIRE(res.at(2) == "C:D:E");
}

TEST_CASE("Ensure excessive limits don't interfere", "[Feat][String]") {
    auto res = stc::string::split("Hello,there", ',', 696969);
    REQUIRE(res.size() == 2);
    REQUIRE(res.at(0) == "Hello");
    REQUIRE(res.at(1) == "there");
}

TEST_CASE("Ensure the special-case charification works", "[Feat][String]") {
    std::string string = "This is sparta";
    auto res = stc::string::split(string, "");
    REQUIRE(res.size() == string.size());
    for (size_t i = 0; i < string.size(); ++i) {
        REQUIRE(res.at(i) == std::string(1, string.at(i)));
    }

    res = stc::string::split(string, 0);
    REQUIRE(res.size() == string.size());
    for (size_t i = 0; i < string.size(); ++i) {
        REQUIRE(res.at(i) == std::string(1, string.at(i)));
    }
}

TEST_CASE("Make sure single-character splits don't default to nullptr", "[Regression][String]") {
    auto res = stc::string::split("Hello,there", ",");
    REQUIRE(res.size() == 2);
    REQUIRE(res.at(0) == "Hello");
    REQUIRE(res.at(1) == "there");

    res = stc::string::split("A:B:C:D:E", ":", 2);
    REQUIRE(res.size() == 3);
    REQUIRE(res.at(0) == "A");
    REQUIRE(res.at(1) == "B");
    REQUIRE(res.at(2) == "C:D:E");
}
