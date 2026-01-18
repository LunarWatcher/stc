#include <stc/minilog.hpp>

int main() {
    // minilog::config().level = minilog::Level::ERROR;
    minilog::debug("This is info");
    minilog::info("This is info");
    minilog::warn("This is info");
    minilog::error("This is info");
    minilog::critical("This is info");
}
