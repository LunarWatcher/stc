#include "stc/minilog.hpp"
#include "stc/test/CaptureStream.hpp"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("All functions compile") {
    stc::testutil::CaptureStandardStreams capt;
    minilog::debug("Test {}", "function");
    minilog::info("Test {}", "function");
    minilog::warn("Test {}", "function");
    minilog::error("Test {}", "function");
    minilog::critical("Test {}", "function");

    auto cerr = capt.cerr.content.str();
    auto cout = capt.cout.content.str();

    REQUIRE(cerr == "");
    REQUIRE(cout.find("| debug    | Test function") != std::string::npos);
    REQUIRE(cout.find("| info     | Test function") != std::string::npos);
    REQUIRE(cout.find("| warning  | Test function") != std::string::npos);
    REQUIRE(cout.find("| error    | Test function") != std::string::npos);
    REQUIRE(cout.find("| critical | Test function") != std::string::npos);

}
