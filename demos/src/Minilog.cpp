#include <stc/minilog.hpp>

int main() {
    // minilog::config().level = minilog::Level::ERROR;
    // Minilog as a subsystem of stc has been deprecated and should not be used. Instead, use the standalone minilog
    // library: https://codeberg.org/LunarWatcher/minilog
    // Minilog in stc will be removed by the end of 2026 probably
    minilog::debug("This is info");
    minilog::info("This is info");
    minilog::warn("This is info");
    minilog::error("This is info");
    minilog::critical("This is info");
}
