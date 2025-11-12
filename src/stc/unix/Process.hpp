#pragma once

#ifdef _WIN32
#error "Process.hpp is currently UNIX only, and does not support Windows. Feel free to open a PR to change this"
#endif

/** \file 
 *
 * This file contains a UNIX-only command interface. 
 * It currently requires a UNIX environment. Windows support may be added in the future, but this will require third
 * party help, as I do not hate myself enough to get that deep into the Windows API, and if we're being realistic, no
 * one writes command line dev tools this involved for that shithole of an OS without a UNIX environment being involved.
 *
 * Note that the API used here is not finalised, and is subject to change, including total breakage.
 */

#include <array>
#include <cstdlib>
#include <fcntl.h>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <pty.h>
#include <sstream>
#include <string>
#include <sys/poll.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
#include <variant>
#include <vector>

// TODO: this cannot be "unix", or the build inexplicably dies ("unexpected { before numeric constant")
// It's probably a macro
namespace stc::Unix {

struct LowLevelWrapper {
    ssize_t writeToFd(const std::string& data, int fd) {
        ssize_t bytes = write(fd, data.data(), data.size());
        if (bytes > 0) {
            return bytes;
        }
        return 0;
    }

    ssize_t readFromFd(std::stringstream& out, int fd) {
        std::array<char, 4096> buff;
        ssize_t sum = 0;

        nfds_t nfds = 1;
        pollfd pdfs = {
            .fd = fd,
            .events = POLLIN,
            .revents = 0
        };

        while (poll(&pdfs, nfds, 0)) {
            ssize_t bytes = read(
                fd,
                buff.data(),
                buff.size()
            );
            if (bytes > 0) {
                out << std::string_view {
                    buff.begin(), buff.begin() + bytes 
                };
                sum += bytes;
            }
        }
        return sum;
    }
};

struct Pipe : public LowLevelWrapper {
    std::array<int, 2> fds;
    Pipe() {
        if (pipe(fds.data()) != 0) {
            throw std::runtime_error("Failed to open pipe");
        }
    }

    ~Pipe() {
        die();
    }

    void die() {
        closeRead();
        closeWrite();
    }
    void closeRead() {
        if (fds[0] != -1) {
            close(fds[0]);
        }
        fds[0] = -1;
    }
    void closeWrite() {
        if (fds[1] != -1) {
            close(fds[1]);
        }

        fds[1] = -1;
    }

    int readFd() {
        return fds[0];
    }
    int writeFd() {
        return fds[1];
    }

    ssize_t readData(std::stringstream& out) {
        return readFromFd(out, readFd());
    }
};

struct PTY : public LowLevelWrapper {
    int master, slave;

    PTY() {
        // TODO: figure out if it makes sense to:
        // 1. store the name
        // 2. Allow customising whatever the last two parameters are
        if (openpty(&master, &slave, nullptr, nullptr, nullptr) == -1) {
            throw std::runtime_error("Failed to open PTY");
        }
    }
    ~PTY() {
        die();
    }

    /**
     * Can be used to close the PTY. This should usually not be done unless you have an explicit reason to
     * force-terminate it. This is primarily intended to be called internally by the child process, as destructors are
     * not invoked with exec.
     *
     * \see https://stackoverflow.com/a/17135211
     */
    void die() {
        closeMasterChannel();
        closeSlaveChannel();
    }

    void closeMasterChannel() {
        if (master >= 0) {
            close(master);
        }
        master = -1;
    }
    void closeSlaveChannel() {
        if (slave >= 0) {
            close(slave);
        }
        slave = -1;
    }
    /**
     * Note: this function only writes to the master channel, as it's assumed the result of fork() never returns control
     * back to the code executed by stc. DO NOT USE THIS STRUCT IF YOU BREAK THIS ASSUMPTION! Shit will get weird.
     * Reimplement it from scratch, or open a PR to make this API better.
     */
    ssize_t writeToStdin(const std::string& data) {
        return writeToFd(data, master);
    }

    ssize_t readData(std::stringstream& out) {
        return readFromFd(out, master);
    }
};

struct Pipes {
    std::shared_ptr<Pipe> stdoutPipe = nullptr;
    std::shared_ptr<Pipe> stderrPipe = nullptr;
    std::shared_ptr<Pipe> stdinPipe = nullptr;

    void die() {
        if (stdoutPipe) stdoutPipe->die();
        if (stderrPipe) stderrPipe->die();
        if (stdinPipe) stdinPipe->die();
    }

    ssize_t writeToStdin(const std::string& data) {
        if (stdinPipe == nullptr) {
            throw std::runtime_error("Must open stdin to write to stdin");
        }
        return stdinPipe->writeToFd(data, stdinPipe->writeFd());
    }
};

class Process {
protected:
    std::optional<decltype(fork())> pid = std::nullopt;

    std::optional<
        std::variant<Pipes, std::shared_ptr<PTY>>
    > interface;
    std::stringstream stdoutBuff, stderrBuff;
    std::mutex lock;

    std::thread inputCollector;
    int statusCode = -1;
    bool running = true;

    void doSpawnCommand(
        const std::vector<std::string>& command,
        const std::function<void()>& readImpl,
        const std::function<void()>& prepDuping
    ) {
        std::vector<const char*> convertedCommand;

        convertedCommand.reserve(command.size() + 1);
        for (auto& str : command) {
            convertedCommand.push_back(str.c_str());
        }
        convertedCommand.push_back(nullptr);

        pid = fork();
        if (pid < 0) {
            throw std::runtime_error("Failed to fork");
        } else if (pid == 0) {
            // Child process
            if (prepDuping != nullptr) {
                prepDuping();
            }

            // Close handles if they're opened
            if (interface) {
                std::visit([](auto& resolved) {
                    using T = std::decay_t<decltype(resolved)>;
                    if constexpr (std::is_same_v<T, std::shared_ptr<PTY>>) {
                        resolved->die();
                    } else {
                        resolved.die();
                    }
                }, interface.value());
            }

            // TODO: allow passing environment variables
            execv(convertedCommand.at(0), (char**) convertedCommand.data());
        } else {
            // Parent process
            if (readImpl != nullptr) {
                this->inputCollector = std::thread(
                    std::bind(&Process::run, this, readImpl)
                );
            }
        }
    }

    void run(const std::function<void()>& readImpl) {
        while (true) {
            readImpl();

            int statusResult;
            if (waitpid(*pid, &statusResult, WNOHANG)) {
                statusCode = WEXITSTATUS(statusResult);
                break;
            }
        }
    }
public:
    Process(const std::vector<std::string>& command) {
        doSpawnCommand(command, nullptr, nullptr);
    }
    Process(const std::vector<std::string>& command, const Pipes& pipes) {
        interface = pipes;

        doSpawnCommand(command, [this]() {
            auto& pipes = std::get<Pipes>(this->interface.value());
            if (pipes.stdoutPipe != nullptr) {
                std::lock_guard l(lock);
                pipes.stdoutPipe->readData(
                    stdoutBuff
                );
            }
            if (pipes.stderrPipe != nullptr) {
                std::lock_guard l(lock);
                pipes.stderrPipe->readData(
                    stderrBuff
                );
            }
        }, [&]() {
            if (pipes.stdinPipe != nullptr) {
                dup2(pipes.stdinPipe->readFd(), STDIN_FILENO);
            }
            if (pipes.stdoutPipe != nullptr) {
                dup2(pipes.stdoutPipe->writeFd(), STDOUT_FILENO);
            }
            if (pipes.stderrPipe != nullptr) {
                dup2(pipes.stderrPipe->writeFd(), STDERR_FILENO);
            }
            std::get<Pipes>(*interface).die();
        });
    }
    Process(const std::vector<std::string>& command, const std::shared_ptr<PTY>& pty) {
        if (pty == nullptr) {
            throw std::runtime_error(
                "pty cannot be null. If you don't want to attach anything, use the non-pipe/non-PTY constructor instead"
            );
        }
        interface = pty;
        doSpawnCommand(command, [this]() {
            auto pty = std::get<std::shared_ptr<PTY>>(this->interface.value());

            {
                std::lock_guard l(lock);
                pty->readData(
                    stdoutBuff
                );
            }

        }, [&]() {
            dup2(pty->slave, STDIN_FILENO);
            dup2(pty->slave, STDOUT_FILENO);
            dup2(pty->slave, STDERR_FILENO);
            std::get<std::shared_ptr<PTY>>(*interface)->die();
        });

        
    }

    virtual ~Process() = default;

    /**
     * Returns the stdout output. 
     *
     * Note that:
     * * If stdout == stderr, stdout may actually be contained in getStderrBuffer instead.
     * * If using a PTY, this also includes some input, as defined by weird PTY internal bullshit that I don't
     *   understand
     * * If not using any pipes, or not capturing stdout, this will always be empty
     *
     * \param reset Whether or not to reset the buffer. This can be useful if you want to progressively get output.
     */
    std::string getStdoutBuffer(bool reset = false) {
        std::lock_guard g(lock);
        auto str = stdoutBuff.str();
        if (reset) {
            stdoutBuff = {};
        }
        return str;
    }

    /**
     * Returns the stderr output. 
     *
     * Note that:
     * * If stdout == stderr, stderr may actually be contained in getStderrBuffer instead.
     * * If using a PTY, this method returns nothing
     * * If not using any pipes, or not capturing stderr, this will always be empty
     *
     * \param reset Whether or not to reset the buffer. This can be useful if you want to progressively get output.
     */
    std::string getStderrBuffer(bool reset = false) {
        std::lock_guard g(lock);
        auto str = stderrBuff.str();
        if (reset) {
            stderrBuff = {};
        }
        return str;
    }

    /**
     * Used to write to stdin.
     *
     * \returns the number of bytes written. This does not have to be used for anything, as it's mainly intended for use
     *          in tests of stc, but it's there if you need it for something.
     * \throws std::runtime_error if not using pipe or PTY mode, or if using pipe mode and the stdin pipe is null
     */
    ssize_t writeToStdin(const std::string& data) {
        if (interface) {
            return std::visit([&](auto& resolved) -> ssize_t {
                using T = std::decay_t<decltype(resolved)>;
                if constexpr (std::is_same_v<T, std::shared_ptr<PTY>>) {
                    return resolved->writeToStdin(data);
                } else {
                    return resolved.writeToStdin(data);
                }
            }, interface.value());
        } else {
            throw std::runtime_error("Must use pty or pipe mode to write to stdin");
        }
    }

    /**
     * Waits for the process to exit. This is a blocking function call.
     *
     * \returns the exit code for the process.
     */
    int block() {
        if (this->interface) {
            if (inputCollector.joinable()) {
                inputCollector.join();
            }
            return statusCode;
        } else {
            int statusResult;
            waitpid(*pid, &statusResult, 0);
            statusCode = WEXITSTATUS(statusResult);
            return statusCode;
        }
    }

    /**
     * Sends sigkill to the process
     */
    void stop() {
        kill(*pid, SIGKILL);
    }

    /**
     * Sends sigterm to the process.
     */
    void terminate() {
        kill(*pid, SIGTERM);
    }

    void closeStdin() {
        if (!this->interface.has_value()) {
            throw std::runtime_error("Must use pipe or pty mode to use this function");
        }
    }

    std::optional<int> getExitCode() {
        if (statusCode != -1) {
            return statusCode;
        }
        return std::nullopt;
    }

};

}
