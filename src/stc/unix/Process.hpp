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
#include <atomic>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <format>
#include <functional>
#include <iostream>
#include <memory>
#include <map>
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

        // we need a small timeout here to prevent race conditions
        while (poll(&pdfs, nfds, 10)) {
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

/**
 * Shorthand for creating a new pipe. Saves a few characters, does nothing special aside calling std::make_shared
 */
inline std::shared_ptr<Pipe> createPipe() {
    return std::make_shared<Pipe>();
}

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

    /**
     * Utility function for creating a Pipes instance where stdout and stderr are both captured. Stdin is controlled by
     * withStdin.
     */
    static Pipes separate(bool withStdin = true) {
        return Pipes {
            createPipe(),
            createPipe(),
            withStdin ? createPipe() : nullptr
        };
    }
    /**
     * Utility function for creating a Pipes instance where stdout and stderr are linked. Stdin is controlled by
     * withStdin
     */
    static Pipes shared(bool withStdin = true) {
        auto outPipe = createPipe();
        return Pipes {
            outPipe,
            outPipe,
            withStdin ? createPipe() : nullptr
        };

    }
};


/**
 * Shorthand for creating a new PTY. Saves a few characters, does nothing special aside calling std::make_shared
 */
inline std::shared_ptr<PTY> createPTY() {
    return std::make_shared<PTY>();
}

struct Environment {
    std::map<std::string, std::string> env;
    /**
     * Whether or not to start with `os.environ`. If false, nothing included in `os.environ` is forwarded.
     */
    bool extendEnviron = true;
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
    std::atomic<int> statusCode = -1;
    std::atomic<std::optional<bool>> exitedNormally;
    bool running = true;

    char* const* createEnviron(
        const std::optional<Environment>& env
    ) {
        if (env == std::nullopt) {
            return environ;
        }

        size_t size = 0;
        for (char **env = environ; *env != nullptr; env++) {
            ++size;
        }

        // This is disgusting, but it beats fucking around with the real raw types. This will technically leak a vector,
        // but it does not matter because it's disappeared once exec is called:
        // https://stackoverflow.com/a/3617385
        // Would prefer to do this better, but I just don't want to
        std::vector<char*>* data = new std::vector<char*>;
        if (env->extendEnviron && size > 0) {
            data->assign(
                environ, environ + size
            );
        }

        data->reserve(
            // Existing data or 0
            data->size()
            // nullptr
            + 1
            // Extra envs
            + env->env.size()
        );
        for (const auto& [k, v] : env->env) {
            if (k.find('=') != std::string::npos) {
                std::cerr << "Env variable key cannot contain =" << std::endl;
                exit(69);
            }

            if (env->extendEnviron) {
                // If we're extending environ, keys here can conflict with environ. They won't conflict internally,
                // because env is a non-multimap, and we enforce keys not containing '='
                data->erase(
                    std::remove_if(
                        data->begin(),
                        data->end(), 
                        [&](const auto& v) -> bool {
                            return strncmp(v, k.data(), k.size()) == 0;
                        }
                    ), data->end()
                );
            }

            std::string combined = std::format(
                "{}={}", k, v
            );
            auto* newStr = strdup(combined.c_str());
            if (newStr == nullptr) {
                std::cerr << "Failed to copy string to env" << std::endl;
                exit(69);
            }
            data->push_back(newStr);
        }
        data->push_back(nullptr);

        return data->data();

    }

    void doSpawnCommand(
        const std::vector<std::string>& command,
        const std::function<void()>& readImpl,
        const std::function<void()>& prepDuping,
        const std::optional<Environment>& env
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

            execve(
                convertedCommand.at(0),
                (char**) convertedCommand.data(),
                createEnviron(env)
            );
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

            int wstatus;
            if (waitpid(*pid, &wstatus, WNOHANG)) {
                if (WIFEXITED(wstatus)) {
                    statusCode = WEXITSTATUS(wstatus);
                    exitedNormally = true;
                } else if (WIFSIGNALED(wstatus)) {
                    statusCode = WTERMSIG(wstatus);
                    exitedNormally = false;
                } else if (WIFSTOPPED(wstatus)) {
                    statusCode = WSTOPSIG(wstatus);
                    exitedNormally = false;
                } else {
                    std::cerr
                        << "WARNING: stc::Unix::Process got an unknown exit type. Cannot set exit code" 
                        << std::endl;
                    exitedNormally = false;
                }
                break;
            }
        }
    }
public:
    Process(
        const std::vector<std::string>& command,
        const std::optional<Environment>& env = std::nullopt
    ) {
        doSpawnCommand(command, nullptr, nullptr, env);
    }
    Process(
        const std::vector<std::string>& command,
        const Pipes& pipes,
        const std::optional<Environment>& env = std::nullopt
    ) {
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
        }, env);
    }
    Process(
        const std::vector<std::string>& command,
        const std::shared_ptr<PTY>& pty,
        const std::optional<Environment>& env = std::nullopt
    ) {
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
        }, env);

        
    }

    virtual ~Process() {
        stop();
    }

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
     * Sends sigterm to the process.
     */
    void stop() {
        if (statusCode == -1) {
            kill(*pid, SIGTERM);
        }
    }

    /**
     * Sends sigkill to the process. You should prefer using stop() over this if possible, as sigkill skips cleanup in
     * the child process, which isn't always acceptable.
     */
    void sigkill() {
        if (statusCode == -1) {
            kill(*pid, SIGKILL);
        }
    }

    void closeStdin() {
        if (!this->interface.has_value()) {
            throw std::runtime_error("Must use pipe or pty mode to use this function");
        }
    }

    /**
     * \returns whether or not the process exited normally, i.e. its end wasn't caused by a signal. This does not imply
     *          that the return code is 0, only that it wasn't stopped or terminated.
     *          Returns std::nullopt if the process hasn't exited yet.
     */
    std::optional<bool> hasExitedNormally() {
        return exitedNormally;
    }

    std::optional<int> getExitCode() {
        if (statusCode != -1) {
            return statusCode;
        }
        return std::nullopt;
    }

};

}
