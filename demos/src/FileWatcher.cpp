#include "stc/filewatcher/FileWatcherCommon.hpp"
#include <stc/filewatcher/FileWatcher.hpp>

#include <filesystem>
#include <unordered_map>
#include <iostream>
#include <thread>

int main() {
    std::filesystem::create_directories("fw-sandbox");
    stc::FileWatcher watcher {
        { "fw-sandbox" },
    };

    std::unordered_map<stc::watcher::FileWatcherEventKind, std::string> organicReadableKinds {
        {stc::watcher::FileWatcherEventKind::FileAdded, "FileAdded"},
        {stc::watcher::FileWatcherEventKind::FileEdited, "FileEdited"},
        {stc::watcher::FileWatcherEventKind::FileRemoved, "FileRemoved"},
        {stc::watcher::FileWatcherEventKind::DirectoryAdded, "DirectoryAdded"},
        {stc::watcher::FileWatcherEventKind::DirectoryRemoved, "DirectoryRemoved"},
    };
    
    std::cout << "Watching ./fw-sandbox for changes (for 60 seconds)." << std::endl;
    std::thread t([&]() {
        std::this_thread::sleep_for(std::chrono::seconds(60));
        watcher.stop();
    });
    watcher.block(
        [&](stc::watcher::FileWatcherEventKind event, const std::filesystem::path& path) -> bool {
            std::cout << path << " has event " << organicReadableKinds[event] << std::endl;
            return true;
        }
    );
    t.join();
}
