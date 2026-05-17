#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>

#include <stc/test/TestDirectory.hpp>

TEST_CASE("Test local directory") {
    auto cwd = std::filesystem::current_path();
    auto existing = cwd / "_stc_tests_created_before_test";
    auto nonExisting = cwd / "_stc_tests_created_during_init";

    // Safeguards against TestDirectory failures
    std::filesystem::remove_all(nonExisting);
    std::filesystem::remove_all(existing);

    std::filesystem::create_directories(existing);

    SECTION("Test creation of non-existing directory") {
        REQUIRE(!std::filesystem::exists(nonExisting));
        REQUIRE_NOTHROW([&](){
            stc::testutil::TestDirectory d{nonExisting, true};
            REQUIRE(std::filesystem::exists(nonExisting));

            auto nestedPath = nonExisting / "test" / "nested";
            auto filePath = nestedPath / "files.txt";
            std::filesystem::create_directories(nestedPath);
            {
                std::ofstream f(filePath);
                REQUIRE(bool(f));
                f << "owo" << std::endl;
            }
        }());
        REQUIRE(!std::filesystem::exists(nonExisting));
    }
    SECTION("Test creation of initially existing directory") {
        REQUIRE(std::filesystem::exists(existing));
        REQUIRE_NOTHROW([&](){
            stc::testutil::TestDirectory d{existing, true};
            REQUIRE(std::filesystem::exists(existing));

            auto nestedPath = existing / "test" / "nested";
            auto filePath = nestedPath / "files.txt";
            std::filesystem::create_directories(nestedPath);
            {
                std::ofstream f(filePath);
                REQUIRE(bool(f));
                f << "owo" << std::endl;
            }
        }());
        REQUIRE(!std::filesystem::exists(existing));
    }

    SECTION("Test freeing of non-created directory") {
        REQUIRE(!std::filesystem::exists(nonExisting));
        REQUIRE_NOTHROW([&](){
            stc::testutil::TestDirectory d{nonExisting, false};
            REQUIRE(!std::filesystem::exists(nonExisting));
        }());
        REQUIRE(!std::filesystem::exists(nonExisting));
    }

    SECTION("Test protection against UNIX folders") {
#ifndef _WIN32
        REQUIRE_THROWS(
            stc::testutil::TestDirectory{ "/usr/bin/this_file_does_not_exist" }
        );
        REQUIRE_THROWS(
            stc::testutil::TestDirectory{ "/boot/bin/this_file_does_not_exist" }
        );
#else
        SKIP("This test is incompatible with Windows");
#endif
    }

    SECTION("Test root protection") {
        // Sanity check: make sure dummy mode is set
        stc::testutil::TestDirectory verifyDummyMode {
            "/whatever/path",
            false,
            true
        };
        REQUIRE(verifyDummyMode.dummyMode);

        // Sanity check: make sure dummy mode works as intended
        REQUIRE_NOTHROW([&]() {
            REQUIRE(std::filesystem::exists(existing));
            stc::testutil::TestDirectory d{existing, false, true};
        }());
        REQUIRE(std::filesystem::exists(existing));

        // Sanity check: underlying helper function needs to throw
        REQUIRE_THROWS(verifyDummyMode.checkCompatiblePath("/"));

        REQUIRE_THROWS(
            stc::testutil::TestDirectory {
                "/",
                false,
                true
            }
        );
    }
}
