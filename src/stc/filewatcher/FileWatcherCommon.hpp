#pragma once

#include <chrono>
#include <filesystem>

namespace stc::watcher {

enum class FileWatcherEventKind {
    FileAdded,
    FileEdited,
    FileRemoved,
    DirectoryAdded,
    DirectoryRemoved,
};

struct FileWatcherConfig {
    /**
     * The minimum delay between event dispatch
     */
    std::chrono::milliseconds minimumDelay{0};

    /**
     * Whether or not to return from the block method if all watched files are gone.
     */
    bool stopOnAllUnwatched = true;
};

struct FileWatcherEvent {
    FileWatcherEventKind kind;
    std::filesystem::path eventPath;

    std::chrono::steady_clock::time_point ts;
};

}
