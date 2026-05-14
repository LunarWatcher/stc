#pragma once

#include "stc/Environment.hpp"

namespace stc::testutil {

/**
 * Utility class for making sure an environment variable added to environ is freed properly.
 */
struct FreeEnv {
    std::string name;

    FreeEnv(
        const std::string& name,
        const std::string& initValue = ""
    ) : name(name) {
        if (!initValue.empty()) {
            stc::setEnv(
                name.c_str(), initValue.c_str()
            );
        }
    }

    ~FreeEnv() {
        if (!name.empty()) {
            stc::setEnv(name.c_str(), nullptr);
        }
    }
};

}
