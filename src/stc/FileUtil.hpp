/** \file
 *
 * This file contains functions with filesystem utils, i.e. extensions on top of <filesystem>.
 */
#pragma once

#include <filesystem>
#include <optional>
#include <vector>

namespace stc::FileUtil {

/**
 * Utility for looking up a filename or relative path relative to any of a number of search paths.
 *
 * \param searchPath    A list of directories to search in
 * \param filename      The filename or relative path to find
 *
 * \returns             A path to a file that exists, or an std::nullopt if `filename` doesn't exist in the search path.
 */
inline std::optional<std::filesystem::path> findFile(
    const std::vector<std::filesystem::path>& searchPath,
    const std::filesystem::path& filename
) {
    for (auto& path : searchPath) {
        if (std::filesystem::is_regular_file(path / filename)) {
            return path / filename;
        }
    }
    return std::nullopt;
}

}
