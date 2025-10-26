/** \file */
#pragma once

#include <chrono>
#include <functional>
#include <memory>
#include <thread>
#include <filesystem>

#ifndef _WIN32
#include <fcntl.h>
#include <cstdio>
#include <unistd.h>
#include <sys/file.h>
#else

#define NOMINMAX
#include <windows.h>
#include <handleapi.h>
#endif

namespace stc {

/**
 * Single-use file lock. It should NOT be used on the file meant to be protected, but a second
 * file exclusively meant to indicate whether some other file or resource is currently locked.
 * This class deletes the lockfile after use as well, meaning if you use it on a file you want
 * to use normally, it will be deleted.
 *
 * This won't actually prevent access to the relevant resources, but this strategy allows for
 * programs to respect each other.
 *
 * This class only does locking; any type of error handling has to be done by the programmer
 * using this class.
 *
 * There's two ways to lock a file; either, the constructor can be used, or a helper method
 * within the class can be used. The helper method exists to make a minor framework around
 * a repetitive check with some function callback; see its documentation for the exact callback
 * details.
 * The callback determines whether or not to continue, as well as allowing the programmer
 * to specify a wait handler. Examples include updating a popup, printing a timer
 * in the terminal, printing or rendering a progress bar, etc.
 */
class FileLock {
private:
#ifndef _WIN32
    int fd;
#else
    HANDLE fd;
#endif

    const std::filesystem::path lockPath;

    bool locked = false;

public:
    enum class Errors {
        OPEN_ERROR = 42,
        LOCK_ERROR = 43
    };

    /**
     * This construtor creates the file lock.
     *
     * Throws a value from Errors that can be used for error checking.
     *
     * @param lockPath          The path to the lockfile
     * @param lockNonblocking   Whether or not to use LOCK_NB for Linux, or an equivalent for other
     *                          operating systems if one exists. Default true.
     *                          No effect on Windows due to implementation mechanics
     */
    FileLock(const std::filesystem::path& lockPath, bool lockNonblocking = true) : lockPath(lockPath) {
#ifndef _WIN32
        fd = open(lockPath.c_str(), O_RDWR | O_CREAT, 0666);
        if (fd < 0) {
            throw Errors::OPEN_ERROR;
        }


        // TODO: better error handling of flock()s return value.
        if ((locked = flock(fd, LOCK_EX | (lockNonblocking == true ? LOCK_NB : 0)) == 0) == false) {
            close(fd);
            fd = -1;
            // While removing the file here would look relevant, it isn't.
            // If the file is locked, it obviously shouldn't be removed.
        }

        if (!locked) {
            throw Errors::LOCK_ERROR;
        }
#else
        auto _cppStr = lockPath.string();
        auto str = _cppStr.c_str();
        fd = CreateFileA(
            str,
            GENERIC_WRITE,
            0,
            NULL,
            CREATE_ALWAYS,
            FILE_FLAG_DELETE_ON_CLOSE,
            NULL
        );

        if (fd == INVALID_HANDLE_VALUE) {
            DWORD err = GetLastError();
            if (err == 32) {
                throw Errors::LOCK_ERROR;
            }
            throw Errors::OPEN_ERROR;
        }

        locked = true;
#endif

    }

    ~FileLock() {
        unlock();
    }

    // I might add these in the future with some fancy move stuff
    // or whatever to allow for moving locks around.
    // For now, it sounds useless.
    FileLock(const FileLock&) = delete;
    FileLock& operator=(const FileLock&) = delete;

    /**
     * Returns whether or not this process has a lock on the file.
     * This does NOT indicate whether or not the file is locked by someone.
     * If the constructor throws because it fails to create the lock
     */
    bool hasLock() { return locked; }

    /**
     * Clears the lock.
     *
     * Note that while this can be called an unlimited number of times, it's only useful once. The lock
     * cannot be reacquired when it has been unlocked without creating a new lock for reasons
     * that currently are future Windows compatibility because constness.
     */
    void unlock() {
        if (!locked) return;

        locked = false;
#ifndef _WIN32
        if (fd >= 0) {
            flock(fd, LOCK_UN);
            close(fd);
        }
        // TODO: this results in a race condition. Fix
        if (std::filesystem::exists(lockPath)) {
            std::filesystem::remove(lockPath);
        }
#else
        if (fd != INVALID_HANDLE_VALUE && CloseHandle(fd)) {
            auto _cppStr = lockPath.string();
            auto str = _cppStr.c_str();

            CloseHandle(fd);
        }
#endif
    }

    /**
     * Utility method for lock acquiring; instead of an immediate fast fail or blocking
     * while waiting for an unlock, this method uses failure to call a control method that
     * allows for more responsive user interfaces.
     *
     * The control method can also completely abort the locking. For instance, if you only have
     * so much time to spend on waiting for the lock, your control function can check whether
     * some amount of time has passed before returning false.
     *
     * If there's a UI-centric application, a variable can be used to connect a "cancel" button
     * to the control function, aborting the process if the user has had enough.
     *
     * Automated systems can attempt to use a timer to determine whether to blatantly ignore the
     * lock or not as well. For instance, if it's guaranteed that certain lock applications won't
     * exceed a certain amount of time locked.
     *
     * \param path              The path to the lockfile
     * \param control           The control function; decides whether or not to continue,
     *                          and it's highly recommended it's used to update the user interface.
     * \param sleepSeconds      How long to sleep between attempts; the default is one second.
     *                          Note that due to fundamental inaccuracies in computers, any time
     *                          inserted here isn't guaranteed to be accurate down to the max
     *                          double resolution. Control invokations are not good
     *                          for an exact indication of elapsed time.
     *
     * \returns nullptr         if the lock wasn't acquired after control returns false, or if an OPEN_ERROR is met.
     * \returns lock            if the lock was acquired
     */
    // Fuck you MSVC
#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201907L) || __cplusplus >= 201907L)
    [[nodiscard("Ignoring the return value of dynamicAcquireLock results in the immediate unlock of the returned lock")]]
#else
    [[nodiscard]]
#endif
    static std::shared_ptr<FileLock> dynamicAcquireLock(
        const std::filesystem::path& path,
        std::function<bool()> control,
        unsigned int sleepSeconds = 1
    ) {
        while (control()) {
            try {
                std::shared_ptr<FileLock> lock = std::make_shared<FileLock>(path, true);
                if (lock->hasLock()) {
                    return lock;
                }
            } catch (const Errors& e) {
                if (e == Errors::OPEN_ERROR) {
                    return nullptr;
                }
            }
            if (sleepSeconds != 0) {
                std::this_thread::sleep_for(std::chrono::seconds(sleepSeconds));
            }
        }
        return nullptr;
    }
};

}
