#include <catch2/catch_test_macros.hpp>

#include <sstream>
#include <string>

#include "stc/StdFix.hpp"

TEST_CASE("Ensure getline works like normal", "[getline]") {
    std::stringstream s("This is\na line\nand so is this");

    std::string buff;
    stc::StdFix::getline(s, buff);
    REQUIRE(buff == "This is");
    stc::StdFix::getline(s, buff);
    REQUIRE(buff == "a line");
    stc::StdFix::getline(s, buff);
    REQUIRE(buff == "and so is this");
    stc::StdFix::getline(s, buff);
    REQUIRE(buff.empty());
}

TEST_CASE("Ensure it eats \\r, \\n, and \\r\\n in mixed mode", "[getline]") {
    std::stringstream s("This is a carriage line\rthis is a windows line\r\nand this is a line using the only sane option of the bunch\n");

    std::string buff;
    stc::StdFix::getline(s, buff);
    REQUIRE(buff == "This is a carriage line");
    stc::StdFix::getline(s, buff);
    REQUIRE(buff == "this is a windows line");
    stc::StdFix::getline(s, buff);
    REQUIRE(buff == "and this is a line using the only sane option of the bunch");
    
    for (int i = 0; i < 10; ++i) {
        stc::StdFix::getline(s, buff);
        INFO(buff);
        REQUIRE(buff.empty());
    }

}
