#include <catch2/catch_test_macros.hpp>

#include "stc/test/TestEnvVariable.hpp"

TEST_CASE("Verify that FreeEnv clears the variable after use", "[FreeEnv]") {
    try {
        stc::testutil::TestEnvVariable e("rawr", "__value__");
        REQUIRE(stc::getEnv("rawr") == "__value__");

        throw std::runtime_error("hi");
    } catch (std::runtime_error) {} // NOLINT
    REQUIRE(stc::getEnv("rawr", "yes") == "yes");
}
