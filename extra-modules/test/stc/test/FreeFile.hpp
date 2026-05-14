#pragma once

#include <filesystem>
#include <fstream>

namespace stc::testutil {

struct FreeFile {
    std::filesystem::path file;

    FreeFile(const std::filesystem::path& f, bool create = false) : file(f) {
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
    ~FreeFile() {
        deleteFile();
    }
    void deleteFile() {
        if (std::filesystem::exists(file)) {
            std::filesystem::remove(file);
        }
    }
};

}
