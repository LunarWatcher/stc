#include "stc/test/CaptureStream.hpp"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Verify that stdout can be captured by CaptureStream", "[CaptureStream]") {
    stc::testutil::CaptureStream cap(std::cout);

    std::cout << "I like trains" << std::endl;
    REQUIRE(cap.content.str() == "I like trains\n");

    cap.restore();
    std::cout << "CaptureStream test output after release (if you're seeing this, the test has passed)\n";
    REQUIRE(cap.content.str() == "I like trains\n");
}

TEST_CASE("Verify that stdin can be redirected by CaptureStream", "[CaptureStream]") {
    stc::testutil::CaptureStream cap(std::cin);

    std::string buff;
    REQUIRE_FALSE(std::getline(std::cin, buff));

    // We need to reset here because the last getline sets some error state somewhere, and I don't know how to reset it.
    // .clear() does not seem to work.
    // TODO: This can probably be solved better, I just don't care enough right now
    cap.reset();
    cap.content << "line 1\nline 2\n";

    INFO("This may fail if cap.reset() breaks");
    REQUIRE(std::getline(std::cin, buff));
    REQUIRE(buff == "line 1");

    REQUIRE(std::getline(std::cin, buff));
    REQUIRE(buff == "line 2");

    REQUIRE_FALSE(std::getline(std::cin, buff));
}
