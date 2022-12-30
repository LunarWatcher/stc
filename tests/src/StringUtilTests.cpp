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

TEST_CASE("Ensure multibyte splits work", "[Feat][String]") {
    auto res = stc::string::split("A>=B>=C>=D", ">=");
    REQUIRE(res.size() == 4);
    for (size_t i = 0; i < res.size(); ++i) {
        REQUIRE(res.at(i) == std::string(1, 'A' + i));
    }

    res = stc::string::split("This potato is potato orange", " potato ");
    REQUIRE(res.size() == 3);
    REQUIRE(res.at(0) == "This");
    REQUIRE(res.at(1) == "is");
    REQUIRE(res.at(2) == "orange");
}

TEST_CASE("Ensure limited multibyte splits work", "[Feat][String]") {
    auto res = stc::string::split("A>=B>=C>=D", ">=", 2);
    REQUIRE(res.size() == 3);
    REQUIRE(res.at(0) == "A");
    REQUIRE(res.at(1) == "B");
    REQUIRE(res.at(2) == "C>=D");

    res = stc::string::split("This potato is potato orange", " potato ", 1);
    REQUIRE(res.size() == 2);
    REQUIRE(res.at(0) == "This");
    REQUIRE(res.at(1) == "is potato orange");
}

TEST_CASE("Ensure edges don't affect anything", "[BugProne][String]") {
    // The test-cases exist to test both single characters.
    // The long string is to emphasise one-off errors, if there are any
    for (auto d : std::vector<std::string>{"or", ".", "abcdabcdabcd", "å"}) {
        // Extreme case_ only delimiters
        auto res = stc::string::split(d + d + d, d);
        REQUIRE(res.size() == 4);
        for (auto& str : res) REQUIRE(str.size() == 0);

        // Test open edge
        res = stc::string::split(d + "B" + d + "C", d);
        REQUIRE(res.size() == 3);
        REQUIRE(res.at(0).size() == 0);
        REQUIRE(res.at(1) == "B");
        REQUIRE(res.at(2) == "C");

        // Test close edge
        res = stc::string::split("A" + d + "B" + d, d);
        REQUIRE(res.size() == 3);
        REQUIRE(res.at(0) == "A");
        REQUIRE(res.at(1) == "B");
        REQUIRE(res.at(2).size() == 0);

        // Test both edges at once
        res = stc::string::split(d + "B" + d, d);
        REQUIRE(res.size() == 3);
        REQUIRE(res.at(0).size() == 0);
        REQUIRE(res.at(1) == "B");
        REQUIRE(res.at(2) == "");
    }
}

TEST_CASE("Ensure edges don't affect anything when counts are involved", "[BugProne][String]") {
    auto res = stc::string::split("...", ".", 1);
    REQUIRE(res.size() == 2);
    REQUIRE(res.at(0) == "");
    REQUIRE(res.at(1) == "..");

    res = stc::string::split("...", ".", 2);
    REQUIRE(res.size() == 3);
    REQUIRE(res.at(0) == "");
    REQUIRE(res.at(1) == "");
    REQUIRE(res.at(2) == ".");

    res = stc::string::split("orBor", "or", 1);
    REQUIRE(res.size() == 2);
    REQUIRE(res.at(0).size() == 0);
    REQUIRE(res.at(1) == "Bor");
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
