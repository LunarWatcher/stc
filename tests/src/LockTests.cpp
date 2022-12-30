#include "catch2/catch_test_macros.hpp"

#include "stc/FS.hpp"
#include "stc/FileLock.hpp"

#include <iostream>
#include <fstream>

#ifndef _WIN32
TEST_CASE("Ensure locking works", "[LockTests]") {
    fs::remove("stc_test_lock");

    {
        std::ofstream output("stc_test_lock");
        output << "hi" << std::endl;
        output.close();
        std::ifstream input("stc_test_lock");
        std::string content;
        input >> content;
        REQUIRE(content == "hi");
        fs::remove("stc_test_lock");
    }

    // Repeat the lock 10 times; this ensures it actually unlocks as it should in a normal scenario.
    for (int i = 0; i < 10; ++i) {
        stc::FileLock lock("stc_test_lock");

        REQUIRE(lock.hasLock());
        REQUIRE(fs::exists("stc_test_lock"));
    }
    REQUIRE_FALSE(fs::exists("stc_test_lock"));

    stc::FileLock lock("stc_test_lock");
    REQUIRE(lock.hasLock());


    for (int i = 0; i < 10; ++i) {
        REQUIRE_THROWS([]() {
            stc::FileLock lock("stc_test_lock");
        }(), stc::FileLock::Errors::LOCK_ERROR);
    }

}
#else
#warn "File lock support for Windows has not been added yet"
#endif
