#include "catch2/catch_test_macros.hpp"

#include "stc/FileLock.hpp"

TEST_CASE("Ensure locking works", "[LockTests]") {
    std::filesystem::remove("stc_test_lock");

    // Repeat the lock 10 times; this ensures it actually unlocks as it should in a normal scenario.
    for (int i = 0; i < 10; ++i) {
        stc::FileLock lock("stc_test_lock");

        REQUIRE(lock.hasLock());
        REQUIRE(std::filesystem::exists("stc_test_lock"));
    }
    REQUIRE_FALSE(std::filesystem::exists("stc_test_lock"));

    stc::FileLock lock("stc_test_lock");
    REQUIRE(lock.hasLock());


    for (int i = 0; i < 10; ++i) {
        REQUIRE_THROWS([]() {
            stc::FileLock lock("stc_test_lock");
        }(), stc::FileLock::Errors::LOCK_ERROR);
    }

}

TEST_CASE("Ensure dynamic locking works", "[LockTests]") {
    std::filesystem::remove("stc_test_lock");

    stc::FileLock initLock("stc_test_lock");
    REQUIRE(initLock.hasLock());

    REQUIRE_THROWS([]() {
        stc::FileLock controlLock("stc_test_lock");
    }(), stc::FileLock::Errors::LOCK_ERROR);

    int count = 0;
    auto dynlock = stc::FileLock::dynamicAcquireLock("stc_test_lock", [&]() {
        count += 1;
        if (count == 10) {
            initLock.unlock();
        }
        return true;
    }, 0);
    REQUIRE(count == 10);
    REQUIRE(dynlock->hasLock());
    REQUIRE_FALSE(initLock.hasLock());
}

TEST_CASE("Validate deletion semantics", "[LockTests]") {
    std::filesystem::remove("stc_test_lock");
    SECTION("It should be removed automatically when a new lock is acquired") {
        stc::FileLock initLock("stc_test_lock");
        REQUIRE(initLock.hasLock());
        REQUIRE(std::filesystem::exists("stc_test_lock"));
        initLock.unlock();
        REQUIRE_FALSE(std::filesystem::exists("stc_test_lock"));
    }
}
