#pragma once

#include <filesystem>

namespace stc::testutil {

/**
 * Test utility for RAII ownership of a folder.
 *
 * The folder, if it exists at the time of initialization, is deleted. It's also deleted at the end of the test when
 * this class is destroyed.
 * \note This class is incompatible with symlinks. 
 * \note There are some protections in place to avoid common mistakes that decay to deleting standard UNIX folders, or
 *       the root folder. You cannot take ownership of anything under /usr, /bin, /share, /dev, /etc, /proc, or
 *       /boot. This is just a list of the most risky folders, though there are others that may be dangerous. Make sure
 *       to validate any generated paths you create BEFORE passing them to this class even with these protections in
 *       place.
 */
struct TestDirectory {
    std::filesystem::path folder;
    bool dummyMode = false;

    [[nodiscard("Discarding this object will immediately delete the folder")]]
    TestDirectory(
        const std::filesystem::path& p,
        bool create = false
    ) : folder(p) {
        checkCompatiblePath(p);
        deleteFolder();

        if (create) {
            std::filesystem::create_directories(p);
        }
    }

    [[nodiscard("Discarding this object will immediately delete the folder")]]
    TestDirectory(
        const std::filesystem::path& p,
        bool create,
        bool dummyMode
    ) : folder(p), dummyMode(dummyMode) {
        checkCompatiblePath(p);
        deleteFolder();

        if (create) {
            std::filesystem::create_directories(p);
        }
    }

    ~TestDirectory() {
        deleteFolder();
    }

    void checkCompatiblePath(const std::filesystem::path& p) {
        if (std::filesystem::exists(p) && (
                !std::filesystem::is_directory(p)
                || std::filesystem::is_symlink(p)
        )) {
            throw std::runtime_error(
                "The path you supplied to TestDirectory exists, but is not a directory or is a symlink. "
                "This utility is only compatible with standard directories. "
                "If you were trying to take ownership of a file, use TestFile instead."
            );
        } else if (
            p.string().starts_with("/usr")
            || p.string().starts_with("/bin")
            || p.string().starts_with("/share")
            || p.string().starts_with("/dev")
            || p.string().starts_with("/etc")
            || p.string().starts_with("/proc")
            || p.string().starts_with("/boot")
            || std::filesystem::weakly_canonical(p) == std::filesystem::canonical("/")
        ) {
            throw std::runtime_error(
                "You fucked up"
            );
        }
    }


    void deleteFolder() {
        if (dummyMode) {
            return;
        }
        if (std::filesystem::exists(this->folder)) {
            std::filesystem::remove_all(this->folder);
        }
    }
};

}
