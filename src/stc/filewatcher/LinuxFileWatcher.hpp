#pragma once

#include "FileWatcherCommon.hpp"
#include <array>
#include <cerrno>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <functional>
#include <mutex>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include <poll.h>
#include <sys/inotify.h>
#include <sys/eventfd.h>
#include <unistd.h>

namespace stc::linux_impl {

class LinuxFileWatcher {
private:
    int fd = -1;
    int eventFd = -1;
    watcher::FileWatcherConfig conf;
    std::function<bool(const std::filesystem::path)> includePathPredicate;

    std::unordered_map<int, std::filesystem::path> watchDescriptors;

    bool running = true;
    std::mutex m;
    void watch(const std::filesystem::path& path) {
        if (
            includePathPredicate(path) == false
            || !std::filesystem::exists(path)
        ) {
            return;
        }

        auto wd = inotify_add_watch(
            fd,
            path.c_str(),
            IN_CLOSE_WRITE | IN_CREATE
            | IN_DELETE // used for files
            | IN_DELETE_SELF // used for folders for cleanup
        );

        if (wd < 0) {
            [[unlikely]]
            return;
        } else if (this->watchDescriptors.contains(wd)) {
            [[unlikely]]
            throw std::runtime_error("Attempted to reregister watch descriptor!");
        }
        this->watchDescriptors[wd] = path;
    }
public:
    LinuxFileWatcher(
        const std::vector<std::filesystem::path>& initialWatches,
        const watcher::FileWatcherConfig& conf = {},
        const std::function<bool(const std::filesystem::path)>& includePathPredicate = [](const auto&) { return true; }
    ): conf(conf), includePathPredicate(includePathPredicate) {
        fd = inotify_init1(IN_NONBLOCK);
        if (fd == 0) {
            perror("inotify_init1");
            throw std::runtime_error("Failed to initialize inotify");
        }
        
        eventFd = eventfd(0, EFD_NONBLOCK);
        if (fd == 0) {
            perror("eventfd");
            throw std::runtime_error("Failed to initialize eventfd");
        }


        for (auto& path : initialWatches) {
            auto canonPath = std::filesystem::weakly_canonical(path);
            if (includePathPredicate(canonPath) == false) {
                continue;
            }
            watch(canonPath);

            if (std::filesystem::is_directory(canonPath)) {
                auto it = std::filesystem::recursive_directory_iterator(canonPath);
                auto end = std::filesystem::recursive_directory_iterator();
                do {
                    if (it == end) {
                        break;
                    }
                    auto f = *it;
                    auto abs = std::filesystem::weakly_canonical(f);
                    if (includePathPredicate(abs) == false) {
                        // don't iterate further down this tree
                        it.disable_recursion_pending();
                        continue;
                    }
                    if (std::filesystem::is_directory(abs)) {
                        watch(abs);
                    }
                } while (++it != end);
            }
        }
    }

    ~LinuxFileWatcher() {
        if (fd >= 0) {
            close(fd);
            fd = -1;
        }
        if (eventFd >= 0) {
            close(eventFd);
            eventFd = -1;
        }
    }

    void block(
        const std::function<bool(watcher::FileWatcherEventKind event, const std::filesystem::path& path)>& onEvent
    ) {
        std::array<pollfd, 2> fds = {
            pollfd {
                .fd = this->fd,
                .events = POLLIN,
                .revents = 0,
            },
            pollfd {
                .fd = this->eventFd,
                .events = POLLIN,
                .revents = 0,
            },
        };
        std::array<char, 4096> buff;
        std::chrono::steady_clock::time_point ts = std::chrono::steady_clock::now();
        while (
            running && (
                !conf.stopOnAllUnwatched
                || !watchDescriptors.empty()
            )
        ) {
            ssize_t events = poll(
                fds.data(),
                fds.size(),
                -1
            );

            if (events == -1) {
                if (errno == EINTR) {
                    continue;
                }
                perror("poll");
                throw std::runtime_error("error while polling for events");
            }

            std::lock_guard l(m);

            if (events > 0) {
                // TODO: This is not optimal, but not sure what else to do. The events do not carry timestamp
                // information, so this appears to be necessary to allow timeout logic
                std::vector<watcher::FileWatcherEvent> events;
                if ((fds[1].revents & POLLIN) != 0) {
                    break;
                }

                if ((fds[0].revents & POLLIN) != 0) {
                    // inotify ready
                    while (true) { // Read loop
                        ssize_t size = read( // NOLINT(clang-analyzer-unix.BlockInCriticalSection)
                            fd,
                            buff.data(),
                            buff.size()
                        );
                        if (size < 0 && errno != EAGAIN) {
                            perror("read");
                            throw std::runtime_error("Failed to read inotify events");
                        } else if (size < 0 && errno == EAGAIN) {
                            // Events exhausted
                            break;
                        }

                        const inotify_event* event;
                        for (
                            char *ptr = buff.data();
                            ptr < buff.data() + size;
                            ptr += sizeof(struct inotify_event) + event->len
                        ) {
                            event = (const inotify_event *) ptr;

                            // Guard against edge-case errors where folders are created and deleted rapidly, for example
                            // during tests
                            auto parentIt = this->watchDescriptors.find(event->wd);
                            if (parentIt == this->watchDescriptors.end()) {
                                continue;
                            }
                            std::filesystem::path eventPath = parentIt->second;
                            if (event->len) {
                                // We can't really use the length. Tried to do this, and it results in weird null
                                // handling and what appears to be UB. The docs say that the string itself is
                                // null-terminated, but that it may end with additional \0 for padding purposes, which
                                // we don't care about
                                eventPath = eventPath / std::string(event->name);
                            }

                            watcher::FileWatcherEventKind kind;
                            if (
                                (event->mask & IN_DELETE_SELF) != 0
                            ) {
                                // parentIt in DELETE_SELF is the actual watch itself
                                // Note to self: IN_DELETE_SELF is called without IN_ISDIR
                                this->watchDescriptors.erase(parentIt);
                                // Since we separate folder and directory deletion in the type enum, we currently don't
                                // have a way to differentiate between folder deletion and file deletion. The deletion
                                // of this folder should usually be accompanied by a IN_DELETE in some parent folder,
                                // though this does fail if a root folder (meaning a folder that is directly watched
                                // rather than one that's watched because it's in the file tree) is deleted. I am fine
                                // with ignoring this edge-case for now.
                                continue;
                            } else if ((event->mask & IN_ISDIR) != 0) {
                                // Directory
                                if ((event->mask & IN_CREATE) != 0) {
                                    kind = watcher::FileWatcherEventKind::DirectoryAdded;

                                    if (std::filesystem::exists(eventPath)) {
                                        watch(eventPath);
                                    }
                                } else if ((event->mask & IN_DELETE) != 0) {
                                    kind = watcher::FileWatcherEventKind::DirectoryRemoved;
                                } else {
                                    continue;
                                }
                            } else {
                                // File
                                if ((event->mask & IN_CLOSE_WRITE) != 0) {
                                    kind = watcher::FileWatcherEventKind::FileEdited;
                                } else if ((event->mask & IN_DELETE) != 0) {
                                    kind = watcher::FileWatcherEventKind::FileRemoved;
                                } else if ((event->mask & IN_CREATE) != 0) {
                                    kind = watcher::FileWatcherEventKind::FileAdded;
                                } else {
                                    continue;
                                }
                            }

                            events.push_back({
                                kind,
                                eventPath,
                                std::chrono::steady_clock::now(),
                            });
                        }
                    }
                }

                for (auto& event : events) {
                    if (event.ts < ts + conf.minimumDelay) {
                        continue;
                    }
                    ts = event.ts;
                    if (!onEvent(event.kind, event.eventPath)) {
                        goto done;
                    }
                }

                // Reset the time so the timer also applies between loops if `onEvent` takes a while:tm:
                if (events.size()) {
                    ts = std::chrono::steady_clock::now();
                }
            }
        }

    done:; // load-bearing semicolon until c++23
    }

    void watchPath(const std::filesystem::path& path) {
        std::lock_guard l(m);
        watch(path);
    }

    void stop() {
        running = false;

        uint64_t val = 1; // Note to self; this needs to be the same size as the eventfd
        eventfd_write(eventFd, val);
    }
};

}
