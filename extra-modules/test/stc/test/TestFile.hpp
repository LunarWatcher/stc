#pragma once

#include <filesystem>
#include <fstream>

namespace stc::testutil {

/**
 * Test utility for RAII ownership of a file.
 *
 * The file, if it exists at the time of initialization, is deleted. It's also deleted at the end of the test when this
 * class is destroyed.
 * \note This class is incompatible with symlinks.
 */
struct TestFile {
    std::filesystem::path file;

    [[nodiscard("Discarding this object will immediately delete the file")]]
    TestFile(const std::filesystem::path& f, bool create = false) : file(f) {
        if (std::filesystem::exists(file) && !std::filesystem::is_regular_file(f)) {
            throw std::runtime_error(
                "The file you supplied to TestFile exists, and is not a regular file. "
                "This utility is only compatible with standard files. "
                "If you were trying to take ownership of a directory, use TestDirectory instead."
            );
        }
        deleteFile();

        if (create) {
            std::ofstream of(f);
            if (!of) {
                throw std::runtime_error(
                    "Failed to open test file. Do you have dir permissions?"
                );
            }
        }
    }
    ~TestFile() {
        deleteFile();
    }
    void deleteFile() {
        if (std::filesystem::exists(file)) {
            std::filesystem::remove(file);
        }
    }
};

}
